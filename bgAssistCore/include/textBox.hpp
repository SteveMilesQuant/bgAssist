
#pragma once

#include <font.hpp>
#include <scrollBar.hpp>

#include <string>
#include <stack>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

enum textBoxActionType {
	TEXTBOX_ACTION_UNKNOWN = 0,
	TEXTBOX_ACTION_TYPE = 1,
	TEXTBOX_ACTION_PASTE = 2,
	TEXTBOX_ACTION_MOVE = 3,
	TEXTBOX_ACTION_DELETE = 4
};

class undoRedoUnit {
public:
	enum textBoxActionType action;
	string text;
	int cursorIndex;
	int dragCursorIndex;
	int deleteCursorIndex; // where the cursor belongs after undoing a delete
	int moveTargetIndex; // when moving, we need a third index
	GLboolean goesWithPreviousActionFlag;
	double timeWasUpdated;

	undoRedoUnit();
	~undoRedoUnit();
	undoRedoUnit(const undoRedoUnit & inUnit) { copyUnit(inUnit); }
	undoRedoUnit(const undoRedoUnit && inUnit) { copyUnit(inUnit); }
	undoRedoUnit & operator = (const undoRedoUnit & inUnit) { copyUnit(inUnit); return *this; }

private:
	
	void copyUnit(const undoRedoUnit & inUnit);
};

class textBox {
public:
	GLboolean isEditableFlag; // is the text box editable at all?
	GLfloat cursorWidth; // width of the cursor
	double cursorToggleTime; // how long for the cursor to toggle between drawn and not drawn
	double undoActionPauseTime; // how long of a pause distinguishes two separate typing actions for the undo?

	textBox();
	~textBox();
	textBox(const textBox & inTextBox) { copyTextBox(inTextBox); }
	textBox(const textBox && inTextBox) { copyTextBox(inTextBox); }
	textBox & operator = (const textBox & inTextBox) { copyTextBox(inTextBox); return *this; }

	void setTextProgramId(GLuint inProgramId); // image shader program
	void setCursorProgramId(GLuint inProgramId); // solid color shader program
	void setText(string inText);
	void setTextColor(vec4 inColor);
	void setBoxLocation(vec2 inUpperLeftCornerLocation);
	void setBoxDimensions(vec2 inBoxDim); // zero or negative means unfettered
	void setScrollBarWidth(GLfloat inWidth);
	void loadScrollBarImage(const char * imagePath) { scrollBar.loadImage(imagePath); }
	void setTextHeight(GLfloat inHeight);
	void setFont(font * inTextFont);

	void draw();
	void callGlfwCharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods);
	void callGlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void callGlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void callGlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void callGlfwCursorPosCallback(GLFWwindow* window, double x, double y);
	void deselect();
	GLboolean testPointInBounds(vec2 testPoint); // is this point in the object's bounds?

private:
	string text;
	vec4 textColor;
	GLfloat textHeight;
	font * textFont;
	vec2 upperLeftCornerLocation;
	vec2 boxDimensions;
	GLfloat boxEffectiveWidth; // width minus the scroll bar
	vector<int> lineBreakIndices; // line breaks as indices in text

	vector<vec2> vertices;
	vector<vec2> uvs;
	vector<vec2> cursorVertices;
	GLuint vertexBufferId;
	GLuint uvBufferId;
	GLuint cursorVertexBufferId;
	GLboolean buffersPassedFlag;

	GLuint textProgramId;
	GLuint textureUniformId;
	GLuint textColorUniformId;
	GLuint highlightFlagUniformId;

	GLuint cursorProgramId;
	GLuint cursorColorUniformId;

	int cursorIndex; // index of the cursour within the string ("before" this index in the string)
	int dragCursorIndex; // when we're dragging/highlighting, the cursor index
	GLboolean drawCursorFlag; // flag on whether we're currently drawing the cursor
	double cursorLastToggledTime;
	GLfloat cursorXCoord_textBoxSpace;
	int cursorRowIdx;

	scrollBar scrollBar;
	GLboolean isSelectedFlag; // is this object selected?
	GLboolean draggingFlag; // are we dragging?
	GLboolean movingFlag; // are we moving text?
	int movingTextCursorIndex; // the cursorIndex when we started moving text

	stack<undoRedoUnit> undoStack;
	stack<undoRedoUnit> redoStack;

	void passBuffersToGLM();
	void copyTextBox(const textBox & inTextBox);
	vec2 drawOneChar(char inChar, vec2 upperLeftCorner); // returns the upper right corner
	void drawCursor(vec2 topLocation);
	void analyzeText(int startAtIndex);
	void analyzeScrollBar();
	void setCursorIndex(int inCursorIndex);
	vec2 calcEffectiveLocation();
	void setCursorToCurrentMousPos(vec2 clickPosition_world);
	void replaceTextAtCursor(string replacementText, GLboolean pastingTextFlag, stack<undoRedoUnit> &outStack);
	void deleteTextAtCursor(int origCursorIndex, stack<undoRedoUnit> &outStack);
	void deleteTextAtCursor(stack<undoRedoUnit> &outStack) { deleteTextAtCursor(cursorIndex, outStack); }
	void moveHighlightedTextToCursor(stack<undoRedoUnit> &outStack);
};

