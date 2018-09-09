
#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <prismTop.hpp>
#include <tile.hpp>

class tile;

class token {
public:

	// Constructor, desctructor, copy constructor
	token(int inNsides);
	token(int inNSides, GLfloat inRelativeThickness);
	token(int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius);
	~token();
	token(const token & inToken);

	// Set location (in xy plane only)
	void setLocation(vec2 location);

	// Accessor functions to underlying prism
	void setProgramId(GLuint inProgramId) { tokenPrism.setProgramId(inProgramId); }
	void setCamera(timedMat4 *inCamera) { tokenPrism.setCamera(inCamera); }
	void setProjection(timedMat4 *inProjection) { tokenPrism.setProjection(inProjection); }
	void setLight(lightSource * inLight) { tokenPrism.setLight(inLight); }
	void setAmbientRatio(GLfloat inAmbientRatio) { tokenPrism.setAmbientRatio(inAmbientRatio); }
	void setSpecularRatio(GLfloat inSpecularRatio) { tokenPrism.setSpecularRatio(inSpecularRatio); }
	void loadFaceImage(const char * imagepath) { tokenPrism.loadFaceImage(imagepath, true); }
	void loadSideImage(const char * imagepath) { tokenPrism.loadSideImage(imagepath, true); }
	void draw() { tokenPrism.draw(); }
	void setGlfwCursorPosCallback(void(*inFunc)(GLFWwindow* window, double x, double y)) { tokenPrism.glfwCursorPosCallback = inFunc; }
	void setGlfwMouseButtonCallback(void(*inFunc)(GLFWwindow* window, int button, int action, int mods)) { tokenPrism.glfwMouseButtonCallback = inFunc; }

	// Parent tile
	tile * parentTile;
private:
	prismTop tokenPrism;
	GLfloat relativeThickness;
	GLfloat relativeRadius;
};



#endif

