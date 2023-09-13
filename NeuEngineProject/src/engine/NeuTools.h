#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include "NeuComponent/IO/io.h"
#include "NeuDataStructure.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
//#include "NeuCore.h"
#define VKCREATEPROCESS NeuVkCreateProcess

//Helper Function集散地

namespace Neu {
	//檢測是否為VK_SUCCESS時 在throw error一串太佔空間了所以獨立出來
	void NeuVkCreateProcess(VkResult result, const char *error_message);

	GLFWwindow *GetGlfwWindow();

	void MakeModelMatrix(glm::mat4 &model, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate);
	void MakeViewProjMatrix(UboViewProj &viewProj, glm::vec3 eye, glm::vec3 center, glm::vec3 up, float fovy, float aspectRatio, float near, float far);


	const std::string shaderSrc(const std::string src);

	//讀取檔案
	static std::vector<char>readFile(const std::string &filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();//找到寫入指針位置確定文件大小
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	//讀取spirv
	static std::vector<uint32_t>readSpirVFile(const std::string &filename) {
		std::ifstream file(filename, std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file!");

		std::vector<char> spirv((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

		file.close();

		// Copy data from the char-vector to a new uint32_t-vector
		std::vector<uint32_t> spv(spirv.size() / sizeof(uint32_t));
		memcpy(spv.data(), spirv.data(), spirv.size());

		return spv;
	}


	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	static uint32_t triangleCount = 0;
	void resetTriangleCnt();
	void addTriangleCnt(uint32_t triangle);
	uint32_t getTriangleCnt();

}