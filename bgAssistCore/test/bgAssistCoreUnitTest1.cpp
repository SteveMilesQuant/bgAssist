
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
		(*prismIter)->getCamera(),
		(*prismIter)->getProjection(),
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


void dragPrismImageBegin(prismTop * thisPrism) {
	if (thisPrism) thisPrism->dragImageBegin();
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

	selectedPrism->dragImage(shift);
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
			selectedPrism->getCamera(),
			selectedPrism->getProjection(),
			origin,
			direction
		);

		// Camera ray's intersection with N(0,0,1), p_0(0,0,prismPosition.z)
		t = (prismPosition.z - origin.z) / direction.z;
		newPosition = origin + t * direction;

		selectedPrism->setTranslation(newPosition);
		selectedPrism->updateModelMatrix();
		selectedPrism->updateMVP();
	}
}

// Actions for a mouse click
static void clickAction(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			selectedPrism = findSelectedPrism(window);

			if (selectedPrism) {
				if (selectedPrism->doWhenSelected) {
					selectedPrism->doWhenSelected(selectedPrism);
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
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
	int winWidth = 1024;
	int winHeight = 768;
	window = glfwCreateWindow(winWidth, winHeight, "Playground", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Set callbacks
	glfwSetMouseButtonCallback(window, clickAction);
	glfwSetCursorPosCallback(window, dragAction);

	// Load shaders
	string shaderPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/shaders/";
	string vertexShaderPath = shaderPath + "TransformVertexShader.vertexshader";
	string fragmentShaderPath = shaderPath + "TextureFragmentShader.fragmentshader";
	GLuint programID = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

	// Get matrix nad texture ids
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint TextureID = glGetUniformLocation(programID, "prismTopTexture");

	// Create camera
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, -2, 6), // Camera location, in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Hex prism
	prismTop hexPrism(6);
	hexPrism.glfwCursorPosCallback = dragSelectedPrismImage;
	hexPrism.doWhenSelected = dragPrismImageBegin;
	hexPrism.setCamera(&View);
	hexPrism.setProjection(&Projection);
	hexPrism.setScale(vec3(1.0f, 1.0f, 1.0f / 10.0f));
	hexPrism.setTranslation(vec3(1.5f, 0.0f, 0.0f));
	hexPrism.updateModelMatrix();

	hexPrism.setUvScale(vec2(0.5, 0.5));
	hexPrism.setUvCenter(vec2(0.5, 0.5));
	string imagePath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/test/images/";
	string unicornPath = imagePath + "unicorn.bmp";
	hexPrism.loadBMP(unicornPath.c_str());
	hexPrism.dragImageBegin();

	hexPrism.passBuffersToGLM(GL_DYNAMIC_DRAW);
	hexPrism.updateMVP();
	allPrisms.push_back(&hexPrism);

	// Penta prism
	prismTop pentaPrism(5);
	pentaPrism.glfwCursorPosCallback = dragSelectedPrism;
	pentaPrism.doWhenSelected = NULL;
	pentaPrism.setCamera(&View);
	pentaPrism.setProjection(&Projection);
	pentaPrism.setScale(vec3(1.0f, 1.0f, 1.0f / 10.0f));
	pentaPrism.setTranslation(vec3(-1.5f, 0.0f, 0.0f));
	pentaPrism.updateModelMatrix();

	pentaPrism.setUvScale(vec2(0.5, 0.5));
	pentaPrism.setUvCenter(vec2(0.5, 0.5));
	pentaPrism.loadBMP(unicornPath.c_str());

	pentaPrism.passBuffersToGLM(GL_STATIC_DRAW);
	pentaPrism.updateMVP();
	allPrisms.push_back(&pentaPrism);

	do {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);

		hexPrism.draw(MatrixID, TextureID);
		pentaPrism.draw(MatrixID, TextureID);

		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0);

	glfwDestroyWindow(window);
	glfwTerminate();
}


