
#define BGASSIST_BUILDER_WORLD

#include <tile.hpp>
#include <token.hpp>
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

// Global objects
token masterToken(6);
class vector<tile *> allTiles;
class tile * selectedTile = NULL;
class token * selectedToken = NULL;
struct pair<double, double> clickMousePos(-1.0, -1.0);
GLboolean beginDragFlag = false;


static GLboolean testTokenSelection(GLFWwindow* window, token * inToken) {
	int screenWidth, screenHeight;
	vec3 origin, direction;
	float intersection_distance; 
	
	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	screenPosToWorldRay(
		(int)clickMousePos.first,
		screenHeight - (int)clickMousePos.second,
		screenWidth, screenHeight,
		inToken->getCamera().getMatrix(),
		inToken->getProjection().getMatrix(),
		origin,
		direction
	);

	if (testRayOBBIntersection(
		origin,
		direction,
		inToken->getMinCoords(),
		inToken->getMaxCoords(),
		inToken->getModelMatrix(),
		intersection_distance)
		)
	{
		return true;
	}

	return false;
}


static tile * findSelectedTile(GLFWwindow* window) {
	vector<tile *>::iterator tileIter = allTiles.begin();
	int screenWidth, screenHeight;
	vec3 origin, direction;
	float intersection_distance;

	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	screenPosToWorldRay(
		(int)clickMousePos.first,
		screenHeight - (int)clickMousePos.second,
		screenWidth, screenHeight,
		(*tileIter)->getCamera().getMatrix(),
		(*tileIter)->getProjection().getMatrix(),
		origin,
		direction
	);

	for (; tileIter != allTiles.end(); tileIter++) {
		if (!(*tileIter)) continue;
		tile & currentTile = *(*tileIter);
		if (testRayOBBIntersection(
			origin,
			direction,
			currentTile.getMinCoords(),
			currentTile.getMaxCoords(),
			currentTile.getModelMatrix(),
			intersection_distance)
			)
		{
			return &currentTile;
		}
	}

	return NULL;
}


static token * findSelectedToken(GLFWwindow* window) {
	tile & firstTile = *(allTiles[0]);
	int screenWidth, screenHeight;
	vec3 origin, direction;
	float intersection_distance;

	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	screenPosToWorldRay(
		(int)clickMousePos.first,
		screenHeight - (int)clickMousePos.second,
		screenWidth, screenHeight,
		firstTile.getCamera().getMatrix(),
		firstTile.getProjection().getMatrix(),
		origin,
		direction
	);

	// Find the current tile
	tile * currentTileP = findSelectedTile(window);
	if (!currentTileP) return NULL;
	tile & currentTile = *currentTileP;

	// Loop through tokens to find the current token
	list<token *> & tokenList = currentTile.tokenList;
	list<token *>::iterator tokenIter = tokenList.begin();
	for (; tokenIter != tokenList.end(); tokenIter) {
		if (!(*tokenIter)) continue;
		token & currentToken = *(*tokenIter);
		if (testRayOBBIntersection(
			origin,
			direction,
			currentToken.getMinCoords(),
			currentToken.getMaxCoords(),
			currentToken.getModelMatrix(),
			intersection_distance)
			)
		{
			return &currentToken;
		}
	}

	return NULL;
}


// Global callback for any mouse clicks
static void clickAction(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		glfwGetCursorPos(window, &clickMousePos.first, &clickMousePos.second);
		beginDragFlag = false;
		token * origSelectedToken = selectedToken;
		
		if (testTokenSelection(window, &masterToken)) selectedToken = &masterToken;
		else selectedToken = findSelectedToken(window);

		// If we selected something else and the originally selected item is temporary, delete it
		if (origSelectedToken && origSelectedToken != selectedToken &&
			origSelectedToken != &masterToken && !origSelectedToken->parentTile)
		{
			delete[] origSelectedToken;
			origSelectedToken = NULL;
		}
	}
	else {
		clickMousePos.first = -1.0;
		clickMousePos.second = -1.0;
	}

	// If a token is selected, call its callback
	if (selectedToken) selectedToken->callGlfwMouseButtonCallback(window, button, action, mods);
}


