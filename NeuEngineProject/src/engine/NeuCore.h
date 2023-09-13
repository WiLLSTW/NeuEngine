#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "NeuComponent/IO/io.h"
#include "NeuDataStructure.h"
#include <set>
#include <algorithm>
#include "NeuTools.h"
#include "thirdParty/Imgui/SystemGUI.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "NeuVulkan/VulkanCore.h"


namespace Neu{

	class NECore {

	private:

		/*FPS上限 如果為0=無極限*/
		uint32_t FPS_LIMIT = 0;
		void drawFrame();

		void update(double dt) {
			NeuGui::updateGuiEventStart();
			ImGui::Begin("System");
			ImGui::Text("FrameRate:%.2f", (1.0 / dt));
			VkPhysicalDeviceMemoryProperties vpdmp;
			vkGetPhysicalDeviceMemoryProperties(NEVulkanCore::GetInstance()->nowPhysicalDevice(), &vpdmp);
			ImGui::Text("Triangle Count:%d", getTriangleCnt());
			ImGui::End();
			resetTriangleCnt();


			NEVulkanCore::GetInstance()->update();
			NeuGui::updateGuiEventEnd();



		}
		void draw(double dt) {
			NeuGlfwIO::GetInstance()->loop();
			auto index = NEVulkanCore::GetInstance()->startFrame();

			NEVulkanCore::GetInstance()->endFrame(index);

		}

		/*GameLoop用*/
		void vulkanInit() {
			NeuGlfwIO::GetInstance()->init();
			NEVulkanCore::GetInstance()->init();
			NeuGui::initGui();

		}
		void loop() {
			while (!glfwWindowShouldClose(GetGlfwWindow())) {
				static double oldTime = 0;
				static double nowTime = glfwGetTime();
			
				nowTime = glfwGetTime();
				auto dt = nowTime - oldTime;

				if (FPS_LIMIT>0&&dt < 1.0 / FPS_LIMIT) {
					continue;
				}


				update(dt);
				draw(dt);
				
				oldTime = nowTime;
			};
		}
		void release() {
			NeuGlfwIO::GetInstance()->release();


			NEVulkanCore::GetInstance()->release();//放最下面
		};



		
	protected:
		static NECore *sInstance;
		NECore() {};

	public:
	
		~NECore() {

		};

		void run() {
			vulkanInit();
			loop();
			release();

		};
		void setFpsLimit(uint32_t fps) {
			FPS_LIMIT = fps;
		}
		uint32_t getFpsLimit() {
			return FPS_LIMIT;
		}

		static NECore *getInstance() {
			if (sInstance == nullptr) {
				sInstance = new NECore();
			}
			return sInstance;
		}


	};
}

