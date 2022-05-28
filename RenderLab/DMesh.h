#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

class DMesh {
public:
	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec3 normal;
	};

	typedef uint16_t Index;
	static constexpr Index IMAX = UINT16_MAX;
	static constexpr VkIndexType TYPE = VK_INDEX_TYPE_UINT16;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

