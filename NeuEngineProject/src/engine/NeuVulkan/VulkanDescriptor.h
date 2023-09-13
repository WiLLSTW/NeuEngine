#pragma once
#include <vulkan/vulkan.h>
#include <list>
#include <unordered_map>
#include <vector>
#include <iostream>

class NEUVKDescriptorSetLayoutInfo {

public:
	NEUVKDescriptorSetLayoutInfo() {};
	~NEUVKDescriptorSetLayoutInfo() {};
	int32_t set = -1;
	std::vector<VkDescriptorSetLayoutBinding>BindingsArray;

};

class NEUVKDescriptorSetLayoutsInfo {
public:
	struct BindInfo {
		int32_t set;
		int32_t binding;
	};
	std::unordered_map<std::string, BindInfo>   paramsMap;
	std::vector<NEUVKDescriptorSetLayoutInfo>     setLayouts;


	NEUVKDescriptorSetLayoutsInfo() {};
	~NEUVKDescriptorSetLayoutsInfo() {};

	VkDescriptorType GetDescriptorType(int32_t _set, int32_t _binding) {
		for (int16_t i = 0; i < setLayouts.size(); i++) {
			if (setLayouts[i].set == _set) {
				for (int16_t j = 0; j < setLayouts[i].BindingsArray.size(); j++) {
					if (setLayouts[i].BindingsArray[j].binding == _binding) {
						return setLayouts[i].BindingsArray[j].descriptorType;
					}
				}
			}
		}

		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	};

	void AddDescriptorSetLayoutBinding(const std::string &_varName, int32_t _set, VkDescriptorSetLayoutBinding _binding) {
		NEUVKDescriptorSetLayoutInfo *setLayout = nullptr;

		for (int32_t i = 0; i < setLayouts.size(); i++) {
			if (setLayouts[i].set == _set) {
				setLayout = &(setLayouts[i]);
				break;
			}

		}
		if (setLayout == nullptr) {
			setLayouts.push_back({});
			setLayout = &(setLayouts[setLayouts.size() - 1]);
		}

		for (int32_t i = 0; i < setLayout->BindingsArray.size(); i++) {
			VkDescriptorSetLayoutBinding &setBinding = setLayout->BindingsArray[i];
			if (setBinding.binding == _binding.binding&&setBinding.descriptorType == _binding.descriptorType) {
				setBinding.stageFlags = setBinding.stageFlags | _binding.stageFlags;
				return;
			}
		}

		setLayout->set = _set;
		setLayout->BindingsArray.emplace_back(_binding);

		BindInfo paramInfo = {};
		paramInfo.set = _set;
		paramInfo.binding = _binding.binding;
		paramsMap.insert(std::make_pair(_varName, paramInfo));
	};
};

class NEUVKDescriptorSet {
public:
	NEUVKDescriptorSet() {};
	~NEUVKDescriptorSet() {};


};