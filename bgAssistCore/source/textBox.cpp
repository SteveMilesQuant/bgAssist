#include <stdafx.h>

#include <textBox.hpp>
#include <glfwExt.hpp>

#include <algorithm>

undoRedoUnit::undoRedoUnit() {
	action = TEXTBOX_ACTION_UNKNOWN;
	text.clear();
	cursorIndex = 0;
	dragCursorIndex = 0;
	deleteCursorIndex = 0;
	moveTargetIndex = 0;
	goesWithPreviousActionFlag = false;
	timeWasUpdated = glfwGetTime();
}

void undoRedoUnit::copyUnit(const undoRedoUnit & inUnit) {
	action = inUnit.action;
	text = inUnit.text;
	cursorIndex = inUnit.cursorIndex;
	dragCursorIndex = inUnit.dragCursorIndex;
	deleteCursorIndex = inUnit.deleteCursorIndex;
	moveTargetIndex = inUnit.moveTargetIndex;
	goesWithPreviousActionFlag = false;
	timeWasUpdated = inUnit.timeWasUpdated;
}

undoRedoUnit::~undoRedoUnit() {
	// Nothing to do
}

textBox::textBox() {
	upperLeftCornerLocation = vec2(0,0);
	locationXShift = 0.0f;
	textFont = NULL;
	textHeight = 0;
	textColor = vec4(1);
	isEditableFlag = false;
	cursorWidth = 0;
	cursorToggleTime = 0.5;
	undoActionPauseTime = 5;

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
	wrapTextFlag = false;
	isSelectedFlag = false;
	draggingFlag = false;
	movingFlag = false;

	// undoStack and redoStack should come in empty
}

void textBox::copyTextBox(const textBox & inTextBox) {
	if (this == &inTextBox) return;

	upperLeftCornerLocation = inTextBox.upperLeftCornerLocation;
	locationXShift = inTextBox.locationXShift;
	textFont = inTextBox.textFont;
	textHeight = inTextBox.textHeight;
	textColor = inTextBox.textColor;
	isEditableFlag = inTextBox.isEditableFlag;
	cursorWidth = inTextBox.cursorWidth;
	cursorToggleTime = inTextBox.cursorToggleTime;
	undoActionPauseTime = inTextBox.undoActionPauseTime;

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
	wrapTextFlag = inTextBox.wrapTextFlag;
	isSelectedFlag = false;
	draggingFlag = false;
	movingFlag = false;

	while (!undoStack.empty()) undoStack.pop();
	while (!redoStack.empty()) redoStack.pop();
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
	setCursorIndex(0);
	dragCursorIndex = cursorIndex;
	analyzeText(0); // fully reanalyze the text for line breaks
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
	analyzeText(0); // fully reanalyze the text for line breaks
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
	analyzeText(0); // fully reanalyze the text for line breaks
}

void textBox::setScrollBarWidth(GLfloat inWidth) {
	boxEffectiveWidth = boxDimensions.x - inWidth; 
	vec2 origDim = scrollBar.getDimensions();
	scrollBar.setDimensions(vec2(inWidth, boxDimensions.y));
	scrollBar.setLocation(upperLeftCornerLocation + vec2(boxEffectiveWidth, 0));
	analyzeText(0); // fully reanalyze the text for line breaks
}

void textBox::loadScrollBarImage(const char * imagePath) {
	scrollBar.loadImage(imagePath);
	GLboolean origWrapTextFlag = wrapTextFlag;
	if (scrollBar.isImageLoaded()) wrapTextFlag = true;
	else wrapTextFlag = false;
	if (origWrapTextFlag != wrapTextFlag) analyzeText(0);
}

