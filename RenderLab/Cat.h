#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <set>
#include <glm/glm.hpp>
#include <fstream>
#include <chrono>
#include "Core.h"
#include "DMesh.h"

constexpr char shader_testVert[] = ".\\shaders\\test.vert.spv";
constexpr char shader_testFrag[] = ".\\shaders\\test.frag.spv";
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class Cat {
private:
	COREFUNC(initializeWindow);
	COREFUNC(cleanupWindow);
	COREFUNC(windowLoop);
	COREFUNC(initializeVulkan);
	COREFUNC(cleanupVulkan);
	COREFUNC(checkInstanceExtensionsAndLayers);
	COREFUNC(setupDebuger);
	COREFUNC(disableDebuger);
	COREFUNC(pickupPhysicalDevice);
	COREFUNC(createLogicalDevice);
	COREFUNC(cleanupDevice);
	COREFUNC(createSurface);
	COREFUNC(cleanupSurface);
	COREFUNC(createSwapChain);
	COREFUNC(cleanupSwapChain);
	COREFUNC(createImageViews);
	COREFUNC(cleanupImageViews);
	COREFUNC(createGraphicsPipeline);
	COREFUNC(cleanupPipeline);
	COREFUNC(createRenderPass);
	COREFUNC(cleanupRenderPass);
	COREFUNC(createFramebuffers);
	COREFUNC(cleanupFramebuffers);
	COREFUNC(createCommandPool);
	COREFUNC(cleanupCommandPool);
	COREFUNC(createCommandBuffers);
	COREFUNC(createSyncObjects);
	COREFUNC(cleanupSyncObjects);
	COREFUNC(createVertexBuffer);
	COREFUNC(destroyVertexBuffer);
	COREFUNC(createIndexBuffer);
	COREFUNC(destroyIndexBuffer);
	COREFUNC(createDescriptorSetLayout);
	COREFUNC(destroyDescriptorSetLayout);
	COREFUNC(createUniformBuffers);
	COREFUNC(cleanupUniformBuffers);
	COREFUNC(createDescriptorPool);
	COREFUNC(destroyDescriptorPool);
	COREFUNC(createDescriptorSets);
	COREFUNC(cleanupDescriptorSets);

	static void addGlfwRequiredExtensions();
	static void addCatRequiredExtensions();
	static void addDeviceRequiredExtensions();
	static bool compareExtensions();
	static void addCatRequiredLayers();
	static bool compareLayers();
	static uint32_t getSuitableDevice(VkPhysicalDevice, uint32_t* minScore);
	static QueueFamilyIndices getRequiredQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);
	static SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>&);
	static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
	static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);
	static void loadShader(const std::string& path, std::vector<char>* pCode);
	static VkShaderModule createShaderModule(const std::vector<char>& code);
	static void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	static void drawFrame();
	static void recreateSwapChain();
	static void cleanupSwapChainRuntime();
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
	static void updateUniformBuffers();

public:
	void registSelf(Core* core);

private:
	static const uint32_t windowCount = 1;
	static const uint32_t mainWindowID = 0;
	static uint32_t mwinWidth;
	static uint32_t mwinHeight;
	static const char* mwinTitle;
	static std::vector<std::string> requiredInstanceExtensions;
	static std::vector<std::string> requiredInstanceLayers;
	static std::vector<std::string> requiredDeviceExtensions;
	static Core* core;
	static std::vector<VkSemaphore> imageAvailableSemaphores;
	static std::vector<VkSemaphore> renderFinishedSemaphores;
	static std::vector<VkFence> inFlightFences;
	static size_t currentFrame;
	static VkSwapchainKHR oldSwapChain;
	static bool frameBufferResized;
	static std::vector<DMesh::Vertex> testMesh;
	static std::vector<uint16_t> testIndex;
};

