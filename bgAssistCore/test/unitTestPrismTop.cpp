
#include <prismTop.hpp>
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


// World space information
class prismTop * selectedPrism = NULL;
struct pair<double, double> dragMouseStartPos(-1.0, -1.0);
class vector<prismTop *> allPrisms;


static prismTop * findSelectedPrism(GLFWwindow* window) {
	vector<prismTop *>::iterator prismIter = allPrisms.begin();
	int screenWidth, screenHeight;
	vec3 origin, direction;
	float intersection_distance;

	glfwGetWindowSize(window, &screenWidth, &screenHeight);
	glfwGetCursorPos(window, &dragMouseStartPos.first, &dragMouseStartPos.second);

	screenPosToWorldRay(
		(int)dragMouseStartPos.first,
		screenHeight - (int)dragMouseStartPos.second,
		screenWidth, screenHeight,
		(*prismIter)->getCamera().getMatrix(),
		(*prismIter)->getProjection().getMatrix(),
		origin,
		direction
	);

	for (; prismIter != allPrisms.end(); prismIter++) {
		if (testRayOBBIntersection(
			origin,
			direction,
			(*prismIter)->getMinCoords(),
			(*prismIter)->getMaxCoords(),
			(*prismIter)->getModelMatrix(),
			intersection_distance)
			) {
			return *prismIter;
		}
	}

	return NULL;
}


void dragPrismImageBegin(GLFWwindow* window, int button, int action, int mods) {
	if (selectedPrism) selectedPrism->dragFaceImageBegin();
}

static void dragSelectedPrismImage(GLFWwindow* window, double x, double y)
{
	vec2 shift;
	int width, height;

	if (dragMouseStartPos.first < 0 || dragMouseStartPos.second < 0 || selectedPrism == NULL)
		return;

	glfwGetWindowSize(window, &width, &height);
	shift.x = (float)(dragMouseStartPos.first - x) / width;
	shift.y = (float)(y - dragMouseStartPos.second) / height;

	selectedPrism->dragFaceImage(shift);
}


static void dragSelectedPrism(GLFWwindow* window, double x, double y)
{
	if (selectedPrism) {
		int screenWidth, screenHeight;
		float t;
		vec3 origin, direction, newPosition;
		vec3 prismPosition = selectedPrism->getTranslation();

		glfwGetWindowSize(window, &screenWidth, &screenHeight);
		screenPosToWorldRay(
			(int)x,
			screenHeight - (int)y,
			screenWidth, screenHeight,
			selectedPrism->getCamera().getMatrix(),
			selectedPrism->getProjection().getMatrix(),
			origin,
			direction
		);

		// Camera ray's intersection with N(0,0,1), p_0(0,0,prismPosition.z)
		t = (prismPosition.z - origin.z) / direction.z;
		newPosition = origin + t * direction;

		selectedPrism->setTranslation(newPosition);
	}
}

// Actions for a mouse click
static void clickAction(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			selectedPrism = findSelectedPrism(window);

			if (selectedPrism) {
				if (selectedPrism->glfwMouseButtonCallback) {
					selectedPrism->glfwMouseButtonCallback(window, button, action, mods);
				}
			}
			else {
				dragMouseStartPos.first = -1.0;
				dragMouseStartPos.second = -1.0;
			}
		}
		else { // action == GLFW_RELEASE
			dragMouseStartPos.first = -1.0;
			dragMouseStartPos.second = -1.0;
			selectedPrism = NULL;
		}
	}
}


// Actions for dragging after a mouse click
static void dragAction(GLFWwindow* window, double x, double y) {
	if (selectedPrism && selectedPrism->glfwCursorPosCallback) {
		selectedPrism->glfwCursorPosCallback(window, x, y);
	}
}



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

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Set callbacks
	glfwSetMouseButtonCallback(window, clickAction);
	glfwSetCursorPosCallback(window, dragAction);

	// Initialize flags
	GLboolean fullscreenFlag = false;
	GLboolean screenUpdatedFlag = false;

	// Load shaders
	string shaderPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/shaders/";
	string vertexShaderPath = shaderPath + "StandardShading.vertexshader";
	string fragmentShaderPath = shaderPath + "StandardShading.fragmentshader";
	GLuint programID = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

	// Create camera and projection
	timedMat4 Projection = timedMat4(perspective(radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f));
	glm::mat4 FrontView = glm::lookAt(
		glm::vec3(0, -2, 5), // Camera location, in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	timedMat4 Camera = timedMat4(FrontView);

	glm::mat4 BackView = glm::lookAt(
		glm::vec3(0, -2, -6), // Camera location, in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	GLboolean onFrontView = true;
	GLboolean viewUpdated = false;

	// Set up light
	lightSource Light;
	Light.position = vec3(0, -4, 5);

	// Hex prism
	prismTop hexPrism(6);
	hexPrism.setProgramId(programID);
	hexPrism.glfwCursorPosCallback = dragSelectedPrismImage;
	hexPrism.glfwMouseButtonCallback = dragPrismImageBegin;
	hexPrism.setCamera(&Camera);
	hexPrism.setProjection(&Projection);
	hexPrism.setLight(&Light);
	hexPrism.setScale(vec3(1.0f, 1.0f, 1.0f / 10.0f));
	hexPrism.setTranslation(vec3(1.5f, 0.0f, 0.0f));

	hexPrism.setUvScale(vec2(0.5, 0.5));
	hexPrism.setUvCenter(vec2(0.5, 0.5));
	string imagePath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/test/images/";
	string unicornPath = imagePath + "unicorn_BMP_DXT3_1.DDS";
	hexPrism.loadFaceImage(unicornPath.c_str(), true);
	string sideImagePath = imagePath + "cardboard_sides.bmp";
	hexPrism.loadSideImage(sideImagePath.c_str(), false);

	allPrisms.push_back(&hexPrism);

	// Penta prism
	prismTop pentaPrism(5);
	pentaPrism.setProgramId(programID);
	pentaPrism.glfwCursorPosCallback = dragSelectedPrism;
	pentaPrism.glfwMouseButtonCallback = NULL;
	pentaPrism.setCamera(&Camera);
	pentaPrism.setProjection(&Projection);
	pentaPrism.setLight(&Light);
	pentaPrism.setScale(vec3(1.0f, 1.0f, 1.0f / 4.0f));
	pentaPrism.setTranslation(vec3(-1.5f, 0.0f, 0.0f));

	pentaPrism.setUvScale(vec2(0.5, 0.5));
	pentaPrism.setUvCenter(vec2(0.5, 0.5));
	unicornPath = imagePath + "unicorn.bmp"; // use BMP this time
	pentaPrism.loadFaceImage(unicornPath.c_str(), false);
	pentaPrism.loadSideImage(sideImagePath.c_str(), false);

	allPrisms.push_back(&pentaPrism);

	do {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);

		// Change view by pressing space bar
		// This should really be done with a callback
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !viewUpdated) {
			if (onFrontView) {
				Camera.setMatrix(BackView);
				onFrontView = false;
			}
			else {
				Camera.setMatrix(FrontView);
				onFrontView = true;
			}
			viewUpdated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			viewUpdated = false;
		}

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

		// Draw all prisms
		vector<prismTop *>::iterator prismIter = allPrisms.begin();
		for (; prismIter != allPrisms.end(); prismIter++) {
			(*prismIter)->draw();
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


