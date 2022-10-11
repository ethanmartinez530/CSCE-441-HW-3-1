#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Triangle.h"


#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 1024

GLFWwindow *window;
bool lButtonPressed;
bool rButtonPressed;


float color[WINDOW_HEIGHT][WINDOW_WIDTH][3];
float depth[WINDOW_HEIGHT][WINDOW_WIDTH];


std::vector<Triangle> triangleVector;
std::vector<float*> texture;

bool isOpenGL = true;
bool isTextured = false;
float eyeDistance = 5.0f;
int textureMode = 0;
int colorMode = 0;
float angle = 0;

std::string mainName = "Assignment3 - Ethan Martinez";

int texWidth, texHeight;

GLuint texID;

void ClearFrameBuffer()
{
	memset(&color[0][0][0], 0.0f, sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT * 3);
}

void Display()
{	
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f), float(WINDOW_WIDTH) / float(WINDOW_HEIGHT), 0.1f, 100.0f);
	glm::mat4 modelViewMatrix = glm::lookAt(eyeDistance * glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

	if (isOpenGL)
	{
		if (isTextured)
			glBindTexture(GL_TEXTURE_2D, texID);
		
		for (int i = 0; i < triangleVector.size(); i++)
			triangleVector[i].RenderOpenGL(modelViewMatrix, projectionMatrix, isTextured);
		
		if (isTextured)
			glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		for (int i = 0; i < triangleVector.size(); i++)
			triangleVector[i].RenderCPU(modelViewMatrix, projectionMatrix, color, depth, WINDOW_HEIGHT, WINDOW_WIDTH);

		glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_FLOAT, &color[0][0][0]);
		ClearFrameBuffer();
	}

	glFlush();
}

/*
	Color triangles when color mode is changed
	Mode 0: randomly color each triangle
	Mode 1: randomly color each vertex
	Mode 2: color each vertex according to its z value
*/
void colorTriangles() {
	switch (colorMode) {
	case 0:
		for (int i = 0; i < triangleVector.size(); i++) {
			glm::vec3 vc;
			vc.x = float(rand()) / float(RAND_MAX);
			vc.y = float(rand()) / float(RAND_MAX);
			vc.z = float(rand()) / float(RAND_MAX);
			for (int j = 0; j < 3; j++) { triangleVector[i].setVertColor(&vc, j); }
		}
		break;

	case 1:
		for (int i = 0; i < triangleVector.size(); i++) {
			for (int j = 0; j < 3; j++) {
				glm::vec3 vc;
				vc.x = float(rand()) / float(RAND_MAX);
				vc.y = float(rand()) / float(RAND_MAX);
				vc.z = float(rand()) / float(RAND_MAX);
				triangleVector[i].setVertColor(&vc, j);
			}
		}
		break;

	case 2:
		float maxZ = triangleVector[0].getVertPos(0)->z;
		float minZ = triangleVector[0].getVertPos(0)->z;
		for (int i = 0; i < triangleVector.size(); i++) {
			for (int j = 0; j < 3; j++) {
				float currZ = triangleVector[i].getVertPos(j)->z;
				if (currZ > maxZ) { maxZ = currZ; }
				if (currZ < minZ) { minZ = currZ; }
			}
		}

		for (int i = 0; i < triangleVector.size(); i++) {
			for (int j = 0; j < 3; j++) {
				glm::vec3 vc;
				vc.x = 0;
				vc.y = (triangleVector[i].getVertPos(j)->z - minZ) / (maxZ - minZ);
				vc.z = 0;
				triangleVector[i].setVertColor(&vc, j);
			}
		}
		break;
	}
}

// Keyboard character callback function
void CharacterCallback(GLFWwindow* lWindow, unsigned int key)
{
	switch (key) 
	{
	case '0':
		colorMode = 0;
		colorTriangles();
		break;
	case '1':
		colorMode = 1;
		colorTriangles();
		break;
	case '2':
		colorMode = 2;
		colorTriangles();
		break;
	case 'w':
		eyeDistance *= (1 - 0.05);
		break;
	case 's':
		eyeDistance *= (1 + 0.05);
		break;
	case 'a':
		angle -= 0.01;
		break;
	case 'd':
		angle += 0.01;
		break;
	case ' ':
		isOpenGL = !isOpenGL;
		break;
	case 't':
	{
		if (!texture.empty())
			isTextured = !isTextured;
		break;
	}
		
	case 'n':
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		textureMode = 0;
		break;
	case 'l':
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		textureMode = 1;
		break;
	case 'm':
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		textureMode = 2;
		break;
	case 'q':
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	default:
		break;
	}


}