// On click and release, 
static void masterTokenClickAction(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		selectedToken = new token(masterToken);
		selectedToken->passBuffersToGLM(true);
		GLfloat masterTokenRadius = masterToken.getRadius();
		selectedToken->setRadius(10.0f * masterTokenRadius);
		selectedToken->setThickness(masterTokenRadius);
		selectedToken->setLocation(vec2(0, 0));
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

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// TODO: set callbacks
	//glfwSetMouseButtonCallback(window, clickAction);

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
	mat4 View = lookAt(
		vec3(0, -2.5, 3), // Camera location, in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	timedMat4 Camera = timedMat4(View);

	// Set up light
	lightSource Light;
	Light.position = vec3(1, -5, 5);
	Light.power = 60.0f;

	// Create second camera for master/design tokens
	mat4 View2 = lookAt(
		vec3(0, 0, 1), // Camera location, in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	timedMat4 Camera2 = timedMat4(View2);

	lightSource Light2;
	Light2.position = vec3(0, 0, 12);
	Light2.power = 100.0f;

	// Find the upper right corner for camera2
	vec3 origin, direction;
	screenPosToWorldRay(
		mode->width,
		mode->height,
		mode->width, mode->height,
		Camera2.getMatrix(),
		Projection.getMatrix(),
		origin,
		direction
	);
	// Camera ray's intersection with x-y plane
	vec3 upperRightCorner2 = origin - origin.z / direction.z * direction;

	// Paths to images
	string imagePath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/test/images/";
	string sideImagePath = imagePath + "cardboard_sides_BMP_DXT3_1.DDS";
	string leftFaceImagePath = imagePath + "somethingWeird_BMP_DXT3_1.DDS";
	string rightFaceImagePath = imagePath + "outdoorTile_BMP_DXT3_1.DDS";
	string swordImagePath = imagePath + "sword_BMP_DXT3_1.DDS";

	// Create starting objects
	tile leftTile(true, ivec2(1, 2));
	leftTile.setProgramId(programID); 
	leftTile.setCamera(&Camera);
	leftTile.setProjection(&Projection);
	leftTile.setLight(&Light);
	leftTile.setAmbientRatio(0.2f);
	leftTile.loadFaceImage(leftFaceImagePath.c_str());
	leftTile.loadSideImage(sideImagePath.c_str());
	leftTile.setLocation(vec2(-0.5f,0.0f));
	allTiles.push_back(&leftTile);

	tile rightTile = leftTile;
	rightTile.loadFaceImage(rightFaceImagePath.c_str());
	rightTile.setLocation(vec2(0.5f, 0.0f));
	allTiles.push_back(&rightTile);

	token masterToken2(6);
	GLfloat masterTokenRadius = 1.0f/20.0f;
	masterToken2.passBuffersToGLM(true);
	// masterToken.setGlfwMouseButtonCallback(masterTokenClickAction);
	masterToken2.setProgramId(programID);
	masterToken2.setCamera(&Camera2);
	masterToken2.setProjection(&Projection);
	masterToken2.setLight(&Light2);
	masterToken2.setAmbientRatio(0.2f);
	masterToken2.loadFaceImage(swordImagePath.c_str());
	masterToken2.loadSideImage(sideImagePath.c_str());
	masterToken2.setRotation(pi<GLfloat>() / 8.0f, vec3(-1.0f, 0.0f, 0.0f));
	masterToken2.setRadius(masterTokenRadius);
	masterToken2.setThickness(masterTokenRadius/10.f);
	masterToken2.setLocation(vec2(upperRightCorner2.x - 2.0f*masterTokenRadius, upperRightCorner2.y - 1.5f*masterTokenRadius));

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
		masterToken2.draw();
		//if (selectedToken) selectedToken->draw();

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


