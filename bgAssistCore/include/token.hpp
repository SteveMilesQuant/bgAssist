
#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <prismTop.hpp>
#include <tile.hpp>

class tile;

class token {
public:

	// Constructor, desctructor, copy constructor
	token(GLboolean builderWorldFlag, int inNSides);
	token(GLboolean builderWorldFlag, int inNSides, GLfloat inRelativeThickness);
	token(GLboolean builderWorldFlag, int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius);
	~token();
	token(const token & inToken);

	// Set relative thickness and radius
	// Relative to getGlobalTileUnitLength() in tile.hpp
	void setRelativeThickness(GLfloat inRelativeThickness);
	void setRelativeRadius(GLfloat inRelativeRadius);
	void setThickness(GLfloat inThickness);
	void setRadius(GLfloat inRadius);

	// Set location (in xy plane only)
	void setLocation(vec2 location);

	// Accessor functions to underlying prism
	void setProgramId(GLuint inProgramId) { tokenPrism.setProgramId(inProgramId); }
	void setCamera(timedMat4 *inCamera) { tokenPrism.setCamera(inCamera); }
	void setProjection(timedMat4 *inProjection) { tokenPrism.setProjection(inProjection); }
	void setLight(lightSource * inLight) { tokenPrism.setLight(inLight); }
	void setAmbientRatio(GLfloat inAmbientRatio) { tokenPrism.setAmbientRatio(inAmbientRatio); }
	void setSpecularRatio(GLfloat inSpecularRatio) { tokenPrism.setSpecularRatio(inSpecularRatio); }
	void setRotation(GLfloat inRadians, vec3 inAxis) { tokenPrism.setRotation(inRadians, inAxis); }
	void loadFaceImage(const char * imagepath) { tokenPrism.loadFaceImage(imagepath, true); }
	void loadSideImage(const char * imagepath) { tokenPrism.loadSideImage(imagepath, true); }
	void draw() { tokenPrism.draw(); }
	void setGlfwCursorPosCallback(void(*inFunc)(GLFWwindow* window, double x, double y)) { tokenPrism.glfwCursorPosCallback = inFunc; }
	void setGlfwMouseButtonCallback(void(*inFunc)(GLFWwindow* window, int button, int action, int mods)) { tokenPrism.glfwMouseButtonCallback = inFunc; }

	// Parent tile
	tile * parentTile;
private:
	prismTop tokenPrism;
	GLfloat thickness;
	GLfloat radius;

	void constructToken(GLboolean builderWorldFlag, int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius);
};



#endif

