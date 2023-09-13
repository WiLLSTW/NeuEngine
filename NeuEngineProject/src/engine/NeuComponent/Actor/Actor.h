#pragma once
#include <glm/glm.hpp>
#include "../mesh.h"
namespace Neu {

	class Actor {
	public:
		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 rotation;
		Mesh mesh;

		void init();
		void update();
		void draw() {

		}
		void release();



	};

}