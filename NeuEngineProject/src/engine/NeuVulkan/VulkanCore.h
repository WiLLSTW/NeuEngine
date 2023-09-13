#pragma once
#include "../NeuDataStructure.h"
#include "../NeuTools.h"
#include <iostream>
#include <vector>
#include <array>
#include <set>
#include <algorithm>
#include "../thirdParty/Imgui/SystemGUI.h"
#include <stdio.h>
#include "../NeuComponent/Camera.h"
#include "NeuVulkanTools.h"
#include "../NeuComponent/mesh.h"
#include "../Config.h"
#include "Texture.h"
#include "../NeuComponent/lighting.h"
#include "Shader.h"
namespace Neu {

	class NEVulkanCore {
	private:
#ifdef NDEBUG
		const bool enableValidationLayer = false;
#else
		const bool enableValidationLayer = true;
#endif


		Model Meshmodel;
		Model Ballmodel;
		Mesh  FloorMesh;
		PcNormal pcNormal;

		NEVulkanCore() {};
		VulkanSet set;						//Vulkan Resource Set
		VulkanExts ext;						//extension
		VulkanSwapChainSets swapChainSet;
		VulkanRenderPassSet renderPassSet;
		VulkanDescriptorSets descriptorSets;
		UniformBufferSet	uboSets;
		PipelineSet			graphicsPipeline;
		PipelineSet			lightingPipeline;


		
		bool framebufferResized = false;
		uint32_t currentFrame = 0;

		std::vector<PipelineRenderSet> renderSet;
		Camera camera;

		NEUVKShader shader;

		
		void createVulkanInstance();
		void setupDebugMessanger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createSwapChainImageViews();
		void createRenderPass();
		void createDescriptionLayout();
		void createCommandPool();
		void createDepthResources();
		void createFrameBuffers();//óvç›DepthResourcesënåöîVå„
		void createVulkanPipeline();
		VkShaderModule createShaderModule(const std::vector<char>&spirvCode);


		void createPipeline(PipelineSet &pipeline, const std::string vertexShader, const std::string fragmentShader);
		void createMainPipeline();
		void createLightPipeline();
		//void createPipelineWGeometryShader(PipelineSet pipeline);

		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffer();
		void createSyncObjects();

		void createTextureImage();
		void createTextureImageView();
		void createTextureSampler();

		bool checkValidationLayerSupport();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& vdumCreateInfo);
		/*ÈÑÊöëwçÄñ⁄*/
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		//é˘óvéxâáìIKHR ExtensionéÌóﬁòaèW
		const std::vector<const char*>deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		/*ÈÑÊöëwçˆåÎêuëßóAèo*/
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}
		bool isDeviceSuitable(VkPhysicalDevice device);
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		QueueFamilyIndices  findQueueFamilies(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwaipChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>&availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();
		void recreateSwapChain();
		void cleanupSwapChain();
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void updateUniformBuffer(void*bufferMapped, const void *_src, uint32_t size) {
			memcpy(bufferMapped, _src, size);
		}
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	public:
		NEVulkanCore(const NEVulkanCore&) = delete;
		NEVulkanCore& operator=(const NEVulkanCore&) = delete;
		NEVulkanCore(NEVulkanCore&&) = delete;
		NEVulkanCore& operator=(NEVulkanCore&&) = delete;
		static NEVulkanCore* GetInstance() {
			static NEVulkanCore instance;
			return &instance;
		}

		void init() {
			VulkanToolsSetting(&set);
			createVulkanInstance();
			setupDebugMessanger();
			createSurface();
			pickPhysicalDevice();
			createLogicalDevice();
			createSwapChain();
			createSwapChainImageViews();
			createRenderPass();


			createDescriptionLayout();
			createVulkanPipeline();
			createCommandPool();
			createDepthResources();
			//createImguiResources();
			createFrameBuffers();//óvç›DepthResourcesënåöîVå„


			TextureManager::GetInstance()->init();
			TextureManager::GetInstance()->addTexture("./res/texture/white.png");
			TextureManager::GetInstance()->addTexture("./res/texture/flaski/albedo.png");
			TextureManager::GetInstance()->addTexture("./res/texture/sample.png");



			createUniformBuffers();
			createDescriptorPool();
			createDescriptorSets();
			createCommandBuffer();
			createSyncObjects();
			camera.init(glm::vec3(200.0f, 200.0f, 200.0f), glm::vec3(0.0f, 0.0f, 0.0f));

			Meshmodel.init("./res/model/Flaski.fbx");
			Ballmodel.init("./res/model/Ball.fbx");
			pcNormal.selector = 0;
			MakeModelMatrix(pcNormal.model, glm::vec3(0, 0, 0), glm::vec3(1,1, 1), glm::vec3(0, 0, 0));
			LightSystem::GetInstance()->init();
			
			std::vector<uint16_t>i;


			FloorMesh.init(vertices, indicies);

		}
		const std::vector<Vertex>vertices = {
				{{-100.0f, -100.f,-5.0f}, {0.0f, 0.0f, 1.0f},{1.0f,0.0f}},
				{{100.0f, -100.f,-5.0f}, {0.0f, 0.0f, 1.0f},{0.0f,0.0f}},
				{{100.0f, 100.f,-5.0f}, {0.0f, 0.0f, 1.0f},{0.0f,1.0f}},
				{{-100.0f, 100.f,-5.0f}, {0.0f, 0.0f, 1.0f},{1.0f,1.0f}},
		};
		const std::vector<uint16_t>indicies = {
			{0,1,2,0,2,3}
		};

		void update() {
			static int select;
			ImGui::Begin("ModelSelector");
			ImGui::RadioButton("Texture White", &select, 0); ImGui::SameLine();
			ImGui::RadioButton("Texture Flaski", &select, 1); ImGui::SameLine();
			ImGui::RadioButton("Texture Sample", &select, 2);
			ImGui::End();

			LightSystem::GetInstance()->Update();

			pcNormal.selector = select;
		}


		void startDraw();

		void endDraw();


		void release();

		void releaseVulkanSet();
		void releaseSwapChainAndRenderPassSet();

		VkPhysicalDevice nowPhysicalDevice() {
			return set.PhysicalDevices.at(set.NowPhysicalDevice);
		}




		VulkanSet *getVulkanSet() {
			return &set;
		}
		VulkanSwapChainSets *getVulkanSwapChainSet() {
			return &swapChainSet;
		}
		VulkanRenderPassSet *getVulkanRenderPassSet(){
			return &renderPassSet;
		}
		VulkanDescriptorSets *getVulkanDescriptorSets() {
			return &descriptorSets;
		}

		uint32_t startFrame();
		void endFrame(uint32_t imageIndex);


		void updateUboProjView(UboViewProj viewProj) {
			updateUniformBuffer(uboSets.ViewProjection.at(currentFrame).Mapped, &viewProj, sizeof(viewProj));
		}
		
		void updateUboMaterial (UboMaterial material) {
			updateUniformBuffer(uboSets.Material.at(currentFrame).Mapped, &material, sizeof(material));
		}

		void updateUboLight(UboLight light) {
			updateUniformBuffer(uboSets.Light.at(currentFrame).Mapped, &light, sizeof(light));
		}
		void drawItem(VkCommandBuffer commandBuffer,VkBuffer *vertexBuffers,VkBuffer indexBuffer,VkPipelineLayout pipelineLayout,uint32_t indexSize) {
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.DescriptorSets[currentFrame], 0, nullptr);
			vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);
		}
	};

}