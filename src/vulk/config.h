#pragma once

#include <cglm/cglm.h>

extern const bool enableValidationLayers;
extern const char* validationLayers[1];
extern const char* requiredDeviceExtensions[1];

constexpr Size g_maxFramesInFlight = 2;

typedef struct {
   mat4 model;
   mat4 view;
   mat4 proj;
} UniformBufferObject;

