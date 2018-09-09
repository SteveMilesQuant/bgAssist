
#define BGASSIST_BUILDER_WORLD

#include <tile.hpp>
#include <shader.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <glfwExt.hpp>

class vector<tile *> allTiles;



int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

																						// Open a window and create its OpenGL context
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	//GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Playground", primaryMonitor, NULL);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Playground", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar(); 
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// TODO: set callbacks

	// Initialize flags
	GLboolean fullscreenFlag = false;
	GLboolean screenUpdatedFlag = false;

	// Load shaders
	string shaderPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/shaders/";
	string vertexShaderPath = shaderPath + "StandardShading.vertexshader";
	string fragmentShaderPath = shaderPath + "StandardShading.fragmentshader";
	GLuint programID = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

	// Create camera
	timedMat4 Projection = timedMat4(glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f));
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, -2.5, 3), // Camera location, in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	timedMat4 Camera = timedMat4(View);

	// Set up light
	lightSource Light;
	Light.position = vec3(1, -5, 5);
	Light.power = 60.0f;

	// Paths to images
	string imagePath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/test/images/";
	string sideImagePath = imagePath + "cardboard_sides_BMP_DXT3_1.DDS";
	string leftFaceImagePath = imagePath + "somethingWeird_BMP_DXT3_1.DDS";
	string rightFaceImagePath = imagePath + "outdoorTile_BMP_DXT3_1.DDS";

	// TODO: create master token in upper right
	tile leftTile(ivec2(1, 2), true);
	leftTile.setProgramId(programID); 
	leftTile.setCamera(&Camera);
	leftTile.setProjection(&Projection);
	leftTile.setLight(&Light);
	leftTile.setAmbientRatio(0.2);
	leftTile.loadFaceImage(leftFaceImagePath.c_str());
	leftTile.loadSideImage(sideImagePath.c_str());
	leftTile.setLocation(vec2(-0.5f,0.0f));
	allTiles.push_back(&leftTile);

	tile rightTile= leftTile;
	rightTile.loadFaceImage(rightFaceImagePath.c_str());
	rightTile.setLocation(vec2(0.5f, 0.0f));
	allTiles.push_back(&rightTile);

	do {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);

		// Change between fullscreen and windowed with 'f'
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
			if (fullscreenFlag) {
				glfwSetWindowMonitor(window, NULL, 0, 0, mode->width, mode->height, mode->refreshRate);
				fullscreenFlag = false;
			}
			else {
				glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
				fullscreenFlag = true;
			}
			screenUpdatedFlag = true;
		}
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
			screenUpdatedFlag = false;
		}

		// Draw all objects
		vector<tile *>::iterator tileIter = allTiles.begin();
		for (; tileIter != allTiles.end(); tileIter++) {
			(*tileIter)->draw();
			list<token *> &tokenList = (*tileIter)->tokenList;
			list<token *>::iterator tokenIter = tokenList.begin();
			for (; tokenIter != tokenList.end(); tokenIter++) {
				(*tokenIter)->draw();
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);
	glfwDestroyWindow(window);
	glfwTerminate();
}


