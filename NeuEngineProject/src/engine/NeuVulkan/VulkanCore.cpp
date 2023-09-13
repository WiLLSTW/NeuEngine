#include "VulkanCore.h"
using namespace Neu;
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>


void NEVulkanCore::createVulkanInstance() {
	if (enableValidationLayer && !checkValidationLayerSupport()) {
		throw std::runtime_error("This Device May Not Support Validation Layer");
	}
	VkApplicationInfo appInfo{};														//創建Vulkan Application資訊
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "NeuEngine Project";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "NeuEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceInfo{};												//創建Instance資訊
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;

	auto extensions = NeuGlfwIO::GetInstance()->getRequiredExtensions(enableValidationLayer);
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	//如果支援驗證層的話就實作
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayer) {
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceInfo.ppEnabledLayerNames = validationLayers.data();
		populateDebugMessengerCreateInfo(debugCreateInfo);
		instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		instanceInfo.enabledLayerCount = 0;

		instanceInfo.pNext = nullptr;
	}
	VKCREATEPROCESS(vkCreateInstance(&instanceInfo, nullptr, &set.Instance), "failed to create instance!");
}

bool NEVulkanCore::checkValidationLayerSupport() {

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void NEVulkanCore::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& vdumCreateInfo) {
	vdumCreateInfo = {};
	vdumCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	vdumCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	vdumCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	vdumCreateInfo.pfnUserCallback = debugCallback;
}

void NEVulkanCore::setupDebugMessanger() {
	if (!enableValidationLayer)return;

	VkDebugUtilsMessengerCreateInfoEXT vdumCreateInfo;
	populateDebugMessengerCreateInfo(vdumCreateInfo);

	VKCREATEPROCESS(CreateDebugUtilsMessengerEXT(set.Instance, &vdumCreateInfo, nullptr, &ext.DebugMessenger), "failed to setup debug messenger");
}
VkResult NEVulkanCore::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void NEVulkanCore::createSurface() {
	VKCREATEPROCESS(glfwCreateWindowSurface(set.Instance, NeuGlfwIO::GetInstance()->getWindow(), nullptr, &set.Surface), "failed to create window suface!");
}

void NEVulkanCore::pickPhysicalDevice() {

	/*先看有多少支援Vulkan的GPU*/
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(set.Instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan Support!");
	}

	set.PhysicalDevices.resize(deviceCount);
	vkEnumeratePhysicalDevices(set.Instance, &deviceCount, set.PhysicalDevices.data());

	for (int i = 0; i < deviceCount; i++) {
		if (isDeviceSuitable(set.PhysicalDevices.at(i))) {
			set.NowPhysicalDevice = i;
			break;
		}
	}

	if (nowPhysicalDevice() == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

}

bool NEVulkanCore::checkDeviceExtensionSupport(VkPhysicalDevice device) {

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto&extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();

}

bool NEVulkanCore::isDeviceSuitable(VkPhysicalDevice device) {

	//VkPhysicalDeviceProperties deviceProperties;
	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

	QueueFamilyIndices indices = findQueueFamilies(device);
	bool extensaionSupported = checkDeviceExtensionSupport(device);

	bool swaipChainAdequate = false;  //Adequate=足夠的
	if (glfwExtensionSupported) {
		SwapChainSupportDetails swapChainSupport = querySwaipChainSupport(device);
		swaipChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}
	return indices.isComplete() && glfwExtensionSupported&&swaipChainAdequate;

}

QueueFamilyIndices  NEVulkanCore::findQueueFamilies(VkPhysicalDevice device) {

	QueueFamilyIndices indices;

	//先檢查有幾個QueueFamilies
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	//再將QueueFamilies的資訊抓過來
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto & queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, set.Surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}
		//有值就跳出
		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

void NEVulkanCore::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(nowPhysicalDevice());
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t>uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentFamily.value() };
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.emplace_back(queueCreateInfo);
	}

	set.DeviceFeatures.samplerAnisotropy = VK_TRUE;

	//deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	//deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &set.DeviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	//
	if (enableValidationLayer) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	VKCREATEPROCESS(vkCreateDevice(nowPhysicalDevice(), &deviceCreateInfo, nullptr, &set.Device), "failed to create logical device!");

	vkGetDeviceQueue(set.Device, indices.graphicsFamily.value(), 0, &set.GraphicsQueue);
	vkGetDeviceQueue(set.Device, indices.presentFamily.value(), 0, &set.PresentQueue);//要記得定義prsentQueue
}

