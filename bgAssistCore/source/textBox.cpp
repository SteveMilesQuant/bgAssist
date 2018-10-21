#include <stdafx.h>

#include <textBox.hpp>
#include <glfwExt.hpp>

#include <algorithm>

textBox::textBox() {
	upperLeftCornerLocation = vec2(0,0);
	textFont = NULL;
	textHeight = 0;
	textColor = vec4(1);
	isEditableFlag = false;
	cursorWidth = 0;
	cursorToggleTime = 0.5;

	text.clear();
	boxDimensions = vec2(0, 0); // this means don't wrap text at all
	boxEffectiveWidth = 0;
	lineBreakIndices.clear();

	vertices.clear();
	vertices.resize(4);
	uvs.clear();
	uvs.resize(4);
	cursorVertices.clear();
	cursorVertices.resize(4);
	vertexBufferId = -1;
	uvBufferId = -1;
	cursorVertexBufferId = -1;
	buffersPassedFlag = false;

	textProgramId = -1;
	textureUniformId = -1;
	textColorUniformId = -1;
	highlightFlagUniformId = -1;

	cursorProgramId = -1;
	cursorColorUniformId = -1;

	cursorIndex = 0;
	dragCursorIndex = 0;
	drawCursorFlag = false;
	cursorLastToggledTime = glfwGetTime();
	cursorRowIdx = 0;

	// scrollBar has its own initializer
	isSelectedFlag = false;
	draggingFlag = false;
}

void textBox::copyTextBox(const textBox & inTextBox) {
	if (this == &inTextBox) return;

	upperLeftCornerLocation = inTextBox.upperLeftCornerLocation;
	textFont = inTextBox.textFont;
	textHeight = inTextBox.textHeight;
	textColor = inTextBox.textColor;
	isEditableFlag = inTextBox.isEditableFlag;
	cursorWidth = inTextBox.cursorWidth;
	cursorToggleTime = inTextBox.cursorToggleTime;

	text = inTextBox.text;
	boxDimensions = inTextBox.boxDimensions;
	boxEffectiveWidth = inTextBox.boxEffectiveWidth;
	lineBreakIndices = inTextBox.lineBreakIndices;

	vertices.clear();
	vertices.resize(4);
	uvs.clear();
	uvs.resize(4);
	cursorVertices.clear();
	cursorVertices.resize(4);
	vertexBufferId = -1;
	uvBufferId = -1;
	cursorVertexBufferId = -1;
	buffersPassedFlag = false;

	textProgramId = inTextBox.textProgramId;
	textureUniformId = inTextBox.textureUniformId;
	textColorUniformId = inTextBox.textColorUniformId;
	highlightFlagUniformId = inTextBox.highlightFlagUniformId;

	cursorProgramId = inTextBox.cursorProgramId;
	cursorColorUniformId = inTextBox.cursorColorUniformId;

	cursorIndex = inTextBox.cursorIndex;
	dragCursorIndex = inTextBox.dragCursorIndex;
	drawCursorFlag = inTextBox.drawCursorFlag;
	cursorLastToggledTime = inTextBox.cursorLastToggledTime;
	cursorRowIdx = inTextBox.cursorRowIdx;

	scrollBar = inTextBox.scrollBar;
	isSelectedFlag = false;
	draggingFlag = false;
}

textBox::~textBox() {
	if (vertexBufferId >= 0) glDeleteBuffers(1, &vertexBufferId);
	if (uvBufferId >= 0) glDeleteBuffers(1, &uvBufferId);
	if (cursorVertexBufferId >= 0) glDeleteBuffers(1, &cursorVertexBufferId);
}