// If we've scrolled, we have a hidden upper left corner
// Calculate this value from the user-set upper left corner and total text height
vec2 textBox::calcEffectiveLocation() {
	int nRows = (int)lineBreakIndices.size() + 1;
	GLfloat effectiveBoxHeight = textHeight * nRows;
	GLfloat posAdj = scrollBar.getBarRelativePosition() * effectiveBoxHeight;
	return upperLeftCornerLocation + vec2(locationXShift, posAdj);
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
	int highlightCursorIndex;
	if (movingFlag) highlightCursorIndex = movingTextCursorIndex;
	else highlightCursorIndex = cursorIndex;
	int dragMin = std::min(dragCursorIndex, highlightCursorIndex);
	int dragMax = std::max(dragCursorIndex, highlightCursorIndex);
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
	vec2 uvOrigDim = uvs[3] - uvs[0];
	vec2 verticesOrigDim = vertices[3] - vertices[0];
	verticesOrigDim.y *= -1;
	GLfloat trimTop = vertices[0].y - upperLeftCornerLocation.y;
	GLfloat trimBottom = (boxDimensions.y > 0)? upperLeftCornerLocation.y - boxDimensions.y - vertices[3].y : 0;
	GLfloat trimLeft = upperLeftCornerLocation.x - vertices[0].x;
	GLfloat trimRight = (boxDimensions.x > 0 && !wrapTextFlag)? vertices[3].x - upperLeftCornerLocation.x - boxDimensions.x : 0;

	if (fmax(trimTop, trimBottom) >= verticesOrigDim.y || fmax(trimLeft, trimRight) >= verticesOrigDim.x) {
		goto Done; // nothing left to draw
	}
	if (trimTop > 0) {
		for (int i = 0; i < 2; i++) {
			vertices[i].y -= trimTop;
			uvs[i].y += trimTop / verticesOrigDim.y * uvOrigDim.y;
		}
	}
	if (trimBottom > 0) {
		for (int i = 2; i < 4; i++) {
			vertices[i].y += trimBottom;
			uvs[i].y -= trimBottom / verticesOrigDim.y * uvOrigDim.y;
		}
	}
	if (trimLeft > 0) {
		for (int i = 0; i < 4; i += 2) {
			vertices[i].x += trimLeft;
			uvs[i].x += trimLeft / verticesOrigDim.x * uvOrigDim.x;
		}
	}
	if (trimRight > 0) {
		for (int i = 1; i < 4; i += 2) {
			vertices[i].x -= trimRight;
			uvs[i].x -= trimRight / verticesOrigDim.x * uvOrigDim.x;
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
	GLfloat maxX = upperLeftCornerLocation.x + boxDimensions.x;
	GLfloat minX = upperLeftCornerLocation.x;
	if (cursorVertices[0].y < minY || cursorVertices[3].y > maxY ||
		cursorVertices[0].x < minX || cursorVertices[3].x > maxX) {
		return;
	}
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
	char charInsert = (char)codepoint;

	replaceTextAtCursor(string(1, charInsert), false, undoStack);
	while (!redoStack.empty()) redoStack.pop(); // when we do anything other than undo/redo, clear the redo stack
}

// Key callback
// Delete for Backspace and Delete buttons
// New line for Enter button
// Arrow buttons to move cursor
void textBox::callGlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// If it isn't editable, then UP and DOWN can just move the scroll bar a line
	if (!isEditableFlag) {
		int nRows = (int)lineBreakIndices.size() + 1;
		scrollBar.scrollRelativeBarJump = 1.0f / nRows;
		scrollBar.callGlfwKeyCallback(window, key, scancode, action, mods);
		return;
	}

	// We don't have any actions for release
	// Font is just required in general - can't draw or anything without it
	if (action == GLFW_RELEASE || !textFont) return;

	int i;
	int deleteCursorIndex = cursorIndex;

	switch (key) {
	case GLFW_KEY_DELETE:
		if (deleteCursorIndex == dragCursorIndex && deleteCursorIndex < text.length()) deleteCursorIndex++;
		deleteTextAtCursor(deleteCursorIndex, undoStack);
		while (!redoStack.empty()) redoStack.pop(); // when we do anything other than undo/redo, clear the redo stack
		break;
	case GLFW_KEY_BACKSPACE:
		if (deleteCursorIndex == dragCursorIndex && deleteCursorIndex > 0) deleteCursorIndex--;
		deleteTextAtCursor(deleteCursorIndex, undoStack);
		while (!redoStack.empty()) redoStack.pop(); // when we do anything other than undo/redo, clear the redo stack
		break;
	case GLFW_KEY_ENTER:
		replaceTextAtCursor("\n", false, undoStack);
		while (!redoStack.empty()) redoStack.pop(); // when we do anything other than undo/redo, clear the redo stack
		break;
	case GLFW_KEY_LEFT:
		setCursorIndex(cursorIndex - 1);
		if (!(mods & GLFW_MOD_SHIFT)) dragCursorIndex = cursorIndex;
		analyzeText(cursorIndex); // TODO: make an analyze cursor function instead
		break;
	case GLFW_KEY_RIGHT:
		setCursorIndex(cursorIndex + 1);
		if (!(mods & GLFW_MOD_SHIFT)) dragCursorIndex = cursorIndex;
		analyzeText(cursorIndex); // TODO: make an analyze cursor function instead
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
		analyzeText(cursorIndex);
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
		analyzeText(cursorIndex);
		break; }
	case GLFW_KEY_UP:
	case GLFW_KEY_DOWN: {
		int startAtIndex, endAtCnt;
		if (!wrapTextFlag) return;

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
		analyzeText(cursorIndex); // TODO: make an analyze cursor function instead
		break; }
	// Copy or cut
	case GLFW_KEY_C:
	case GLFW_KEY_X:
		// Copy/cut text to clipboard
		if (mods & GLFW_MOD_CONTROL && dragCursorIndex != cursorIndex) {
			movingFlag = false;
			draggingFlag = false;

			int dragMin = std::min(dragCursorIndex, cursorIndex);
			int dragMax = std::max(dragCursorIndex, cursorIndex);
			string subtext = text.substr(dragMin, dragMax - dragMin);
			if (key == GLFW_KEY_X) deleteTextAtCursor(undoStack);
			glfwSetClipboardString(window, subtext.c_str());
			while (!redoStack.empty()) redoStack.pop(); // when we do anything other than undo/redo, clear the redo stack
		}
		break;
	// Paste
	case GLFW_KEY_V:
		if (mods & GLFW_MOD_CONTROL) {
			movingFlag = false;
			draggingFlag = false;

			string insertText(glfwGetClipboardString(window));
			replaceTextAtCursor(insertText, true, undoStack);
			while (!redoStack.empty()) redoStack.pop(); // when we do anything other than undo/redo, clear the redo stack
		}
		break;
	// Undo and redo
	case GLFW_KEY_Z:
	case GLFW_KEY_Y: {
		stack<undoRedoUnit> &thisStack = (key == GLFW_KEY_Z) ? undoStack : redoStack;
		stack<undoRedoUnit> &otherStack = (key == GLFW_KEY_Z) ? redoStack : undoStack;

		if (mods & GLFW_MOD_CONTROL && !thisStack.empty()) {
			// Exit moving/dragging if they undo or redo
			movingFlag = false;
			draggingFlag = false;

			undoRedoUnit undoAction;
			GLboolean firstPassFlag = true;
			do {
				undoAction = thisStack.top();
				thisStack.pop();

				int dragMin = std::min(undoAction.cursorIndex, undoAction.dragCursorIndex);
				int dragMax = std::max(undoAction.cursorIndex, undoAction.dragCursorIndex);
				GLboolean didSomethingFlag = true; // most things do something

				switch (undoAction.action) {
				case TEXTBOX_ACTION_TYPE:
				case TEXTBOX_ACTION_PASTE:
					dragCursorIndex = dragMin + (int)undoAction.text.length();
					cursorIndex = dragCursorIndex;
					deleteTextAtCursor(dragMin, otherStack);
					break;
				case TEXTBOX_ACTION_DELETE:
					cursorIndex = undoAction.deleteCursorIndex;
					dragCursorIndex = undoAction.deleteCursorIndex;
					replaceTextAtCursor(undoAction.text, true, otherStack);
					break;
				case TEXTBOX_ACTION_MOVE:
					if (undoAction.moveTargetIndex < dragMin) {
						// Last time we moved backwards
						dragCursorIndex = undoAction.moveTargetIndex; // start
						movingTextCursorIndex = undoAction.moveTargetIndex + dragMax - dragMin; // end
						cursorIndex = dragMax; // target
					}
					else {
						// Last time we moved forward
						dragCursorIndex = undoAction.moveTargetIndex - dragMax + dragMin; // start
						movingTextCursorIndex = undoAction.moveTargetIndex; // end
						cursorIndex = dragMin; // target
					}
					moveHighlightedTextToCursor(otherStack);
					break;
				default:
					// Do nothing
					didSomethingFlag = false;
					break;
				}

				// Fix goesWithPreviousActionFlag
				if (didSomethingFlag) {
					if (firstPassFlag) otherStack.top().goesWithPreviousActionFlag = false;
					else otherStack.top().goesWithPreviousActionFlag = true;
				}

				// After we're done, set the cursors to what they were before this action
				cursorIndex = undoAction.cursorIndex;
				dragCursorIndex = undoAction.dragCursorIndex;

				firstPassFlag = false;
			} while (undoAction.goesWithPreviousActionFlag && !thisStack.empty());
		}
		break; }
	default: break;
	}
}

