#include "Cat.h"

uint32_t Cat::mwinWidth = 800;
uint32_t Cat::mwinHeight = 600;
const char* Cat::mwinTitle = "Cat Render";
std::vector<std::string> Cat::requiredInstanceExtensions = {};
std::vector<std::string> Cat::requiredInstanceLayers = {};
std::vector<std::string> Cat::requiredDeviceExtensions = {};
Core* Cat::core = nullptr;
std::vector<VkSemaphore> Cat::imageAvailableSemaphores = {};
std::vector<VkSemaphore> Cat::renderFinishedSemaphores = {};
std::vector<VkFence> Cat::inFlightFences = {};
size_t Cat::currentFrame = 0;
VkSwapchainKHR Cat::oldSwapChain = nullptr;
bool Cat::frameBufferResized = false;
std::vector<DMesh::Vertex> Cat::testMesh = {
	{ { -0.8f, -0.8f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.8f, -0.8f }, { 1.0f, 1.0f, 1.0f } },
	{ { 0.8f, 0.8f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.8f, 0.8f }, { 0.0f, 1.0f, 0.0f } }
};

void Cat::registSelf(Core* core) {
	this->core = core;
	BIND_INIT_CORE(core, initializeWindow);
	BIND_INIT_CORE(core, checkInstanceExtensionsAndLayers);
	BIND_INIT_CORE(core, initializeVulkan);
	BIND_INIT_CORE(core, setupDebuger);
	BIND_INIT_CORE(core, createSurface);
	BIND_INIT_CORE(core, pickupPhysicalDevice);
	BIND_INIT_CORE(core, createLogicalDevice);
	BIND_INIT_CORE(core, createSwapChain);
	BIND_INIT_CORE(core, createImageViews);
	BIND_INIT_CORE(core, createRenderPass);
	BIND_INIT_CORE(core, createGraphicsPipeline);
	BIND_INIT_CORE(core, createFramebuffers);
	BIND_INIT_CORE(core, createCommandPool);
	BIND_INIT_CORE(core, createVertexBuffer);
	BIND_INIT_CORE(core, createCommandBuffers);
	BIND_INIT_CORE(core, createSyncObjects);

	BIND_RUN_CORE(core, windowLoop);

	BIND_EXIT_CORE(core, cleanupSyncObjects);
	BIND_EXIT_CORE(core, cleanupCommandPool);
	BIND_EXIT_CORE(core, cleanupFramebuffers);
	BIND_EXIT_CORE(core, cleanupPipeline);
	BIND_EXIT_CORE(core, cleanupRenderPass);
	BIND_EXIT_CORE(core, cleanupImageViews);
	BIND_EXIT_CORE(core, cleanupSwapChain);
	BIND_EXIT_CORE(core, destroyVertexBuffer);
	BIND_EXIT_CORE(core, cleanupDevice);
	BIND_EXIT_CORE(core, cleanupSurface);
	BIND_EXIT_CORE(core, disableDebuger);
	BIND_EXIT_CORE(core, cleanupVulkan);
	BIND_EXIT_CORE(core, cleanupWindow);
}

void Cat::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

}

bool Cat::initializeWindow() {
	glfwInit();

	for (auto it = core->windows.begin(); it != core->windows.end(); it++) {
		glfwDestroyWindow(*it);
	}
	core->windows.resize(windowCount);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	core->windows[mainWindowID] = glfwCreateWindow(mwinWidth, mwinHeight, mwinTitle, nullptr, nullptr);

	glfwSetFramebufferSizeCallback(core->windows[mainWindowID], [] (GLFWwindow*, int width, int height) {
			frameBufferResized = true;
		});

	return true;
}

bool Cat::windowLoop() {
	while (!glfwWindowShouldClose(core->windows[mainWindowID])) {
		glfwPollEvents();
		try {
			drawFrame();
		} catch (std::exception& err) {
			core->log.err() << err.what();
			return false;
		}
	}

	vkDeviceWaitIdle(core->device);

	return true;
}

