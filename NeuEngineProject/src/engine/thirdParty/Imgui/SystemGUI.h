#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "API/imgui.h"
#include "API/imgui_impl_glfw.h"
#include "API/imgui_impl_vulkan.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../NeuVulkan/VulkanCore.h"

#include "../../NeuDataStructure.h"
#include "../../NeuTools.h"
#include "../../NeuComponent/IO/io.h"
namespace NeuGui {
	static int                      g_MinImageCount = 2;
	static bool                     g_SwapChainRebuild = false;
	static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;
	static VkAllocationCallbacks*   g_Allocator = nullptr;
	static ImDrawData*				g_DrawData = nullptr;
	void initGui();
	void updateGuiEventStart();
	void updateGuiEventEnd();
	void drawGuiEvent(VkCommandBuffer commandBuffer);
	void endGuiEvent();


	void setupGuiDescriptor(Neu::VulkanSet *set);
	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		printf("[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}




}