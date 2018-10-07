#include <stdafx.h>

#include <textBox.hpp>

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
	boxWidth = 0; // this means don't wrap text at all
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

	cursorProgramId = -1;
	cursorColorUniformId = -1;

	cursorIndex = 0;
	drawCursorFlag = false;
	cursorLastToggledTime = glfwGetTime();
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
	boxWidth = inTextBox.boxWidth;
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

	cursorProgramId = inTextBox.cursorProgramId;
	cursorColorUniformId = inTextBox.cursorColorUniformId;

	cursorIndex = inTextBox.cursorIndex;
	drawCursorFlag = inTextBox.drawCursorFlag;
	cursorLastToggledTime = inTextBox.cursorLastToggledTime;
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
}

void textBox::setCursorProgramId(GLuint inProgramId) {
	cursorProgramId = inProgramId;
	cursorColorUniformId = glGetUniformLocation(inProgramId, "objectColor");
}

void textBox::setText(string inText) {
	text = inText;
	setCursorIndex((int)text.length()); 
	analyzeText(0, false); // fully reanalyze the text for line breaks
}

void textBox::setBoxWidth(GLfloat inBoxWidth) {
	boxWidth = inBoxWidth;
	analyzeText(0, false); // fully reanalyze the text for line breaks
}

void textBox::draw() {
	
	passBuffersToGLM();

	glUseProgram(textProgramId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textFont->getFontImageId());
	glUniform1i(textureUniformId, 0);
	glUniform4f(textColorUniformId, textColor.r, textColor.g, textColor.b, textColor.a);

	vec2 corner = upperLeftCornerLocation;
	int b = 0;
	for (int i = 0; i < text.length(); i++) {
		if (i == cursorIndex) drawCursor(corner);
		if (b < lineBreakIndices.size() && lineBreakIndices[b] == i) {
			if (text[i] == '-') corner = drawOneChar(text[i], corner);
			corner = vec2(upperLeftCornerLocation.x, corner.y - textHeight);
			b++;
			continue;
		}
		corner = drawOneChar(text[i], corner);
	}

	if (cursorIndex >= text.length()) drawCursor(corner);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

vec2 textBox::drawOneChar(char inChar, vec2 upperLeftCorner) {
	float charWidth = textFont->getCharUnitWidth(inChar) * textHeight;

	textFont->setUvs(inChar, uvs);

	vec2 upperRightCorner = upperLeftCorner + vec2(charWidth, 0);
	vertices[0] = upperLeftCorner;
	vertices[1] = upperRightCorner;
	vertices[2] = upperLeftCorner + vec2(0, -textHeight);
	vertices[3] = upperRightCorner + vec2(0, -textHeight);

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

	return upperRightCorner;
}

void textBox::drawCursor(vec2 topLocation) {
	if (!isEditableFlag) return;

	double newTime = glfwGetTime();
	if (newTime - cursorLastToggledTime > cursorToggleTime) {
		drawCursorFlag = !drawCursorFlag;
		cursorLastToggledTime = newTime;
	}
	if (!drawCursorFlag) return;

	glUseProgram(cursorProgramId);
	glUniform4f(cursorColorUniformId, textColor.r, textColor.g, textColor.b, textColor.a);

	cursorVertices[0] = topLocation;
	cursorVertices[1] = topLocation + vec2(cursorWidth, 0);
	cursorVertices[2] = topLocation + vec2(0, -textHeight);
	cursorVertices[3] = topLocation + vec2(cursorWidth, -textHeight);

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


void textBox::callGlfwCharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods) {
	if (!isEditableFlag) return;
	char charInsert = (char) codepoint;

	text.insert(cursorIndex, &charInsert, 1);
	setCursorIndex(cursorIndex + 1);
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
	if (!isEditableFlag || action == GLFW_RELEASE) return;

	int i;

	switch (key) {
	case GLFW_KEY_DELETE:
		if (text.length() > 0 && cursorIndex < text.length()) {
			text.erase(cursorIndex, 1);
			analyzeText(cursorIndex, true);
		}
		break;
	case GLFW_KEY_BACKSPACE:
		if (text.length() > 0 && cursorIndex >= 1) {
			text.erase(cursorIndex-1, 1);
			setCursorIndex(cursorIndex - 1); 
			analyzeText(cursorIndex, true);
		}
		break;
	case GLFW_KEY_ENTER: {
		string newLine = "\n";
		text.insert(cursorIndex, newLine);
		setCursorIndex(cursorIndex + 1); 
		analyzeText(cursorIndex-1, false);
		break; }
	case GLFW_KEY_LEFT:
		setCursorIndex(cursorIndex - 1);
		analyzeText(cursorIndex, false); // TODO: make an analyze cursor function instead
		break;
	case GLFW_KEY_RIGHT:
		setCursorIndex(cursorIndex + 1);
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
		analyzeText(cursorIndex, false); // TODO: make an analyze cursor function instead
		break; }
	default: break;
	}
}

void textBox::callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (!isEditableFlag || action != GLFW_PRESS) return;

	int screenWidth, screenHeight;
	double x, y;
	vec2 clickPosition_world;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);
	glfwGetCursorPos(window, &x, &y);
	clickPosition_world.x = 2.0f * (GLfloat)x / screenWidth - 1.0f;
	clickPosition_world.y = 1.0f - 2.0f * (GLfloat)y / screenHeight;
	vec2 tempVec = clickPosition_world - upperLeftCornerLocation;
	vec2 clickPosition_textbox(tempVec.x, -tempVec.y); // need to reverse direction of y again

	// Find the row
	int rowIdx = (int)(clickPosition_textbox.y / textHeight);
	int charIdx, endRowCnt;
	rowIdx = std::min( (int) std::max(rowIdx, (int)0), (int)lineBreakIndices.size());
	if (rowIdx < 1) charIdx = 0;
	else charIdx = lineBreakIndices[rowIdx-1]+1;

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


// Analyzes text for line breaks and also figures out where the cursor belongs in the worldspace
void textBox::analyzeText(int startAtIndex, GLboolean forDeletionFlag) {
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
		if (i == cursorIndex) cursorXCoord_textBoxSpace = lineWidth;

		float charWidth = textFont->getCharUnitWidth(text[i]) * textHeight;
		lineWidth += charWidth;
		lineWidthFromLastSpace += charWidth;

		if (text[i] == ' ' || text[i] == '\n') {
			lastSpaceIdx = i;
			lineWidthFromLastSpace = 0;
		}
		
		if (lastSpaceIdx >= startAtIndex) {
			if (text[i] == '\n' || lineWidth + charWidth > boxWidth) {
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

	if (cursorIndex >= text.length()) cursorXCoord_textBoxSpace = lineWidth;
}

// Set the cursor index, which is the index of the char it is to the left of
// If the index is equal to the size of the string, then it's at the end
void textBox::setCursorIndex(int inCursorIndex) {
	if (!isEditableFlag) return;
	cursorIndex = (inCursorIndex > 0)? inCursorIndex : 0;
	cursorIndex = (cursorIndex < (int)text.length())? cursorIndex : (int)text.length();

	// Find where the line starts
	int lineStartIdx = 0;
	int rowIdx = 0;
	for (vector<int>::iterator iter = lineBreakIndices.begin(); iter != lineBreakIndices.end(); iter++) {
		if (*iter <= cursorIndex) {
			lineStartIdx = *iter;
			rowIdx++;
		}
		else break;
	}
}
