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
		template <int rows, int cols, int colors>
		void RenderCPU(glm::mat4& modelViewMatrix, glm::mat4& projectionMatrix, float(&cBuffer)[rows][cols][colors], float(&zBuffer)[rows][cols], int h, int w)
		{
			//std::cout << "Begin RenderCPU\n";
			
			// Convert verticies to NDC then to screen space
			glm::vec4 hCoords[3];
			glm::vec4 ndc[3];
			glm::vec4 screenCoords[3];
			glm::mat4 viewport(0.0f);
			viewport[0][0] = w / 2;
			viewport[1][1] = h / 2;
			viewport[2][2] = 1;
			viewport[3][0] = w / 2;
			viewport[3][1] = h / 2;
			viewport[3][3] = 1;
			for (int i = 0; i < 3; i++) {
				hCoords[i] = { v[i].x, v[i].y, v[i].z, 1 };
				ndc[i] = (projectionMatrix * modelViewMatrix * hCoords[i]);
				ndc[i] /= ndc[i].w;
				screenCoords[i] = viewport * ndc[i];
			}

			// Rasterize
			glm::vec2 max = { screenCoords[0].x, screenCoords[0].y };
			glm::vec2 min = { screenCoords[0].x, screenCoords[0].y };
			for (int i = 0; i < 3; i++) {
				if (screenCoords[i].x > max.x) { max.x = screenCoords[i].x; }
				if (screenCoords[i].y > max.y) { max.y = screenCoords[i].y; }
				if (screenCoords[i].x < min.x) { min.x = screenCoords[i].x; }
				if (screenCoords[i].y < min.y) { min.y = screenCoords[i].y; }
			}
			//std::cout << "Max: " << max.x << ", " << max.y << std::endl;
			//std::cout << "Min: " << min.x << ", " << min.y << std::endl;

			for (int y = min.y; y <= max.y; y++) {
				for (int x = min.x; x <= max.x; x++) {
					//std::cout << x << ", " << y << std::endl;
					float alpha = (-(x - screenCoords[1].x) * (screenCoords[2].y - screenCoords[1].y) + (y - screenCoords[1].y) * (screenCoords[2].x - screenCoords[1].x)) / (-(screenCoords[0].x - screenCoords[1].x) * (screenCoords[2].y - screenCoords[1].y) + (screenCoords[0].y - screenCoords[1].y) * (screenCoords[2].x - screenCoords[1].x));
					float beta = (-(x - screenCoords[2].x) * (screenCoords[0].y - screenCoords[2].y) + (y - screenCoords[2].y) * (screenCoords[0].x - screenCoords[2].x)) / (-(screenCoords[1].x - screenCoords[2].x) * (screenCoords[0].y - screenCoords[2].y) + (screenCoords[1].y - screenCoords[2].y) * (screenCoords[0].x - screenCoords[2].x));
					float gamma = 1 - alpha - beta;

					if ((0 <= alpha <= 1) && (0 <= beta <= 1) && (alpha + beta <= 1)) {
						glm::vec3 pointColor = alpha * c[0] + beta * c[1] + gamma * c[2];
						cBuffer[y][x][0] = pointColor.x;
						cBuffer[y][x][1] = pointColor.y;
						cBuffer[y][x][2] = pointColor.z;
					}
				}
			}
			
			//std::cout << "End RenderCPU\n";
		}

		// Getters and setters
		glm::vec3* getVertPos(int i) { return &v[i]; }
		glm::vec3* getVertColors(int i) { return &c[i]; }
		void setVertColor(glm::vec3* vc, int i) { c[i] = *vc; }
};
