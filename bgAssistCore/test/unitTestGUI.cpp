

#include <shader.hpp>
#include <glfwExt.hpp>

#include <string>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

/*
#include <ft2build.h>
#include FT_FREETYPE_H
*/

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

	// Initialize freetype
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not init freetype library\n");
		return 1;
	}

	string fontPath = "C:/Users/Steve/Desktop/programming/bgAssist/bgAssistCore/fonts/";
	string freeSansFontPath = fontPath + "FreeSans.ttf";
	FT_Face face;
	if (FT_New_Face(ft, freeSansFontPath.c_str(), 0, &face)) {
		fprintf(stderr, "Could not open font\n");
		return 1;
	}

	GLuint textureId = glGetUniformLocation(textProgramId, "textShadingTexture");
	GLuint textColorId = glGetUniformLocation(textProgramId, "textColor");

	GLuint vertexBufferId, uvBufferId;
	GLuint textImageId;

	int screenWidth, screenHeight;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);
	float sx = 2.0f / screenWidth;
	float sy = 2.0f / screenHeight;

	GLboolean buffersPassedFlag = false;

	vector<vec2> vertices;
	vector<vec2> uvs;

	vec4 red( 1, 0, 0, 1 );

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(textProgramId);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		FT_Set_Pixel_Sizes(face, 0, 48);

		if (!buffersPassedFlag) {
			if (FT_Load_Char(face, 'C', FT_LOAD_RENDER)) {
				fprintf(stderr, "Could not load letter.\n");
				return 1;
			}

			FT_GlyphSlot g = face->glyph;

			glActiveTexture(GL_TEXTURE0); // might not need
			glGenTextures(1, &textImageId);
			glBindTexture(GL_TEXTURE_2D, textImageId);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g->bitmap.width, g->bitmap.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, g->bitmap.buffer);

			float x = -1 + 8 * sx;
			float y = 1 - 50 * sy;

			float x2 = x + g->bitmap_left * sx;
			float y2 = -y - g->bitmap_top * sy;
			float w = g->bitmap.width * sx;
			float h = g->bitmap.rows * sy;

			uvs.clear();
			uvs.push_back(vec2(0, 0));
			uvs.push_back(vec2(1, 0));
			uvs.push_back(vec2(0, 1));
			uvs.push_back(vec2(1, 1));

			vertices.clear();
			vertices.push_back(vec2(x2, -y2));
			vertices.push_back(vec2(x2 + w, -y2));
			vertices.push_back(vec2(x2, -y2 - h));
			vertices.push_back(vec2(x2 + w, -y2 - h));

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

