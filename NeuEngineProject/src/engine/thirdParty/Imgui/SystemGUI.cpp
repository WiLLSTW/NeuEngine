#include "SystemGUI.h"


void NeuGui::initGui() {
	auto set = Neu::NEVulkanCore::GetInstance()->getVulkanSet();
	auto renderPassSet = Neu::NEVulkanCore::GetInstance()->getVulkanRenderPassSet();

	setupGuiDescriptor(set);
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui::StyleColorsLight();


	ImGui_ImplGlfw_InitForVulkan(Neu::NeuGlfwIO::GetInstance()->getWindow(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = set->Instance;
	init_info.PhysicalDevice = set->PhysicalDevices[set->NowPhysicalDevice];
	init_info.Device = set->Device;
	init_info.QueueFamily = set->QueueFamily;
	init_info.Queue = set->PresentQueue;
	init_info.PipelineCache = set->PipelineCache;
	init_info.DescriptorPool = g_DescriptorPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = g_MinImageCount;
	init_info.ImageCount = 2;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = g_Allocator;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info, renderPassSet->RenderPass);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool =set->CommandPool;
	allocInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	Neu::NeuVkCreateProcess(vkAllocateCommandBuffers(set->Device, &allocInfo, &commandBuffer), "Failed to create imgui command buffer");
	

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	Neu::NeuVkCreateProcess(vkBeginCommandBuffer(commandBuffer, &begin_info), "Failed to begin imgui command buffer");
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &commandBuffer;
	Neu::NeuVkCreateProcess(vkEndCommandBuffer(commandBuffer), "Failed to end imgui command buffer");
	
	vkQueueSubmit(set->GraphicsQueue, 1, &end_info, VK_NULL_HANDLE);

	vkDeviceWaitIdle(set->Device);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
	vkFreeCommandBuffers(set->Device, set->CommandPool, 1, &commandBuffer);


}

void NeuGui::updateGuiEventStart() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void NeuGui::updateGuiEventEnd() {
	ImGui::Render();
	g_DrawData = ImGui::GetDrawData();
}

void NeuGui::drawGuiEvent(VkCommandBuffer commandBuffer) {
	
	ImGui_ImplVulkan_RenderDrawData(g_DrawData, commandBuffer);

}

void NeuGui::endGuiEvent() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void NeuGui::setupGuiDescriptor(Neu::VulkanSet *set) {

	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	Neu::NeuVkCreateProcess(vkCreateDescriptorPool(set->Device, &pool_info, g_Allocator, &g_DescriptorPool), "Failed to create imgui descriptorPool");

}

