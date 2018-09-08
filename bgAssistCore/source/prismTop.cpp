
#include <stdafx.h>

#include <prismTop.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

#include <GL/glew.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

// Constructor
prismTop::prismTop(int nSidesIn) {
	nSides = (nSidesIn > 2) ? nSidesIn : 3;

	// Initialize to regular scale, no translation, no rotation
	scaling = vec3(1.0f, 1.0f, 1.0f);
	translation = vec3(0.0f, 0.0f, 0.0f);
	rotationRadians = 0.0f;
	rotationAxis = vec3(1.0f, 0.0f, 0.0f);
	uvScale = vec2(1, 1);
	uvCenter = vec2(0, 0);
	maxCoords = vec3(0, 0, 1);
	minCoords = vec3(0, 0, -1);
	ddsFaceLoadedFlag = false;
	ddsSideLoadedFlag = false;
	copiedFaceImageFlag = false;
	copiedSideImageFlag = false;

	faceVertexBufferData.clear();
	faceUvBufferData.clear();
	sideVertexBufferData.clear();
	sideUvBufferData.clear();

	glfwMouseButtonCallback = NULL;
	glfwCursorPosCallback = NULL;

	faceImageChangedFlag = true;
	updateModelMatrixFlag = true;
	updateMVPFlag = true;
	timeMVPUpdated = 0;
}

// Copy constructor
prismTop::prismTop(const prismTop &inPrismTop) {
	nSides = inPrismTop.nSides;
	scaling = inPrismTop.scaling;
	translation = inPrismTop.translation;
	rotationRadians = inPrismTop.rotationRadians;
	rotationAxis = inPrismTop.rotationAxis;
	minCoords = inPrismTop.minCoords;
	maxCoords = inPrismTop.maxCoords;
	glfwMouseButtonCallback = inPrismTop.glfwMouseButtonCallback;
	glfwCursorPosCallback = inPrismTop.glfwCursorPosCallback;
	ddsFaceLoadedFlag = inPrismTop.ddsFaceLoadedFlag;
	ddsSideLoadedFlag = inPrismTop.ddsSideLoadedFlag;
	textureId = inPrismTop.textureId;
	mvpId = inPrismTop.mvpId;
	faceImageId = inPrismTop.faceImageId;
	copiedFaceImageFlag = true;
	sideImageId = inPrismTop.sideImageId;
	copiedSideImageFlag = true;

	// Let passBuffersToGLM generate the vertices
	faceVertexBufferData.clear();
	faceUvBufferData.clear();
	sideVertexBufferData.clear();
	sideUvBufferData.clear();

	faceImageChangedFlag = true;
	updateModelMatrixFlag = true;
	updateMVPFlag = true;
	timeMVPUpdated = 0;
}

// Desctuctor
prismTop::~prismTop() {
	glDeleteBuffers(1, &faceVertexBufferId);
	glDeleteBuffers(1, &faceUvBufferId);
	if (!copiedFaceImageFlag) glDeleteTextures(1, &faceImageId);

	glDeleteBuffers(1, &sideVertexBufferId);
	glDeleteBuffers(1, &sideUvBufferId);
	if (!copiedSideImageFlag) glDeleteTextures(1, &sideImageId);
}

