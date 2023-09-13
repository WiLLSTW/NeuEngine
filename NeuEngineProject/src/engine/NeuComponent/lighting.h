#pragma once
#include <glm/glm.hpp>
#include "../NeuDataStructure.h"
#include "../Config.h"
namespace Neu {

	class Light {
	public:
		float color[3] = { 1.0f,1.0f,1.0f };
		float pos[3] = { 0.0f,0.0f,0.0f };
		float dir[3] = { 1.0f,0.0f,0.0f };
		float britness = 1;
		float ambient = 0.0;
		float diffuse = 0.6;
		float specular = 0.3;


		virtual void init() {
		};
		virtual void GuiUpdate(int id) {};
		virtual void update() {};

		void setPos(glm::vec3 _pos) {
			pos[0] = _pos.x;
			pos[1] = _pos.y;
			pos[2] = _pos.z;
		}
		void setPos(float _x, float _y, float _z) {
			pos[0] = _x;
			pos[1] = _y;
			pos[2] = _z;
		}
		void setDirection(glm::vec3 _dir) {
			dir[0] = _dir.x;
			dir[1] = _dir.y;
			dir[2] = _dir.z;
		}
		void setDirection(float _x, float _y, float _z) {
			dir[0] = _x;
			dir[1] = _y;
			dir[2] = _z;
		}
		void setColor(glm::vec3 _color) {
			color[0] = _color.x;
			color[1] = _color.y;
			color[2] = _color.z;
		}
		void setColor(float _r, float _g, float _b) {
			color[0] = _r;
			color[1] = _g;
			color[2] = _b;
		}


	};


	class DirLight :public Light{

	public:
		DirLightStruct data;
		void init() {
			data.ambient = glm::vec3(0.1, 0.1, 0.1);
			data.diffuse = glm::vec3(1.0, 1.0, 1.0);
			data.specular = glm::vec3(0.4f, 0.4f, 0.4f);
			data.direction = glm::vec3(-1.f, 0, 0.f);
		}


		void GuiUpdate(int id) {
			char dirId[20];
			char ambId[20];
			char diffId[20];
			char specId[20];
			char colorId[20];
			char brightId[20];
			sprintf(dirId, "Directional##Dir%d", id);
			sprintf(ambId, "Ambient##Dir%d", id);
			sprintf(diffId, "Diffuse##Dir%d", id);
			sprintf(specId, "Specular##Dir%d", id);
			sprintf(colorId, "Color##Dir%d", id);
			sprintf(brightId, "Brightness##Dir%d", id);

			ImGui::ColorEdit3(colorId, color);
			ImGui::InputFloat3(dirId, dir);
			ImGui::DragFloat(brightId, &britness, 0.01f, 0.0f,10);
			ImGui::SliderFloat(ambId, &ambient, 0.0f, 1.0f);
			ImGui::SliderFloat(diffId, &diffuse,0.0f,1.0f);
			ImGui::SliderFloat(specId, &specular,0.0f,1.0f);


		}

		void update() {
			data.ambient.x = color[0] * ambient*britness;		data.ambient.y = color[1] * ambient*britness;		data.ambient.z = color[2] * ambient*britness;
			data.diffuse.x = color[0] * diffuse*britness;		data.diffuse.y = color[1] * diffuse*britness;		data.diffuse.z = color[2] * diffuse*britness;
			data.specular.x = color[0] * specular*britness;		data.specular.y = color[1] * specular*britness;		data.specular.z = color[2] * specular*britness;
			
			data.direction = glm::vec3(dir[0],dir[1],dir[2]);
		}

	};

	class PointLight :public Light {
	public:
		PointLightStruct data;
		void init() {
			data.pos = glm::vec3(0, 0, 0);
			data.ambient = glm::vec3(0.0, 0.0, 0.0);
			data.diffuse = glm::vec3(1.0, 1.0, 1.0);
			data.specular = glm::vec3(0.6, 0.6, 0.6);
			data.linear = 0.045;
			data.constant = 1;
			data.quadratic = 0.0075;
		}

