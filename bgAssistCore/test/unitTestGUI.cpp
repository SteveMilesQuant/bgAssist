
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


textBox drawTextBox;

void worldCharCallback(GLFWwindow* window, unsigned int codepoint, int mods) {
	drawTextBox.callGlfwCharModsCallback(window, codepoint, mods);
}

void worldKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	drawTextBox.callGlfwKeyCallback(window, key, scancode, action, mods);
}

void worldMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	drawTextBox.callGlfwMouseButtonCallback(window, button, action, mods);
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

	// Black background
	glClearColor(0, 0, 0, 1);

	// Load shaders
	string shaderPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/shaders/";
	string vertexShaderPath = shaderPath + "TextShading.vertexshader";
	string fragmentShaderPath = shaderPath + "TextShading.fragmentshader";
	GLuint textProgramId = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

	// Load font
	string fontPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/fonts/";
	string testFont = fontPath + "InkFree_BMP_DXT3_1.DDS";
	string fontMethPath = fontPath + "InkFree.csv";
	font inkFreeFont;
	inkFreeFont.loadFont(testFont.c_str(), fontMethPath.c_str());

	int screenWidth, screenHeight;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);
	float marginWidth = 48.0f * 2.0f / screenWidth;
	float textHeight_screen = 48.0f * 2.0f / screenHeight;
	vec2 startPos(-1.0f + marginWidth, 1.0f - textHeight_screen);

	drawTextBox.setProgramId(textProgramId);
	drawTextBox.textFont = &inkFreeFont;
	drawTextBox.isEditableFlag = true;
	drawTextBox.textHeight = textHeight_screen;
	drawTextBox.setBoxWidth(2.0f*(1.0f- marginWidth));
	drawTextBox.setText("Draw this text now! This is too long! Long! Long! Longgg");
	drawTextBox.upperLeftCornerLocation = startPos;
	drawTextBox.textColor = vec4(1, 0, 0, 1); // red, opaque

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawTextBox.draw();

		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up
	glDeleteProgram(textProgramId);
	glDeleteVertexArrays(1, &VertexArrayID);
	glfwDestroyWindow(window);
	glfwTerminate();
}

