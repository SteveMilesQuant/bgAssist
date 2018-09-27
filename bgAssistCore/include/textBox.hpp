
#pragma once

#include <font.hpp>

#include <string>
using namespace std;

#include <glm/gtx/transform.hpp>
using namespace glm;

class textBox {
public:
	string text;
	vec2 upperLeftCornerLocation;
	font * textFont;
	GLfloat textHeight;
	vec4 textColor;

	textBox();
	~textBox();
	textBox(const textBox & inTextBox) { copyTextBox(inTextBox); }
	textBox(const textBox && inTextBox) { copyTextBox(inTextBox); }
	textBox & operator = (const textBox & inTextBox) { copyTextBox(inTextBox); return *this; }

	void setProgramId(GLuint inProgramId);

	void draw();

private:
	vector<vec2> vertices;
	vector<vec2> uvs;
	GLuint vertexBufferId;
	GLuint uvBufferId;
	GLboolean buffersPassedFlag;

	GLuint programId;
	GLuint textureId;
	GLuint textColorId;

	void passBuffersToGLM();
	void copyTextBox(const textBox & inTextBox);
	vec2 drawOneChar(char inChar, vec2 upperLeftCorner); // returns the upper right corner
};