bool Cat::cleanupWindow() {
	for (auto it = core->windows.begin(); it != core->windows.end(); it++) {
		glfwDestroyWindow(*it);
	}
	glfwTerminate();
	return true;
}

bool Cat::initializeVulkan() {
	VkApplicationInfo appInfo = { };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Cat Render";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> ppExtensions = VKHelper::strs2pp(requiredInstanceExtensions);
	std::vector<const char*> ppLayers = VKHelper::strs2pp(requiredInstanceLayers);
	VkInstanceCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.enabledExtensionCount = CG(requiredInstanceExtensions.size());
	createInfo.ppEnabledExtensionNames = ppExtensions.data();
	if (core->log.validationEnable) {
		createInfo.enabledLayerCount = CG(requiredInstanceLayers.size());
		createInfo.ppEnabledLayerNames = ppLayers.data();
	}

	if (vkCreateInstance(&createInfo, nullptr, &core->instance) != VK_SUCCESS) {
		core->log.err() << "Failed to create Vulkan instance.";
		return false;
	}
	
	return true;
}

bool Cat::cleanupVulkan() {
	vkDestroyInstance(core->instance, nullptr);
	return true;
}

void Cat::addGlfwRequiredExtensions() {
	uint32_t glfwVulkanExtensionCount = 0;
	const char** glfwVulkanExtensions;

	glfwVulkanExtensions = glfwGetRequiredInstanceExtensions(&glfwVulkanExtensionCount);

	for (uint32_t i = 0; i < glfwVulkanExtensionCount; i++) {
		Cat::requiredInstanceExtensions.push_back(glfwVulkanExtensions[i]);
	}
}

