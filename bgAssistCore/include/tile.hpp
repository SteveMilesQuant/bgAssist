
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
	tile(ivec2 inDimensions);
	tile(ivec2 inDimensions, GLfloat inRelativeThickness);
	~tile();
	tile(const tile &inTile);

	// Set location (in xy plane only)
	void setLocation(vec2 location);

	// Accessor functions to underlying prism
	void setCamera(timedMat4 *inCamera) { rectPrism.setCamera(inCamera); }
	void setProjection(timedMat4 *inProjection) { rectPrism.setProjection(inProjection); }
	void setMatrixId(GLuint inMatrixId) { rectPrism.setMatrixId(inMatrixId); }
	void setTextureId(GLuint inTextureId) { rectPrism.setTextureId(inTextureId); }
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
	ivec2 dimensions;
	GLfloat relativeThickness;
	prismTop rectPrism;
};




#endif

