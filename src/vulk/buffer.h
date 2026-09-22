#pragma once

#include <vulkan/vulkan.h>
#include <vulk/device.h>
#include <utils.h>

typedef struct {
   vectorT(VkBuffer) buffers;
   vectorT(VkDeviceMemory) memory;
   vectorT(void*) mapped;
} UniformBufferContainer;

VkBuffer buffer_create(Device device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory* bufferMemory);
void buffer_copy(VkCommandPool commandPool, Device device, Queues queues, VkBuffer src, VkBuffer dest, VkDeviceSize size);

VkBuffer buffer_create_vertex(Device device, Queues queues, VkCommandPool commandPool, VkDeviceSize bufferSize, vectorT(void) vertices, VkDeviceMemory* vertexBufferMemory);
VkBuffer buffer_create_index(Device device, Queues queues, VkCommandPool commandPool, VkDeviceSize bufferSize, vectorT(u16) indices, VkDeviceMemory* indexBufferMemory);
UniformBufferContainer buffer_create_uniform(Device device, Queues queues, VkCommandPool commandPool, VkDeviceSize bufferSize, Allocator* allocator);
