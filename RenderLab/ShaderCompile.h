#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include "Log.h"

#define SHADER_COMPILE

#define GLSL_COMPILER "C:\\VulkanSDK\\1.3.204.1\\Bin\\glslangValidator.exe"
#define SHADER_COMPILE_COMMAND(compiler, input, output) "%s -V %s -o %s", compiler, input, output

#ifdef SHADER_COMPILE

void compileShader(const std::string& source, std::string output) {
	char command[512];
	char compiler[] = GLSL_COMPILER;
	sprintf_s(command, SHADER_COMPILE_COMMAND(compiler, source.c_str(), output.c_str()));
	std::cout << "\n";
	int stats = system(command);
	std::cout << "\n";
	Log().info() << "compile shader: " << source << " return " << stats << "\n";
}

#else

inline void compileShader(const std::string& source, std::string output) {}

#endif // SHADER_COMPILE


