#pragma once

#include "Application.h"

#include "Log.h"

#include "Enums.h"

#include "Utils.h"

#include "Input.h"

#include "Camera.h"

#include "Buffer.h"
#include "Layout.h"

#include "Texture.h"
#include "Mesh.h"
#include "Skybox.h"

#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "Shader.h"

// TODO: Move them elsewhere?
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/trigonometric.hpp>