// Create a vector of triangles. Considers the texture coordinates if they are available.
void CreateTriangleVector(std::vector<glm::vec3> &vertices, std::vector<glm::vec2>& texCoords)
{
	for (int i = 0; i < vertices.size() / 3; i++)
	{
		Triangle myTriangle;

		if (texCoords.empty())
			myTriangle = Triangle(vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2]);
		else
			myTriangle = Triangle(vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2], 
								texCoords[i * 3 + 0], texCoords[i * 3 + 1], texCoords[i * 3 + 2]);

		triangleVector.push_back(myTriangle);
	}
}

// Load the geometry and texture coordinates if available
void LoadModel(char* name, std::vector<glm::vec3> &vertices, std::vector<glm::vec2>& texCoords)
{
	// Taken from Shinjiro Sueda with slight modification
	std::string meshName(name);
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if (!rc) {
		std::cerr << errStr << std::endl;
	}
	else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					vertices.push_back(glm::vec3(attrib.vertices[3 * idx.vertex_index + 0],
												 attrib.vertices[3 * idx.vertex_index + 1],
												 attrib.vertices[3 * idx.vertex_index + 2]));
					if (!attrib.texcoords.empty()) {
						texCoords.push_back(glm::vec2(attrib.texcoords[2 * idx.texcoord_index + 0],
							attrib.texcoords[2 * idx.texcoord_index + 1]));
					}
				}
				index_offset += fv;
			}
		}
	}
}

// Load texture and create downsampled versions of it for mipmapping
void LoadTexture(char* name)
{
	std::string texName(name);
	int c;
	stbi_set_flip_vertically_on_load(true);
	stbi_hdr_to_ldr_gamma(1.0f);
	float* image = stbi_loadf(texName.c_str(), &texWidth, &texHeight, &c, 0);
	
	if (!image)
		std::cerr << texName << " not found" << std::endl;
	else if (c != 3)
		std::cerr << texName << " must have 3 channels (RGB)" << std::endl;
	else if ((texWidth % 2) != 0 || (texHeight % 2) != 0)
		std::cerr << " must be a power of 2" << std::endl;
	else
		texture.push_back(image);

	int length = std::min(texWidth, texHeight);
	int numLevels = log2(length);
	
	float** downImages = new float* [numLevels];
	for (int i = 0; i < numLevels; i++)
		downImages[i] = new float[texWidth * texHeight * c];

	for (int i = 0; i < numLevels; i++)
	{
		int curWidth = texWidth / pow(2, i + 1);
		int curHeight = texHeight / pow(2, i + 1);
		float* temp = new float[curWidth * curHeight * c];
		stbir_resize_float(image, texWidth, texHeight, 0, temp, curWidth, curHeight, 0, c);
		stbir_resize_float(temp, texWidth / pow(2, i + 1), texHeight / pow(2, i + 1), 0, downImages[i], texWidth, texHeight, 0, c);
		texture.push_back(downImages[i]);
		stbi_image_free(temp);
	}


	if (!texture.empty())
	{
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_FLOAT, texture[0]);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
		
}

std::string WindowTitle(std::string mainName)
{
	std::string hardwareName;
	if (isOpenGL)
		hardwareName = " - GPU";
	else
		hardwareName = " - CPU";

	std::string textureMethod;
	if (textureMode == 0)
		textureMethod = " - Nearest";
	else if (textureMode == 1)
		textureMethod = " - Bilinear";
	else if (textureMode == 2)
		textureMethod = " - Mipmap";

	std::string colorMethod;
	if (textureMode == 0)
		colorMethod = " - Mode 0";
	else if (textureMode == 1)
		colorMethod = " - Mode 1";
	else if (textureMode == 2)
		colorMethod = " - Mode 2";

	if (isTextured)
		return (mainName + hardwareName + std::string(" - Textured") + textureMethod);
	else
		return (mainName + hardwareName + std::string(" - Colored") + colorMethod);
}

void Init()
{
	srand(time(NULL));
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WindowTitle(mainName).c_str(), NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetCharCallback(window, CharacterCallback);
	glewExperimental = GL_TRUE;
	glewInit();
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	ClearFrameBuffer();

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	LoadModel("../resources/bunny.obj", vertices, texCoords);
	
	if (!texCoords.empty())
	{
		LoadTexture("../resources/earth.jpg");
		if (texture.empty())
			isTextured = false;
	}
	else
		isTextured = false;
		
	CreateTriangleVector(vertices, texCoords);
	
}



int main()
{	
	Init();
	while ( glfwWindowShouldClose(window) == 0) 
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Display();
		glfwSwapBuffers(window);
		glfwPollEvents();
		glfwSetWindowTitle(window, WindowTitle(mainName).c_str());
	}

	glfwTerminate();
	return 0;
}