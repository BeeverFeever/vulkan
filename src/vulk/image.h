#pragma once

#include <vulkan/vulkan.h>

#include <vulk/device.h>
#include <vulk/queues.h>

bool image_transition_layout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
VkImage image_create(Device device, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory* imageMemory);
VkImage texture_image_create(char* path, Device device, Queues queues, VkCommandPool commandPool, VkDeviceMemory* textureImageMemory);
VkImageView image_view_create(Device device, VkImage image, VkFormat format);
VkImageView texture_image_view_create(Device device, VkImage image);
VkSampler texture_sampler_create(Device device);
