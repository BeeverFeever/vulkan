#pragma once

#include <vulk/device.h>
#include <utils.h>

VkShaderModule shader_module_create(String filepath, Device device, Allocator* allocator);
