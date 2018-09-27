
#pragma once

#include <vector>
using namespace std;

#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

class font {
public:
	font();
	~font();
	font(const font & inFont) { copyFont(inFont); }
	font(const font && inFont) { copyFont(inFont); }
	font & operator = (const font & inFont) { copyFont(inFont); return *this; }

	void loadFont(const char * fontImagePath, const char * fontMetadataPath);
	void setUvs(char inChar, vector<vec2> &outUvs);
	float getCharUnitWidth(char inChar) { return charTrueWidths[inChar - startCharInt]; }
	GLuint getFontImageId() { return fontImageId; }

private:
	GLuint fontImageId;
	int imageWidthInCells;
	int imageHeightInCells;
	vector<float> charTrueWidths;
	int startCharInt;
	GLboolean imageCopiedFlag;
	void copyFont(const font & inFont);
};