// Click on text box: move cursor to that position
// Click on scroll bar in non-bar space: jump scroll
void textBox::callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (!textFont) return;

	// Note where we clicked and that this object has been selected
	vec2 clickPosition_world = screenPosTo2DCoord(window);
	isSelectedFlag = true;

	// On release, move text, if we're moving it
	// Also end dragging
	if (action == GLFW_RELEASE) {
		draggingFlag = false;
		if (movingFlag) {
			int dragMin = std::min(movingTextCursorIndex, dragCursorIndex);
			int dragMax = std::max(movingTextCursorIndex, dragCursorIndex);
			setCursorToCurrentMousPos(clickPosition_world);

			// If we're actually moving it, then move it
			if (isEditableFlag && (cursorIndex < dragMin || cursorIndex >= dragMax)) {
				moveHighlightedTextToCursor(undoStack);
				while (!redoStack.empty()) redoStack.pop(); // when we do anything other than undo/redo, clear the redo stack
			}
			else {
				cursorIndex = movingTextCursorIndex;
				analyzeText(cursorIndex);
			}
			movingFlag = false;
			scrollBar.callGlfwMouseButtonCallback(window, button, action, mods);
			return;
		}
		else movingFlag = false;
	}

	// If we clicked on the scroll bar, use its callback
	// Also tell the scroll bar when we release
	if (scrollBar.testPointInBounds(clickPosition_world) || action == GLFW_RELEASE) {
		int nRows = (int)lineBreakIndices.size() + 1;
		scrollBar.scrollRelativeBarJump = 1.0f / nRows;
		scrollBar.callGlfwMouseButtonCallback(window, button, action, mods);
		return;
	}

	// Text box only cares about presses, not releases (for now)
	if (!isEditableFlag || action != GLFW_PRESS) return;

	int origCursorIndex = cursorIndex;
	int dragMin = std::min(cursorIndex, dragCursorIndex);
	int dragMax = std::max(cursorIndex, dragCursorIndex);

	// Set the text cursor to the current mouse position
	setCursorToCurrentMousPos(clickPosition_world);

	// Note if we're moving already highlighted text
	if (dragMin != dragMax && dragMin <= cursorIndex && cursorIndex < dragMax) {
		movingFlag = true;
		draggingFlag = false;
		movingTextCursorIndex = origCursorIndex;
	}
	else {
		// Note that we're dragging
		draggingFlag = true;
		movingFlag = false;
		dragCursorIndex = cursorIndex;
	}
}


