
#pragma once

#include <font.hpp>
#include <scrollBar.hpp>

#include <string>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

class textBox {
public:
	font * textFont;
	GLboolean isEditableFlag;
	GLfloat cursorWidth;
	double cursorToggleTime;

	textBox();
	~textBox();
	textBox(const textBox & inTextBox) { copyTextBox(inTextBox); }
	textBox(const textBox && inTextBox) { copyTextBox(inTextBox); }
	textBox & operator = (const textBox & inTextBox) { copyTextBox(inTextBox); return *this; }

	void setTextProgramId(GLuint inProgramId); // image shader program
	void setCursorProgramId(GLuint inProgramId); // solid color shader program
	void setText(string inText);
	void setTextColor(vec4 inColor);
	void setBoxLocation(vec2 inUpperLeftCornerLocation);
	void setBoxDimensions(vec2 inBoxDim); // zero or negative means unfettered
	void setScrollBarWidth(GLfloat inWidth);
	void loadScrollBarImage(const char * imagePath) { scrollBar.loadImage(imagePath); }
	void setTextHeight(GLfloat inHeight);
	// TODO: create setFont

	void draw();
	void callGlfwCharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods);
	void callGlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void callGlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
	string text;
	vec4 textColor;
	GLfloat textHeight;
	vec2 upperLeftCornerLocation;
	vec2 boxDimensions;
	GLfloat boxEffectiveWidth; // width minus the scroll bar
	vector<int> lineBreakIndices; // line breaks as indices in text

	vector<vec2> vertices;
	vector<vec2> uvs;
	vector<vec2> cursorVertices;
	GLuint vertexBufferId;
	GLuint uvBufferId;
	GLuint cursorVertexBufferId;
	GLboolean buffersPassedFlag;

	GLuint textProgramId;
	GLuint textureUniformId;
	GLuint textColorUniformId;

	GLuint cursorProgramId;
	GLuint cursorColorUniformId;

	int cursorIndex; // index of the cursour within the string ("before" this index in the string)
	GLboolean drawCursorFlag; // flag on whether we're currently drawing the cursor
	double cursorLastToggledTime;
	GLfloat cursorXCoord_textBoxSpace;
	int cursorRowIdx;

	scrollBar scrollBar;

	void passBuffersToGLM();
	void copyTextBox(const textBox & inTextBox);
	vec2 drawOneChar(char inChar, vec2 upperLeftCorner); // returns the upper right corner
	void drawCursor(vec2 topLocation);
	void analyzeText(int startAtIndex, GLboolean forDeletionFlag);
	void analyzeScrollBar();
	void setCursorIndex(int inCursorIndex);
	vec2 calcEffectiveLocation();
};