void textBox::passBuffersToGLM() {
	if (buffersPassedFlag) return;
	vec2 zero(0,0);

	// Just need to fill it with anything for now
	// Each char will overwrite this later, anyway
	for (int i = 0; i < 4; i++) {
		uvs[i] = zero;
		vertices[i] = zero;
		cursorVertices[i] = zero;
	}
	
	glUseProgram(textProgramId);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec2), &vertices[0], GL_DYNAMIC_DRAW);

	glGenBuffers(1, &uvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_DYNAMIC_DRAW);

	glUseProgram(cursorProgramId);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenBuffers(1, &cursorVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, cursorVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, cursorVertices.size() * sizeof(vec2), &cursorVertices[0], GL_DYNAMIC_DRAW);

	buffersPassedFlag = true;
}

void textBox::setTextProgramId(GLuint inProgramId) {
	textProgramId = inProgramId;
	textureUniformId = glGetUniformLocation(inProgramId, "textImageTexture");
	textColorUniformId = glGetUniformLocation(inProgramId, "textColor");
	highlightFlagUniformId = glGetUniformLocation(inProgramId, "highlightFlag");
	scrollBar.setProgramId(inProgramId);
}

void textBox::setCursorProgramId(GLuint inProgramId) {
	cursorProgramId = inProgramId;
	cursorColorUniformId = glGetUniformLocation(inProgramId, "objectColor");
}

void textBox::setText(string inText) {
	text = inText;
	setCursorIndex((int)text.length());
	dragCursorIndex = cursorIndex;
	analyzeText(0, false); // fully reanalyze the text for line breaks
}

void textBox::setTextColor(vec4 inColor) {
	textColor = inColor;
	scrollBar.barColor = inColor;
	scrollBar.barColor[3] *= 0.5; // about half as transparent for the bar
}

void textBox::setTextHeight(GLfloat inHeight) {
	textHeight = inHeight;
	analyzeScrollBar();
}

void textBox::setFont(font * inTextFont) {
	if (!inTextFont) return;
	textFont = inTextFont;
	analyzeText(0, false); // fully reanalyze the text for line breaks
}

void textBox::setBoxLocation(vec2 inUpperLeftCornerLocation) {
	upperLeftCornerLocation = inUpperLeftCornerLocation;
	scrollBar.setLocation(upperLeftCornerLocation + vec2(boxEffectiveWidth, 0));
}

void textBox::setBoxDimensions(vec2 inBoxDim) {
	boxDimensions = inBoxDim;
	vec2 origDim = scrollBar.getDimensions();
	boxEffectiveWidth = boxDimensions.x - origDim.x;
	scrollBar.setDimensions(vec2(origDim.x, boxDimensions.y));
	scrollBar.setLocation(upperLeftCornerLocation + vec2(boxEffectiveWidth, 0));
	analyzeText(0, false); // fully reanalyze the text for line breaks
}

void textBox::setScrollBarWidth(GLfloat inWidth) {
	boxEffectiveWidth = boxDimensions.x - inWidth; 
	vec2 origDim = scrollBar.getDimensions();
	scrollBar.setDimensions(vec2(inWidth, boxDimensions.y));
	scrollBar.setLocation(upperLeftCornerLocation + vec2(boxEffectiveWidth, 0));
	analyzeText(0, false); // fully reanalyze the text for line breaks
}

// If we've scrolled, we have a hidden upper left corner
// Calculate this value from the user-set upper left corner and total text height
vec2 textBox::calcEffectiveLocation() {
	int nRows = (int)lineBreakIndices.size() + 1;
	GLfloat effectiveBoxHeight = textHeight * nRows;
	GLfloat posAdj = scrollBar.getBarRelativePosition() * effectiveBoxHeight;
	return upperLeftCornerLocation + vec2(0, posAdj);
}

