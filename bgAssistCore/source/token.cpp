
#include <stdafx.h>

#include <token.hpp>

GLfloat defaultTokenRadius = 12.5f;
GLfloat defaultTokenThickness = 2.0f;

// Constructors
token::token(int inNsides) {
	token(inNsides, defaultTokenThickness, defaultTokenRadius);
}
token::token(int inNSides, GLfloat inRelativeThickness) {
	token(inNSides, inRelativeThickness, defaultTokenRadius);
}
token::token(int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius) {
	GLfloat unitLen = getGlobalTileUnitLength();
	relativeThickness = inRelativeThickness;
	relativeRadius = inRelativeRadius;
	GLfloat radius = inRelativeRadius / unitLen;
	tokenPrism = prismTop(inNSides);
	tokenPrism.setScale(vec3(radius, radius, relativeThickness/ unitLen));
	setLocation(vec2(0, 0));
	tokenPrism.setUvCenter(vec2(0.5, 0.5));
	tokenPrism.setUvScale(vec2(0.5, 0.5));
	parentTile = NULL;

	// In the builder, we may move images around
#ifdef BGASSIST_BUILDER_WORLD
	tokenPrism.passBuffersToGLM(GL_DYNAMIC_DRAW);
#else
	tokenPrism.passBuffersToGLM(GL_STATIC_DRAW);
#endif
}

// Destructor
token::~token() {
	// Nothing to do right now
}

// Copy constructor
token::token(const token &inToken) {
	relativeRadius = inToken.relativeRadius;
	relativeThickness = inToken.relativeThickness;
	tokenPrism = inToken.tokenPrism;
}

// Set location (in xy plane only)
void token::setLocation(vec2 location) {
	tokenPrism.setTranslation(vec3(location, 0.5*relativeThickness / getGlobalTileUnitLength())); // bottom level with the xy plane
}
