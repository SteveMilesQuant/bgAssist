
#include <stdafx.h>

#include <tile.hpp>


// Unit length is used to scale everything
// Set it in millimeters, for example
GLfloat tileUnitLength = 88.0f;
void setGlobalTileUnitLength(GLfloat inLength) {
	tileUnitLength = max(inLength, sqrt(FLT_EPSILON));
}
GLfloat getGlobalTileUnitLength() {
	return tileUnitLength;
}

// Constructors
GLfloat defaultTileThickness = 5.0f;
void tile::constructTile(GLboolean builderWorldFlag, ivec2 inDimensions, GLfloat inRelativeThickness) {
	ivec2 tmpDimensions = ivec2(max(inDimensions[0], 1), max(inDimensions[1], 1));
	tileDimensions[0] = min(tmpDimensions[0], tmpDimensions[1]);
	tileDimensions[1] = max(tmpDimensions[0], tmpDimensions[1]);
	relativeThickness = inRelativeThickness;
	rectPrism.setNSides(4);
	setLocation(vec2(0, 0));
	rectPrism.setUvCenter(vec2(0.5, 0.5));

	GLfloat scale = 1.0f/sqrt(2.0f);
	rectPrism.setScale(vec3(scale*tileDimensions.x, scale*tileDimensions.y, 0.5f*relativeThickness / getGlobalTileUnitLength()));
	rectPrism.setUvScale(vec2(scale, scale));

	// In the builder, we may move images around
	if (builderWorldFlag) rectPrism.passBuffersToGLM(GL_DYNAMIC_DRAW);
	else rectPrism.passBuffersToGLM(GL_STATIC_DRAW);
}
tile::tile(GLboolean builderWorldFlag, ivec2 inDimensions) {
	constructTile(builderWorldFlag, inDimensions, defaultTileThickness);
}
tile::tile(GLboolean builderWorldFlag, ivec2 inDimensions, GLfloat inRelativeThickness) {
	constructTile(builderWorldFlag, inDimensions, inRelativeThickness);
}


// Destructor
tile::~tile(){
	// Nothing to do right now
}

// Copy constructor
tile::tile(const tile &inTile) {
	tileDimensions = inTile.tileDimensions;
	rectPrism = inTile.rectPrism;
	relativeThickness = inTile.relativeThickness;
}

// Set location (in xy plane only)
void tile::setLocation(vec2 location) {
	rectPrism.setTranslation(vec3(location, -0.5f*relativeThickness/ getGlobalTileUnitLength())); // top level with the xy plane
}

// Test an intersection with a ray
GLboolean tile::testRayOBBIntersection(vec3 ray_origin, vec3 ray_direction) {
	float intersection_distance; // unused
	return ::testRayOBBIntersection(ray_origin,
		ray_direction,
		rectPrism.getMinCoords(),
		rectPrism.getMaxCoords(),
		rectPrism.getModelMatrix(),
		intersection_distance);
}