// Pass the buffers to GLM only once: after you generate them
// uvStaticOrDynamic should be GL_DYNAMIC_DRAW if you wish to update
//    the image on the face of the prism, or GL_STATIC_DRAW otherwise.
void prismTop::passBuffersToGLM(GLuint uvStaticOrDynamicForFaceImage) {
	int nFaceCoordinates = nSides * 3;
	faceVertexBufferData.reserve(nFaceCoordinates);
	faceUvBufferData.reserve(nFaceCoordinates);

	int nSideCoordinates = nSides * 6;
	sideVertexBufferData.reserve(nSideCoordinates);
	sideUvBufferData.reserve(nSideCoordinates);
	
	fillVerticesAndUVs(false);

	glGenBuffers(1, &faceVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, faceVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, faceVertexBufferData.size() * sizeof(vec3), &faceVertexBufferData[0], GL_STATIC_DRAW);
	glGenBuffers(1, &faceUvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, faceUvBufferId);
	glBufferData(GL_ARRAY_BUFFER, faceUvBufferData.size() * sizeof(vec2), &faceUvBufferData[0], uvStaticOrDynamicForFaceImage);

	glGenBuffers(1, &sideVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, sideVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sideVertexBufferData.size() * sizeof(vec3), &sideVertexBufferData[0], GL_STATIC_DRAW);
	glGenBuffers(1, &sideUvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, sideUvBufferId);
	glBufferData(GL_ARRAY_BUFFER, sideUvBufferData.size() * sizeof(vec2), &sideUvBufferData[0], GL_STATIC_DRAW);
}


// Update the model matrix whenever you change the position of the object
void prismTop::updateModelMatrix() { 
	if (!updateModelMatrixFlag) return;
	modelMatrix = translate(mat4(), translation) * rotate(rotationRadians, rotationAxis) * scale(scaling);
	updateModelMatrixFlag = false;
	updateMVPFlag = true;
}

// Update the MPV whenever the camera, projection, or matrix changes
void prismTop::updateMVP() {
	if (projection && projection->timeUpdated() > timeMVPUpdated) updateMVPFlag = true;
	if (camera && camera->timeUpdated() > timeMVPUpdated) updateMVPFlag = true;
	if (!updateMVPFlag) return;
	if (projection && camera) {
		MVP = projection->getMatrix() * camera->getMatrix() * modelMatrix;
	}
	else if (projection) {
		MVP = projection->getMatrix() * modelMatrix;
	}
	else if (camera) {
		MVP = camera->getMatrix() * modelMatrix;
	}
	else {
		MVP = modelMatrix;
	}
	updateMVPFlag = false;
	timeMVPUpdated = glfwGetTime();
}

// Update the face image when you move it
void prismTop::upateFaceImage() {
	if (!faceImageChangedFlag) return;

	int nCoordinates = nSides * 9;
	fillVerticesAndUVs(true);
	glBindBuffer(GL_ARRAY_BUFFER, faceUvBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, faceUvBufferData.size() * sizeof(vec2), &faceUvBufferData[0]);
	faceImageChangedFlag = false;
}

// Copy face image uv vector from another prismTop
void prismTop::copyFaceImageUvs(const prismTop &inPrismTop) {
	if (nSides != inPrismTop.nSides || textureId != inPrismTop.textureId) return;
	faceUvBufferData = inPrismTop.faceUvBufferData;
	faceImageChangedFlag = true;
}

// Draw the object in the main loop
void prismTop::draw() {
	// Update the image, model matrix and MVP
	// These routines will check if this is needed
	upateFaceImage();
	updateModelMatrix();
	updateMVP();

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(mvpId, 1, GL_FALSE, &MVP[0][0]);


	// DRAW FACE
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, faceImageId);
	glUniform1i(textureId, 0);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, faceVertexBufferId);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // 3 dimensions of space
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : texture
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, faceUvBufferId);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw our object(s)
	glDrawArrays(GL_TRIANGLES, 0, nSides * 3);
	glDisableVertexAttribArray(0);

	// DRAW SIDES
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sideImageId);
	glUniform1i(textureId, 0);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, sideVertexBufferId);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // 3 dimensions of space
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : texture
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, sideUvBufferId);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw our object(s)
	glDrawArrays(GL_TRIANGLES, 0, nSides * 6);
	glDisableVertexAttribArray(0);
}


