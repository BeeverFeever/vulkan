#pragma once

#include <vulkan/vulkan.h>

#include <vulk/window.h>
#include <vulk/device.h>

#include <utils.h>

VkCommandPool command_pool_create(Device device, Window window);
vectorT(VkCommandBuffer) command_buffers_create(Device device, VkCommandPool commandPool, Allocator* allocator);
VkCommandBuffer command_begin_single_time_commands(VkCommandPool commandPool, Device device);
void command_end_single_time_commands(VkQueue queue, VkCommandBuffer* commandBuffer);