// Draw text and maybe scroll bar
void textBox::draw() {
	if (!textFont) return;
	
	passBuffersToGLM();

	glUseProgram(textProgramId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textFont->getFontImageId());
	glUniform1i(textureUniformId, 0);
	glUniform4f(textColorUniformId, textColor.r, textColor.g, textColor.b, textColor.a);
	
	vec2 effectiveUpperLeftCorner = calcEffectiveLocation();
	vec2 corner = effectiveUpperLeftCorner;
	int dragMin = std::min(dragCursorIndex, cursorIndex);
	int dragMax = std::max(dragCursorIndex, cursorIndex);
	int b = 0;
	for (int i = 0; i < text.length(); i++) {
		// Draw the cursor
		if (i == cursorIndex) drawCursor(corner);

		// Line break
		if (b < lineBreakIndices.size() && lineBreakIndices[b] == i) {
			if (text[i] == '-') corner = drawOneChar(text[i], corner);
			corner = vec2(effectiveUpperLeftCorner.x, corner.y - textHeight);
			b++;
			continue;
		}

		// Evaluate whether this text is highlighted
		if (i >= dragMin && i < dragMax) glUniform1i(highlightFlagUniformId, 1);
		else glUniform1i(highlightFlagUniformId, 0);

		// Draw one character
		corner = drawOneChar(text[i], corner);
	}

	if (cursorIndex >= text.length()) drawCursor(corner);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// Now draw the scroll bar
	scrollBar.draw();
}

// Draw one character at a time
// Make sure we don't draw anything that has crept outside the box
vec2 textBox::drawOneChar(char inChar, vec2 upperLeftCorner) {
	float charWidth = textFont->getCharUnitWidth(inChar) * textHeight;

	textFont->setUvs(inChar, uvs);

	vec2 upperRightCorner = upperLeftCorner + vec2(charWidth, 0);
	vertices[0] = upperLeftCorner;
	vertices[1] = upperRightCorner;
	vertices[2] = upperLeftCorner + vec2(0, -textHeight);
	vertices[3] = upperRightCorner + vec2(0, -textHeight);

	// Adjust for scrolling out of box
	GLfloat uvOrigHeight = uvs[3].y - uvs[0].y;
	GLfloat verticesOrigHeight = vertices[0].y - vertices[3].y;
	GLfloat trimTop = vertices[0].y - upperLeftCornerLocation.y;
	GLfloat trimBottom = (boxDimensions.y > 0)? upperLeftCornerLocation.y - boxDimensions.y - vertices[3].y : 0;
	if (fmax(trimTop, trimBottom) >= verticesOrigHeight) goto Done; // nothing left to draw
	if (trimTop > 0) {
		for (int i = 0; i < 2; i++) {
			vertices[i].y = vertices[i].y - trimTop;
			uvs[i].y = uvs[i].y + trimTop / verticesOrigHeight * uvOrigHeight;
		}
	}
	if (trimBottom > 0) {
		for (int i = 2; i < 4; i++) {
			vertices[i].y = vertices[i].y + trimBottom;
			uvs[i].y = uvs[i].y - trimBottom / verticesOrigHeight * uvOrigHeight;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec2), &vertices[0]);

	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, uvs.size() * sizeof(vec2), &uvs[0]);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // 3 dimensions of space
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : texture
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

Done:
	return upperRightCorner;
}