// Fill the vertices and UV
void prismTop::fillVerticesAndUVs(GLboolean faceOnly) {
	int i;
	vec3 faceCenter(0.0f, 0.0f, 1.0f);
	vec3 v3RotAxis(0.0f, 0.0f, 1.0f); // Rotate about z+
	mat4x4 matRot = rotate(2.0f * pi<float>() / nSides, v3RotAxis);
	vec4 startPoint = rotate(pi<float>() / nSides, v3RotAxis) * vec4(1, 0, 1, 0);
	vec4 thisPoint = startPoint;
	vec4 nextPoint = matRot * thisPoint;

	// Clear the data
	faceVertexBufferData.clear();
	faceUvBufferData.clear();
	if (!faceOnly) {
		sideVertexBufferData.clear();
		sideUvBufferData.clear();
	}
	
	for (i = 0; i < nSides; i++) {
		if (i == nSides - 1) nextPoint = startPoint;

		// Update OBB (min and max coordinates used for picking and collision)
		minCoords.x = min(minCoords.x, thisPoint.x);
		minCoords.y = min(minCoords.y, thisPoint.y);
		maxCoords.x = max(maxCoords.x, thisPoint.x);
		maxCoords.y = max(maxCoords.y, thisPoint.y);

		// Front triangle
		faceVertexBufferData.push_back(faceCenter);
		faceVertexBufferData.push_back(vec3(thisPoint));
		faceVertexBufferData.push_back(vec3(nextPoint));
		faceUvBufferData.push_back(uvCenter);
		faceUvBufferData.push_back(vec2(thisPoint.x*uvScale.x, thisPoint.y*uvScale.x) + uvCenter);
		faceUvBufferData.push_back(vec2(nextPoint.x*uvScale.x, nextPoint.y*uvScale.x) + uvCenter);
		
		// Sometimes we only update the face
		if (!faceOnly) {

			// First half of side
			sideVertexBufferData.push_back(vec3(thisPoint));
			sideUvBufferData.push_back(vec2(1, 1));
			sideVertexBufferData.push_back(vec3(thisPoint.x, thisPoint.y, -1));
			sideUvBufferData.push_back(vec2(1, 0));
			sideVertexBufferData.push_back(vec3(nextPoint));
			sideUvBufferData.push_back(vec2(0, 1));

			// Second half of side
			sideVertexBufferData.push_back(vec3(nextPoint));
			sideUvBufferData.push_back(vec2(0, 1));
			sideVertexBufferData.push_back(vec3(nextPoint.x, nextPoint.y, -1));
			sideUvBufferData.push_back(vec2(0, 0));
			sideVertexBufferData.push_back(vec3(thisPoint.x, thisPoint.y, -1));
			sideUvBufferData.push_back(vec2(1, 0));

		}

		// Move to next set
		thisPoint = nextPoint;
		nextPoint = matRot * thisPoint;
	}

	// DDS inverts the Y coordinates
	if (ddsFaceLoadedFlag) {
		for (i = 0; i < nSides*3; i++) {
			faceUvBufferData[i].y = 1.0f - faceUvBufferData[i].y;
		}
	}
	if (ddsSideLoadedFlag && !faceOnly) {
		for (i = 0; i < nSides * 6; i++) {
			sideUvBufferData[i].y = 1.0f - sideUvBufferData[i].y;
		}
	}
}

// Load face image, either from a bmp or a dds
void prismTop::loadFaceImage(const char * imagepath, GLboolean ddsFormatFlag) {
	ddsFaceLoadedFlag = ddsFormatFlag;
	if (ddsFormatFlag) faceImageId = ::loadDDS(imagepath);
	else faceImageId = loadBMP_custom(imagepath);
	copiedFaceImageFlag = false;
}


// Load side image, either from a bmp or a dds
void prismTop::loadSideImage(const char * imagepath, GLboolean ddsFormatFlag) {
	ddsSideLoadedFlag = ddsFormatFlag;
	if (ddsFormatFlag) sideImageId = ::loadDDS(imagepath);
	else sideImageId = loadBMP_custom(imagepath);
	copiedSideImageFlag = false;
}



