#include "image.h"
#include "vulkan/vulkan_core.h"

#include <stdio.h>

#include <vulkan/vulkan.h>
#include <stb_image.h>

#include <vulk/buffer.h>
#include <vulk/device.h>
#include <vulk/queues.h>
#include <vulk/commands.h>

bool image_transition_layout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
   VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {
         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
         .levelCount = 1, 
         .layerCount = 1,
      },
   };

   VkPipelineStageFlags sourceStage;
   VkPipelineStageFlags destinationStage;

   if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      // nothing to wait on, transfer writes must wait for the transition
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
   } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      // fragment shader reads must wait for the copy to finish
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
   } else {
      fprintf(stderr, "unsupported layout transition!\n");
      return false;
   }

   vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);

   return true;
}

VkImage image_create(Device device, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory* imageMemory) {
   VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .extent = {(u32)width, (u32)height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
   };

   VkImage image = {};
   if (vkCreateImage(device.logical, &imageInfo, nullptr, &image) != VK_SUCCESS) {
      printf("Failed to create image\n");
      exit(0);
   }

   VkMemoryRequirements memReqs = {};
   vkGetImageMemoryRequirements(device.logical, image, &memReqs);

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memReqs.size;
   allocInfo.memoryTypeIndex = find_memory_type(device, memReqs.memoryTypeBits, properties);

   vkAllocateMemory(device.logical, &allocInfo, nullptr, imageMemory);
   vkBindImageMemory(device.logical, image, *imageMemory, 0);

   return image;
}

VkImage texture_image_create(char* path, Device device, Queues queues, VkCommandPool commandPool, VkDeviceMemory* textureImageMemory) {
   i32 width, height, channels;
   stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
   VkDeviceSize imageSize = width * height * 4;
   if (!pixels) {
      printf("failed to load image: %s\n", path);
      exit(EXIT_FAILURE);
   }

   VkDeviceMemory stagingMemory;
   VkBuffer stagingBuffer = buffer_create(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingMemory);

   void* data;
   vkMapMemory(device.logical, stagingMemory, 0, imageSize, 0, &data);
   memcpy(data, pixels, imageSize);
   vkUnmapMemory(device.logical, stagingMemory);

   stbi_image_free(pixels);

   VkImage textureImage = image_create(device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImageMemory);

   VkCommandBuffer commandBuffer = {};
   commandBuffer = command_begin_single_time_commands(commandPool, device);
   image_transition_layout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
   buffer_copy_to_image(commandBuffer, stagingBuffer, textureImage, width, height);
   image_transition_layout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
   command_end_single_time_commands(queues.graphics, &commandBuffer);

   vkDestroyBuffer(device.logical, stagingBuffer, nullptr);
   vkFreeMemory(device.logical, stagingMemory, nullptr);

   return textureImage;
}

VkImageView image_view_create(Device device, VkImage image, VkFormat format) {
   VkImageViewCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   createInfo.image = image;
   createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
   createInfo.format = format;
   createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
   createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
   createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
   createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
   createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   createInfo.subresourceRange.baseMipLevel = 0;
   createInfo.subresourceRange.levelCount = 1;
   createInfo.subresourceRange.baseArrayLayer = 0;
   createInfo.subresourceRange.layerCount = 1;

   VkImageView imageView;
   if (vkCreateImageView(device.logical, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
      fprintf(stderr, "failed to create image views\n");
      exit(EXIT_FAILURE);
   }
   return imageView;
}

VkImageView texture_image_view_create(Device device, VkImage image) {
   VkImageView imageView = image_view_create(device, image, VK_FORMAT_R8G8B8A8_SRGB);

   return imageView;
}

VkSampler texture_sampler_create(Device device) {
   VkPhysicalDeviceProperties properties;
   vkGetPhysicalDeviceProperties(device.physical, &properties);
   VkSamplerCreateInfo samplerInfo = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR, 
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .mipLodBias = 0.0f,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
   };
   VkSampler sampler;
   if (vkCreateSampler(device.logical, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
      printf("[error] failed to create image sampler\n");
   }
   return sampler;
}
