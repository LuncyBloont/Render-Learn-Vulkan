#include <iostream>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "Core.h"
#include "Cat.h"
#include "Log.h"
#include "ShaderCompile.h"

inline void compilerRequiredShaders() {
	compileShader(".\\shaders\\test.vert", shader_testVert);
	compileShader(".\\shaders\\test.frag", shader_testFrag);
}

int main() {

	Core core;
	Cat cat;

	uint64_t test = 123;
	uint32_t arr[16] = { 3, 6, 8, 23, 8 };

	LOG_D(test, % llu);

	LOG_D_FOR(it, arr, arr + 16,
		LOG_P(*it, % u)
	);

	ASSERT_LOG_D(core.log.err(), 45 == 3);

	Log().warn() << "some warning...";

	try {
		compilerRequiredShaders();
		cat.registSelf(&core);
		core.init();
		core.run();
		core.exit();
	} catch (const std::exception& e) {
		std::cerr << std::endl << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
