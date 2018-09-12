
#include <stdafx.h>

#include <token.hpp>

GLfloat defaultTokenRadius = 12.5f;
GLfloat defaultTokenThickness = 2.0f;

// Constructors
void token::constructToken(GLboolean builderWorldFlag, int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius) {
	GLfloat unitLen = getGlobalTileUnitLength();
	thickness = inRelativeThickness / unitLen;
	radius = inRelativeRadius / unitLen;
	tokenPrism = prismTop(inNSides);
	tokenPrism.setScale(vec3(radius, radius, thickness));
	setLocation(vec2(0, 0));
	tokenPrism.setUvCenter(vec2(0.5, 0.5));
	tokenPrism.setUvScale(vec2(0.5, 0.5));
	parentTile = NULL;

	// In the builder, we may move images around
	if (builderWorldFlag) tokenPrism.passBuffersToGLM(GL_DYNAMIC_DRAW);
	else tokenPrism.passBuffersToGLM(GL_STATIC_DRAW);
}
token::token(GLboolean builderWorldFlag, int inNSides) {
	constructToken(builderWorldFlag, inNSides, defaultTokenThickness, defaultTokenRadius);
}
token::token(GLboolean builderWorldFlag, int inNSides, GLfloat inRelativeThickness) {
	constructToken(builderWorldFlag, inNSides, inRelativeThickness, defaultTokenRadius);
}
token::token(GLboolean builderWorldFlag, int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius) {
	constructToken(builderWorldFlag, inNSides, inRelativeThickness, inRelativeRadius);
}

// Destructor
token::~token() {
	// Nothing to do right now
}

// Copy constructor
token::token(const token &inToken) {
	radius = inToken.radius;
	thickness = inToken.thickness;
	tokenPrism = inToken.tokenPrism;
}

// Set relative thickness and radius
// Relative to getGlobalTileUnitLength() in tile.hpp
void token::setRelativeThickness(GLfloat inRelativeThickness) {
	GLfloat unitLen = getGlobalTileUnitLength();
	thickness = inRelativeThickness;
	tokenPrism.setScale(vec3(radius, radius, thickness));
}
void token::setRelativeRadius(GLfloat inRelativeRadius) {
	GLfloat unitLen = getGlobalTileUnitLength();
	radius = inRelativeRadius / unitLen;
	tokenPrism.setScale(vec3(radius, radius, thickness));
}

// Set absolute radius or thickness
void token::setThickness(GLfloat inThickness) {
	thickness = inThickness;
	tokenPrism.setScale(vec3(radius, radius, thickness));
}
void token::setRadius(GLfloat inRadius) {
	radius = inRadius;
	tokenPrism.setScale(vec3(radius, radius, thickness));
}

// Set location (in xy plane only)
void token::setLocation(vec2 location) {
	tokenPrism.setTranslation(vec3(location, 0.5*thickness)); // bottom level with the xy plane
}
