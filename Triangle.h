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
		void RenderCPU(glm::mat4& modelViewMatrix, glm::mat4& projectionMatrix, float(&cBuffer)[rows][cols][colors], float(&zBuffer)[rows][cols], int h, int w, bool isTextured, int textureMode, std::vector<float*> texture, int tw, int th)
		{
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
			
			// Find bounding box
			glm::vec2 max = { screenCoords[0].x, screenCoords[0].y };
			glm::vec2 min = { screenCoords[0].x, screenCoords[0].y };
			for (int i = 0; i < 3; i++) {
				if (screenCoords[i].x > max.x) { max.x = screenCoords[i].x; }
				if (screenCoords[i].y > max.y) { max.y = screenCoords[i].y; }
				if (screenCoords[i].x < min.x) { min.x = screenCoords[i].x; }
				if (screenCoords[i].y < min.y) { min.y = screenCoords[i].y; }
			}
			if (max.x > w) { max.x = w; }
			if (min.x < 0) { min.x = 0; }
			if (max.y > h) { max.y = h; }
			if (min.y < 0) { min.y = 0; }

			// Rasterize and color
			for (int y = min.y; y <= max.y; y++) {
				for (int x = min.x; x <= max.x; x++) {
					float alpha = (-(x - screenCoords[1].x) * (screenCoords[2].y - screenCoords[1].y) + (y - screenCoords[1].y) * (screenCoords[2].x - screenCoords[1].x)) / (-(screenCoords[0].x - screenCoords[1].x) * (screenCoords[2].y - screenCoords[1].y) + (screenCoords[0].y - screenCoords[1].y) * (screenCoords[2].x - screenCoords[1].x));
					float beta = (-(x - screenCoords[2].x) * (screenCoords[0].y - screenCoords[2].y) + (y - screenCoords[2].y) * (screenCoords[0].x - screenCoords[2].x)) / (-(screenCoords[1].x - screenCoords[2].x) * (screenCoords[0].y - screenCoords[2].y) + (screenCoords[1].y - screenCoords[2].y) * (screenCoords[0].x - screenCoords[2].x));
					float gamma = 1 - alpha - beta;

					// Check inside triangle
					if ((0 <= alpha && alpha <= 1) && (0 <= beta && beta <= 1) && (alpha + beta <= 1)) {
						glm::vec3 point = alpha * screenCoords[0] + beta * screenCoords[1] + gamma * screenCoords[2];
						glm::vec3 pointColor = alpha * c[0] + beta * c[1] + gamma * c[2];
						glm::vec2 textureCoords = alpha * t[0] + beta * t[1] + gamma * t[2];

						textureCoords.x = tw * map(textureCoords.x);
						textureCoords.y = th * map(textureCoords.y);
						
						// Check depth buffer
						if ((point.z < zBuffer[y][x]) && (0 <= x && x < w) && (0 <= y && y < h)) {
							if (!isTextured) {
								cBuffer[y][x][0] = pointColor.x;
								cBuffer[y][x][1] = pointColor.y;
								cBuffer[y][x][2] = pointColor.z;
							}
							else if (textureMode == 0) {
								glm::vec3 texColors = getTexColor(textureCoords.x, textureCoords.y, tw, texture);
								cBuffer[y][x][0] = texColors.x;
								cBuffer[y][x][1] = texColors.y;
								cBuffer[y][x][2] = texColors.z;
							}
							else if (textureMode == 1) {
								glm::vec3 u00 = getTexColor(floor(textureCoords.x), floor(textureCoords.y), tw, texture);
								glm::vec3 u01 = getTexColor(floor(textureCoords.x), ceil(textureCoords.y), tw, texture);
								glm::vec3 u10 = getTexColor(ceil(textureCoords.x), floor(textureCoords.y), tw, texture);
								glm::vec3 u11 = getTexColor(ceil(textureCoords.x), ceil(textureCoords.y), tw, texture);

								glm::vec3 u0 = lerp(textureCoords.x - floor(textureCoords.x), u00, u10);
								glm::vec3 u1 = lerp(textureCoords.x - floor(textureCoords.x), u01, u11);
								glm::vec3 u = lerp(textureCoords.y - floor(textureCoords.y), u0, u1);

								cBuffer[y][x][0] = u.x;
								cBuffer[y][x][1] = u.y;
								cBuffer[y][x][2] = u.z;
							}
							else if (textureMode == 2) {

							}
							
							zBuffer[y][x] = point.z;
						}
					}
				}
			}
		}

		// Getters and setters
		glm::vec3* getVertPos(int i) { return &v[i]; }
		glm::vec3* getVertColors(int i) { return &c[i]; }
		void setVertColor(glm::vec3* vc, int i) { c[i] = *vc; }

		// Map texture coordinates to [0, 1]
		float map(float coord) {
			while (coord < 0) { coord++; }
			while (coord > 1) { coord--; }
			return coord;
		}

		glm::vec3 getTexColor(float x, float y, float tw, std::vector<float*> texture) {
			glm::vec3 ret;
			int ind = 3 * (floor(x) + floor(y) * tw);
			ret.x = texture[0][ind];
			ret.y = texture[0][ind+1];
			ret.z = texture[0][ind+2];
			return ret;
		}

		// Linear interpolation
		glm::vec3 lerp(float x, glm::vec3 v0, glm::vec3 v1) {return v0 + x * (v1 - v0);}
};
