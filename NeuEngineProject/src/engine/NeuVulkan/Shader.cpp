#include "Shader.h"
using namespace Neu;

NEUVKShaderModule* NEUVKShaderModule::CreateShaderModule(const char* _filename, VkShaderStageFlagBits _type) {

	std::vector<char> dataPtr;
	uint32_t dataSize = 0;
	dataPtr = readFile(shaderSrc(_filename));


	VkShaderModuleCreateInfo moduleCreateInfo{};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = dataPtr.size();
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(dataPtr.data());

	VkShaderModule l_module;
	VKCREATEPROCESS(vkCreateShaderModule(getVkDevice(), &moduleCreateInfo, nullptr, &l_module), "Failed to create shader module");

	NEUVKShaderModule *shaderModule = new NEUVKShaderModule();
	shaderModule->binaryData = dataPtr;
	shaderModule->module = l_module;
	shaderModule->stage = _type;

	return shaderModule;
}

NEUVKShader *NEUVKShader::CreateShader(bool _dynamicUBO,const char *_vert, const char*_frag, const char *_geom) {
	NEUVKShaderModule *vertModule = _vert ? NEUVKShaderModule::CreateShaderModule(_vert, VK_SHADER_STAGE_VERTEX_BIT) : nullptr;
	NEUVKShaderModule *fragModule = _frag ? NEUVKShaderModule::CreateShaderModule(_frag, VK_SHADER_STAGE_FRAGMENT_BIT) : nullptr;
	NEUVKShaderModule *geomModule = _geom ? NEUVKShaderModule::CreateShaderModule(_geom, VK_SHADER_STAGE_GEOMETRY_BIT) : nullptr;

	NEUVKShader *shader = new NEUVKShader();
	shader->dynamicUBO = _dynamicUBO;
	shader->vertModule = vertModule;
	shader->fragModule = fragModule;
	shader->geoModule = geomModule;
	
	shader->CompileShader();
	return shader;
}

void NEUVKShader::CompileShader() {
	ProcessData(vertModule);
	ProcessData(fragModule);
	ProcessData(geoModule);
	CreateInputInfo();
	CreateLayout();
}

void NEUVKShader::ProcessData(NEUVKShaderModule* _module) {
	if (!_module) {
		return;
	}
	VkPipelineShaderStageCreateInfo shaderCreateInfo{};
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderCreateInfo.stage = _module->stage;
	shaderCreateInfo.module = _module->module;
	shaderCreateInfo.pName = "main";
	shaderCreateInfos.emplace_back(shaderCreateInfo);

	spirv_cross::Compiler compiler(reinterpret_cast<uint32_t *>(_module->binaryData.data()), _module->binaryData.size() / 4);
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	ProcessUniformBuffer(compiler, resources, _module->stage);
	ProcessTextures(compiler, resources, _module->stage);
	ProcessInput(compiler, resources, _module->stage);
};


void NEUVKShader::ProcessUniformBuffer(spirv_cross::Compiler &_compiler, spirv_cross::ShaderResources &_resources, VkShaderStageFlags _stageFlag) {
	for (uint16_t i = 0; i < _resources.uniform_buffers.size(); i++) {
		//抓取Uniform Buffer的資訊
		spirv_cross::Resource &res = _resources.uniform_buffers[i];
		spirv_cross::SPIRType type = _compiler.get_type(res.type_id);
		spirv_cross::SPIRType base_type = _compiler.get_type(res.base_type_id);
		const std::string &varName = _compiler.get_name(res.id);
		const std::string &typeName = _compiler.get_name(res.base_type_id);
		uint32_t uboStructSize = _compiler.get_declared_struct_size(type);

		//找尋binding跟set的位置
		int32_t set = _compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		int32_t binding = _compiler.get_decoration(res.id, spv::DecorationBinding);

		VkDescriptorSetLayoutBinding setLayoutBinding = {};
		setLayoutBinding.stageFlags = _stageFlag;
		setLayoutBinding.descriptorCount = 1;
		//檢測是否為Dynamic
		setLayoutBinding.descriptorType = (typeName.find("Dynamic") != std::string::npos/*檢查是否有Dynamic*/ || dynamicUBO) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setLayoutBinding.binding = binding;
		setLayoutBinding.pImmutableSamplers = nullptr;

		setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);


		/*因為有可能同一個UBO綁定複數*/
		auto it = UBOInfo.find(varName);
		if (it == UBOInfo.end()) {
			/*假如沒找到相關Ubo資訊的話就新增一個的話*/
			UniformBufferInfo bufferInfo = {};
			bufferInfo.set = set;
			bufferInfo.binding = binding;
			bufferInfo.bufferSize = uboStructSize;
			bufferInfo.stageFlags = _stageFlag;
			bufferInfo.descriptorType = setLayoutBinding.descriptorType;
			UBOInfo.emplace(varName, bufferInfo);
		}
		else {
			//如有類似Ubo資訊的話就追加值
			it->second.stageFlags |= _stageFlag;
		}


	}
};