// Draw the cursor: cut off any part that's out of bounds
void textBox::drawCursor(vec2 topLocation) {
	// Don't draw the cursor if the text box isn't selected
	if (!isEditableFlag || !isSelectedFlag) return;

	double newTime = glfwGetTime();
	if (newTime - cursorLastToggledTime > cursorToggleTime) {
		drawCursorFlag = !drawCursorFlag;
		cursorLastToggledTime = newTime;
	}
	if (!drawCursorFlag) return;

	cursorVertices[0] = topLocation;
	cursorVertices[1] = topLocation + vec2(cursorWidth, 0);
	cursorVertices[2] = topLocation + vec2(0, -textHeight);
	cursorVertices[3] = topLocation + vec2(cursorWidth, -textHeight);

	// Don't draw any part of the cursor that's out of the current view
	GLfloat maxY = upperLeftCornerLocation.y;
	GLfloat minY = upperLeftCornerLocation.y - boxDimensions.y;
	if (cursorVertices[0].y <= minY || cursorVertices[3].y >= maxY) return;
	for (int i = 0; i < 2; i++) {
		cursorVertices[i].y = fmin(cursorVertices[i].y, maxY);
		cursorVertices[i + 2].y = fmax(cursorVertices[i+2].y, minY);
	}

	glUseProgram(cursorProgramId);
	glUniform4f(cursorColorUniformId, textColor.r, textColor.g, textColor.b, textColor.a);

	glBindBuffer(GL_ARRAY_BUFFER, cursorVertexBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, cursorVertices.size() * sizeof(vec2), &cursorVertices[0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, cursorVertexBufferId);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, (int)cursorVertices.size());

	glDisableVertexAttribArray(0);

	// Go back to text program
	glUseProgram(textProgramId);
	glUniform4f(textColorUniformId, textColor.r, textColor.g, textColor.b, textColor.a);

}

// Type a letter at cursor
void textBox::callGlfwCharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods) {
	if (!isEditableFlag) return;
	char charInsert = (char) codepoint;

	if (cursorIndex == dragCursorIndex) {
		text.insert(cursorIndex, &charInsert, 1);
		setCursorIndex(cursorIndex + 1);
	}
	else {
		int dragMin = std::min(dragCursorIndex, cursorIndex);
		int dragMax = std::max(dragCursorIndex, cursorIndex);
		text.replace(dragMin, dragMax - dragMin, 1, charInsert);
		setCursorIndex(dragMin + 1);
	}
	dragCursorIndex = cursorIndex;
	if (charInsert == '-' || charInsert == ' ' || charInsert == '\n') {
		int i, startAnalysisAt;
		for (i = (int)lineBreakIndices.size() - 1; i >= 0; i--) {
			if (lineBreakIndices[i] < cursorIndex - 1) {
				break;
			}
		}
		if (i < 1) startAnalysisAt = 0;
		else startAnalysisAt = lineBreakIndices[i - 1] + 1;
		analyzeText(startAnalysisAt, false);
	}
	else analyzeText(cursorIndex-1, false);
}

