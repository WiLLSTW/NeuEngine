#pragma once
#include<GLFW/glfw3.h>

namespace Neu {

	class NeuGlfwIO{
	public:
		NeuGlfwIO(const NeuGlfwIO&) = delete;
		NeuGlfwIO& operator=(const NeuGlfwIO&) = delete;
		NeuGlfwIO(NeuGlfwIO&&) = delete;
		NeuGlfwIO& operator=(NeuGlfwIO&&) = delete;
		static NeuGlfwIO* GetInstance() {
			static NeuGlfwIO instance;
			return &instance;
		}


		void init() {
			initGlfw();
		};
		void loop() {
			glfwPollEvents();
		}
		void release() {
			glfwTerminate();
			glfwDestroyWindow(window);
		};


		/*outputóAèo*/
		/*GLFWéã‚x*/
		GLFWwindow *getWindow(){
			return window;
		};

		std::vector<const char*> getRequiredExtensions(bool enableValidationLayer) {
			uint32_t glfwExtensionCount = 0;
			const char ** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*>extensions(glfwExtensions, glfwExtensionCount + glfwExtensions);

			
			if (enableValidationLayer) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}


			return extensions;
		}


		/*inputóAì¸*/
		int IsKeyPressed(int keycode);
		//int IsKeyReleased(KeyCode keycode);
		//int IsMouseButtonPressed(MouseButton mouse_button);
		//int IsMouseButtonReleased(MouseButton mouse_button);
		//V2f GetMousePosition();
		//float GetScrollYOffset();
	private:
		NeuGlfwIO() {};
		/*éã‚xã…å¿*/
		uint32_t WINDOW_WIDTH = 1920;
		uint32_t WINDOW_HEIGHT = 1080;


		void initGlfw() {
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//For Vulkan Window
			window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "NeuEngine", nullptr, nullptr);
			glfwSetWindowUserPointer(window, this);
			//glfwSetFrameBufferSize
		}
		GLFWwindow *window;
	};

}