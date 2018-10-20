
#include <glfwExt.hpp>
#include <shader.hpp>
#include <texture.hpp>
#include <font.hpp>
#include <textBox.hpp>

#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

typedef textBox allObjectTypes;
vector<allObjectTypes *> objectList;
allObjectTypes * selectedObject = NULL;

void worldCharCallback(GLFWwindow* window, unsigned int codepoint, int mods) {
	if (selectedObject) selectedObject->callGlfwCharModsCallback(window, codepoint, mods);
}

void worldKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (selectedObject) selectedObject->callGlfwKeyCallback(window, key, scancode, action, mods);
}

void worldMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	allObjectTypes * origSelectedObject = selectedObject;
	if (action == GLFW_PRESS) {
		selectedObject = NULL;
		vec2 clickPosition_world = screenPosTo2DCoord(window);
		for (int i = 0; i < objectList.size(); i++) {
			if (objectList[i] && objectList[i]->testPointInBounds(clickPosition_world)) {
				selectedObject = objectList[i];
				break;
			}
		}
	}
	if (origSelectedObject && origSelectedObject != selectedObject) origSelectedObject->deselect();
	if (selectedObject) selectedObject->callGlfwMouseButtonCallback(window, button, action, mods);
}

void worldScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	if (selectedObject) selectedObject->callGlfwScrollCallback(window, xoffset, yoffset);
}

void worldCursorPosCallback(GLFWwindow* window, double x, double y) {
	if (selectedObject) selectedObject->callGlfwCursorPosCallback(window, x, y);
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
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "unitTestGUI", NULL, NULL);
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
	//glEnable(GL_CULL_FACE);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Set callbacks
	glfwSetCharModsCallback(window, worldCharCallback);
	glfwSetKeyCallback(window, worldKeyCallback);
	glfwSetMouseButtonCallback(window, worldMouseButtonCallback);
	glfwSetScrollCallback(window, worldScrollCallback);
	glfwSetCursorPosCallback(window, worldCursorPosCallback);

	// Black background
	glClearColor(0, 0, 0, 1);

	// Load shaders
	string shaderPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/shaders/";
	string vertexShaderPath = shaderPath + "2DText.vertexshader";
	string fragmentShaderPath = shaderPath + "2DText.fragmentshader";
	GLuint twodimImageProgramId = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

	vertexShaderPath = shaderPath + "2DSolidColor.vertexshader";
	fragmentShaderPath = shaderPath + "2DSolidColor.fragmentshader";
	GLuint twodimSoloColorProgramId = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

	// Load font
	string fontPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/fonts/";
	string testFont = fontPath + "InkFree_BMP_DXT3_1.DDS";
	string fontMetaPath = fontPath + "InkFree.csv";
	font inkFreeFont;
	inkFreeFont.loadFont(testFont.c_str(), fontMetaPath.c_str());

	int screenWidth, screenHeight;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);
	float marginWidth = 48.0f * 2.0f / screenWidth;
	float textHeight_screen = 48.0f * 2.0f / screenHeight;
	vec2 startPos(-1.0f + marginWidth, 1.0f - textHeight_screen);

	// Reset global objects
	objectList.clear();
	selectedObject = NULL;

	// Create our text box
	textBox drawTextBox;
	drawTextBox.setTextProgramId(twodimImageProgramId);
	drawTextBox.setCursorProgramId(twodimSoloColorProgramId);
	drawTextBox.textFont = &inkFreeFont;
	drawTextBox.isEditableFlag = true;
	drawTextBox.cursorWidth = 4.0f / screenWidth;
	drawTextBox.setTextHeight(textHeight_screen);
	drawTextBox.setBoxDimensions(vec2(2.0f*(1.0f- marginWidth), 5.5 * textHeight_screen));
	drawTextBox.setText("Draw this text now! This is too long! Long! Long! Longgg");
	drawTextBox.setBoxLocation(startPos);
	drawTextBox.setTextColor(vec4(1, 0, 0, 1)); // red, opaque

	string imagePath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/test/images/";
	string scrollBarImagePath = imagePath + "scrollBar_BMP_DXT3_1.DDS";
	drawTextBox.setScrollBarWidth(10.0f * 2.0f / screenWidth);
	drawTextBox.loadScrollBarImage(scrollBarImagePath.c_str());
	objectList.push_back(&drawTextBox);

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawTextBox.draw();

		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up
	objectList.clear();
	glDeleteProgram(twodimImageProgramId);
	glDeleteVertexArrays(1, &VertexArrayID);
	glfwDestroyWindow(window);
	glfwTerminate();
}