// Key callback
// Delete for Backspace and Delete buttons
// New line for Enter button
// Arrow buttons to move cursor
void textBox::callGlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (!isEditableFlag || action == GLFW_RELEASE || !textFont) return;

	int i;

	switch (key) {
	case GLFW_KEY_DELETE:
		if (text.length() > 0 && cursorIndex < text.length()) {
			if (cursorIndex == dragCursorIndex) text.erase(cursorIndex, 1);
			else {
				int dragMin = std::min(dragCursorIndex, cursorIndex);
				int dragMax = std::max(dragCursorIndex, cursorIndex);
				text.erase(dragMin, dragMax - dragMin);
				cursorIndex = dragMin;
				dragCursorIndex = cursorIndex;
			}
			analyzeText(cursorIndex, true);
		}
		break;
	case GLFW_KEY_BACKSPACE:
		if (text.length() > 0 && cursorIndex >= 1) {
			if (cursorIndex == dragCursorIndex) {
				text.erase(cursorIndex - 1, 1);
				setCursorIndex(cursorIndex - 1);
			}
			else {
				int dragMin = std::min(dragCursorIndex, cursorIndex);
				int dragMax = std::max(dragCursorIndex, cursorIndex);
				text.erase(dragMin, dragMax - dragMin);
				cursorIndex = dragMin;
			}
			dragCursorIndex = cursorIndex;
			analyzeText(cursorIndex, true);
		}
		break;
	case GLFW_KEY_ENTER: {
		if (cursorIndex == dragCursorIndex) {
			text.insert(cursorIndex, 1, '\n');
			setCursorIndex(cursorIndex + 1);
		}
		else {
			int dragMin = std::min(dragCursorIndex, cursorIndex);
			int dragMax = std::max(dragCursorIndex, cursorIndex);
			text.replace(dragMin, dragMax - dragMin, 1, '\n');
			setCursorIndex(dragMin + 1);
		}
		dragCursorIndex = cursorIndex;
		analyzeText(cursorIndex-1, false);
		break; }
	case GLFW_KEY_LEFT:
		setCursorIndex(cursorIndex - 1);
		if (!(mods & GLFW_MOD_SHIFT)) dragCursorIndex = cursorIndex;
		analyzeText(cursorIndex, false); // TODO: make an analyze cursor function instead
		break;
	case GLFW_KEY_RIGHT:
		setCursorIndex(cursorIndex + 1);
		if (!(mods & GLFW_MOD_SHIFT)) dragCursorIndex = cursorIndex;
		analyzeText(cursorIndex, false); // TODO: make an analyze cursor function instead
		break;
	case GLFW_KEY_HOME: {
		for (i = (int)lineBreakIndices.size() - 1; i >= 0; i--) {
			if (lineBreakIndices[i] < cursorIndex) {
				cursorIndex = lineBreakIndices[i] + 1;
				break;
			}
		}
		if (i < 0) cursorIndex = 0;
		if (!(mods & GLFW_MOD_SHIFT)) dragCursorIndex = cursorIndex;
		analyzeText(cursorIndex, false);
		break; }
	case GLFW_KEY_END: {
		for (i = 0; i < lineBreakIndices.size(); i++) {
			if (lineBreakIndices[i] >= cursorIndex) {
				cursorIndex = lineBreakIndices[i];
				break;
			}
		}
		if (i == lineBreakIndices.size()) cursorIndex = (int)text.length();
		if (!(mods & GLFW_MOD_SHIFT)) dragCursorIndex = cursorIndex;
		analyzeText(cursorIndex, false);
		break; }
	case GLFW_KEY_UP:
	case GLFW_KEY_DOWN: {
		int startAtIndex, endAtCnt;

		if (key == GLFW_KEY_DOWN) {
			startAtIndex = (int)text.length();
			endAtCnt = (int)text.length();
			for (i = 0; i < lineBreakIndices.size(); i++) {
				if (lineBreakIndices[i] >= cursorIndex) {
					startAtIndex = lineBreakIndices[i] + 1;
					if (i + 1 < lineBreakIndices.size()) endAtCnt = lineBreakIndices[i + 1] + 1;
					break;
				}
			}
		}
		else {
			startAtIndex = 0;
			endAtCnt = 0;
			for (i = (int)lineBreakIndices.size()-1; i >= 0; i--) {
				if (lineBreakIndices[i] < cursorIndex) {
					endAtCnt = lineBreakIndices[i] + 1;
					if (i > 0) startAtIndex = lineBreakIndices[i - 1] + 1;
					break;
				}
			}
		}

		float lineWidth = 0.0f;
		float lastLineWidth = 0.0f;
		for (i = startAtIndex; i < endAtCnt; i++) {
			lineWidth += textFont->getCharUnitWidth(text[i]) * textHeight;
			if (lineWidth > cursorXCoord_textBoxSpace) {
				if (cursorXCoord_textBoxSpace - lastLineWidth < lineWidth - cursorXCoord_textBoxSpace) {
					setCursorIndex(i);
				}
				else if (i == endAtCnt - 1) setCursorIndex(i);
				else setCursorIndex(i + 1);
				break;
			}
			lastLineWidth = lineWidth;
		}
		if (i == text.length()) setCursorIndex(i);
		else if (i == endAtCnt) setCursorIndex(i-1);
		if (!(mods & GLFW_MOD_SHIFT)) dragCursorIndex = cursorIndex;
		analyzeText(cursorIndex, false); // TODO: make an analyze cursor function instead
		break; }
	default: break;
	}
}