void Cat::addCatRequiredExtensions() {
	// Add the required extensions' name to Cat::requiredExtensions.
	if (core->log.validationEnable) {
		requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
}

void Cat::addDeviceRequiredExtensions() {
	requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

bool Cat::compareExtensions() {
	uint32_t availableCount = 0;
	if (vkEnumerateInstanceExtensionProperties(nullptr, &availableCount, nullptr) != VK_SUCCESS) {
		core->log.err() << "Failed to get VK extensions' count.";
		return false;
	}

	std::vector<VkExtensionProperties> available(availableCount);
	if (vkEnumerateInstanceExtensionProperties(nullptr, &availableCount, available.data()) != VK_SUCCESS) {
		core->log.err() << "Failed to get VK extensions' info.";
		return false;
	}

	for (uint32_t i = 0; i < requiredInstanceExtensions.size(); i++) {
		bool have = false;
		for (uint32_t j = 0; j < availableCount; j++) {
			if (strcmp(available[j].extensionName, requiredInstanceExtensions[i].c_str()) == 0) {
				have = true;
				break;
			}
		}
		if (!have) {
			core->log.err() << "Required extension " << requiredInstanceExtensions[i] << " missing.";
			return false;
		}
	}
	return true;
}

bool Cat::checkInstanceExtensionsAndLayers() {
	requiredInstanceExtensions.clear();
	addGlfwRequiredExtensions();
	addCatRequiredExtensions();

	requiredInstanceLayers.clear();
	addCatRequiredLayers();

	if (compareExtensions() == false) {
		return false;
	}

	if (compareLayers() == false) {
		return false;
	}

	return true;
}

bool Cat::setupDebuger() {
	if (core->log.validationEnable) {
		auto enable = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(core->instance, 
			"vkCreateDebugUtilsMessengerEXT");
		if (enable) {
			VkDebugUtilsMessengerCreateInfoEXT createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
			createInfo.pfnUserCallback = [](
					VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
					VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
					const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
					void*                                            pUserData
				){
					switch (messageSeverity)
					{
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
						// ((Log*)pUserData)->info() << pCallbackData->pMessage;
						break;
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
						((Log*)pUserData)->info() << pCallbackData->pMessage;
						break;
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
						((Log*)pUserData)->warn() << pCallbackData->pMessage;
						break;
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
						((Log*)pUserData)->err() << pCallbackData->pMessage;
						break;
					case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
						((Log*)pUserData)->err() << pCallbackData->pMessage;
						break;
					default:
						break;
					}
					return VK_FALSE;
			};
			createInfo.pUserData = &core->log;
			if (enable(core->instance, &createInfo, nullptr, &core->debugMessager) != VK_SUCCESS) {
				core->log.err() << "Failed to enable debuger for Vulkan.";
				return false;
			}
		} else {
			core->log.err() << "Failed to find debuger enable function.";
			return false;
		}
	}
	return true;
}

bool Cat::disableDebuger() {
	if (core->log.validationEnable) {
		auto disable = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(core->instance, 
			"vkDestroyDebugUtilsMessengerEXT");
		if (disable) {
			disable(core->instance, core->debugMessager, nullptr);
		} else {
			core->log.err() << "Filed to find debuger disable function.";
			return false;
		}
	}
	return true;
}

bool Cat::pickupPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(core->instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		core->log.err() << "No Vulkan supported device found!";
		return false;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(core->instance, &deviceCount, devices.data());

	LOG_D_FOR(it, devices.begin(), devices.end(),
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(*it, &properties);
		LOG_D(properties.deviceName,% s);
	);

	uint32_t maxScore = 0;
	for (auto it = devices.begin(); it != devices.end(); it++) {
		uint32_t mins;
		uint32_t score = getSuitableDevice(*it, &mins);
		if (score >= mins && score >= maxScore) {
			maxScore = score;
			core->physicalDevice = *it;
		}
	}
	if (core->physicalDevice != VK_NULL_HANDLE) {
		VkPhysicalDeviceProperties chosenProperties;
		vkGetPhysicalDeviceProperties(core->physicalDevice, &chosenProperties);
		LOG_D(chosenProperties.deviceName, % s);
	} else {
		core->log.err() << "No suitable device found!";
	}
	
	return core->physicalDevice;
}

bool Cat::createLogicalDevice() {
	QueueFamilyIndices indices = getRequiredQueueFamilyIndices(core->physicalDevice, core->surface);

	VkPhysicalDeviceFeatures requiredFeatures{};

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> requiredIndices{ 
		indices.graphicsIndice.value(), 
		indices.presentIndice.value(),
		indices.transferIndice.value()
	};
	float queuePriority = 1.0f;
	for (const auto& indice : requiredIndices) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indice;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.enabledExtensionCount = CG(requiredDeviceExtensions.size());
	std::vector<const char*> ppDeviceExtensions = VKHelper::strs2pp(requiredDeviceExtensions);
	createInfo.ppEnabledExtensionNames = ppDeviceExtensions.data();
	createInfo.enabledLayerCount = CG(requiredInstanceLayers.size());
	std::vector<const char*> ppLayerNames = VKHelper::strs2pp(requiredInstanceLayers);
	createInfo.ppEnabledLayerNames = ppLayerNames.data();
	createInfo.pEnabledFeatures = &requiredFeatures;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = CG(queueCreateInfos.size());

	if (vkCreateDevice(core->physicalDevice, &createInfo, nullptr, &core->device) != VK_SUCCESS) {
		core->log.err() << "Failed to create logical device.";
		return false;
	}

	vkGetDeviceQueue(core->device, indices.graphicsIndice.value(), 0, &core->graphicsQueue);
	vkGetDeviceQueue(core->device, indices.presentIndice.value(), 0, &core->presentQueue);

	return true;
}

bool Cat::cleanupDevice() {
	vkDestroyDevice(core->device, nullptr);
	return true;
}

bool Cat::createSurface() {
	if (glfwCreateWindowSurface(core->instance, core->windows[mainWindowID], nullptr,
		&core->surface) != VK_SUCCESS) {
		return false;
	}
	return true;
}

bool Cat::cleanupSurface() {
	vkDestroySurfaceKHR(core->instance, core->surface, nullptr);
	return true;
}

bool Cat::createSwapChain() {
	SwapChainDetails details = getSwapChainDetails(core->physicalDevice);
	VkSurfaceFormatKHR format = chooseSurfaceFormat(details.formats);
	VkPresentModeKHR presnetMode = chooseSwapPresentMode(details.presentModes);
	VkExtent2D extent = chooseSwapExtent(details.capabilities);

	uint32_t imageCount = glm::min(
		details.capabilities.minImageCount + 1, details.capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = core->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = format.format;
	createInfo.imageExtent = extent;
	createInfo.imageColorSpace = format.colorSpace;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = getRequiredQueueFamilyIndices(core->physicalDevice, core->surface);
	uint32_t indicesArray[] = { indices.graphicsIndice.value(), indices.presentIndice.value() };
	if (indices.graphicsIndice == indices.presentIndice) {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.queueFamilyIndexCount = 0;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.pQueueFamilyIndices = indicesArray;
		createInfo.queueFamilyIndexCount = sizeof(indicesArray) / sizeof(uint32_t);
	}

	createInfo.preTransform = details.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presnetMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = nullptr; // oldSwapChain;

	if (vkCreateSwapchainKHR(core->device, &createInfo, nullptr, &core->swapchain) != VK_SUCCESS) {
		core->log.err() << "Failed to create swapchain.";
		return false;
	}

	vkGetSwapchainImagesKHR(core->device, core->swapchain, &imageCount, nullptr);
	core->swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(core->device, core->swapchain, &imageCount, core->swapChainImages.data());
	ASSERT_LOG_D_RUN(core->log.err(), imageCount > 0,
		LOG_D(imageCount, % d);
	);
	core->swapChainFormat = format.format;
	core->swapChainExtent = extent;

	oldSwapChain = core->swapchain;

	return true;
}

bool Cat::cleanupSwapChain() {
	vkDestroySwapchainKHR(core->device, core->swapchain, nullptr);
	return true;
}

bool Cat::createImageViews() {
	core->swapChainImageViews.resize(core->swapChainImages.size());
	for (size_t i = 0; i < core->swapChainImageViews.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = core->swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = core->swapChainFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;

		if (vkCreateImageView(core->device, &createInfo, nullptr, &core->swapChainImageViews[i]) != VK_SUCCESS) {
			core->log.err() << "Failed to create wap chian image:[" << i << "].";
			return false;
		}
	}
	return true;
}

bool Cat::cleanupImageViews() {
	for (auto imageView : core->swapChainImageViews) {
		vkDestroyImageView(core->device, imageView, nullptr);
	}
	return true;
}

bool Cat::createGraphicsPipeline() {
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	try {
		std::vector<char> testVertShader;
		std::vector<char> testFragShader;
		loadShader(shader_testVert, &testVertShader);
		loadShader(shader_testFrag, &testFragShader);
		vertShaderModule = createShaderModule(testVertShader);
		fragShaderModule = createShaderModule(testFragShader);
	} catch (const std::exception& err) {
		core->log.err() << err.what();
		return false;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStagesInfo[] = {
		vertShaderStageInfo, fragShaderStageInfo
	};

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
	VkVertexInputBindingDescription bindingDescription = DMesh::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 2> attributeDescription = DMesh::getAttributeDescriptions();
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = CG(attributeDescription.size());
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescription.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(core->swapChainExtent.width);
	viewport.height = static_cast<float>(core->swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = core->swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multiSampleCreateInfo{};
	multiSampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampleCreateInfo.minSampleShading = 1.0f;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
	colorBlendAttachmentState.colorWriteMask = 
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
	colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(core->device, &pipelineLayoutCreateInfo, nullptr, &core->pipelineLayout) !=
		VK_SUCCESS) {
		core->log.err() << "Failed to create pipeline layout.";
		return false;
	}

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStagesInfo;
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multiSampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = core->pipelineLayout;
	pipelineCreateInfo.renderPass = core->renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(core->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, 
		nullptr, &core->graphicsPipeline) != VK_SUCCESS) {
		core->log.err() << "Failed to create graphics pipeline.";
		return false;
	}

	vkDestroyShaderModule(core->device, vertShaderModule, nullptr);
	vkDestroyShaderModule(core->device, fragShaderModule, nullptr);
	return true;
}

bool Cat::cleanupPipeline() {
	vkDestroyPipeline(core->device, core->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(core->device, core->pipelineLayout, nullptr);
	return true;
}

bool Cat::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = core->swapChainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference attachmentReference{};
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachmentReference;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo passCreateInfo{};
	passCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passCreateInfo.attachmentCount = 1;
	passCreateInfo.pAttachments = &colorAttachment;
	passCreateInfo.subpassCount = 1;
	passCreateInfo.pSubpasses = &subpass;
	passCreateInfo.dependencyCount = 1;
	passCreateInfo.pDependencies = &dependency;
	
	if (vkCreateRenderPass(core->device, &passCreateInfo, nullptr, &core->renderPass) != VK_SUCCESS) {
		core->log.err() << "Failed to create render pass.";
		return false;
	}

	return true;
}

bool Cat::cleanupRenderPass() {
	vkDestroyRenderPass(core->device, core->renderPass, nullptr);
	return true;
}

bool Cat::createFramebuffers() {
	core->swapChainFramebuffers.resize(core->swapChainImageViews.size());

	for (size_t i = 0; i < core->swapChainImageViews.size(); i++) {
		VkImageView attachments[] = {
			core->swapChainImageViews[i]
		};
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = core->renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = core->swapChainExtent.width;
		framebufferCreateInfo.height = core->swapChainExtent.height;
		framebufferCreateInfo.layers = 1;
		if (vkCreateFramebuffer(
			core->device, &framebufferCreateInfo, nullptr, &core->swapChainFramebuffers[i]) !=
			VK_SUCCESS) {
			core->log.err() << "Failed to create framebuffer [" << i << "]";
			return false;
		}
	}

	return true;
}

bool Cat::cleanupFramebuffers() {
	for (auto& framebuffer : core->swapChainFramebuffers) {
		vkDestroyFramebuffer(core->device, framebuffer, nullptr);
	}
	return true;
}

bool Cat::createCommandPool() {
	QueueFamilyIndices familyIndices = getRequiredQueueFamilyIndices(core->physicalDevice, core->surface);
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = familyIndices.graphicsIndice.value();
	if (vkCreateCommandPool(core->device, &commandPoolCreateInfo, nullptr, &core->commandPool) !=
		VK_SUCCESS) {
		core->log.err() << "Failed to create command pool.";
		return false;
	}
	return true;
}

bool Cat::cleanupCommandPool() {
	vkDestroyCommandPool(core->device, core->commandPool, nullptr);
	return true;
}

bool Cat::createCommandBuffers() {
	core->commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = core->commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 2;
	if (vkAllocateCommandBuffers(core->device, &commandBufferAllocateInfo, core->commandBuffers.data()) !=
		VK_SUCCESS) {
		core->log.err() << "Failed to allocate command buffer.";
		return false;
	}

	return true;
}

bool Cat::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(core->device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(core->device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(core->device, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			core->log.err() << "Failed to create semphores and fences.";
			return false;
		}
	}

	return true;
}

bool Cat::cleanupSyncObjects() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(core->device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(core->device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(core->device, inFlightFences[i], nullptr);
	}

	return true;
}

bool Cat::createVertexBuffer() {
	// destroyVertexBuffer();

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof(DMesh::Vertex) * testMesh.size();
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer toCreate = VK_NULL_HANDLE;
	if (vkCreateBuffer(core->device, &bufferCreateInfo, nullptr, &toCreate) != VK_SUCCESS) {
		core->log.err() << "Failed to crea tetesting vertex buffer.";
		return false;
	}

	VkMemoryRequirements memoryRequirments;
	vkGetBufferMemoryRequirements(core->device, toCreate, &memoryRequirments);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirments.size;
	try {
	allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirments.memoryTypeBits, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	} catch (std::exception& err) {
		core->log.err() << err.what();
		return false;
	}

	VkDeviceMemory memory;
	if (vkAllocateMemory(core->device, &allocateInfo, nullptr, &memory) != VK_SUCCESS) {
		core->log.err() << "Failed to create device memory.";
		return false;
	}

	vkBindBufferMemory(core->device, toCreate, memory, 0);
	core->vertexBuffersMemory.push_back(memory);
	core->vertexBuffers.push_back(toCreate);
	// New vertex buffer regist to Core(Cat).

	void* data;
	vkMapMemory(core->device, memory, 0, bufferCreateInfo.size, 0, &data);
	memcpy(data, testMesh.data(), bufferCreateInfo.size);
	vkUnmapMemory(core->device, memory);

	return true;
}

bool Cat::destroyVertexBuffer() {
	for (auto buffer : core->vertexBuffers) {
		vkDestroyBuffer(core->device, buffer, nullptr);
	}
	core->vertexBuffers.clear();

	for (auto memory : core->vertexBuffersMemory) {
		vkFreeMemory(core->device, memory, nullptr);
	}
	core->vertexBuffersMemory.clear();

	return true;
}

void Cat::addCatRequiredLayers() {
	if (core->log.validationEnable) {
		requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	}
}
bool Cat::compareLayers() {
	uint32_t availableCount = 0;
	if (vkEnumerateInstanceLayerProperties(&availableCount, nullptr) != VK_SUCCESS) {
		core->log.err() << "Failed to get VK layers' count.";
		return false;
	}

	std::vector<VkLayerProperties> available(availableCount);
	if (vkEnumerateInstanceLayerProperties(&availableCount, available.data()) != VK_SUCCESS) {
		core->log.err() << "Failed to get VK layers' info.";
		return false;
	}

	for (uint32_t i = 0; i < requiredInstanceLayers.size(); i++) {
		bool have = false;
		for (uint32_t j = 0; j < availableCount; j++) {
			if (strcmp(available[j].layerName, requiredInstanceLayers[i].c_str()) == 0) {
				have = true;
				break;
			}
		}
		if (!have) {
			core->log.err() << "Required layer " << requiredInstanceLayers[i] << " missing.";
			return false;
		}
	}
	return true;
}

uint32_t Cat::getSuitableDevice(VkPhysicalDevice device, uint32_t* minScore) {
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceProperties(device, &properties);
	vkGetPhysicalDeviceFeatures(device, &features);
	QueueFamilyIndices indices = getRequiredQueueFamilyIndices(device, core->surface);

	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	bool suitForExtensions = true;
	requiredDeviceExtensions.clear();
	addDeviceRequiredExtensions();
	for (size_t i = 0; i < requiredDeviceExtensions.size(); i++) {
		suitForExtensions = false;
		for (size_t j = 0; j < extensions.size(); j++) {
			if (strcmp(extensions[j].extensionName, requiredDeviceExtensions[i].c_str()) == 0) {
				suitForExtensions = true;
				break;
			}
		}
		if (!suitForExtensions) break;
	}

	bool swapChainAdequate = false;
	if (suitForExtensions) {
		SwapChainDetails details = getSwapChainDetails(device);
		swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
	}

	uint32_t score = 0;

	auto addScore = [&score] () { score += 1; score <<= 1; };

	if (indices.graphicsIndice.has_value() && 
		indices.presentIndice.has_value() && 
		indices.transferIndice.has_value()) { addScore(); }
	if (suitForExtensions && swapChainAdequate) { addScore(); }
	*minScore = score;
	if (features.geometryShader) { addScore(); }
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { addScore(); }

	return score;
}

QueueFamilyIndices Cat::getRequiredQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices{};
	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
	std::vector<VkQueueFamilyProperties> families(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());
	
	for (size_t i = 0; i < familyCount; i++) {
		if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsIndice = CG(i);
		}
		
		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, CG(i), surface, &presentSupport);
		if (presentSupport) {
			indices.presentIndice = CG(i);
		}

		if (families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			indices.transferIndice = CG(i);
		}
	}

	return indices;
}