void NEUVKShader::ProcessTextures(spirv_cross::Compiler &_compiler, spirv_cross::ShaderResources &_resources, VkShaderStageFlags _stageFlag) {

	for (uint16_t i = 0; i < _resources.sampled_images.size(); i++) {
		spirv_cross::Resource &res = _resources.sampled_images[i];
		spirv_cross::SPIRType type = _compiler.get_type(res.type_id);
		spirv_cross::SPIRType base_type = _compiler.get_type(res.base_type_id);
		const std::string &varName = _compiler.get_name(res.id);


		//找尋binding跟set的位置
		int32_t set = _compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		int32_t binding = _compiler.get_decoration(res.id, spv::DecorationBinding);

		VkDescriptorSetLayoutBinding setLayoutBinding = {};
		setLayoutBinding.stageFlags = _stageFlag;
		setLayoutBinding.descriptorCount = 1;
		setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setLayoutBinding.binding = binding;
		setLayoutBinding.pImmutableSamplers = nullptr;

		setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

		auto it = ImageInfo.find(varName);
		if (it == ImageInfo.end()) {
			/*假如沒找到相關Ubo資訊的話就新增一個的話*/
			CombinedImageInfo bufferInfo = {};
			bufferInfo.set = set;
			bufferInfo.binding = binding;
			bufferInfo.stageFlags = _stageFlag;
			bufferInfo.descriptorType = setLayoutBinding.descriptorType;
			ImageInfo.emplace(varName, bufferInfo);
		}
		else {
			it->second.stageFlags |= _stageFlag;//假設其他的buffer做過那就追加
		}
	}
};

void NEUVKShader::ProcessPushConstant(spirv_cross::Compiler &_compiler, spirv_cross::ShaderResources &_resources, VkShaderStageFlags _stageFlags){

	for (uint16_t i = 0; i < _resources.push_constant_buffers.size(); i++) {
		spirv_cross::Resource &res = _resources.push_constant_buffers[i];
		spirv_cross::SPIRType type = _compiler.get_type(res.type_id);
		spirv_cross::SPIRType base_type = _compiler.get_type(res.base_type_id);
		const std::string &varName = _compiler.get_name(res.id);
		const std::string &typeName = _compiler.get_name(res.base_type_id);
		uint32_t uboStructSize = _compiler.get_declared_struct_size(type);

	}

}

void NEUVKShader::ProcessInput(spirv_cross::Compiler &_compiler, spirv_cross::ShaderResources &_resources, VkShaderStageFlags _stageFlags) {
	if (_stageFlags != VK_SHADER_STAGE_VERTEX_BIT) {
		return;
	}

	for (uint16_t i = 0; i < _resources.stage_inputs.size(); i++) {
		spirv_cross::Resource &res = _resources.stage_inputs[i];
		spirv_cross::SPIRType type = _compiler.get_type(res.type_id);

		const std::string &varName = _compiler.get_name(res.id);
		int32_t inputAttributeSize = type.vecsize;
		VertexInputType attributeType = StringToVertexType(varName.c_str());

		/*用到不明input名字的話需要去做歸檔*/
		if (attributeType == VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_NONE) {
			if (inputAttributeSize == 1) {
				attributeType = VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT;
			}
			else if (inputAttributeSize == 2) {
				attributeType = VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT2;
			}
			else if (inputAttributeSize == 3) {
				attributeType = VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT3;
			}
			else if (inputAttributeSize == 4) {
				attributeType = VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT4;
			}
			std::cout << "Warning:Cannot Recognized Attribute:" << varName << std::endl;
		}

		int16_t location = _compiler.get_decoration(res.id, spv::DecorationLocation);
		AttributeSet attribute = {};
		attribute.location = location;
		attribute.type = attributeType;
		inputAttributes.emplace_back(attribute);
	}
};

