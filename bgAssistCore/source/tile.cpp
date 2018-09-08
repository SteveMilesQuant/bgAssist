
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

// Constructor
GLfloat defaultTileThickness = 2.0f;
tile::tile(ivec2 inDimensions) {
	tile(inDimensions, defaultTileThickness);
}
tile::tile(ivec2 inDimensions, GLfloat inRelativeThickness) {
	ivec2 tmpDimensions = ivec2(max(inDimensions[0], 1), max(inDimensions[1], 1));
	dimensions[0] = min(tmpDimensions[0], tmpDimensions[1]);
	dimensions[1] = max(tmpDimensions[0], tmpDimensions[1]);
	relativeThickness = inRelativeThickness;
	rectPrism = prismTop(4);
	rectPrism.setScale(vec3((GLfloat)dimensions.x, (GLfloat)dimensions.y, 0.5*relativeThickness/ getGlobalTileUnitLength()));
	setLocation(vec2(0, 0));
	rectPrism.setUvCenter(vec2(0.5, 0.5));
	GLfloat scale = sqrt(2.0f) / 4.0f;
	rectPrism.setUvScale(vec2(scale, scale));

	// In the builder, we may move images around
#ifdef BGASSIST_BUILDER_WORLD
	rectPrism.passBuffersToGLM(GL_DYNAMIC_DRAW);
#else
	rectPrism.passBuffersToGLM(GL_STATIC_DRAW);
#endif
}

// Destructor
tile::~tile(){
	// Nothing to do right now
}

// Copy constructor
tile::tile(const tile &inTile) {
	dimensions = inTile.dimensions;
	rectPrism = inTile.rectPrism;
}

// Set location (in xy plane only)
void tile::setLocation(vec2 location) {
	rectPrism.setTranslation(vec3(location, -0.5*relativeThickness/ getGlobalTileUnitLength())); // top level with the xy plane
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