SwapChainDetails Cat::getSwapChainDetails(VkPhysicalDevice device) {
	SwapChainDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, core->surface, &details.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, core->surface, &formatCount, nullptr);
	details.formats.resize(formatCount);
	if (formatCount > 0) {
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device, core->surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, core->surface, &presentModeCount, nullptr);
	details.presentModes.resize(presentModeCount);
	if (presentModeCount > 0) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, core->surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkPresentModeKHR Cat::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes) {
	for (auto& mode : modes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return mode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Cat::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
	for (auto& format : formats) {
		if (format.format == VK_FORMAT_R8G8B8A8_SRGB && 
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}
	return formats[0];
}

VkExtent2D Cat::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(core->windows[mainWindowID], &width, &height);
		VkExtent2D actualExtent{
			CG(width),
			CG(height)
		};
		actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

void Cat::loadShader(const std::string& path, std::vector<char>* pCode) {
	std::ifstream fileInputStream(path, std::ios::ate | std::ios::binary);
	if (fileInputStream.is_open()) {
		size_t fileSize = fileInputStream.tellg();
		pCode->resize(fileSize);
		fileInputStream.seekg(0);
		fileInputStream.read(pCode->data(), fileSize);
		fileInputStream.close();
	} else {
		throw std::runtime_error("Can't open shader file.");
	}
}

VkShaderModule Cat::createShaderModule(const std::vector<char>& code) {
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	createInfo.codeSize = code.size();
	if (vkCreateShaderModule(core->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module.");
	}

	return shaderModule;
}

void Cat::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to start record command.");
	}

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = core->renderPass;
	renderPassBeginInfo.framebuffer = core->swapChainFramebuffers[imageIndex];

	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = core->swapChainExtent;

	VkClearValue clearColor{
		{ { 0.0f, 0.0f, 0.0f, 1.0f } }
	};
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, core->graphicsPipeline);

	std::vector<VkDeviceSize> offsets;
	offsets.resize(core->vertexBuffers.size(), 0);
	vkCmdBindVertexBuffers(commandBuffer, 0, CG(core->vertexBuffers.size()), 
		core->vertexBuffers.data(), offsets.data());

	vkCmdDraw(commandBuffer, CG(testMesh.size()), 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record the command buffer.");
	}
}