void NEUVKShader::CreateInputInfo() {

	//sorting the attribute
	std::sort(inputAttributes.begin(), inputAttributes.end(), [](const AttributeSet &a, const AttributeSet &b)->bool {
		return a.location < b.location;
	});


	InputBindings.clear();
	InputBindings.shrink_to_fit();


	//InputBinding
	if (inputAttributes.size() > 0) {
		int stride = 0;
		for (int16_t i = 0; i < inputAttributes.size(); i++) {
			stride += VertexTypeToSize(inputAttributes[i].type);
		}
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = stride;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		InputBindings.emplace_back(vertexInputBinding);
	}

	//attribute info
	int16_t location = 0;
	if (inputAttributes.size() > 0) {
		int32_t offset = 0;
		for (int16_t i = 0; i < inputAttributes.size(); i++) {
			VkVertexInputAttributeDescription inputDescription{};
			inputDescription.binding = 0;
			inputDescription.location = location;
			inputDescription.format = VertexTypeToFormat(inputAttributes[i].type);
			inputDescription.offset = offset;
			offset += VertexTypeToSize(inputAttributes[i].type);
			InputAttributeDescriptions.emplace_back(inputDescription);
			location++;
		}
	}
	
}


void NEUVKShader::CreateLayout() {
	std::vector<NEUVKDescriptorSetLayoutInfo>&setLayouts=setLayoutsInfo.setLayouts;

	//先對Set做排序再對Layout做排序

	//set排序
	std::sort(setLayouts.begin(), setLayouts.end(), [](const NEUVKDescriptorSetLayoutInfo &a, const NEUVKDescriptorSetLayoutInfo &b)->bool {
		return a.set < b.set;
	});

	//binding排序
	for (int16_t i = 0; i < setLayouts.size(); i++) {
		std::vector<VkDescriptorSetLayoutBinding>&binding = setLayouts[i].BindingsArray;
		std::sort(binding.begin(), binding.end(), [](const VkDescriptorSetLayoutBinding &a, const VkDescriptorSetLayoutBinding &b)->bool {
			return a.binding < b.binding;
		});
	}

	for (int16_t i = 0; i < setLayoutsInfo.setLayouts.size(); i++) {
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		NEUVKDescriptorSetLayoutInfo &setLayoutInfo = setLayoutsInfo.setLayouts[i];

		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo{};
		descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descSetLayoutInfo.bindingCount = setLayoutInfo.BindingsArray.size();
		descSetLayoutInfo.pBindings = setLayoutInfo.BindingsArray.data();

		VKCREATEPROCESS(vkCreateDescriptorSetLayout(getVkDevice(), &descSetLayoutInfo, nullptr, &descriptorSetLayout),"Failed To Create Descriptor Set Layout");

		descriptorSetLayouts.emplace_back(descriptorSetLayout);
	}

	//Push constants
	std::array<VkPushConstantRange, 1>pushConstantRanges;
	pushConstantRanges[0].offset = 0;
	pushConstantRanges[0].size = sizeof(PcNormal);
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_ALL;

	VkPipelineLayoutCreateInfo pipeLineLayoutCreateInfo{};
	pipeLineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeLineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
	pipeLineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipeLineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();
	pipeLineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
	

	VKCREATEPROCESS(vkCreatePipelineLayout(getVkDevice(), &pipeLineLayoutCreateInfo, nullptr, &pipelineLayout),"Failed to create pipelineLayout");

}

VertexInputType NEUVKShader::StringToVertexType(const char *name) {

	if (strcmp(name, "inPosition") == 0|| strcmp(name,"fragPosition")==0) {
		return VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_COLOR;
	}
	else if (strcmp(name, "inTexCoord")==0||strcmp(name,"fragTexcoord") == 0) {
		return VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UV;
	}
	else if (strcmp(name, "inNormal") == 0|| strcmp(name, "fragNormal") == 0) {
		return VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_NORMAL;

	}
	else if (strcmp(name, "inColor") == 0 || strcmp(name, "fragColor") == 0) {
		return VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_COLOR;
	}

	return VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_NONE;
}


int16_t NEUVKShader::VertexTypeToSize(VertexInputType inputType) {
	switch (inputType) {
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_COLOR:
		return 3 * sizeof(float);
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UV:
		return 2 * sizeof(float);
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_NORMAL:
		return 3 * sizeof(float);
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_POSITION:
		return 3 * sizeof(float);
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT:
		return sizeof(float);
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT2:
		return 2*sizeof(float);
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT3:
		return 3*sizeof(float);
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT4:
		return 4*sizeof(float);
	default:
		return 0;
		
	}

	return 0;
};

VkFormat NEUVKShader::VertexTypeToFormat(VertexInputType inputType) {
	switch (inputType) {
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_COLOR:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UV:
		return VK_FORMAT_R32G32_SFLOAT;
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_NORMAL:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_POSITION:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT2:
		return VK_FORMAT_R32G32_SFLOAT;
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VertexInputType::NEU_VK_VERTEX_INPUT_TYPE_UNRECOGNAIZED_FLOAT4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		return VK_FORMAT_UNDEFINED;

	}

	return VK_FORMAT_UNDEFINED;
}