// The scroll bar may be dragging, so notify it of a cursor move
void textBox::callGlfwCursorPosCallback(GLFWwindow* window, double x, double y) {
	if (draggingFlag || movingFlag) {
		vec2 clickPosition_world = screenPosTo2DCoord(window, x, y);
		setCursorToCurrentMousPos(clickPosition_world);
		// Don't update dragCursorIndex/movingTextCursorIndex here
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
	movingFlag = false;
	scrollBar.deselect();
}

// Test whether a 2D point is inside the bounds of this object
GLboolean textBox::testPointInBounds(vec2 testPoint) {
	return testPointInBox(testPoint, upperLeftCornerLocation, boxDimensions);
}


// Analyzes text for line breaks and also figures out where the cursor belongs in the worldspace
void textBox::analyzeText(int startAtIndex) {
	if (!textFont) {
		lineBreakIndices.clear();
		return;
	}

	GLboolean canBreakNow = false; // always go one extra line
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
	// Always go one line back, since we could have inserted a line break
	if (i < 0) startAtIndex = 0;

	float lineWidth = 0, lineWidthFromLastSpace = 0;
	float cursorLineWidthFromLastSpace = 0;
	int lastSpaceIdx = startAtIndex - 1;
	for (int i = startAtIndex; i < text.length(); i++) {
		if (i == cursorIndex) {
			setCursorXcoord(lineWidth);
			cursorRowIdx = (int)lineBreakIndices.size();
			cursorLineWidthFromLastSpace = lineWidthFromLastSpace;
		}

		float charWidth = textFont->getCharUnitWidth(text[i]) * textHeight;
		lineWidth += charWidth;
		lineWidthFromLastSpace += charWidth;

		// Don't break any lines if we're not wrapping text
		if (!wrapTextFlag) continue;

		if (text[i] == ' ' || text[i] == '\n') {
			lastSpaceIdx = i;
			lineWidthFromLastSpace = 0;
		}
		
		if (lastSpaceIdx >= startAtIndex) {
			if (text[i] == '\n' || lineWidth + charWidth > boxEffectiveWidth) {
				lineBreakIndices.push_back(lastSpaceIdx);
				lineWidth = lineWidthFromLastSpace;
				if (cursorIndex <= i && cursorIndex > lastSpaceIdx) {
					setCursorXcoord(cursorLineWidthFromLastSpace);
					cursorRowIdx = (int)lineBreakIndices.size();
				}
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
		setCursorXcoord(lineWidth);
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
	vec2 clickPosition_textbox = clickPosition_world - effectiveUpperLeftCorner;
	clickPosition_textbox.y *= -1.0f; // need to reverse direction of y again

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
	setCursorXcoord(lineWidth);
}

// Insert text at cursor, or replace highlighted text
//		replacementText: text you want to insert or overwrite
//		pastingTextFlag: flag for whether we're pasting or typing (for undo/redo action tracking)
void textBox::replaceTextAtCursor(string replacementText, GLboolean pastingTextFlag, stack<undoRedoUnit> &outStack) {
	int startAnalysisIdx;
	if (cursorIndex == dragCursorIndex) {
		// Update the undo stack with this action
		// If the last action was a "type", then just append to that
		undoRedoUnit undoAction, topUndoAction;
		double time = glfwGetTime();
		if (!outStack.empty()) topUndoAction = outStack.top();
		if (topUndoAction.action == TEXTBOX_ACTION_TYPE &&
			!pastingTextFlag &&
			(undoActionPauseTime == 0 || time - topUndoAction.timeWasUpdated <= undoActionPauseTime))
		{
			outStack.pop();
			undoAction = topUndoAction;
		}
		else {
			undoAction.action = (pastingTextFlag)? TEXTBOX_ACTION_PASTE : TEXTBOX_ACTION_TYPE;
			undoAction.cursorIndex = cursorIndex;
			undoAction.dragCursorIndex = dragCursorIndex;
		}
		undoAction.timeWasUpdated = time;
		undoAction.text.append(replacementText);
		outStack.push(undoAction);

		// Execute typing or (or pasting)
		text.insert(cursorIndex, replacementText);
		startAnalysisIdx = cursorIndex;
		setCursorIndex(cursorIndex + (int)replacementText.length());
	}
	else {
		int dragMin = std::min(dragCursorIndex, cursorIndex);
		int dragMax = std::max(dragCursorIndex, cursorIndex);
		int len = dragMax - dragMin;

		// Update the undo stack with a delete and type/paste
		undoRedoUnit deleteAction;
		deleteAction.action = TEXTBOX_ACTION_DELETE;
		deleteAction.cursorIndex = cursorIndex;
		deleteAction.dragCursorIndex = dragCursorIndex;
		deleteAction.timeWasUpdated = glfwGetTime();
		deleteAction.text = text.substr(dragMin, len);
		deleteAction.goesWithPreviousActionFlag = false;
		outStack.push(deleteAction);

		undoRedoUnit typeAction;
		typeAction.action = (pastingTextFlag) ? TEXTBOX_ACTION_PASTE : TEXTBOX_ACTION_TYPE;
		typeAction.cursorIndex = dragMin;
		typeAction.dragCursorIndex = dragMin;
		typeAction.timeWasUpdated = deleteAction.timeWasUpdated;
		typeAction.text = replacementText;
		typeAction.goesWithPreviousActionFlag = true;
		outStack.push(typeAction);

		// Execute replacement
		startAnalysisIdx = dragMin;
		text.replace(dragMin, len, replacementText);
		setCursorIndex(dragMin + (int)replacementText.length());
	}
	dragCursorIndex = cursorIndex;
	analyzeText(std::max(startAnalysisIdx, 0));
}

// Delete text at cursor
void textBox::deleteTextAtCursor(int deleteCursorIndex, stack<undoRedoUnit> &outStack) {
	// Use deleteCursorIndex instead of cursorIndex
	if (deleteCursorIndex != dragCursorIndex) {
		int dragMin = std::min(dragCursorIndex, deleteCursorIndex);
		int dragMax = std::max(dragCursorIndex, deleteCursorIndex);
		int len = dragMax - dragMin;

		// Note the erasure in the undo or redo stack
		undoRedoUnit deleteAction;
		deleteAction.action = TEXTBOX_ACTION_DELETE;
		deleteAction.text = text.substr(dragMin, len);
		deleteAction.cursorIndex = cursorIndex;
		deleteAction.dragCursorIndex = dragCursorIndex;
		deleteAction.deleteCursorIndex = dragMin;
		deleteAction.goesWithPreviousActionFlag = false;
		deleteAction.timeWasUpdated = glfwGetTime();
		outStack.push(deleteAction);

		text.erase(dragMin, len);
		cursorIndex = dragMin;
		dragCursorIndex = dragMin;
		analyzeText(cursorIndex);
	}
}

// Move highlighted text to cursor
void textBox::moveHighlightedTextToCursor(stack<undoRedoUnit> &outStack) {
	int dragMin = std::min(movingTextCursorIndex, dragCursorIndex);
	int dragMax = std::max(movingTextCursorIndex, dragCursorIndex);
	int startAnalysisAt;

	// Note action for undo/redo
	undoRedoUnit moveAction;
	moveAction.action = TEXTBOX_ACTION_MOVE;
	moveAction.cursorIndex = movingTextCursorIndex;
	moveAction.dragCursorIndex = dragCursorIndex;
	moveAction.moveTargetIndex = cursorIndex;
	moveAction.goesWithPreviousActionFlag = false;
	moveAction.timeWasUpdated = glfwGetTime();
	outStack.push(moveAction);

	// If we're moving backwards, delete then insert
	// If we're moving forward, insert then delete
	string subtext = text.substr(dragMin, dragMax - dragMin);
	if (cursorIndex < dragMin) {
		text.erase(dragMin, subtext.length());
		text.insert(cursorIndex, subtext);
		startAnalysisAt = cursorIndex;
		dragCursorIndex = cursorIndex + (int)subtext.length();
	}
	else {
		text.insert(cursorIndex, subtext);
		text.erase(dragMin, subtext.length());
		startAnalysisAt = dragMin;
		dragCursorIndex = cursorIndex - (int)subtext.length();
	}
	analyzeText(startAnalysisAt);
}


// Set cursorXCoord_textBoxSpace
void textBox::setCursorXcoord(GLfloat inXCoord_textBoxSpace) {
	vec2 effectiveUpperLeftCorner = calcEffectiveLocation();
	cursorXCoord_textBoxSpace = inXCoord_textBoxSpace;
	if (cursorXCoord_textBoxSpace + effectiveUpperLeftCorner.x + cursorWidth > upperLeftCornerLocation.x + boxDimensions.x) {
		locationXShift = boxDimensions.x - (cursorXCoord_textBoxSpace + cursorWidth);
	}
	else if (cursorXCoord_textBoxSpace + effectiveUpperLeftCorner.x < upperLeftCornerLocation.x) {
		locationXShift = -cursorXCoord_textBoxSpace;
	}
}
