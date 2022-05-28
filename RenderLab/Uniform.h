#pragma once
#include "glm/glm.hpp"

namespace Uniform {
	struct MVP {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};
	struct Toy {
		glm::vec4 resolution;
		glm::float32 time;
	};
}