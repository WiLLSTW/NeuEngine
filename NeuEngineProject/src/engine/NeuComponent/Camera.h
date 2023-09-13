#pragma once
#include <glm/glm.hpp>
#include "../NeuTools.h"
namespace Neu {
	class Camera {
		glm::vec3 position=glm::vec3(0,0,0);
		glm::vec3 lookAt=glm::vec3(0,1,0);

		float fovy = glm::radians(45.0f);
		float far = 30.0f;
		float near = 0.1f;
		
		float velocity;
		glm::vec3 positionTo;
		glm::vec3 lookAtTo;

		float aspectRatio=16.0f/9.0f;

	public:
		Camera(){}
		~Camera(){}
		void init(glm::vec3 position, glm::vec3 lookAt) {
			setCameraPosition(position);
			setCameraPosition(lookAt);
		};
		void update();
		void draw();
		void setCameraPosition(float x, float y, float z) {
			position = glm::vec3(x, y, z);
		};
		void setCameraPosition(glm::vec3 pos) {
			position = pos;
		};
		void setNear(float n) {
			near = n;
		}
		void setFar(float f) {
			far = f;
		}
		void setAspect(float width, float height) {
			aspectRatio=width / height;
		}
		void setLookAtPosition(float x, float y, float z) {
			lookAt = glm::vec3(x, y, z);
		};
		void setLookAtPosition(glm::vec3 la) {
			lookAt = la;
		};

		UboViewProj makeUboViewProj() {
			UboViewProj uvp;
			MakeViewProjMatrix(uvp, position,lookAt, glm::vec3(0.0f, 0.0f, 1.0f), fovy, aspectRatio, near, far);
			return uvp;

		};
	};
}
