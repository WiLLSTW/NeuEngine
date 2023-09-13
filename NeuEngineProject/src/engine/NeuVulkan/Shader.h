#pragma once
#include <vulkan/vulkan.h>
#include <list>
#include "../NeuDataStructure.h"
#include <iostream>
#include <vector>
#include "NeuVulkanTools.h"
#include "../NeuTools.h"
#include <spirv_cross/spirv_cross.hpp>
#include <map>
#include "VulkanDescriptor.h"
#include <memory>

namespace Neu {


	class NEUVKShaderModule {
	public:
		NEUVKShaderModule() {};
		~NEUVKShaderModule() {
			if (module != VK_NULL_HANDLE) {
				vkDestroyShaderModule(getVulkanSet()->Device, module, nullptr);
			}
			binaryData.clear();
			binaryData.shrink_to_fit();
		}
		static NEUVKShaderModule* CreateShaderModule(const char* _filename, VkShaderStageFlagBits _type);


		VkShaderModule				module;
		std::vector<char>		binaryData;
		VkShaderStageFlagBits		stage;
	};


	class NEUVKShader {
	public:
		NEUVKShader() {};
		~NEUVKShader() {};

		static NEUVKShader* CreateShader(bool _dynamicUBO, const char *_vert, const char*_frag, const char *_geom);
	
		struct UniformBufferInfo {
			uint32_t set = 0;
			uint32_t binding = 0;
			uint32_t bufferSize = 0;
			VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			VkShaderStageFlags stageFlags = 0;
		};

		struct CombinedImageInfo {
			uint32_t set = 0;
			uint32_t binding = 0;
			VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			VkShaderStageFlags stageFlags = 0;

		};
		struct PushConstantInfo {
			uint32_t offset;
			uint32_t bufferSize = 0;
			VkShaderStageFlags stageFlags = 0;
		};


		struct AttributeSet {
			VertexInputType type;
			int16_t location;
		};
		std::vector<AttributeSet>inputAttributes;


		std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;
		std::vector<VkDescriptorSetLayoutBinding> pipelineLayoutBindings;

		std::vector<VkVertexInputBindingDescription>InputBindings;
		std::vector<VkVertexInputAttributeDescription>InputAttributeDescriptions;
		bool dynamicUBO = false;
		NEUVKShaderModule *vertModule;
		NEUVKShaderModule *fragModule;
		NEUVKShaderModule *geoModule;


		std::vector<VkDescriptorSetLayout>	descriptorSetLayouts;
		VkPipelineLayout					pipelineLayout;

		std::unordered_map<std::string, UniformBufferInfo>UBOInfo;
		std::unordered_map<std::string, CombinedImageInfo>ImageInfo;

		NEUVKDescriptorSetLayoutsInfo setLayoutsInfo;

		//void Set

		void CompileShader();

		void ProcessData(NEUVKShaderModule* module);

		void ProcessUniformBuffer(spirv_cross::Compiler &_compiler, spirv_cross::ShaderResources &_resources, VkShaderStageFlags _stageFlag);

		void ProcessTextures(spirv_cross::Compiler &_compiler, spirv_cross::ShaderResources &_resources, VkShaderStageFlags _stageFlag);

		void ProcessInput(spirv_cross::Compiler &_compiler, spirv_cross::ShaderResources &_resources, VkShaderStageFlags _stageFlags);

		void ProcessPushConstant(spirv_cross::Compiler &_compiler, spirv_cross::ShaderResources &_resources, VkShaderStageFlags _stageFlags);

		void CreateInputInfo();

		void CreateLayout();

		inline VertexInputType StringToVertexType(const char *name);
		inline int16_t VertexTypeToSize(VertexInputType);
		inline VkFormat VertexTypeToFormat(VertexInputType);
	};




}

