
#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <prismTop.hpp>
#include <tile.hpp>

#include <list>
using namespace std;

class tile;

class token {
public:

	// Constructor, desctructor, copy constructor
	token();
	token(int inNSides);
	token(int inNSides, GLfloat inRelativeThickness);
	token(int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius);
	~token();
	token(const token & inToken) { copyToken(inToken); }
	token(const token && inToken) { copyToken(inToken); }
	token & operator = (const token & inToken) { copyToken(inToken); return *this; }

	// Set relative thickness and radius
	// Relative to getGlobalTileUnitLength() in tile.hpp
	void setRelativeThickness(GLfloat inRelativeThickness);
	void setRelativeRadius(GLfloat inRelativeRadius);
	void setThickness(GLfloat inThickness);
	void setRadius(GLfloat inRadius);
	void setDesignTokenFlag(GLboolean inDesignTokenFlag) { designTokenFlag = inDesignTokenFlag; }
	GLfloat getThickness() { return thickness; }
	GLfloat getRadius() { return radius; }
	void setParentToken(token * inParent);
	token * getParentToken() { return parentToken; }
	void removeParentToken();
	void setParentTile(tile * inParent);
	tile * getParentTile() { return parentTile; }
	void removeParentTile();
	void addChild(token * inChild);
	void removeChild(token * inChild);

	// Set location (in xy plane only)
	void setLocation(vec2 location);

	// Accessor functions to underlying prism
	void setNSides(int inNSides); // should also set sides for children
	void setProgramId(GLuint inProgramId) { tokenPrism.setProgramId(inProgramId); }
	void setFaceImageTransientFlag(GLboolean inFaceImageTransientFlag) { tokenPrism.setFaceImageTransientFlag(inFaceImageTransientFlag); }
	void setCamera(timedMat4 *inCamera) { tokenPrism.setCamera(inCamera); }
	void setProjection(timedMat4 *inProjection) { tokenPrism.setProjection(inProjection); }
	void setLight(lightSource * inLight) { tokenPrism.setLight(inLight); }
	void setAmbientRatio(GLfloat inAmbientRatio) { tokenPrism.setAmbientRatio(inAmbientRatio); }
	void setSpecularRatio(GLfloat inSpecularRatio) { tokenPrism.setSpecularRatio(inSpecularRatio); }
	void setRotation(GLfloat inRadians, vec3 inAxis) { tokenPrism.setRotation(inRadians, inAxis); }
	timedMat4 & getCamera() { return tokenPrism.getCamera(); }
	timedMat4 & getProjection() { return tokenPrism.getProjection(); }
	mat4 & getModelMatrix() { return tokenPrism.getModelMatrix(); }
	vec3 & getMaxCoords() { return tokenPrism.getMaxCoords(); }
	vec3 & getMinCoords() { return tokenPrism.getMinCoords(); }
	vec2 & getUvScale() { return tokenPrism.getUvScale(); }
	vec2 & getUvCenter() { return tokenPrism.getUvCenter(); }
	void loadFaceImage(const char * imagepath) { tokenPrism.loadFaceImage(imagepath, true); }
	void loadSideImage(const char * imagepath) { tokenPrism.loadSideImage(imagepath, true); }
	void draw() { tokenPrism.draw(); }
	void setGlfwCursorPosCallback(void(*inFunc)(GLFWwindow* window, double x, double y)) { tokenPrism.glfwCursorPosCallback = inFunc; }
	void setGlfwMouseButtonCallback(void(*inFunc)(GLFWwindow* window, int button, int action, int mods)) { tokenPrism.glfwMouseButtonCallback = inFunc; }
	void setGlfwKeyCallback(void(*inFunc)(GLFWwindow* window, int key, int scancode, int action, int mods)) { tokenPrism.glfwKeyCallback = inFunc; }
	void callGlfwCursorPosCallback(GLFWwindow* window, double x, double y) { if (tokenPrism.glfwCursorPosCallback) tokenPrism.glfwCursorPosCallback(window, x, y);  }
	void callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) { if (tokenPrism.glfwMouseButtonCallback) tokenPrism.glfwMouseButtonCallback(window, button, action, mods); }
	void callGlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) { if (tokenPrism.glfwKeyCallback) tokenPrism.glfwKeyCallback(window, key, scancode, action, mods); }
	void dragFaceImageBegin() { tokenPrism.dragFaceImageBegin(); }
	void dragFaceImage(vec2 shiftFromStart) { tokenPrism.dragFaceImage(shiftFromStart); }
	void copyFaceImageUvs(const token &inToken);

private:
	tile * parentTile; // parent tile
	token * parentToken; // parent token
	list<token *> childTokens; // child tokens

	GLfloat thickness;
	GLfloat radius;
	prismTop tokenPrism;
	GLboolean designTokenFlag;

	void constructToken(int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius);
	void copyToken(const token & inToken);
};



#endif

