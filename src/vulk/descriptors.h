#pragma once

#include <vulk/device.h>
#include <vulk/pipeline.h>

#include <utils.h>

extern VkDescriptorBufferInfo ubo_get_descriptor_info(VkBuffer buffer);

VkDescriptorPool descriptor_pool_create(Device device);
vectorT(VkDescriptorSet)
    descriptor_set_create(vectorT(VkBuffer) uniformBuffers,
                          VkDescriptorPool pool, Device device,
                          GraphicsPipeline pipeline, Allocator *allocator);