// Click on text box: move cursor to that position
// Click on scroll bar in non-bar space: jump scroll
void textBox::callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (!textFont) return;

	// Note that we've been selected (we won't draw text cursor if we haven't)
	isSelectedFlag = true;
	if (action == GLFW_RELEASE) draggingFlag = false;

	// If we clicked on the scroll bar, use its callback
	// Also tell the scroll bar when we release
	vec2 clickPosition_world = screenPosTo2DCoord(window);
	if (action == GLFW_RELEASE || scrollBar.testPointInBounds(clickPosition_world)) {
		return scrollBar.callGlfwMouseButtonCallback(window, button, action, mods);
	}

	// Text box only cares about presses, not releases (for now)
	if (!isEditableFlag || action != GLFW_PRESS) return;

	// Note a start to the drag (for highlighting)
	draggingFlag = true;

	// Set the text cursor to the current mouse position
	setCursorToCurrentMousPos(clickPosition_world);
	if (!(mods & GLFW_MOD_SHIFT)) dragCursorIndex = cursorIndex;
}


// The scroll bar may be dragging, so notify it of a cursor move
void textBox::callGlfwCursorPosCallback(GLFWwindow* window, double x, double y) {
	if (draggingFlag) {
		vec2 clickPosition_world = screenPosTo2DCoord(window, x, y);
		setCursorToCurrentMousPos(clickPosition_world);
		// Don't update dragCursorIndex here
	}
	else scrollBar.callGlfwCursorPosCallback(window, x, y);
}

// Scroll text
void textBox::callGlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	int nRows = (int)lineBreakIndices.size() + 1;
	GLfloat trueBoxHeight = textHeight * nRows;
	if (boxDimensions.y <= 0 || trueBoxHeight <= boxDimensions.y) return;

	// For each click up or down, move one line
	scrollBar.scrollRelativeBarJump = 1.0f / nRows;
	scrollBar.callGlfwScrollCallback(window, xoffset, yoffset);
}

// Deselect: stop drawing cursor and scroll bar should stop dragging
void textBox::deselect() {
	isSelectedFlag = false;
	draggingFlag = false;
	scrollBar.deselect();
}

// Test whether a 2D point is inside the bounds of this object
GLboolean textBox::testPointInBounds(vec2 testPoint) {
	return testPointInBox(testPoint, upperLeftCornerLocation, boxDimensions);
}


// Analyzes text for line breaks and also figures out where the cursor belongs in the worldspace
void textBox::analyzeText(int startAtIndex, GLboolean forDeletionFlag) {
	if (!textFont) return;

	GLboolean canBreakNow = !forDeletionFlag;
	int i;
	for (i = (int)lineBreakIndices.size()-1; i >= 0; i--) {
		if (lineBreakIndices[i] < startAtIndex) {
			startAtIndex = lineBreakIndices[i] + 1;
			if (canBreakNow) break;
			else {
				lineBreakIndices.pop_back();
				canBreakNow = true;
			}
		}
		else lineBreakIndices.pop_back();
	}
	if (i < 0) startAtIndex = 0;

	float lineWidth = 0, lineWidthFromLastSpace = 0;
	int lastSpaceIdx = startAtIndex - 1;
	for (int i = startAtIndex; i < text.length(); i++) {
		if (i == cursorIndex) {
			cursorXCoord_textBoxSpace = lineWidth;
			cursorRowIdx = (int)lineBreakIndices.size();
		}

		float charWidth = textFont->getCharUnitWidth(text[i]) * textHeight;
		lineWidth += charWidth;
		lineWidthFromLastSpace += charWidth;

		if (text[i] == ' ' || text[i] == '\n') {
			lastSpaceIdx = i;
			lineWidthFromLastSpace = 0;
		}
		
		if (lastSpaceIdx >= startAtIndex) {
			if (text[i] == '\n' || lineWidth + charWidth > boxEffectiveWidth) {
				lineBreakIndices.push_back(lastSpaceIdx);
				lineWidth = lineWidthFromLastSpace;
			}
		}

		// Check hyphen afterwards because we would still draw the hyphen
		// And if the hyphen goes over, we have a problem
		if (text[i] == '-') {
			lastSpaceIdx = i;
			lineWidthFromLastSpace = 0;
		}
	}

	if (cursorIndex >= text.length()) {
		cursorXCoord_textBoxSpace = lineWidth;
		cursorRowIdx = (int)lineBreakIndices.size();
	}

	// Analyze the scroll bar
	analyzeScrollBar();
}

