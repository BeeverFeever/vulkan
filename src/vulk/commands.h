#pragma once

#include <vulkan/vulkan.h>

#include <vulk/window.h>
#include <vulk/device.h>

#include <utils.h>

VkCommandPool command_pool_create(Device device, Window window);
vectorT(VkCommandBuffer) command_buffers_create(Device device, VkCommandPool commandPool, Allocator* allocator);
