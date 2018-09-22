
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
class token * selectedToken = NULL;
class token * lastSelectedToken = NULL;
struct pair<double, double> clickMousePos(-1.0, -1.0);
GLboolean beginDragFlag = false;


static tile * findSelectedTile(vec3 ray_origin, vec3 ray_direction) {
	vector<tile *>::iterator tileIter = allTiles.begin();

	for (; tileIter != allTiles.end(); tileIter++) {
		if (!(*tileIter)) continue;
		tile & currentTile = *(*tileIter);
		if (currentTile.testRayOBBIntersection(
			ray_origin,
			ray_direction))
		{
			return &currentTile;
		}
	}

	return NULL;
}

static tile * findSelectedTile(GLFWwindow* window, double x, double y) {
	tile & firstTile = *(allTiles[0]);
	int screenWidth, screenHeight;
	vec3 origin, direction;

	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	screenPosToWorldRay(
		(int)x,
		screenHeight - (int)y,
		screenWidth, screenHeight,
		firstTile.getCamera().getMatrix(),
		firstTile.getProjection().getMatrix(),
		origin,
		direction
	);

	return findSelectedTile(origin, direction);
}


static token * findSelectedToken(GLFWwindow* window) {
	tile & firstTile = *(allTiles[0]);
	int screenWidth, screenHeight;
	vec3 origin, direction;

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
	tile * currentTileP = findSelectedTile(origin, direction);
	if (!currentTileP) return NULL;
	else return currentTileP->findChildRayIntersection(origin, direction);
}


// Global callback for any mouse clicks
static void clickAction(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		glfwGetCursorPos(window, &clickMousePos.first, &clickMousePos.second);
		beginDragFlag = false;
		token * origSelectedToken = selectedToken;
		
		if (selectedToken &&
			selectedToken->testRayOBBIntersection(window, clickMousePos.first, clickMousePos.second)) {
			selectedToken = selectedToken;
		}
		else if (masterToken.testRayOBBIntersection(window, clickMousePos.first, clickMousePos.second)) {
			selectedToken = &masterToken;
		}
		else selectedToken = findSelectedToken(window);

		// If we selected something else and the originally selected item is temporary, delete it
		if (origSelectedToken && origSelectedToken != selectedToken &&
			origSelectedToken != &masterToken && !origSelectedToken->getParentTile())
		{
			delete origSelectedToken;
			origSelectedToken = NULL;
		}
	}
	else {
		clickMousePos.first = -1.0;
		clickMousePos.second = -1.0;
		lastSelectedToken = selectedToken;
	}

	// If a token is selected, call its callback
	if (selectedToken) selectedToken->callGlfwMouseButtonCallback(window, button, action, mods);
}


// Global callback for mouse move
static void moveAction(GLFWwindow* window, double x, double y) {
	if (clickMousePos.first >= 0.0 && clickMousePos.second >= 0.0f) beginDragFlag = true;
	if (selectedToken) selectedToken->callGlfwCursorPosCallback(window, x, y);
}

// Global callback for keystrokes
static void keyAction(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (lastSelectedToken) lastSelectedToken->callGlfwKeyCallback(window, key, scancode, action, mods);
}


// Map token callbacks
// Deselect on release
// Find location on drag
static void mapTokenDeselect(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		selectedToken = NULL;
	}
}
static void mapTokenDragAction(GLFWwindow* window, double x, double y) {
	if (!beginDragFlag || !selectedToken) return;

	int screenWidth, screenHeight;
	vec3 origin, direction;

	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	screenPosToWorldRay(
		(int)x,
		screenHeight - (int)y,
		screenWidth, screenHeight,
		selectedToken->getCamera().getMatrix(),
		selectedToken->getProjection().getMatrix(),
		origin,
		direction
	);

	tile * newParentTile = findSelectedTile(origin, direction);
	if (!newParentTile) return;

	GLfloat t = origin.z / direction.z;
	vec3 newPosition = origin - t * direction;

	selectedToken->setLocation(vec2(newPosition));
	selectedToken->setParentTile(newParentTile);
}
static void mapTokenDelete(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (lastSelectedToken && (key == GLFW_KEY_DELETE || key == GLFW_KEY_BACKSPACE) && action == GLFW_PRESS) {
		if (lastSelectedToken == selectedToken) selectedToken = NULL;
		delete lastSelectedToken;
		lastSelectedToken = NULL;
	}
}

