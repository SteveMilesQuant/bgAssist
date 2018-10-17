
#pragma once

#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

class scrollBar {
public:
	vec4 barColor;
	GLfloat scrollRelativeBarJump;

	scrollBar();
	~scrollBar();
	scrollBar(const scrollBar & inScrollBar) { copyScrollBar(inScrollBar); }
	scrollBar(const scrollBar && inScrollBar) { copyScrollBar(inScrollBar); }
	scrollBar & operator = (const scrollBar & inScrollBar) { copyScrollBar(inScrollBar); return *this; }

	void setProgramId(GLuint inProgramId);
	void setLocation(vec2 inUpperLeftCornerLocation);
	void setDimensions(vec2 inDim);
	void setBarRelativePosition(GLfloat inPosition); // set position on [0,1-barLength] (0 at top)
	void setBarRelativeLength(GLfloat inLength); // relative length on [0,1] (1 for full length)
	vec2 getDimensions() { return dimensions; }
	GLfloat getBarRelativePosition() { return barRelativePosition; }

	void loadImage(const char * imagePath);
	void draw();
	void callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void callGlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void callGlfwCursorPosCallback(GLFWwindow* window, double x, double y);

private:
	vec2 upperLeftCornerLocation;
	vec2 dimensions;
	GLfloat barRelativePosition;
	GLfloat barRelativeLength;

	GLuint programId;
	GLuint textureUniformId;
	GLuint textColorUniformId;

	vector<vec2> vertices;
	vector<vec2> uvs;
	GLuint vertexBufferId;
	GLuint uvBufferId;
	GLboolean buffersPassedFlag;

	GLboolean updateVerticesFlag;

	GLuint barImageId;
	GLboolean imageCopiedFlag;

	GLboolean draggingFlag; // are we dragging this right now?
	vec2 dragLocationBegin;
	GLfloat dragBarPositionBegin;

	void copyScrollBar(const scrollBar & inScrollBar);
	void fillVerticesAndUvs(GLboolean verticesOnlyFlag);
	void passBuffersToGLM();
	void updateVertices();
};

