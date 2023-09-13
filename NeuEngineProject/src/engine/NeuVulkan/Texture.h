#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <string>
#include "NeuVulkanTools.h"
#include "../NeuDataStructure.h"
#include "../Config.h"


namespace Neu {
	class TextureManager {
		TextureManager() {};
		uint16_t cnt = 0;
		std::array<VkSampler, NEUVK_CONFIGS_DESCRIPTOR_SAMPLER_COUNT>SamplerSet;
		std::array<VkImage, NEUVK_CONFIGS_DESCRIPTOR_SAMPLER_COUNT>ImageSet;
		std::array<VkImageView, NEUVK_CONFIGS_DESCRIPTOR_SAMPLER_COUNT>ImageViewSet;
		std::array<VkDeviceMemory, NEUVK_CONFIGS_DESCRIPTOR_SAMPLER_COUNT>ImageMemorySet;
		std::array<std::string, NEUVK_CONFIGS_DESCRIPTOR_SAMPLER_COUNT>ImageName;

		void createTextureImageView(uint16_t ct) {
			ImageViewSet[ct] = createImageView(ImageSet[ct], VK_FORMAT_R8G8B8A8_SRGB);
		}

		void createTextureImage(const char* src, uint16_t ct);

		void createTextureSampler() {
			VulkanSet *set = getVulkanSet();

			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;

			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(set->PhysicalDevices[set->NowPhysicalDevice], &properties);
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			for (uint16_t i = 0; i < 1; i++) {
				VKCREATEPROCESS(vkCreateSampler(set->Device, &samplerInfo, nullptr, &SamplerSet[i]), "failed to create texture sampler!");
			}

		}
	public:



		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;
		TextureManager(TextureManager&&) = delete;
		TextureManager& operator=(TextureManager&&) = delete;
		static TextureManager* GetInstance() {
			static TextureManager instance;
			return &instance;
		}



		void init() {
			createTextureSampler();
		}

		void release () {
			VulkanSet *set = getVulkanSet();
			auto device = set->Device;
			for (int i = 0; i < NEUVK_CONFIGS_DESCRIPTOR_SAMPLER_COUNT;i++) {
				vkDestroySampler(device, SamplerSet[i], nullptr);
				vkDestroyImageView(device, ImageViewSet[i], nullptr);
				if (i < cnt) {
					vkDestroyImage(device, ImageSet[i], nullptr);
				}

				vkFreeMemory(device, ImageMemorySet[i], nullptr);
			}
		}

		uint16_t addTexture(const char* src) {
			if (cnt >= NEUVK_CONFIGS_DESCRIPTOR_SAMPLER_COUNT) {
				printf("Texture is full");
				return 0;
			}
			createTextureImage(src,cnt);
			createTextureImageView(cnt);

			uint16_t re = cnt;
			cnt++;

			return re;
		};


		std::vector<VkDescriptorImageInfo>  createDescriptorImageInfos() {
			std::vector<VkDescriptorImageInfo> descriptorImageInfos;
			for (uint16_t i = 0; i < NEUVK_CONFIGS_DESCRIPTOR_SAMPLER_COUNT; i++) {
				VkDescriptorImageInfo info;
				info.sampler = SamplerSet[0];
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				if (i >= cnt) {
					info.imageView = ImageViewSet[0];
				}
				else {
					info.imageView = ImageViewSet[i];
				}
				

				descriptorImageInfos.emplace_back(info);
			}
			return descriptorImageInfos;
		}

	};
}