void Cat::drawFrame() {
	vkWaitForFences(core->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult swapChianStat = vkAcquireNextImageKHR(core->device, core->swapchain, UINT64_MAX, 
		imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (swapChianStat == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	} else if (swapChianStat != VK_SUCCESS && swapChianStat != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to get one image to render.");
	}

	// Only reset the fence if submitting can be done.
	vkResetFences(core->device, 1, &inFlightFences[currentFrame]);
	
	ASSERT_LOG_D_RUN(core->log.err(), imageIndex >= 0 && imageIndex < core->swapChainFramebuffers.size(),
		LOG_D(imageIndex, %d);
		throw std::runtime_error("Get swap chain image index error.");
	);

	vkResetCommandBuffer(core->commandBuffers[currentFrame], 0);
	recordCommandBuffer(core->commandBuffers[currentFrame], imageIndex);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &core->commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(core->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit graphics queue.");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { core->swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr;

	VkResult presentStat = vkQueuePresentKHR(core->presentQueue, &presentInfo);
	if (presentStat == VK_ERROR_OUT_OF_DATE_KHR || presentStat == VK_SUBOPTIMAL_KHR || frameBufferResized) {
		recreateSwapChain();
		frameBufferResized = false;
	} else if (presentStat != VK_SUCCESS) {
		throw std::runtime_error("Failed to present image to view.");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Cat::recreateSwapChain() {
	int width, height;
	glfwGetFramebufferSize(core->windows[mainWindowID], &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(core->windows[mainWindowID], &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(core->device);

	cleanupSwapChainRuntime();

	if (!(createSwapChain() &&
		createImageViews() &&
		createRenderPass() &&
		createGraphicsPipeline() &&
		createFramebuffers())) {
		throw std::runtime_error("Failed to recreate swap chain.");
	}
}

void Cat::cleanupSwapChainRuntime() {
	if (!(cleanupFramebuffers() &&
		cleanupPipeline() &&
		cleanupRenderPass() &&
		cleanupImageViews() &&
		cleanupSwapChain())) {
		throw std::runtime_error("Failed to clean old swap chain.");
	}
}

uint32_t Cat::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(core->physicalDevice, &deviceMemoryProperties);
	
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find usable memory!");
	return uint32_t();
}
