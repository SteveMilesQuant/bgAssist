#include <stdafx.h>

#include <font.hpp>
#include <texture.hpp>

#include <fstream>
#include <string>
using namespace std;

font::font() { 
	fontImageId = -1;
	imageCopiedFlag = false;
	imageWidthInCells = 0;
	imageHeightInCells = 0;
	charTrueWidths.clear();
	startCharInt = 0;
}

void font::copyFont(const font & inFont) {
	if (this == &inFont) return;

	fontImageId = inFont.fontImageId;
	imageCopiedFlag = true;
	imageWidthInCells = inFont.imageWidthInCells;
	imageHeightInCells = inFont.imageHeightInCells;
	charTrueWidths = inFont.charTrueWidths;
	startCharInt = inFont.startCharInt;
}

font::~font() {
	if (!imageCopiedFlag) glDeleteTextures(1, &fontImageId);
}

void font::loadFont(const char * fontImagePath, const char * fontMetadataPath) {
	fontImageId = loadDDS(fontImagePath);
	imageCopiedFlag = false;

	ifstream file(fontMetadataPath, ifstream::in);

	imageWidthInCells = 0;
	startCharInt = 0;
	charTrueWidths.clear();

	if (file.is_open()) {
		GLboolean firstArgFlag = true;
		GLboolean spaceReservedFlag = false;
		string argName, argValueString;
		int imageWidth(0), imageHeight(0), cellWidth(0), cellHeight(0);
		float fontHeight(0);


		while (!file.eof()) {
			char delim = (firstArgFlag) ? ',' : '\n';

			if (firstArgFlag) {
				getline(file, argName, delim);
			}
			else {
				getline(file, argValueString, delim);
				if (argName == "Image Width") imageWidth = stoi(argValueString);
				else if (argName == "Image Height") imageHeight = stoi(argValueString);
				else if (argName == "Cell Width") cellWidth = stoi(argValueString);
				else if (argName == "Cell Height") cellHeight = stoi(argValueString);
				else if (argName == "Font Height") fontHeight = (float)stoi(argValueString);
				else if (argName == "Start Char") startCharInt = stoi(argValueString);
				else if (argName.find("Base Width") < argName.npos) {
					int endIdx = (int)argName.find(' ', 5);
					string charIdxString = argName.substr(5, endIdx - 5);
					int charIdx = stoi(charIdxString);
					charTrueWidths[charIdx] = (float)stoi(argValueString) / fontHeight;
				}
			}

			if (imageWidthInCells == 0 && imageWidth > 0 && cellWidth > 0) imageWidthInCells = imageWidth / cellWidth;
			if (imageHeightInCells == 0 && imageHeight > 0 && cellHeight > 0) imageHeightInCells = imageHeight / cellHeight;

			if (!spaceReservedFlag && imageWidthInCells > 0 && imageHeightInCells > 0) {
				charTrueWidths.resize(imageWidthInCells*imageHeightInCells);
				spaceReservedFlag = true;
			}

			// Toggle flag: switch between comma separated and newline
			firstArgFlag = !firstArgFlag;
		}
	}
	file.close();
}



void font::setUvs(char inChar, vector<vec2> &outUvs) {
	int writeLetter = (int)inChar - startCharInt;

	int imageRow = writeLetter / imageWidthInCells;
	int imageCol = writeLetter % imageWidthInCells;

	vec2 upperLeft((float)imageCol / imageWidthInCells, (float)imageRow / imageWidthInCells);

	GLfloat charHeight = 1.0f / imageHeightInCells;
	GLfloat charWidth = charTrueWidths[writeLetter] / imageWidthInCells;

	if (outUvs.size() < 4) outUvs.resize(4);
	outUvs[0] = upperLeft;
	outUvs[1] = upperLeft + vec2(charWidth, 0);
	outUvs[2] = upperLeft + vec2(0, charHeight);
	outUvs[3] = upperLeft + vec2(charWidth, charHeight);
}

