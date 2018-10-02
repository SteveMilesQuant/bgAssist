
#pragma once

#include <font.hpp>

#include <string>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

class textBox {
public:
	vec2 upperLeftCornerLocation;
	font * textFont;
	GLfloat textHeight;
	vec4 textColor;
	GLboolean isEditableFlag;

	textBox();
	~textBox();
	textBox(const textBox & inTextBox) { copyTextBox(inTextBox); }
	textBox(const textBox && inTextBox) { copyTextBox(inTextBox); }
	textBox & operator = (const textBox & inTextBox) { copyTextBox(inTextBox); return *this; }

	void setProgramId(GLuint inProgramId);
	void setText(string inText);
	void setBoxWidth(GLfloat inBoxWidth);
	// TODO: create setFont, setTextHeight

	void draw();
	void callGlfwCharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods);
	void callGlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

private:
	string text;
	GLfloat boxWidth;
	vector<int> lineBreakIndices; // line breaks as indices in text

	vector<vec2> vertices;
	vector<vec2> uvs;
	GLuint vertexBufferId;
	GLuint uvBufferId;
	GLboolean buffersPassedFlag;

	GLuint programId;
	GLuint textureId;
	GLuint textColorId;

	GLFWcursor* cursor; // GLFW cursor object
	int cursorIndex; // index of the cursour within the string ("before" this index in the string)

	void passBuffersToGLM();
	void copyTextBox(const textBox & inTextBox);
	vec2 drawOneChar(char inChar, vec2 upperLeftCorner); // returns the upper right corner
	void analyzeText(int startAtIndex, GLboolean forDeletionFlag);
	void setCursorIndex(int inCursorIndex);
};

