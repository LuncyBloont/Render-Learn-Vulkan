#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

class DMesh {
public:
	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
		// glm::vec2 uv;
		// glm::vec3 normal;
	};

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