// Set the cursor index, which is the index of the char it is to the left of
// If the index is equal to the size of the string, then it's at the end
// This function should always be followed by a call to analyzeText
void textBox::setCursorIndex(int inCursorIndex) {
	if (!isEditableFlag) return;
	cursorIndex = (inCursorIndex > 0)? inCursorIndex : 0;
	cursorIndex = (cursorIndex < (int)text.length())? cursorIndex : (int)text.length();
}


// If the text is longer than the height of the box, let the scroll bar pop up
// Position of line i = textHeight * (i - nRows * barRelativePos)
void textBox::analyzeScrollBar() {
	int nRows = (int)lineBreakIndices.size() + 1;
	GLfloat trueBoxHeight = textHeight * nRows;

	// If we don't have a box height limit or if the text fully
	//   fits in the box, just set the bar's relative length to
	//   1 (and thus it won't draw)
	if (boxDimensions.y <= 0 || trueBoxHeight <= boxDimensions.y) {
		scrollBar.setBarRelativeLength(1.0f);
		scrollBar.setBarRelativePosition(0.0f);
	}
	else {
		GLfloat barLength = boxDimensions.y / trueBoxHeight;
		scrollBar.setBarRelativeLength(barLength);

		// Move the bar so that we stay with the cursor
		GLfloat origPos = scrollBar.getBarRelativePosition();
		GLfloat maxPos = (GLfloat) cursorRowIdx / nRows;
		GLfloat minPos = ((GLfloat)(cursorRowIdx + 1) - boxDimensions.y / textHeight) / nRows;
		GLfloat newPos = fmin(fmax(origPos, minPos), maxPos);
		scrollBar.setBarRelativePosition(newPos);
	}
}


// Set the cursor to wherever the mouse currently is
void textBox::setCursorToCurrentMousPos(vec2 clickPosition_world) {
	vec2 effectiveUpperLeftCorner = calcEffectiveLocation();
	vec2 tempVec = clickPosition_world - effectiveUpperLeftCorner;
	vec2 clickPosition_textbox(tempVec.x, -tempVec.y); // need to reverse direction of y again

	// Find the row
	int rowIdx = (int)(clickPosition_textbox.y / textHeight);
	int charIdx, endRowCnt;
	rowIdx = std::min((int)std::max(rowIdx, (int)0), (int)lineBreakIndices.size());
	if (rowIdx < 1) charIdx = 0;
	else charIdx = lineBreakIndices[rowIdx - 1] + 1;

	// Initialize cursor index
	if (rowIdx < lineBreakIndices.size()) {
		endRowCnt = lineBreakIndices[rowIdx];
		setCursorIndex(lineBreakIndices[rowIdx]);
	}
	else {
		endRowCnt = (int)text.length();
		setCursorIndex((int)text.length());
	}

	// Find the column
	GLfloat lineWidth = 0.0f;
	for (; charIdx < endRowCnt; charIdx++) {
		GLfloat charWidth = textFont->getCharUnitWidth(text[charIdx]) * textHeight;
		if (lineWidth + charWidth / 2.0f > clickPosition_textbox.x) {
			setCursorIndex(charIdx);
			break;
		}
		lineWidth += charWidth;
	}
	cursorXCoord_textBoxSpace = lineWidth;
}