void NEVulkanCore::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwaipChainSupport(nowPhysicalDevice());

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.Formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.PresentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.Capabilities);
	uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;

	if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount) {
		imageCount = swapChainSupport.Capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfoKHR{};
	swapChainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfoKHR.surface = set.Surface;
	swapChainCreateInfoKHR.minImageCount = imageCount;
	swapChainCreateInfoKHR.imageFormat = surfaceFormat.format;
	swapChainCreateInfoKHR.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfoKHR.imageExtent = extent;
	swapChainCreateInfoKHR.imageArrayLayers = 1;
	swapChainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(nowPhysicalDevice());
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		swapChainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfoKHR.queueFamilyIndexCount = 2;
		swapChainCreateInfoKHR.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		swapChainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfoKHR.queueFamilyIndexCount = 0; // Optional
		swapChainCreateInfoKHR.pQueueFamilyIndices = nullptr; // Optional
	}
	swapChainCreateInfoKHR.preTransform = swapChainSupport.Capabilities.currentTransform;
	swapChainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfoKHR.presentMode = presentMode;
	swapChainCreateInfoKHR.clipped = VK_TRUE;

	swapChainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

	VKCREATEPROCESS(vkCreateSwapchainKHR(set.Device, &swapChainCreateInfoKHR, nullptr, &swapChainSet.SwapChain), "failed to create swap chain!");


	vkGetSwapchainImagesKHR(set.Device, swapChainSet.SwapChain, &imageCount, nullptr);
	swapChainSet.Images.resize(imageCount);
	vkGetSwapchainImagesKHR(set.Device, swapChainSet.SwapChain, &imageCount, swapChainSet.Images.data());

	swapChainSet.ImageFormat = surfaceFormat.format;
	swapChainSet.Extent = extent;

}


SwapChainSupportDetails NEVulkanCore::querySwaipChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, set.Surface, &details.Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, set.Surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, set.Surface, &formatCount, details.Formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, set.Surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, set.Surface, &presentModeCount, details.PresentModes.data());
	}



	return details;
}


