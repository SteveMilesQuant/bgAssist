
#include <stdafx.h>

#include <token.hpp>

GLfloat defaultTokenRadius = 12.5f;
GLfloat defaultTokenThickness = 2.0f;

// Constructors
void token::constructToken(int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius) {
	GLfloat unitLen = getGlobalTileUnitLength();
	thickness = inRelativeThickness / unitLen;
	radius = inRelativeRadius / unitLen;
	tokenPrism.setNSides(inNSides);
	tokenPrism.setScale(vec3(radius, radius, thickness));
	setLocation(vec2(0, 0));
	tokenPrism.setUvCenter(vec2(0.5, 0.5));
	tokenPrism.setUvScale(vec2(0.5, 0.5));
	parentTile = NULL;
	parentToken = NULL;
	childTokens.clear();
	designTokenFlag = false;
}
token::token() {
	constructToken(3, defaultTokenThickness, defaultTokenRadius);
}
token::token(int inNSides) {
	constructToken(inNSides, defaultTokenThickness, defaultTokenRadius);
}
token::token(int inNSides, GLfloat inRelativeThickness) {
	constructToken(inNSides, inRelativeThickness, defaultTokenRadius);
}
token::token(int inNSides, GLfloat inRelativeThickness, GLfloat inRelativeRadius) {
	constructToken(inNSides, inRelativeThickness, inRelativeRadius);
}

// Destructor
token::~token() {
	// If this is a design token, copy design points back to parent token before deleting it
	if (designTokenFlag && parentToken) {
		parentToken->setNSides(tokenPrism.getNSides());
		parentToken->copyFaceImageUvs(*this);
	}

	// Remove from parents
	removeParentToken();
	removeParentTile();
}

// Copy constructor
void token::copyToken(const token &inToken) {
	if (this == &inToken) return;

	// Public
	parentTile = NULL; // don't copy parent tile - we'll find our own tile
	parentToken = inToken.parentToken; // do copy parent token
	childTokens.clear(); // don't copy children

	// Private
	thickness = inToken.thickness;
	radius = inToken.radius;
	tokenPrism = inToken.tokenPrism;
	designTokenFlag = inToken.designTokenFlag;
}

// Set relative thickness and radius
// Relative to getGlobalTileUnitLength() in tile.hpp
void token::setRelativeThickness(GLfloat inRelativeThickness) {
	GLfloat unitLen = getGlobalTileUnitLength();
	thickness = inRelativeThickness / unitLen;
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

// Copy face image UVs and do so for all children (recursive)
void token::copyFaceImageUvs(const token &inToken) {
	tokenPrism.copyFaceImageUvs(inToken.tokenPrism);

	list<token *>::iterator childTokensIter = childTokens.begin();
	for (; childTokensIter != childTokens.end(); childTokensIter++) {
		(*childTokensIter)->copyFaceImageUvs(inToken);
	}
}


void token::setNSides(int inNSides) { 
	tokenPrism.setNSides(inNSides);

	list<token *>::iterator tokenIter = childTokens.begin();
	for (; tokenIter != childTokens.end(); tokenIter++) {
		(*tokenIter)->setNSides(inNSides);
	}
}


// Test an intersection with a ray
GLboolean token::testRayOBBIntersection(GLFWwindow* window, double xpos, double ypos) {
	int screenWidth, screenHeight;
	vec3 ray_origin, ray_direction;
	float intersection_distance; // unused

	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	screenPosToWorldRay(
		(int)xpos,
		screenHeight - (int)ypos,
		screenWidth, screenHeight,
		tokenPrism.getCamera().getMatrix(),
		tokenPrism.getProjection().getMatrix(),
		ray_origin,
		ray_direction
	);

	return testRayOBBIntersection(ray_origin, ray_direction);
}
GLboolean token::testRayOBBIntersection(vec3 ray_origin, vec3 ray_direction) {
	float intersection_distance; // unused
	return ::testRayOBBIntersection(ray_origin,
		ray_direction,
		tokenPrism.getMinCoords(),
		tokenPrism.getMaxCoords(),
		tokenPrism.getModelMatrix(),
		intersection_distance);
}


void token::setParentToken(token * inParent) {
	if (parentToken == inParent) return;
	removeParentToken();
	parentToken = inParent;
	if (parentToken) parentToken->addChild(this);
}

void token::removeParentToken() {
	if (!parentToken) return;
	parentToken->removeChild(this);
	parentToken = NULL;
}


void token::setParentTile(tile * inParent) {
	if (parentTile == inParent) return;
	removeParentTile();
	parentTile = inParent;
	if (parentTile) parentTile->addChildToken(this);
}

void token::removeParentTile() {
	if (!parentTile) return;
	parentTile->removeChildToken(this);
}


void token::removeChild(token * inChild) {
	list<token *>::iterator tokenIter = childTokens.begin();
	for (; tokenIter != childTokens.end(); tokenIter++) {
		if (*tokenIter == inChild) {
			childTokens.erase(tokenIter);
			break;
		}
	}
}

void token::addChild(token * inChild) {
	if (!inChild) return;
	removeChild(inChild);
	inChild->setParentToken(this);
	childTokens.push_back(inChild);
}


