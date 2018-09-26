
#include <glfwExt.hpp>
#include <shader.hpp>
#include <texture.hpp>

#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;


void loadFontMeta(const char * csvPath, int & outWidth, vector<float> &outCellWidths) {
	int width = 0, height = 0;
	ifstream file(csvPath, ifstream::in);

	outCellWidths.clear();

	if (file.is_open()) {
		GLboolean firstArgFlag = true;
		GLboolean spaceReservedFlag = false;
		string argName, argValueString;
		int imageWidth(0), imageHeight(0), cellWidth(0), cellHeight(0);
		float fontHeight(0);

		while (!file.eof()) {
			char delim = (firstArgFlag) ? ',' : '\n';

			if (firstArgFlag) {
				getline(file, argName, delim);
			}
			else {
				getline(file, argValueString, delim);
				if (argName == "Image Width") imageWidth = stoi(argValueString);
				else if (argName == "Image Height") imageHeight = stoi(argValueString);
				else if (argName == "Cell Width") cellWidth = stoi(argValueString);
				else if (argName == "Cell Height") cellHeight = stoi(argValueString);
				else if (argName == "Font Height") fontHeight = stoi(argValueString);
				else if (argName.find("Base Width") < argName.npos) {
					int endIdx = argName.find(' ', 5);
					string charIdxString = argName.substr(5, endIdx-5);
					int charIdx = stoi(charIdxString);
					outCellWidths[charIdx] = (float) stoi(argValueString) / fontHeight;
				}
			}

			if (width == 0 && imageWidth > 0 && cellWidth > 0) width = imageWidth / cellWidth;
			if (height == 0 && imageHeight > 0 && cellHeight > 0) height = imageHeight / cellHeight;

			if (!spaceReservedFlag && width > 0 && height > 0) {
				outCellWidths.resize(width*height);
				spaceReservedFlag = true;
			}

			firstArgFlag = !firstArgFlag;
		}
	}
	file.close();

	outWidth = width;
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

	// White background
	glClearColor(1, 1, 1, 1);

	// Load shaders
	string shaderPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/shaders/";
	string vertexShaderPath = shaderPath + "TextShading.vertexshader";
	string fragmentShaderPath = shaderPath + "TextShading.fragmentshader";
	GLuint textProgramId = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());


	GLuint textureId = glGetUniformLocation(textProgramId, "textShadingTexture");
	GLuint textColorId = glGetUniformLocation(textProgramId, "textColor");

	GLuint vertexBufferId, uvBufferId;
	GLuint textImageId;

	string fontPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/fonts/";
	string testFont = fontPath + "MalgunGothic_BMP_DXT3_1.DDS";
	textImageId = loadDDS(testFont.c_str());

	int fontImageWidth, fontImageHeight;
	vector<float> charWidths;
	string fontMethPath = fontPath + "MalgunGothic.csv";
	loadFontMeta(fontMethPath.c_str(), fontImageWidth, charWidths);
	fontImageHeight = charWidths.size() / fontImageWidth;

	int screenWidth, screenHeight;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);
	float fontHeight = 48.0f * 2.0f / screenWidth;

	GLboolean buffersPassedFlag = false;

	vector<vec2> vertices;
	vector<vec2> uvs;

	vec4 red( 1, 0, 0, 1 );

	int writeLetter = (int) 'd';

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(textProgramId);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (!buffersPassedFlag) {
			int imageRow = writeLetter / fontImageWidth;
			int imageCol = writeLetter % fontImageWidth;
			float charWidth = charWidths[writeLetter] * fontHeight;

			vec2 uvUpperLeft((float)imageCol / fontImageWidth, (float) imageRow / fontImageHeight);
			uvs.clear();
			uvs.push_back(uvUpperLeft);
			uvs.push_back(uvUpperLeft+vec2(charWidths[writeLetter] / fontImageWidth, 0));
			uvs.push_back(uvUpperLeft + vec2(0, 1.0f/ fontImageHeight));
			uvs.push_back(uvUpperLeft + vec2(charWidths[writeLetter] / fontImageWidth, 1.0f / fontImageHeight));

			vec2 vertexUpperLeft(0, 0);
			vertices.clear();
			vertices.push_back(vertexUpperLeft);
			vertices.push_back(vertexUpperLeft+vec2(charWidth, 0));
			vertices.push_back(vertexUpperLeft + vec2(0, -fontHeight));
			vertices.push_back(vertexUpperLeft + vec2(charWidth, -fontHeight));

			glGenBuffers(1, &vertexBufferId);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec2), &vertices[0], GL_DYNAMIC_DRAW);

			glGenBuffers(1, &uvBufferId);
			glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
			glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_DYNAMIC_DRAW);

			buffersPassedFlag = true;
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textImageId);
		glUniform1i(textureId, 0);
		glUniform4f(textColorId, red.r, red.g, red.b, red.a);

		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // 3 dimensions of space
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : texture
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

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