VkSurfaceFormatKHR NEVulkanCore::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&availableFormats) {
	for (const auto&availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR NEVulkanCore::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>&availablePresentModes) {
	for (const auto&availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D NEVulkanCore::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	//不知道為什麼std::numeric_limit不能用 所以我只好直接叫unsigned int 的max值
	if (capabilities.currentExtent.width != 0xffffffff) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(NeuGlfwIO::GetInstance()->getWindow(), &width, &height);
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;

	}
}

void NEVulkanCore::createSwapChainImageViews() {

	swapChainSet.ImageViews.resize(swapChainSet.Images.size());

	for (size_t i = 0; i < swapChainSet.Images.size(); i++) {
		swapChainSet.ImageViews[i] = createImageView(swapChainSet.Images[i], swapChainSet.ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	}
}


void NEVulkanCore::createRenderPass() {
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkSubpassDescription>subpasses;
	std::vector<VkSubpassDependency>subpassDepndencies;



	//Color Attahment
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainSet.ImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments.emplace_back(colorAttachment);

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Depth Buffer Attachment
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments.emplace_back(depthAttachment);

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



	//Subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpasses.emplace_back(subpass);

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDepndencies.emplace_back(dependency);



	////Imgui Attachment
	//VkAttachmentDescription imguiAttachment = {};
	//imguiAttachment.format = swapChainSet.ImageFormat;
	//imguiAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	//imguiAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//imguiAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	//imguiAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//imguiAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//imguiAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//imguiAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//imguiAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//attachments.emplace_back(imguiAttachment);

	////Subpass for imgui
	//VkAttachmentReference imguiAttachmentRef = {};
	//imguiAttachmentRef.attachment = 0;
	//imguiAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//VkAttachmentReference imguiAttachmentRefDepth = {};
	//imguiAttachmentRefDepth.attachment = 1;
	//imguiAttachmentRefDepth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//VkSubpassDescription subpass_imgui{};
	//subpass_imgui.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//subpass_imgui.colorAttachmentCount = 1;
	//subpass_imgui.pColorAttachments = &colorAttachmentRef;
	//subpass_imgui.pDepthStencilAttachment = &depthAttachmentRef;
	//subpasses.emplace_back(subpass_imgui);

	//VkSubpassDependency imguiDependency{};
	//imguiDependency.srcSubpass = 0;
	//imguiDependency.dstSubpass = 1;
	//imguiDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//imguiDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//imguiDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//imguiDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//subpassDepndencies.emplace_back(imguiDependency);

	//Render Pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDepndencies.size());
	renderPassInfo.pDependencies = subpassDepndencies.data();

	VKCREATEPROCESS(vkCreateRenderPass(set.Device, &renderPassInfo, nullptr, &renderPassSet.RenderPass), "failed to create render pass");

}

VkFormat NEVulkanCore::findSupportedFormat(const std::vector<VkFormat>&candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(nowPhysicalDevice(), format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures&features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format");
}

VkFormat NEVulkanCore::findDepthFormat() {
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void NEVulkanCore::createCommandPool() {
	QueueFamilyIndices queueFamilyIndicies = findQueueFamilies(nowPhysicalDevice());

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndicies.graphicsFamily.value();

	VKCREATEPROCESS(vkCreateCommandPool(set.Device, &poolInfo, nullptr, &set.CommandPool), "failed to create command pool");

}

void NEVulkanCore::createDepthResources() {
	VkFormat depthFormat = findDepthFormat();
	createImage(swapChainSet.Extent.width, swapChainSet.Extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, swapChainSet.DepthBuffer.Image, swapChainSet.DepthBuffer.Memory);
	swapChainSet.DepthBuffer.ImageView = createImageView(swapChainSet.DepthBuffer.Image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	transitionImageLayout(swapChainSet.DepthBuffer.Image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}


void NEVulkanCore::createFrameBuffers() {
	swapChainSet.FrameBuffers.resize(swapChainSet.ImageViews.size());
	for (size_t i = 0; i < swapChainSet.FrameBuffers.size(); i++) {
		std::array<VkImageView, 2> attachments = {
			swapChainSet.ImageViews[i],
			swapChainSet.DepthBuffer.ImageView,
			//swapChainSet.ImguiBuffer.ImageView
		};

		VkFramebufferCreateInfo frameBufferInfo{};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = renderPassSet.RenderPass;
		frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferInfo.pAttachments = attachments.data();
		frameBufferInfo.width = swapChainSet.Extent.width;
		frameBufferInfo.height = swapChainSet.Extent.height;
		frameBufferInfo.layers = 1;

		VKCREATEPROCESS(vkCreateFramebuffer(set.Device, &frameBufferInfo, nullptr, &swapChainSet.FrameBuffers[i]), "failed to create framebuffer");
	}
}


void NEVulkanCore::createDescriptionLayout() {
	/*
	 *DescriptorSetLayout綁定項目
	 *1:Ubo(Uniform Buffer Object)
	 *2:Sampler
	*/

	std::vector<VkDescriptorSetLayoutBinding>bindings;
	//VkDescriptorSetLayoutBinding uboModelLayoutBinding{};
	//uboModelLayoutBinding.binding = 0;
	//uboModelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//uboModelLayoutBinding.descriptorCount = 1;
	//uboModelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	//uboModelLayoutBinding.pImmutableSamplers = nullptr;
	//bindings.emplace_back(uboModelLayoutBinding);

	VkDescriptorSetLayoutBinding uboViewProjLayoutBinding{};
	uboViewProjLayoutBinding.binding = 0;
	uboViewProjLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboViewProjLayoutBinding.descriptorCount = 1;
	uboViewProjLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	uboViewProjLayoutBinding.pImmutableSamplers = nullptr;
	bindings.emplace_back(uboViewProjLayoutBinding);

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 8;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings.emplace_back(samplerLayoutBinding);
	

	VkDescriptorSetLayoutBinding uboLightLayoutBinding{};
	uboViewProjLayoutBinding.binding = 2;
	uboViewProjLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboViewProjLayoutBinding.descriptorCount = 1;
	uboViewProjLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	uboViewProjLayoutBinding.pImmutableSamplers = nullptr;
	bindings.emplace_back(uboViewProjLayoutBinding);

	VkDescriptorSetLayoutBinding uboMaterialLayoutBinding{};
	uboMaterialLayoutBinding.binding = 3;
	uboMaterialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboMaterialLayoutBinding.descriptorCount = 1;
	uboMaterialLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	uboMaterialLayoutBinding.pImmutableSamplers = nullptr;
	bindings.emplace_back(uboMaterialLayoutBinding);
	
	//samplerLayoutBinding.descriptorType=VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	VKCREATEPROCESS(vkCreateDescriptorSetLayout(set.Device, &layoutInfo, nullptr, &descriptorSets.DescriptorSetLayout), "faied to create descriptor set layout");

}

void NEVulkanCore::createUniformBuffers() {
	VkDeviceSize ViewProjbufferSize = sizeof(UboViewProj);
	uboSets.ViewProjection.resize(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);

	VkDeviceSize LightBufferSize = sizeof(UboLight);
	uboSets.Light.resize(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);

	VkDeviceSize MaterialSize = sizeof(UboMaterial);
	uboSets.Material.resize(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT; i++) {

		createBuffer(ViewProjbufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uboSets.ViewProjection[i]);
		vkMapMemory(set.Device, uboSets.ViewProjection[i].Memory, 0, ViewProjbufferSize, 0, &uboSets.ViewProjection[i].Mapped);


		createBuffer(LightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uboSets.Light[i]);
		vkMapMemory(set.Device, uboSets.Light[i].Memory, 0, LightBufferSize, 0, &uboSets.Light[i].Mapped);

		createBuffer(MaterialSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uboSets.Material[i]);
		vkMapMemory(set.Device, uboSets.Material[i].Memory, 0, MaterialSize, 0, &uboSets.Material[i].Mapped);
	}
}


void NEVulkanCore::createDescriptorPool() {

	std::array<VkDescriptorPoolSize, 4> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[3].descriptorCount = static_cast<uint32_t>(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);




	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	poolInfo.maxSets = static_cast<uint32_t>(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	VKCREATEPROCESS(vkCreateDescriptorPool(set.Device, &poolInfo, nullptr, &descriptorSets.DescriptorPool), "failed to create descriptor pool");

}
void NEVulkanCore::createDescriptorSets() {
	std::vector<VkDescriptorSetLayout> layouts(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT, descriptorSets.DescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorSets.DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.DescriptorSets.resize(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	VKCREATEPROCESS(vkAllocateDescriptorSets(set.Device, &allocInfo, descriptorSets.DescriptorSets.data()), "failed to allocate descriptor sets");

	for (size_t i = 0; i < NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT; i++) {
		//VkDescriptorBufferInfo bufferModelInfo{};
		//bufferModelInfo.buffer = uboSets.Model[i].Buffer;
		//bufferModelInfo.offset = 0;
		//bufferModelInfo.range = sizeof(PcModel);

		VkDescriptorBufferInfo bufferViewProjProjInfo{};
		bufferViewProjProjInfo.buffer = uboSets.ViewProjection[i].Buffer;
		bufferViewProjProjInfo.offset = 0;
		bufferViewProjProjInfo.range = sizeof(UboViewProj);

		std::vector<VkDescriptorImageInfo> imageInfos = TextureManager::GetInstance()->createDescriptorImageInfos();

		VkDescriptorBufferInfo bufferLightInfo{};
		bufferLightInfo.buffer = uboSets.Light[i].Buffer;
		bufferLightInfo.offset = 0;
		bufferLightInfo.range = sizeof(UboLight);

		VkDescriptorBufferInfo bufferMateirlaInfo{};
		bufferMateirlaInfo.buffer = uboSets.Material[i].Buffer;
		bufferMateirlaInfo.offset = 0;
		bufferMateirlaInfo.range = sizeof(UboMaterial);


		std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
		//descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrites[0].dstSet = descriptorSets.DescriptorSets[i];
		//descriptorWrites[0].dstBinding = 0;
		//descriptorWrites[0].dstArrayElement = 0;
		//descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//descriptorWrites[0].descriptorCount = 1;
		//descriptorWrites[0].pBufferInfo = &bufferModelInfo;
		//descriptorWrites[0].pImageInfo = nullptr;
		//descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets.DescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferViewProjProjInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets.DescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = imageInfos.size();
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pImageInfo = imageInfos.data();
		descriptorWrites[1].pTexelBufferView = nullptr;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets.DescriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &bufferLightInfo;
		descriptorWrites[2].pImageInfo = nullptr;
		descriptorWrites[2].pTexelBufferView = nullptr;

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = descriptorSets.DescriptorSets[i];
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pBufferInfo = &bufferMateirlaInfo;
		descriptorWrites[3].pImageInfo = nullptr;
		descriptorWrites[3].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(set.Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}


}


void NEVulkanCore::createCommandBuffer() {
	set.CommandBuffers.resize(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = set.CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = set.CommandBuffers.size();

	VKCREATEPROCESS(vkAllocateCommandBuffers(set.Device, &allocInfo, set.CommandBuffers.data()), "failed to create command buffer");

}


void NEVulkanCore::createSyncObjects() {
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	renderPassSet.ImageAvailableSemaphores.resize(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	renderPassSet.RenderFinishedSemaphores.resize(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);
	renderPassSet.InFlightFences.resize(NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(set.Device, &semaphoreInfo, nullptr, &renderPassSet.ImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(set.Device, &semaphoreInfo, nullptr, &renderPassSet.RenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(set.Device, &fenceInfo, nullptr, &renderPassSet.InFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

uint32_t NEVulkanCore::startFrame() {
	vkWaitForFences(set.Device, 1, &renderPassSet.InFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(set.Device, swapChainSet.SwapChain, UINT64_MAX, renderPassSet.ImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return -1;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	result = vkResetFences(set.Device, 1, &renderPassSet.InFlightFences[currentFrame]);

	vkResetCommandBuffer(set.CommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
	recordCommandBuffer(set.CommandBuffers[currentFrame], imageIndex);

	//updateUniformBuffer(currentFrame);
	//vkResetFences(device, 1, &inFlightFences[currentFrame]);

	//vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);


	return imageIndex;



}
void NEVulkanCore::endFrame(uint32_t imageIndex) {

	if (imageIndex == -1)
		return;
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { renderPassSet.ImageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &set.CommandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { renderPassSet.RenderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VKCREATEPROCESS(vkQueueSubmit(set.GraphicsQueue, 1, &submitInfo, renderPassSet.InFlightFences[currentFrame]), "failed to submit draw command buffer");


	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChainSet.SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	//presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(set.PresentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT;
}


void NEVulkanCore::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	/*要記得製造beginBuffer*/
	VKCREATEPROCESS(vkBeginCommandBuffer(commandBuffer, &beginInfo), "failed to begin recording command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPassSet.RenderPass;
	renderPassInfo.framebuffer = swapChainSet.FrameBuffers[imageIndex];

	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = swapChainSet.Extent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	updateUboProjView(camera.makeUboViewProj());


	

	//TODO:之後模組化
	UboViewProj viewProj;
	MakeViewProjMatrix(viewProj, glm::vec3(60.0f, 60.0f, 60.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(45.0f), 16 / (float)9.0f, 0.1f, 1000.0f);
	viewProj.viewPos = glm::vec3(60.0f, 60.0f, 60.0f);
	updateUboProjView(viewProj);


	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
	UboLight light;
	LightSystem::GetInstance()->createUboData(light);
	updateUboLight(light);

	UboMaterial material;
	material.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
	material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	material.shiness = 32;
	material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	updateUboMaterial(material);



	//UboLight light;
	//light.ambientStrength = 0.05;
	//light.color = glm::vec3(1.0f, 1.0f, 1.0f);
	//light.pos = glm::vec3(35*glm::sin(omega)*glm::cos(theta), 35 * glm::sin(omega)*glm::sin(theta), 35 * cos(omega));
	//light.shiness = 16;
	//light.specularStrength = 0.6;
	//light.viewPos = glm::vec3(2, 2, 2);
	//updateLight(light);


	//PcNormal lightNormal;
	//MakeModelMatrix(lightNormal.model, light.pos, glm::vec3(1, 1, 1), glm::vec3(0, 0, 0));
	//lightNormal.selector = 0;

	//////////////////////////RenderPass////////////////////////////
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainSet.Extent.width;
		viewport.height = (float)swapChainSet.Extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = swapChainSet.Extent;
		vkCmdSetScissor(commandBuffer, 0, 01, &scissor);

		pcNormal.selector = 1;
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.Pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.PipelineLayout, 0, 1, &descriptorSets.DescriptorSets[currentFrame], 0, nullptr);
		vkCmdPushConstants(commandBuffer, graphicsPipeline.PipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(PcNormal), &pcNormal);
		Meshmodel.drawCmd(commandBuffer);
		pcNormal.selector = 0;
		vkCmdPushConstants(commandBuffer, graphicsPipeline.PipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(PcNormal), &pcNormal);
		FloorMesh.drawCmd(commandBuffer);


		/*光照 model*/
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPipeline.Pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPipeline.PipelineLayout, 0, 1, &descriptorSets.DescriptorSets[currentFrame], 0, nullptr);
		//vkCmdPushConstants(commandBuffer, graphicsPipeline.PipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(PcNormal), &lightNormal);

		//Ballmodel.drawCmd(commandBuffer);




		NeuGui::drawGuiEvent(commandBuffer);

	//Pass 0->1
	//vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass(commandBuffer);

	////////////////////////////RenderPass////////////////////////////
	VKCREATEPROCESS(vkEndCommandBuffer(commandBuffer), "failed to record command buffer");


}




void NEVulkanCore::recreateSwapChain() {
	int width = 0, height = 0;
	glfwGetFramebufferSize(NeuGlfwIO::GetInstance()->getWindow(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(NeuGlfwIO::GetInstance()->getWindow(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(set.Device);

	cleanupSwapChain();

	createSwapChain();
	createSwapChainImageViews();
	createDepthResources();
	createFrameBuffers();
}


void NEVulkanCore::cleanupSwapChain() {
	for (auto framebuffer : swapChainSet.FrameBuffers) {
		vkDestroyFramebuffer(set.Device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainSet.ImageViews) {
		vkDestroyImageView(set.Device, imageView, nullptr);
	}
	

	vkDestroySwapchainKHR(set.Device, swapChainSet.SwapChain, nullptr);
}


void NEVulkanCore::release() {
	TextureManager::GetInstance()->release();
	releaseSwapChainAndRenderPassSet();
	
	releaseVulkanSet();
}

void NEVulkanCore::releaseVulkanSet() {
	vkDestroySurfaceKHR(set.Instance, set.Surface, nullptr);
	vkDestroyPipelineCache(set.Device, set.PipelineCache, nullptr);
	vkDestroyCommandPool(set.Device, set.CommandPool, nullptr);
	vkDestroyDevice(set.Device, nullptr);
	vkDestroyInstance(set.Instance, nullptr);
}

void NEVulkanCore::releaseSwapChainAndRenderPassSet() {
	vkDestroySwapchainKHR(set.Device, swapChainSet.SwapChain, nullptr);
	vkDestroyRenderPass(set.Device, renderPassSet.RenderPass, nullptr);
	for (uint16_t i = 0; i < NEUVK_CONFIGS_MAX_FRAMES_IN_FLIGHT; i++) {

		vkDestroySemaphore(set.Device, renderPassSet.ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(set.Device, renderPassSet.RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(set.Device, renderPassSet.InFlightFences[i], nullptr);


		vkDestroyImage(set.Device, swapChainSet.Images[i], nullptr);
		vkDestroyImageView(set.Device, swapChainSet.ImageViews[i], nullptr);
	}
}


void NEVulkanCore::createPipeline(PipelineSet &pipeline,const std::string vertexShader, const std::string fragmentShader) {


	auto vertShaderCode = readFile(shaderSrc(vertexShader));
	auto fragShaderCode = readFile(shaderSrc(fragmentShader));



	NEUVKShader *shader = NEUVKShader::CreateShader(false,vertexShader.c_str(),fragmentShader.c_str(), nullptr);

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
	
	////vertex shader
	//VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	//vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	//vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	//vertShaderStageInfo.module = vertShaderModule;
	//vertShaderStageInfo.pName = "main";

	////fragment shader(or pixel shader)
	//VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	//fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	//fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	//fragShaderStageInfo.module = fragShaderModule;
	//fragShaderStageInfo.pName = "main";

	//VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


	/*Vertex Input*/
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	/*
	 *Vertex Input BInd
	 */
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescription = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	/*
		Input Assembly
		決定要繪製哪種幾何形狀以及是否Primitive Restart
	*/
	VkPipelineInputAssemblyStateCreateInfo  inputAssemblyCreateInfo{};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;


	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	/*光柵*/
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	/*MultiSampling多重取樣*/
	VkPipelineMultisampleStateCreateInfo multiSampling{};
	multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampling.sampleShadingEnable = VK_FALSE;
	multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampling.minSampleShading = 1.0f;
	multiSampling.pSampleMask = nullptr;
	multiSampling.alphaToCoverageEnable = VK_FALSE;
	multiSampling.alphaToOneEnable = VK_FALSE;

	/*Color Blending*/
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState>dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};


	/*
		Dynamic State
		因為VkPipeline如果要更動會需要進行整個Pipeline的重建 所以我們需要一些東西作為VkPipeline的類似API來用
		這類東西就是Dynamic State
	*/
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	//Push constants
	std::array<VkPushConstantRange, 1>pushConstantRanges;
	pushConstantRanges[0].offset = 0;
	pushConstantRanges[0].size = sizeof(PcNormal);
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_ALL;


	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSets.DescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();


	VKCREATEPROCESS(vkCreatePipelineLayout(set.Device, &pipelineLayoutInfo, nullptr, &pipeline.PipelineLayout), "failed to create pipeline layout");


	/*Depth Buffer*/
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shader->shaderCreateInfos.size();
	pipelineInfo.pStages = shader->shaderCreateInfos.data();

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multiSampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlendCreateInfo;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = pipeline.PipelineLayout;
	pipelineInfo.renderPass = renderPassSet.RenderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;


	VKCREATEPROCESS(vkCreateGraphicsPipelines(set.Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.Pipeline), "failed to create graphics pipeline");

	vkDestroyShaderModule(set.Device, fragShaderModule, nullptr);
	vkDestroyShaderModule(set.Device, vertShaderModule, nullptr);
}

void NEVulkanCore::createMainPipeline() {





}

void NEVulkanCore::createLightPipeline() {

}


void NEVulkanCore::createVulkanPipeline() {
	createPipeline(graphicsPipeline, "testvert.spv", "testfrag.spv");
	createPipeline(lightingPipeline, "lightingvert.spv", "lightingfrag.spv");
}


VkShaderModule NEVulkanCore::createShaderModule(const std::vector<char>&spirvCode) {
	VkShaderModuleCreateInfo vsmCreateInfo{};
	vsmCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vsmCreateInfo.codeSize = spirvCode.size();
	vsmCreateInfo.pCode = reinterpret_cast<const uint32_t*>(spirvCode.data());

	VkShaderModule shaderModule;
	VKCREATEPROCESS(vkCreateShaderModule(set.Device, &vsmCreateInfo, nullptr, &shaderModule), "failed to create shader module");

	return shaderModule;
}

void NEVulkanCore::startDraw() {

}

void NEVulkanCore::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0,0,0 };
	region.imageExtent = { width,height,1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	endSingleTimeCommands(commandBuffer);
}