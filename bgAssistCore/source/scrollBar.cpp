#include <stdafx.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <scrollBar.hpp>

#include <texture.hpp>
#include <glfwExt.hpp>

#include <math.h>

scrollBar::scrollBar() {
	barColor = vec4(1);
	scrollRelativeBarJump = 0;

	upperLeftCornerLocation = vec2(0, 0);
	dimensions = vec2(0, 0);
	barRelativePosition = 0.0f;
	barRelativeLength = 1.0f;

	programId = -1;
	textureUniformId = -1;
	textColorUniformId = -1;
	
	vertices.clear();
	vertices.resize(4);
	uvs.clear();
	uvs.resize(4);
	vertexBufferId = -1;
	uvBufferId = -1;
	buffersPassedFlag = false;

	updateVerticesFlag = true;

	barImageId = -1;
	imageCopiedFlag = false;

	draggingFlag = false;
	dragLocationBegin = vec2(0);
	dragBarPositionBegin = 0;
}

void scrollBar::copyScrollBar(const scrollBar & inScrollBar) {
	if (this == &inScrollBar) return;

	barColor = inScrollBar.barColor;
	scrollRelativeBarJump = inScrollBar.scrollRelativeBarJump;

	upperLeftCornerLocation = inScrollBar.upperLeftCornerLocation;
	dimensions = inScrollBar.dimensions;
	barRelativePosition = inScrollBar.barRelativePosition;
	barRelativeLength = inScrollBar.barRelativeLength;

	programId = inScrollBar.programId;
	textureUniformId = inScrollBar.textureUniformId;
	textColorUniformId = inScrollBar.textColorUniformId;

	vertices.clear();
	vertices.resize(4);
	uvs.clear();
	uvs.resize(4);
	vertexBufferId = -1;
	uvBufferId = -1;
	buffersPassedFlag = false;

	updateVerticesFlag = true;

	barImageId = inScrollBar.barImageId;
	imageCopiedFlag = true;

	draggingFlag = false;
	dragLocationBegin = vec2(0);
	dragBarPositionBegin = 0;
}

scrollBar::~scrollBar() {
	if (!imageCopiedFlag) glDeleteTextures(1, &barImageId);
}

void scrollBar::setProgramId(GLuint inProgramId) {
	programId = inProgramId;
	textureUniformId = glGetUniformLocation(inProgramId, "textImageTexture");
	textColorUniformId = glGetUniformLocation(inProgramId, "textColor");
}

void scrollBar::setLocation(vec2 inUpperLeftCornerLocation) {
	upperLeftCornerLocation = inUpperLeftCornerLocation;
	updateVerticesFlag = true;
}

void scrollBar::setDimensions(vec2 inDim) {
	dimensions = inDim;
	updateVerticesFlag = true;
}

// set position on [0,1-barLength] (0 at top)
void scrollBar::setBarRelativePosition(GLfloat inPosition) {
	barRelativePosition = fmin(fmax(inPosition, 0.0f), 1.0f- barRelativeLength);
	updateVerticesFlag = true;
}

// relative length on [0,1] (1 for full length)
void scrollBar::setBarRelativeLength(GLfloat inLength) {
	barRelativeLength = inLength;
	setBarRelativePosition(barRelativePosition); // length may impact validity of position
}


void scrollBar::loadImage(const char * imagePath) {
	if (!imageCopiedFlag) glDeleteTextures(1, &barImageId);
	barImageId = loadDDS(imagePath);
	imageCopiedFlag = false;
}

void scrollBar::fillVerticesAndUvs(GLboolean verticesOnlyFlag) {
	vec2 barUpperLeftCorner = upperLeftCornerLocation - vec2(0, barRelativePosition*dimensions.y);
	vec2 barDim = vec2(dimensions.x, -dimensions.y * barRelativeLength); // negative y to make math simpler below

	vertices[0] = barUpperLeftCorner;
	vertices[1] = barUpperLeftCorner + vec2(barDim.x, 0);
	vertices[2] = barUpperLeftCorner + vec2(0, barDim.y);
	vertices[3] = barUpperLeftCorner + barDim;

	if (!verticesOnlyFlag) {
		uvs[0] = vec2(0, 0);
		uvs[1] = vec2(1, 0);
		uvs[2] = vec2(0, 1);
		uvs[3] = vec2(1, 1);
	}
}

void scrollBar::passBuffersToGLM() {
	if (buffersPassedFlag) return;

	fillVerticesAndUvs(false);

	glUseProgram(programId);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec2), &vertices[0], GL_DYNAMIC_DRAW);

	glGenBuffers(1, &uvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);

	buffersPassedFlag = true;
	updateVerticesFlag = false;
}

void scrollBar::updateVertices() {
	if (!updateVerticesFlag) return;

	fillVerticesAndUvs(true);
	glUseProgram(programId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec2), &vertices[0]);

	updateVerticesFlag = false;
}



void scrollBar::draw() {
	if (barRelativeLength >= 1.0f) return; // don't draw if we're not really scrolling

	passBuffersToGLM();
	updateVertices();

	glUseProgram(programId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, barImageId);
	glUniform1i(textureUniformId, 0);
	glUniform4f(textColorUniformId, barColor.r, barColor.g, barColor.b, barColor.a);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer : texture
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}


// Scroll up or down is a shift in the bar
void scrollBar::callGlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	GLfloat newBarPosition = barRelativePosition - (GLfloat)yoffset * scrollRelativeBarJump;
	setBarRelativePosition(newBarPosition);
}


// Click above or below: scroll shift
// Click on bar: start drag
void scrollBar::callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	// On release, just stop dragging (we don't care where we released)
	if (action == GLFW_RELEASE) {
		draggingFlag = false;
		return;
	}

	vec2 clickPosition_world = screenPosTo2DCoord(window);
	double topOfBar = upperLeftCornerLocation.y - barRelativePosition * dimensions.y;
	double bottomOfBar = topOfBar - barRelativeLength * dimensions.y;

	if (clickPosition_world.y > topOfBar) {
		callGlfwScrollCallback(window, 0, 1);
	}
	else if (clickPosition_world.y < bottomOfBar) {
		callGlfwScrollCallback(window, 0, -1);
	}
	else {
		draggingFlag = true;
		dragLocationBegin = clickPosition_world;
		dragBarPositionBegin = barRelativePosition;
	}
}


// Drag the scroll bar: just keep up with the mouse
void scrollBar::callGlfwCursorPosCallback(GLFWwindow* window, double x, double y) {
	if (!draggingFlag) return;

	vec2 cursorPosition_world = screenPosTo2DCoord(window, x, y);
	GLfloat y_change = cursorPosition_world.y - dragLocationBegin.y;

	GLfloat newBarPosition = dragBarPositionBegin - y_change / dimensions.y;
	setBarRelativePosition(newBarPosition);
}

// We don't do anything for text input in the scroll bar
void scrollBar::callGlfwCharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods) {
	return;
}

// Up key or down key is a scroll
void scrollBar::callGlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;

	int scrollDirection = 0;
	switch(key) {
		case GLFW_KEY_UP: scrollDirection = 1; break;
		case GLFW_KEY_DOWN: scrollDirection = -1; break;
	}
	if (scrollDirection) callGlfwScrollCallback(window, 0, scrollDirection);
}

// If they click off the scroll bar, make sure we don't continue dragging
void scrollBar::deselect() {
	draggingFlag = false;
}

// Test whether a point is in the scroll bar's bounds
GLboolean scrollBar::testPointInBounds(vec2 testPoint) {
	return testPointInBox(testPoint, upperLeftCornerLocation, dimensions);
}


