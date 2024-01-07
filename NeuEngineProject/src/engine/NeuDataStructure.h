#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <glm/glm.hpp>
#include <array>
#include "Config.h"
namespace Neu {

	struct VulkanSet {
		VkInstance                      Instance;
		std::vector<VkPhysicalDevice>   PhysicalDevices;
		uint32_t						NowPhysicalDevice;
		VkPhysicalDeviceFeatures		DeviceFeatures{};
		VkDevice                        Device;
		uint32_t                        QueueFamily;
		VkQueue                         GraphicsQueue;
		VkQueue							PresentQueue;
		VkPipelineCache                 PipelineCache;
		VkSurfaceKHR					Surface;
		VkCommandPool					CommandPool;
		std::vector<VkCommandBuffer>	CommandBuffers;
	};

	struct VulkanExts {
		VkDebugUtilsMessengerEXT		DebugMessenger;
	};

	struct ImageSet {
		VkImage							Image;
		VkDeviceMemory					Memory;
		VkImageView						ImageView;
	};

	struct VulkanDescriptorSets {
		VkDescriptorSetLayout			DescriptorSetLayout;
		VkDescriptorPool				DescriptorPool;
		std::vector<VkDescriptorSet>	DescriptorSets;
	};

	struct VulkanSwapChainSets {
		VkSwapchainKHR					SwapChain;
		std::vector<VkImage>			Images;
		std::vector<VkImageView>		ImageViews;
		VkFormat						ImageFormat;
		VkExtent2D						Extent;
		ImageSet						DepthBuffer;
		ImageSet						ImguiBuffer;
		std::vector<VkFramebuffer>		FrameBuffers;

	};


	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR		Capabilities;
		std::vector <VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR>	PresentModes;
	};

	struct VulkanRenderPassSet {
		VkRenderPass					RenderPass;
		std::vector<VkSemaphore>		ImageAvailableSemaphores;
		std::vector<VkSemaphore>		RenderFinishedSemaphores;
		std::vector<VkFence>			InFlightFences;

	};

	struct BufferSet {
		VkBuffer						Buffer;
		VkDeviceMemory					Memory;
		void*							Mapped = nullptr;
	};

	struct UniformBufferSet {
		std::vector<BufferSet>			ViewProjection;
		std::vector<BufferSet>			Light;
		std::vector<BufferSet>			Material;
	};

	struct PipelineSet {
		VkPipeline						Pipeline;
		VkPipelineLayout				PipelineLayout;
	};

	struct TextureSet {
		VkSampler						Sampler;
		VkImage							Image;
		VkImageView						ImageView;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t>			graphicsFamily;
		std::optional<uint32_t>			presentFamily;
		//讓struct自己能夠檢查是否完成賦值
		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();

		}
	};


	struct PcNormal {
		glm::mat4			model;
		uint32_t selector;
	};


	struct UboViewProj {
		alignas(16)	glm::mat4			view;
		alignas(16)	glm::mat4			proj;
		alignas(16) glm::vec3			viewPos;
	};

	struct DirLightStruct {

		alignas(16)		glm::vec3		direction;
		alignas(16)		glm::vec3		ambient;
		alignas(16)		glm::vec3		diffuse;
		alignas(16)		glm::vec3		specular;
	};

	struct PointLightStruct {
		alignas(16)		glm::vec3		pos;
		alignas(4)		float			constant;
		alignas(4)		float			linear;
		alignas(4)		float			quadratic;
		alignas(16)		glm::vec3		ambient;
		alignas(16)		glm::vec3		diffuse;
		alignas(16)		glm::vec3		specular;
	};

	struct SpotLightStruct {
		alignas(16)		glm::vec3		pos;
		alignas(16)		glm::vec3		direction;
		alignas(4)		float			cutOff;
		alignas(4)		float			outerCutOff;

		alignas(4)		float			constant;
		alignas(4)		float			linear;
		alignas(4)		float			quadratic;

		alignas(16)		glm::vec3		ambient;
		alignas(16)		glm::vec3		diffuse;
		alignas(16)		glm::vec3		specular;
	};



	struct UboLight {
		DirLightStruct					dirLight;
		PointLightStruct				pointLight[NEUVK_CONFIGS_SPOTLIGHTCOUNT];
		SpotLightStruct					spotLight;
	};
	


	struct UboMaterial {
		alignas(16)		glm::vec3		ambient;
		alignas(16)		glm::vec3		diffuse;
		alignas(16)		glm::vec3		specular;
		alignas(4)		float			shiness;
	};



	struct Vertex {
		glm::vec3						pos;
		glm::vec3						normal;
		glm::vec2						texcoord;
	};

	struct PipelineRenderSet{
		VkBuffer*						VertexBuffer;
		VkBuffer*						IndedBuffer;
		uint32_t*						Size;
	};

}

enum VertexInputType {
	NEU_VK_VERTEX_INPUT_TYPE_NONE = 0,
	NEU_VK_VERTEX_INPUT_TYPE_POSITION,
	NEU_VK_VERTEX_INPUT_TYPE_UV,
	NEU_VK_VERTEX_INPUT_TYPE_NORMAL,
	NEU_VK_VERTEX_INPUT_TYPE_COLOR,
	NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT,
	NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT2,
	NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT3,
	NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT4,
};


/*
layout binding 0  mat4 model
layout binding 1  {mat4 view,mat4 proj}
layout binding 2 Sampler2D albedo
layout binding 3 Sampler2D Normal
layout binding 4 Sampler2D Metallic
layout binding 5 Sampler2D Roughness
layout binding 6 Sampler2D Displacement
layout binding 7 Sampler2D Ambient

layout in 0 vec3 inPosition
layout in 1 vec3 inColor
layout in 2 vec3 inNormal
layout in 3 vec2 inCoord
*/