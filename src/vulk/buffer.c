#include "buffer.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include <vulk/device.h>
#include <vulk/queues.h>
#include <vulk/config.h>

#include <vector.h>

static u32 find_memory_type(Device device, u32 typeFilter, VkMemoryPropertyFlags properties) {
   VkPhysicalDeviceMemoryProperties memProperties;
   vkGetPhysicalDeviceMemoryProperties(device.physical, &memProperties);

   for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
         return i;
      }
   }

   fprintf(stderr, "failed to find suitable memory type!");
   exit(EXIT_FAILURE);
}

VkBuffer buffer_create(Device device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory* bufferMemory) {
   VkBuffer buffer = {};
   VkBufferCreateInfo bufferInfo = {};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size = size;
   bufferInfo.usage = usage;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   if (vkCreateBuffer(device.logical, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
      fprintf(stderr, "failed to create buffer\n");
      exit(EXIT_FAILURE);
   }

   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements(device.logical, buffer, &memRequirements);

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memRequirements.size;
   allocInfo.memoryTypeIndex = find_memory_type(device, memRequirements.memoryTypeBits, properties);

   if (vkAllocateMemory(device.logical, &allocInfo, nullptr, bufferMemory) != VK_SUCCESS) {
      fprintf(stderr, "failed to allocate vertex buffer memory\n");
      exit(EXIT_FAILURE);
   }

   vkBindBufferMemory(device.logical, buffer, *bufferMemory, 0);
   return buffer;
}

void buffer_copy(VkCommandPool commandPool, Device device, Queues queues, VkBuffer src, VkBuffer dest, VkDeviceSize size) {
   VkCommandBufferAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandPool = commandPool;
   allocInfo.commandBufferCount = 1;

   VkCommandBuffer commandBuffer;
   vkAllocateCommandBuffers(device.logical, &allocInfo, &commandBuffer);

   VkCommandBufferBeginInfo beginInfo = {};
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

   vkBeginCommandBuffer(commandBuffer, &beginInfo);

   VkBufferCopy copyRegion = {};
   copyRegion.srcOffset = 0; // Optional
   copyRegion.dstOffset = 0; // Optional
   copyRegion.size = size;
   vkCmdCopyBuffer(commandBuffer, src, dest, 1, &copyRegion);
   vkEndCommandBuffer(commandBuffer);

   VkSubmitInfo submitInfo = {};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &commandBuffer;

   vkQueueSubmit(queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
   vkQueueWaitIdle(queues.graphics);

   vkFreeCommandBuffers(device.logical, commandPool, 1, &commandBuffer);
}

VkBuffer buffer_create_vertex(Device device, Queues queues, VkCommandPool commandPool, VkDeviceSize bufferSize, vectorT(void) vertices, VkDeviceMemory* vertexBufferMemory) {
   VkDeviceMemory stagingBufferMemory;
   VkBuffer stagingBuffer = buffer_create(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBufferMemory);

   void* data;
   vkMapMemory(device.logical, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, vertices, bufferSize);
   vkUnmapMemory(device.logical, stagingBufferMemory);

   VkBuffer outBuffer = buffer_create(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, vertexBufferMemory);
   buffer_copy(commandPool, device, queues, stagingBuffer, outBuffer, bufferSize);

   vkDestroyBuffer(device.logical, stagingBuffer, nullptr);
   vkFreeMemory(device.logical, stagingBufferMemory, nullptr);

   return outBuffer;
}

VkBuffer buffer_create_index(Device device, Queues queues, VkCommandPool commandPool, VkDeviceSize bufferSize, vectorT(u16) indices, VkDeviceMemory* indexBufferMemory) {
   VkDeviceMemory stagingBufferMemory;
   VkBuffer stagingBuffer = buffer_create(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBufferMemory);

   void* data;
   vkMapMemory(device.logical, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, indices, (size_t)bufferSize);
   vkUnmapMemory(device.logical, stagingBufferMemory);

   VkBuffer outBuffer = buffer_create(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBufferMemory);
   buffer_copy(commandPool, device, queues, stagingBuffer, outBuffer, bufferSize);

   vkDestroyBuffer(device.logical, stagingBuffer, nullptr);
   vkFreeMemory(device.logical, stagingBufferMemory, nullptr);

   return outBuffer;
}

UniformBufferContainer buffer_create_uniform(Device device, Queues queues, VkCommandPool commandPool, VkDeviceSize bufferSize, Allocator* allocator) {
   UniformBufferContainer outBuffer = {
      .buffers = vector(VkBuffer, g_maxFramesInFlight, allocator),
      .memory = vector(VkDeviceMemory, g_maxFramesInFlight, allocator),
      .mapped = vector(void*, g_maxFramesInFlight, allocator),
   };

   for (Size i = 0; i < g_maxFramesInFlight; i++) {
      outBuffer.buffers[i] = buffer_create(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &outBuffer.memory[i]);

      vkMapMemory(device.logical, outBuffer.memory[i], 0, bufferSize, 0, &outBuffer.mapped[i]);

      vector_update_length(i, outBuffer.memory);
      vector_update_length(i, outBuffer.mapped);
      vector_update_length(i, outBuffer.buffers);
   }

   return outBuffer;
}

