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
	void RenderOpenGL(glm::mat4& modelViewMatrix, glm::mat4& projectionMatrix, bool textureMode);

	// Rendering the triangle using CPU
	template <int rows, int cols, int colors>
	void RenderCPU(glm::mat4& modelViewMatrix, glm::mat4& projectionMatrix, float(&cBuffer)[rows][cols][colors], float(&zBuffer)[rows][cols], int h, int w, bool isTextured, int textureMode, std::vector<float*> texture, int tw, int th)
	{
		// Convert verticies to NDC then to screen space
		glm::vec4 hCoords[3];
		glm::vec4 ndc[3];
		glm::vec3 zInv;	// Perspective correct interpolation
		glm::vec2 Qsca[3];
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
			zInv[i] = 1 / ndc[i].z;
			Qsca[i] = t[i] * zInv[i];
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
				glm::vec3 abg = barycentric(x, y, screenCoords);
				float alpha = abg.x;
				float beta = abg.y;
				float gamma = abg.z;

				// Check inside triangle
				if ((0 <= alpha && alpha <= 1) && (0 <= beta && beta <= 1) && (alpha + beta <= 1)) {
					glm::vec3 point = alpha * screenCoords[0] + beta * screenCoords[1] + gamma * screenCoords[2];
					glm::vec3 pointColor = alpha * c[0] + beta * c[1] + gamma * c[2];
					glm::vec2 textureCoords = alpha * t[0] + beta * t[1] + gamma * t[2];

					textureCoords.x = tw * map(textureCoords.x);
					textureCoords.y = th * map(textureCoords.y);

					// Check depth buffer
					if ((point.z < zBuffer[y][x]) && (0 <= x && x < w) && (0 <= y && y < h)) {
						glm::vec3 buff;
						
						// Not textured
						if (!isTextured) {
							buff = pointColor;
						}
						// Nearest neighbor
						else if (textureMode == 0) {
							buff = getTexColor(textureCoords.x, textureCoords.y, tw, texture, 0);
						}
						// Bilinear Interpolation
						else if (textureMode == 1) {
							buff = bilinear(textureCoords, tw, texture, 0);
						}
						// Mipmapping
						else if (textureMode == 2) {
							glm::vec3 abgRight = barycentric(x + 1, y, screenCoords);
							glm::vec2 tx = abgRight.x * t[0] + abgRight.y * t[1] + abgRight.z * t[2];
							glm::vec2 dx = tx - textureCoords;	// du, dv
							glm::vec3 abgDown = barycentric(x, y + 1, screenCoords);
							glm::vec2 ty = abgDown.x * t[0] + abgDown.y * t[1] + abgDown.z * t[2];
							glm::vec2 dy = ty - textureCoords;

							float L = findMax(sqrt(dx.x * dx.x + dx.y * dx.y), sqrt(dy.x * dy.x + dy.y * dy.y));
							float D = log2(L);
							//std::cout << "D: " << D << std::endl;
							glm::vec3 c1 = bilinear(textureCoords, tw, texture, floor(D)-1);
							glm::vec3 c2 = bilinear(textureCoords, tw, texture, ceil(D)-1);
							buff = lerp(D - floor(D), c1, c2);
						}

						cBuffer[y][x][0] = buff.x;
						cBuffer[y][x][1] = buff.y;
						cBuffer[y][x][2] = buff.z;
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

	// Get texture color given screen coordinates
	glm::vec3 getTexColor(float x, float y, float tw, std::vector<float*> texture, int i) {
		glm::vec3 ret;
		int ind = 3 * (floor(x) + floor(y) * tw);
		ret.x = texture[i][ind];
		ret.y = texture[i][ind + 1];
		ret.z = texture[i][ind + 2];
		return ret;
	}

	// Linear interpolation
	glm::vec3 lerp(float x, glm::vec3 v0, glm::vec3 v1) { return v0 + x * (v1 - v0); }

	// Bilinear interpolation
	glm::vec3 bilinear(glm::vec2 texCoords, int tw, std::vector<float*> texture, int i) {
		glm::vec3 u00 = getTexColor(floor(texCoords.x), floor(texCoords.y), tw, texture, i);
		glm::vec3 u01 = getTexColor(floor(texCoords.x), ceil(texCoords.y), tw, texture, i);
		glm::vec3 u10 = getTexColor(ceil(texCoords.x), floor(texCoords.y), tw, texture, i);
		glm::vec3 u11 = getTexColor(ceil(texCoords.x), ceil(texCoords.y), tw, texture, i);

		glm::vec3 u0 = lerp(texCoords.x - floor(texCoords.x), u00, u10);
		glm::vec3 u1 = lerp(texCoords.x - floor(texCoords.x), u01, u11);
		glm::vec3 u = lerp(texCoords.y - floor(texCoords.y), u0, u1);
		return u;
	}

	// Calculate alpha, beta, gamma given coordinates
	glm::vec3 barycentric(int x, int y, glm::vec4 coords[3]) {
		float alpha = (-(x - coords[1].x) * (coords[2].y - coords[1].y) + (y - coords[1].y) * (coords[2].x - coords[1].x)) / (-(coords[0].x - coords[1].x) * (coords[2].y - coords[1].y) + (coords[0].y - coords[1].y) * (coords[2].x - coords[1].x));
		float beta = (-(x - coords[2].x) * (coords[0].y - coords[2].y) + (y - coords[2].y) * (coords[0].x - coords[2].x)) / (-(coords[1].x - coords[2].x) * (coords[0].y - coords[2].y) + (coords[1].y - coords[2].y) * (coords[0].x - coords[2].x));
		float gamma = 1 - alpha - beta;
		return glm::vec3{ alpha, beta, gamma };
	}

	// Find maximum
	float findMax(float a, float b) {
		if (a > b) { return a; }
		return b;
	}
};