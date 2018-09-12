
#ifndef TILE_HPP
#define TILE_HPP

#include <prismTop.hpp>
#include <token.hpp>

#include <list>
using namespace std;

class token;

void setGlobalTileUnitLength(GLfloat inLength);
GLfloat getGlobalTileUnitLength();

class tile {
public:

	// Constructor, desctructor, copy constructor
	tile(GLboolean builderWorldFlag, ivec2 inDimensions);
	tile(GLboolean builderWorldFlag, ivec2 inDimensions, GLfloat inRelativeThickness);
	~tile();
	tile(const tile &inTile);

	// Set location (in xy plane only)
	void setLocation(vec2 location);

	// Accessor functions to underlying prism
	void setProgramId(GLuint inProgramId) { rectPrism.setProgramId(inProgramId); }
	void setCamera(timedMat4 *inCamera) { rectPrism.setCamera(inCamera); }
	void setProjection(timedMat4 *inProjection) { rectPrism.setProjection(inProjection); }
	void setLight(lightSource * inLight) { rectPrism.setLight(inLight); }
	void setAmbientRatio(GLfloat inAmbientRatio) { rectPrism.setAmbientRatio(inAmbientRatio); }
	void setSpecularRatio(GLfloat inSpecularRatio) { rectPrism.setSpecularRatio(inSpecularRatio); }
	void loadFaceImage(const char * imagepath) { rectPrism.loadFaceImage(imagepath, true); }
	void loadSideImage(const char * imagepath) { rectPrism.loadSideImage(imagepath, true); }
	void draw() { rectPrism.draw(); }
	void setGlfwCursorPosCallback(void(*inFunc)(GLFWwindow* window, double x, double y)) { rectPrism.glfwCursorPosCallback = inFunc; }
	void setGlfwMouseButtonCallback(void(*inFunc)(GLFWwindow* window, int button, int action, int mods)) { rectPrism.glfwMouseButtonCallback = inFunc; }

	// Test ray intersection
	GLboolean testRayOBBIntersection(vec3 ray_origin, vec3 ray_direction);

	// List of associated tokens
	list<token *> tokenList;

private:
	ivec2 tileDimensions;
	GLfloat relativeThickness;
	prismTop rectPrism;

	// Constructor
	void constructTile(GLboolean builderWorldFlag, ivec2 inDimensions, GLfloat inRelativeThickness);
};




#endif

