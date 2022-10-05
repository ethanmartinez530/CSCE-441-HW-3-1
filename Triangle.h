#pragma once

#include <stdlib.h>
#include <math.h>
#include <vector>

#include <glm/glm.hpp>


class Triangle {
	private:
		glm::vec3 v[3];		// Triangle vertices
		glm::vec3 c[3];		// Vertex color
		glm::vec2 t[3];		// Texture coordinates

	public:

		// Default constructor
		Triangle();

		// Constructor without texture coordinates
		Triangle(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2);

		// Constructor with texture coordinates
		Triangle(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec2& t0, glm::vec2& t1, glm::vec2& t2);

		// Rendering the triangle using OpenGL
		void RenderOpenGL(glm::mat4 &modelViewMatrix, glm::mat4 &projectionMatrix, bool textureMode);

		// Rendering the triangle using CPU
		void RenderCPU();

		// Getters and setters
		glm::vec3* getVertColors() { return &c[0]; }
		//void setColorArray(glm::vec3* nc[3]) { c = nc; }
		void setVertColor(glm::vec3* vc, int i) { c[i] = *vc; }
};