		void GuiUpdate(int id) {

			char posId[20];
			char ambId[20];
			char diffId[20];
			char specId[20];
			char d0Id[20];
			char d1Id[20];
			char d2Id[20];
			char colorId[20];
			char brightId[20];
			sprintf(posId, "Position##PL%d", id);
			sprintf(ambId, "Ambient##PL%d", id);
			sprintf(diffId, "Diffuse##PL%d", id);
			sprintf(specId, "Specular##PL%d", id);
			sprintf(d0Id, "d0##PL%d", id);
			sprintf(d1Id, "d1##PL%d", id);
			sprintf(d2Id, "d2##PL%d", id);
			sprintf(colorId, "Color##PL%d", id);
			sprintf(brightId, "Brightness##PL%d", id);


			ImGui::ColorEdit3(colorId, color);
			ImGui::InputFloat3(posId, pos);
			ImGui::DragFloat(brightId, &britness, 0.01f, 0.0f,10);
			ImGui::SliderFloat(ambId, &ambient, 0.0f, 1.0f);
			ImGui::SliderFloat(diffId, &diffuse, 0.0f, 1.0f);
			ImGui::SliderFloat(specId, &specular, 0.0f, 1.0f);
			ImGui::InputFloat(d0Id, &data.constant);
			ImGui::InputFloat(d1Id, &data.linear);
			ImGui::InputFloat(d2Id, &data.quadratic);
		};


		void update() {
			data.ambient.x = color[0] * ambient*britness;		data.ambient.y = color[1] * ambient*britness;		data.ambient.z = color[2] * ambient*britness;
			data.diffuse.x = color[0] * diffuse*britness;		data.diffuse.y = color[1] * diffuse*britness;		data.diffuse.z = color[2] * diffuse*britness;
			data.specular.x = color[0] * specular*britness;		data.specular.y = color[1] * specular*britness;		data.specular.z = color[2] * specular*britness;
			data.pos = glm::vec3(pos[0], pos[1], pos[2]);
		}

		void draw() {

		}
	};

	class SpotLight :public Light{
	public:
		SpotLightStruct data;

		void init() {
			data.pos = glm::vec3(0, 0, 0);
			data.ambient = glm::vec3(0.0, 0.0, 0.0);
			data.diffuse = glm::vec3(1.0, 1.0, 1.0);
			data.specular = glm::vec3(0.6, 0.6, 0.6);
			data.linear = 0.045;
			data.constant = 1;
			data.quadratic = 0.0075;
			data.cutOff = 1.0;
			data.outerCutOff = 1.4;
		}

		void GuiUpdate(int id) {
			char posId[20];
			char dirId[20];
			char cutOffId[20];
			char outerCutId[20];
			char d1Id[20];
			char d0Id[20];
			char d2Id[20];
			char ambId[20];
			char diffId[20];
			char specId[20];
			char brightId[20];
			char colorId[20];

			sprintf(posId, "Position##SL%d", id);
			sprintf(dirId, "Direction##SL%d", id);
			sprintf(ambId, "Ambient##SL%d", id);
			sprintf(diffId, "Diffuse##SL%d", id);
			sprintf(specId, "Specular##SL%d", id);
			sprintf(d0Id, "d0##SL%d", id);
			sprintf(d1Id, "d1##SL%d", id);
			sprintf(d2Id, "d2##SL%d", id);
			sprintf(cutOffId, "CutOff##SL%d", id);
			sprintf(outerCutId, "OuterCut##SL%d", id);
			sprintf(colorId, "Color##SL%d", id);
			sprintf(brightId, "Brightness##SL%d", id);

			ImGui::ColorEdit3(colorId, color);
			ImGui::InputFloat3(posId, pos);
			ImGui::InputFloat3(dirId, dir);
			ImGui::DragFloat(brightId, &britness, 0.01f, 0.0f, 10);
			ImGui::SliderFloat(ambId, &ambient, 0.0f, 1.0f);
			ImGui::SliderFloat(diffId, &diffuse, 0.0f, 1.0f);
			ImGui::SliderFloat(specId, &specular, 0.0f, 1.0f);
			ImGui::InputFloat(d0Id, &data.constant);
			ImGui::InputFloat(d1Id, &data.linear);
			ImGui::InputFloat(d2Id, &data.quadratic);
			ImGui::DragFloat(cutOffId, &data.cutOff,0.01f);
			ImGui::DragFloat(outerCutId, &data.outerCutOff, 0.01f,0.01f);




		}

