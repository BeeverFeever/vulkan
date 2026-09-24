#include "descriptors.h"
#include "vulkan/vulkan_core.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulk/pipeline.h>
#include <vulkan/vulkan.h>

#include <vulk/device.h>
#include <vulk/config.h>

#include <utils.h>

VkDescriptorPool descriptor_pool_create(Device device) {
   VkDescriptorPool pool = {};
   VkDescriptorPoolSize poolSize[2] = {};
   poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSize[0].descriptorCount = (u32)g_maxFramesInFlight;
   poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   poolSize[1].descriptorCount = (u32)g_maxFramesInFlight;

   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = lengthof(poolSize);
   poolInfo.pPoolSizes = poolSize;
   poolInfo.maxSets = (u32)g_maxFramesInFlight;

   if (vkCreateDescriptorPool(device.logical, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
      fprintf(stderr, "failed to create descriptor pool\n");
      exit(EXIT_FAILURE);
   }

   return pool;
}

vectorT(VkDescriptorSet) descriptor_sets_create(VkImageView textureImageView, VkSampler textureSampler, vectorT(VkBuffer) uniformBuffers, VkDescriptorPool pool, Device device, GraphicsPipeline pipeline, Allocator* allocator) {
   vectorT(VkDescriptorSetLayout) layouts = vector(VkDescriptorSetLayout, g_maxFramesInFlight, allocator);
   for (Size i = 0; i < g_maxFramesInFlight; i++) {
      vector_push_back(layouts, pipeline.descriptorSetLayout);
   }
   VkDescriptorSetAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = pool;
   allocInfo.descriptorSetCount = (u32)g_maxFramesInFlight;
   allocInfo.pSetLayouts = layouts;

   vectorT(VkDescriptorSet) descriptorSets = vector(VkDescriptorSet, g_maxFramesInFlight, allocator);
   if (vkAllocateDescriptorSets(device.logical, &allocInfo, descriptorSets) != VK_SUCCESS) {
      fprintf(stderr, "failed to allocate descriptor sets\n");
      exit(EXIT_FAILURE);
   }
   vector_update_length(g_maxFramesInFlight, descriptorSets);

   for (Size i = 0; i < g_maxFramesInFlight; i++) {
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = textureImageView;
      imageInfo.sampler = textureSampler;

      VkWriteDescriptorSet descriptorWrites[2] = {};
      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = descriptorSets[i];
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pImageInfo = &imageInfo;

      vkUpdateDescriptorSets(device.logical, lengthof(descriptorWrites), descriptorWrites, 0, nullptr);
   }

   return descriptorSets;
}

VkDescriptorSetLayout descriptor_set_layouts_create(VkDevice device, Allocator* allocator) {
   VkDescriptorSetLayout descriptorSetLayout = {};

   VkDescriptorSetLayoutBinding uboLayoutBinding = {};
   uboLayoutBinding.binding = 0;
   uboLayoutBinding.descriptorCount = 1;
   uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
   samplerLayoutBinding.binding = 1;
   samplerLayoutBinding.descriptorCount = 1;
   samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   samplerLayoutBinding.pImmutableSamplers = nullptr;
   samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   // VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding, samplerLayoutBinding};
   vectorT(VkDescriptorSetLayoutBinding) bindings = vector(VkDescriptorSetLayoutBinding, 2, allocator);
   vector_push_back(bindings, uboLayoutBinding);
   vector_push_back(bindings, samplerLayoutBinding);

   VkDescriptorSetLayoutCreateInfo layoutInfo = {};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = (u32)vector_length(bindings);
   layoutInfo.pBindings = bindings;

   if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
      fprintf(stderr, "failed to create descriptor set layout\n");
      exit(EXIT_FAILURE);
   }

   return descriptorSetLayout;
}

