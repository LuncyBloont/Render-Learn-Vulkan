#pragma once
#include <vulkan/vulkan.h>
#include <list>
#include <vector>
#include <optional>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Log.h"

#define TO_COREFUNC(func_name) { func_name, #func_name }
#define COREFUNC(name) static bool name()
#define BIND_INIT_CORE(core, func) core->addFunc(TO_COREFUNC(func), Core::Steps::Init)
#define BIND_RUN_CORE(core, func) core->addFunc(TO_COREFUNC(func), Core::Steps::Run)
#define BIND_EXIT_CORE(core, func) core->addFunc(TO_COREFUNC(func), Core::Steps::Exit)

uint32_t CG(uint64_t number);

class Core;

struct CoreFunc {
	bool (*function) ();
	std::string name;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsIndice;
	std::optional<uint32_t> presentIndice;
	std::optional<uint32_t> transferIndice;
};

struct SwapChainDetails {
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Core {
public:
	enum class Steps {
		Init,
		Run,
		Exit
	};
public:
	void init();
	void run();
	void exit();

	CoreFunc addFunc(CoreFunc, Steps);
	void clearFunc(Steps);

private:
	std::list<CoreFunc> initFunctions;
	std::list<CoreFunc> runFunctions;
	std::list<CoreFunc> exitFunctions;

public:
	Log log;
	std::vector<GLFWwindow*> windows;
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessager = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceMemory> vertexBuffersMemory;
};

namespace VKHelper {
	std::vector<const char*> strs2pp(const std::vector<std::string>& strs);
}