// Design token callbacks
// Drag the face of the design image
static void dragDesignTokenFaceImageBegin(GLFWwindow* window, int button, int action, int mods) {
	if (clickMousePos.first < 0 || clickMousePos.second < 0 || selectedToken == NULL)
		return;

	selectedToken->dragFaceImageBegin();
}
static void dragDesignTokenFaceImage(GLFWwindow* window, double x, double y)
{
	vec2 shift;
	int width, height;

	if (clickMousePos.first < 0 || clickMousePos.second < 0 || selectedToken == NULL || !beginDragFlag)
		return;

	glfwGetWindowSize(window, &width, &height);
	shift.x = (float)(clickMousePos.first - x) / width;
	shift.y = (float)(y - clickMousePos.second) / height;

	selectedToken->dragFaceImage(shift);
}
static void designTokenChangeNSides(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (!selectedToken || action != GLFW_PRESS) return;

	int nSides = 0;

	switch (key) {
		case GLFW_KEY_3: nSides = 3; break;
		case GLFW_KEY_4: nSides = 4; break;
		case GLFW_KEY_5: nSides = 5; break;
		case GLFW_KEY_6: nSides = 6; break;
		case GLFW_KEY_7: nSides = 7; break;
		case GLFW_KEY_8: nSides = 8; break;
		case GLFW_KEY_9: nSides = 9; break;
		default: nSides = 0; break;
	}

	if (nSides > 2) selectedToken->setNSides(nSides);
}

// Master token callbacks
// On click and release, open up a token for design
// On click and drag, open up a token for 
static void masterTokenClickAction(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && !beginDragFlag) {
		selectedToken = new token(masterToken);
		GLfloat masterTokenRadius = masterToken.getRadius();
		GLfloat masterTokenThickness = masterToken.getThickness();
		GLfloat ratio = 6.5f;
		selectedToken->setRadius(ratio * masterTokenRadius);
		selectedToken->setThickness(ratio * masterTokenThickness);
		selectedToken->setLocation(vec2(0, 0));
		selectedToken->setParentToken(&masterToken);
		selectedToken->setDesignTokenFlag(true);
		selectedToken->setGlfwKeyCallback(designTokenChangeNSides);
		selectedToken->setGlfwMouseButtonCallback(dragDesignTokenFaceImageBegin);
		selectedToken->setGlfwCursorPosCallback(dragDesignTokenFaceImage);
	}
}
static void masterTokenDragAction(GLFWwindow* window, double x, double y) {
	if (!beginDragFlag) return;

	tile * hoverTile = findSelectedTile(window, x, y);
	if (hoverTile) {
		selectedToken = new token(masterToken);
		selectedToken->setCamera(&hoverTile->getCamera());
		selectedToken->setProjection(&hoverTile->getProjection());
		selectedToken->setRelativeRadius(12.5f);
		selectedToken->setRelativeThickness(2.0f);
		selectedToken->setRotation(0.0f, vec3(1.0f, 0.0f, 0.0f));
		selectedToken->setParentTile(hoverTile);
		selectedToken->setParentToken(&masterToken);

		selectedToken->setGlfwMouseButtonCallback(mapTokenDeselect);
		selectedToken->setGlfwCursorPosCallback(mapTokenDragAction);
		selectedToken->setGlfwKeyCallback(mapTokenDelete);

		selectedToken->callGlfwCursorPosCallback(window, x, y);
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

	// Set callbacks
	glfwSetMouseButtonCallback(window, clickAction);
	glfwSetCursorPosCallback(window, moveAction);
	glfwSetKeyCallback(window, keyAction);

	// Initialize flags
	GLboolean fullscreenFlag = false;
	GLboolean screenUpdatedFlag = false;

	// Load shaders
	string shaderPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/shaders/";
	string vertexShaderPath = shaderPath + "StandardShading.vertexshader";
	string fragmentShaderPath = shaderPath + "StandardShading.fragmentshader";
	GLuint programID = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

	// Create camera
	timedMat4 Projection = timedMat4(glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f));
	mat4 View = lookAt(
		vec3(0, -2, 2), // Camera location, in World Space
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
	tile leftTile(ivec2(1, 2));
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

	GLfloat masterTokenRadius = 1.0f/20.0f;
	masterToken.setGlfwMouseButtonCallback(masterTokenClickAction);
	masterToken.setGlfwCursorPosCallback(masterTokenDragAction);
	masterToken.setProgramId(programID);
	masterToken.setCamera(&Camera2);
	masterToken.setProjection(&Projection);
	masterToken.setLight(&Light2);
	masterToken.setAmbientRatio(0.3f);
	masterToken.loadFaceImage(swordImagePath.c_str());
	masterToken.loadSideImage(sideImagePath.c_str());
	masterToken.setRotation(pi<GLfloat>() / 8.0f, vec3(-1.0f, 0.0f, 0.0f));
	masterToken.setRadius(masterTokenRadius);
	masterToken.setThickness(masterTokenRadius/10.f);
	masterToken.setLocation(vec2(upperRightCorner2.x - 2.0f*masterTokenRadius, upperRightCorner2.y - 1.5f*masterTokenRadius));

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
			(*tileIter)->draw(); // also draws children
		}
		masterToken.draw();
		if (selectedToken && selectedToken != &masterToken && selectedToken->getParentTile() == NULL) {
			selectedToken->draw();
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