		void update() {
			data.ambient.x = color[0] * ambient*britness;		data.ambient.y = color[1] * ambient*britness;		data.ambient.z = color[2] * ambient*britness;
			data.diffuse.x = color[0] * diffuse*britness;		data.diffuse.y = color[1] * diffuse*britness;		data.diffuse.z = color[2] * diffuse*britness;
			data.specular.x = color[0] * specular*britness;		data.specular.y = color[1] * specular*britness;		data.specular.z = color[2] * specular*britness;

			data.pos = glm::vec3(pos[0], pos[1], pos[2]);
			data.direction = glm::vec3(dir[0], dir[1], dir[2]);

		}



		void draw() {

		}
	};



	class LightSystem {
		LightSystem() {};


		DirLight dirLight;
		PointLight pointLight[NEUVK_CONFIGS_SPOTLIGHTCOUNT];
		SpotLight spotLight;


	public:
		LightSystem(const LightSystem&) = delete;
		LightSystem& operator=(const LightSystem&) = delete;
		LightSystem(LightSystem&&) = delete;
		LightSystem& operator=(LightSystem&&) = delete;
		static LightSystem* GetInstance() {
			static LightSystem instance;
			return &instance;
		}

		void init() {
			dirLight.init();
			for (int i = 0; i < NEUVK_CONFIGS_SPOTLIGHTCOUNT; i++) {
				pointLight[i].init();
			}
			spotLight.init();


			pointLight[0].setColor(0, 0, 1);
			pointLight[1].setColor(0, 1, 0);
			pointLight[2].setColor(1, 0, 0);
		}

		void Update() {

			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			float length = 35;
			float speed = 50;
			pointLight[0].setPos(glm::sin(glm::radians(time * speed)		)*length, glm::cos(glm::radians(time * speed))*length, 0);
			pointLight[1].setPos(glm::sin(glm::radians(time * speed +90) )*length, glm::cos(glm::radians(time * speed +90))*length, 0);
			pointLight[2].setPos(glm::sin(glm::radians(time * speed +180))*length, glm::cos(glm::radians(time * speed +180))*length, 0);
			pointLight[3].setPos(glm::sin(glm::radians(time * speed +270))*length, glm::cos(glm::radians(time * speed +270))*length, 0);

			GuiUpdate();

			dirLight.update();
			for (int i = 0; i < NEUVK_CONFIGS_SPOTLIGHTCOUNT; i++) {
				pointLight[i].update();
			}
			spotLight.update();
		}

		void GuiUpdate() {
			ImGui::Begin("Lighting System");
			if (ImGui::CollapsingHeader("DirLight")) {
				dirLight.GuiUpdate(0);
			}
			if (NEUVK_CONFIGS_SPOTLIGHTCOUNT <= 10)
			{
				for (int i = 0; i < NEUVK_CONFIGS_SPOTLIGHTCOUNT; i++) {
					char collapseHeader[13];
					sprintf(collapseHeader, "PointLight%d", i + 1);
					if (ImGui::CollapsingHeader(collapseHeader)) {
						pointLight[i].GuiUpdate(i);

					}
				}
			}

			if (ImGui::CollapsingHeader("SpotLight")) {
				spotLight.GuiUpdate(0);
			}
			ImGui::End();
		}

		void  createUboData(UboLight &light) {
			light.dirLight = dirLight.data;
			for (int i = 0; i < NEUVK_CONFIGS_SPOTLIGHTCOUNT; i++) {
				light.pointLight[i] = pointLight[i].data;
			}
			light.spotLight = spotLight.data;


			return;
		}
	};


}
