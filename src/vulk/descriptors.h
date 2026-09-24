#pragma once

#include <vulk/device.h>
#include <vulk/pipeline.h>

#include <utils.h>

VkDescriptorPool descriptor_pool_create(Device device);
vectorT(VkDescriptorSet) descriptor_sets_create(VkImageView textureImageView, VkSampler textureSampler, vectorT(VkBuffer) uniformBuffers, VkDescriptorPool pool, Device device, GraphicsPipeline pipeline, Allocator *allocator); 
VkDescriptorSetLayout descriptor_set_layouts_create(VkDevice device, Allocator* allocator);
