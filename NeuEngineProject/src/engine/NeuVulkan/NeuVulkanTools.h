#pragma once
#include "../NeuDataStructure.h"
#include "../NeuTools.h"
//#include "VulkanCore.h"
namespace Neu {
	static VulkanSet *vkSet;

	void VulkanToolsSetting(VulkanSet *_vkSet);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferSet &bufferSet);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer &commandBuffer);

	void copyBuffer(VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize size);

	void copyBufferToImage(VkBuffer &buffer, VkImage image, uint32_t width, uint32_t height);

	//void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	//	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	//	VkBufferCopy copyRegion{};
	//	copyRegion.srcOffset = 0;
	//	copyRegion.dstOffset = 0;
	//	copyRegion.size = size;
	//	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	//	endSingleTimeCommands(commandBuffer);

	//}



	VulkanSet *getVulkanSet();

	VkDevice getVkDevice();

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

	void transitionImageLayout(VkImage &image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

}

