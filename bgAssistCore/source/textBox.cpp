#include <stdafx.h>

#include <textBox.hpp>

textBox::textBox() {
	text.clear();
	upperLeftCornerLocation = vec2(0,0);
	textFont = NULL;
	textHeight = 0;
	textColor = vec4(1);

	vertices.clear();
	vertices.resize(4);
	uvs.clear();
	uvs.resize(4);
	vertexBufferId = -1;
	uvBufferId = -1;
	buffersPassedFlag = false;

	programId = -1;
	textureId = -1;
	textColorId = -1;
}

void textBox::copyTextBox(const textBox & inTextBox) {
	if (this == &inTextBox) return;

	text = inTextBox.text;
	upperLeftCornerLocation = inTextBox.upperLeftCornerLocation;
	textFont = inTextBox.textFont;
	textHeight = inTextBox.textHeight;
	textColor = inTextBox.textColor;

	vertices.clear();
	vertices.resize(4);
	uvs.clear();
	uvs.resize(4);
	vertexBufferId = -1;
	uvBufferId = -1;
	buffersPassedFlag = false;

	programId = inTextBox.programId;
	textureId = inTextBox.textureId;
	textColorId = inTextBox.textColorId;
}

textBox::~textBox() {
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteBuffers(1, &uvBufferId);
}

void textBox::passBuffersToGLM() {
	if (buffersPassedFlag) return;
	vec2 zero(0,0);

	// Just need to fill it with anything for now
	// Each char will overwrite this later, anyway
	for (int i = 0; i < 4; i++) {
		uvs[i] = zero;
		vertices[i] = zero;
	}
	
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec2), &vertices[0], GL_DYNAMIC_DRAW);

	glGenBuffers(1, &uvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_DYNAMIC_DRAW);

	buffersPassedFlag = true;
}

void textBox::setProgramId(GLuint inProgramId) {
	programId = inProgramId;
	textureId = glGetUniformLocation(inProgramId, "textShadingTexture");
	textColorId = glGetUniformLocation(inProgramId, "textColor");
}

void textBox::draw() {
	glUseProgram(programId);

	passBuffersToGLM();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textFont->getFontImageId());
	glUniform1i(textureId, 0);
	glUniform4f(textColorId, textColor.r, textColor.g, textColor.b, textColor.a);

	vec2 corner = upperLeftCornerLocation;
	for (char& c : text) corner = drawOneChar(c, corner);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

vec2 textBox::drawOneChar(char inChar, vec2 upperLeftCorner) {
	float charWidth = textFont->getCharUnitWidth(inChar) * textHeight;

	textFont->setUvs(inChar, uvs);

	vec2 upperRightCorner = upperLeftCorner + vec2(charWidth, 0);
	vertices[0] = upperLeftCorner;
	vertices[1] = upperRightCorner;
	vertices[2] = upperLeftCorner + vec2(0, -textHeight);
	vertices[3] = upperRightCorner + vec2(0, -textHeight);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec2), &vertices[0]);

	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, uvs.size() * sizeof(vec2), &uvs[0]);

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

	return upperRightCorner;
}
