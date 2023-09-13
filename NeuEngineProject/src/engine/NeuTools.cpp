#include "NeuTools.h"

GLFWwindow *Neu::GetGlfwWindow(){
	return NeuGlfwIO::GetInstance()->getWindow();
}


void Neu::NeuVkCreateProcess(VkResult result, const char *error_message) {
	if (result != VK_SUCCESS) {
		throw std::runtime_error(std::string(error_message).c_str());
	}
}


const std::string Neu::shaderSrc(const std::string src) {
	std::stringstream ss;
	ss << "./res/shader/" << src;
	return ss.str();
}


void Neu::MakeModelMatrix(glm::mat4 &model, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate){

	glm::mat4 rotateMatrix = glm::eulerAngleYXZ(glm::radians(rotate.y), glm::radians(rotate.x), glm::radians(rotate.z));
	model = rotateMatrix;
	model = glm::translate(model, translate);
	model = glm::scale(model, scale);

	return;
}

void Neu::MakeViewProjMatrix(Neu::UboViewProj &viewProj, glm::vec3 eye, glm::vec3 center, glm::vec3 up, float fovy, float aspectRatio, float near, float far) {

	viewProj.view = glm::lookAt(eye, center, up);
	viewProj.proj = glm::perspective(fovy, aspectRatio, near, far);
	viewProj.proj[1][1] *= -1;

	return;
}


void Neu::resetTriangleCnt() {
	triangleCount = 0;
};
void Neu::addTriangleCnt(uint32_t triangle) {
	triangleCount += triangle;
}
uint32_t Neu::getTriangleCnt() {
	return triangleCount;
}



//void Neu::