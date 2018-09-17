
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
void prismTop::constructPrismTop(int nSidesIn){
	// Public:
	glfwMouseButtonCallback = NULL;
	glfwCursorPosCallback = NULL;

	// Private:
	nSides = 0;
	setNSides(nSidesIn);

	// Initialize to regular scale, no translation, no rotation
	scaling = vec3(1.0f, 1.0f, 1.0f);
	translation = vec3(0.0f, 0.0f, 0.0f);
	rotationRadians = 0.0f;
	rotationAxis = vec3(1.0f, 0.0f, 0.0f);
	projection = NULL;
	camera = NULL;
	light = NULL;
	ambientRatio = 0.1f;
	specularRatio = 0.3f;

	modelMatrix = mat4(1.0f);
	MVP = mat4(1.0f);
	updateModelMatrixFlag = true;
	updateMVPFlag = true;
	timeMVPUpdated = 0;

	programId = -1;
	mvpId = -1;
	textureId = -1;
	modelMatrixId = -1;
	viewMatrixId = -1;
	lightPositionId = -1;
	lightPowerId = -1;
	lightColorId = -1;
	ambientRatioId = -1;
	specularRatioId = -1;

	uvScale = vec2(1, 1);
	uvCenter = vec2(0, 0);
	startUvCenter = uvCenter;
	faceImageChangedFlag = true;
	ddsFaceLoadedFlag = false;
	ddsSideLoadedFlag = false;
	copiedFaceImageFlag = false;
	copiedSideImageFlag = false;
	faceImageTransientFlag = false;

	/* Done in setNSides(nSidesIn); 
	faceVertexBufferId = -1;
	faceUvBufferId = -1;
	faceNormalBufferId = -1;
	faceVertexBufferData.clear();
	faceUvBufferData.clear();
	faceNormalBufferData.clear();

	sideVertexBufferId = -1;
	sideUvBufferId = -1;
	sideNormalBufferId = -1;
	sideVertexBufferData.clear();
	sideUvBufferData.clear();
	sideNormalBufferData.clear();

	buffsPassedFlag = false;*/

	faceImageId = -1;
	sideImageId = -1;

	minCoords = vec3(0, 0, -1); 
	maxCoords = vec3(0, 0, 1);
}
prismTop::prismTop() { constructPrismTop(3); }
prismTop::prismTop(int nSidesIn) {
	constructPrismTop(nSidesIn);
}

// Copy constructor
void prismTop::copyPrismTop(const prismTop & inPrismTop) {
	if (this == &inPrismTop) return;

	// Public:
	glfwMouseButtonCallback = inPrismTop.glfwMouseButtonCallback;
	glfwCursorPosCallback = inPrismTop.glfwCursorPosCallback;

	// Private:
	nSides = 0;
	setNSides(inPrismTop.nSides);

	scaling = inPrismTop.scaling;
	translation = inPrismTop.translation;
	rotationRadians = inPrismTop.rotationRadians;
	rotationAxis = inPrismTop.rotationAxis;
	projection = inPrismTop.projection;
	camera = inPrismTop.camera;
	light = inPrismTop.light;
	ambientRatio = inPrismTop.ambientRatio;
	specularRatio = inPrismTop.specularRatio;

	modelMatrix = inPrismTop.modelMatrix;
	MVP = inPrismTop.modelMatrix;
	updateModelMatrixFlag = true;
	updateMVPFlag = true;
	timeMVPUpdated = 0;

	programId = inPrismTop.programId;
	mvpId = inPrismTop.mvpId;
	textureId = inPrismTop.textureId;
	modelMatrixId = inPrismTop.modelMatrixId;
	viewMatrixId = inPrismTop.viewMatrixId;
	lightPositionId = inPrismTop.lightPositionId;
	lightPowerId = inPrismTop.lightPowerId;
	lightColorId = inPrismTop.lightColorId;
	ambientRatioId = inPrismTop.ambientRatioId;
	specularRatioId = inPrismTop.specularRatioId;

	uvScale = inPrismTop.uvScale;
	uvCenter = inPrismTop.uvCenter;
	startUvCenter = inPrismTop.startUvCenter; // doesn't need to be copied, but might as well
	faceImageChangedFlag = true;
	ddsFaceLoadedFlag = inPrismTop.ddsFaceLoadedFlag;
	ddsSideLoadedFlag = inPrismTop.ddsSideLoadedFlag;
	copiedFaceImageFlag = true;
	copiedSideImageFlag = true;
	faceImageTransientFlag = inPrismTop.faceImageTransientFlag;

	/* Done in setNSides(nSidesIn);
	faceVertexBufferId = -1;
	faceVertexBufferData.clear();
	faceUvBufferId = -1;
	faceUvBufferData.clear();
	faceNormalBufferId = -1;
	faceNormalBufferData.clear();

	sideVertexBufferId = -1;
	sideVertexBufferData.clear();
	sideUvBufferId = -1;
	sideUvBufferData.clear();
	sideNormalBufferId = -1;
	sideNormalBufferData.clear();

	buffsPassedFlag = false;*/

	faceImageId = inPrismTop.faceImageId;
	sideImageId = inPrismTop.sideImageId;

	minCoords = inPrismTop.minCoords;
	maxCoords = inPrismTop.maxCoords;

}

// Destructor
prismTop::~prismTop() {
	glDeleteBuffers(1, &faceVertexBufferId);
	glDeleteBuffers(1, &faceUvBufferId);
	glDeleteBuffers(1, &faceNormalBufferId);
	if (!copiedFaceImageFlag) glDeleteTextures(1, &faceImageId);

	glDeleteBuffers(1, &sideVertexBufferId);
	glDeleteBuffers(1, &sideUvBufferId);
	glDeleteBuffers(1, &sideNormalBufferId);
	if (!copiedSideImageFlag) glDeleteTextures(1, &sideImageId);
}

// If the number of sides changes, we need to pass the buffers to GLM again
void prismTop::setNSides(int inNSides) { 
	if (nSides == inNSides) return;
	
	nSides = (inNSides > 2) ? inNSides : 3;

	clearBuffers();
}

// If the face can be updated, allow updating it
void prismTop::setFaceImageTransientFlag(GLboolean inFaceImageTransientFlag) { 
	if (inFaceImageTransientFlag == faceImageTransientFlag) return;
	
	faceImageTransientFlag = inFaceImageTransientFlag;

	clearBuffers();
}

// Pass the buffers to GLM only once: after you generate them
// uvStaticOrDynamic should be GL_DYNAMIC_DRAW if you wish to update
//    the image on the face of the prism, or GL_STATIC_DRAW otherwise.
void prismTop::passBuffersToGLM() {
	if (buffsPassedFlag) return; // it doesn't do anything to pass the buffers twice

	GLuint uvStaticOrDynamicForFaceImage = (faceImageTransientFlag) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

	int nFaceCoordinates = nSides * 3;
	faceVertexBufferData.reserve(nFaceCoordinates);
	faceUvBufferData.reserve(nFaceCoordinates);
	faceNormalBufferData.reserve(nFaceCoordinates);

	int nSideCoordinates = nSides * 6;
	sideVertexBufferData.reserve(nSideCoordinates);
	sideUvBufferData.reserve(nSideCoordinates);
	sideNormalBufferData.reserve(nSideCoordinates);
	
	fillVerticesAndUVs(false);

	glGenBuffers(1, &faceVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, faceVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, faceVertexBufferData.size() * sizeof(vec3), &faceVertexBufferData[0], GL_STATIC_DRAW);
	glGenBuffers(1, &faceUvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, faceUvBufferId);
	glBufferData(GL_ARRAY_BUFFER, faceUvBufferData.size() * sizeof(vec2), &faceUvBufferData[0], uvStaticOrDynamicForFaceImage);
	glGenBuffers(1, &faceNormalBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, faceNormalBufferId);
	glBufferData(GL_ARRAY_BUFFER, faceNormalBufferData.size() * sizeof(vec3), &faceNormalBufferData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &sideVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, sideVertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sideVertexBufferData.size() * sizeof(vec3), &sideVertexBufferData[0], GL_STATIC_DRAW);
	glGenBuffers(1, &sideUvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, sideUvBufferId);
	glBufferData(GL_ARRAY_BUFFER, sideUvBufferData.size() * sizeof(vec2), &sideUvBufferData[0], GL_STATIC_DRAW);
	glGenBuffers(1, &sideNormalBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, sideNormalBufferId);
	glBufferData(GL_ARRAY_BUFFER, sideNormalBufferData.size() * sizeof(vec3), &sideNormalBufferData[0], GL_STATIC_DRAW);

	buffsPassedFlag = true;
}

void prismTop::clearBuffers() {
	if (buffsPassedFlag) {
		glDeleteBuffers(1, &faceVertexBufferId);
		glDeleteBuffers(1, &faceUvBufferId);
		glDeleteBuffers(1, &faceNormalBufferId);
		glDeleteBuffers(1, &sideVertexBufferId);
		glDeleteBuffers(1, &sideUvBufferId);
		glDeleteBuffers(1, &sideNormalBufferId);
	}

	faceVertexBufferId = -1;
	faceVertexBufferData.clear();
	faceUvBufferId = -1;
	faceUvBufferData.clear();
	faceNormalBufferId = -1;
	faceNormalBufferData.clear();

	sideVertexBufferId = -1;
	sideVertexBufferData.clear();
	sideUvBufferId = -1;
	sideUvBufferData.clear();
	sideNormalBufferId = -1;
	sideNormalBufferData.clear();

	buffsPassedFlag = false;
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

	// Make sure we're allowed to upate the face
	setFaceImageTransientFlag(true);
	passBuffersToGLM();

	int nCoordinates = nSides * 9;
	fillVerticesAndUVs(true);
	glBindBuffer(GL_ARRAY_BUFFER, faceUvBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, faceUvBufferData.size() * sizeof(vec2), &faceUvBufferData[0]);
	faceImageChangedFlag = false;
}

// Copy face image uv vector from another prismTop
void prismTop::copyFaceImageUvs(const prismTop &inPrismTop) {
	if (nSides != inPrismTop.nSides || textureId != inPrismTop.textureId || faceImageId != inPrismTop.faceImageId) return;
	uvScale = inPrismTop.uvScale;
	uvCenter = inPrismTop.uvCenter;
	faceImageChangedFlag = true;
}

// Draw the object in the main loop
void prismTop::draw() {
	// Update the image, model matrix and MVP
	// These routines will check if this is needed
	passBuffersToGLM();
	upateFaceImage();
	updateModelMatrix();
	updateMVP();

	// Send our transformations to the shader
	glUniformMatrix4fv(mvpId, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);
	if (camera) glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &camera->getMatrix()[0][0]);
	if (light) {
		glUniform3f(lightPositionId, light->position.x, light->position.y, light->position.z);
		glUniform3f(lightColorId, light->color.x, light->color.y, light->color.z);
		glUniform1f(lightPowerId, light->power);
	}
	else {
		lightSource defaultLight;
		glUniform3f(lightPositionId, defaultLight.position.x, defaultLight.position.y, defaultLight.position.z);
		glUniform3f(lightColorId, defaultLight.color.x, defaultLight.color.y, defaultLight.color.z);
		glUniform1f(lightPowerId, defaultLight.power);
	}
	glUniform1f(ambientRatioId, ambientRatio);
	glUniform1f(specularRatioId, specularRatio);

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

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, faceNormalBufferId);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 2, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw our object(s)
	glDrawArrays(GL_TRIANGLES, 0, nSides * 3);

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

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, sideNormalBufferId);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 2, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw our object(s)
	glDrawArrays(GL_TRIANGLES, 0, nSides * 6);

	// Disable vertex attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}


// Fill the vertices and UV
void prismTop::fillVerticesAndUVs(GLboolean faceOnly) {
	int i;
	vec3 faceCenter(0.0f, 0.0f, 1.0f);
	vec3 zaxis(0.0f, 0.0f, 1.0f); // Rotate about z+
	mat4x4 matRot = rotate(2.0f * pi<float>() / nSides, zaxis);
	vec4 startPoint = rotate(pi<float>() / nSides, zaxis) * vec4(1, 0, 1, 0);
	vec4 thisPoint = startPoint;
	vec4 nextPoint = matRot * thisPoint;

	// Clear the data
	faceVertexBufferData.clear();
	faceUvBufferData.clear();
	faceNormalBufferData.clear();
	if (!faceOnly) {
		sideVertexBufferData.clear();
		sideUvBufferData.clear();
		sideNormalBufferData.clear();
	}
	
	for (i = 0; i < nSides; i++) {
		if (i == nSides - 1) nextPoint = startPoint;

		// Update OBB (min and max coordinates used for picking and collision)
		minCoords.x = min(minCoords.x, thisPoint.x);
		minCoords.y = min(minCoords.y, thisPoint.y);
		maxCoords.x = max(maxCoords.x, thisPoint.x);
		maxCoords.y = max(maxCoords.y, thisPoint.y);

		// Front triangle
		// Make sure your triangles go clockwise by vertex (this tells us the front face)
		faceVertexBufferData.push_back(faceCenter);
		faceVertexBufferData.push_back(vec3(thisPoint));
		faceVertexBufferData.push_back(vec3(nextPoint));
		faceUvBufferData.push_back(uvCenter);
		faceUvBufferData.push_back(vec2(thisPoint.x*uvScale.x, thisPoint.y*uvScale.y) + uvCenter);
		faceUvBufferData.push_back(vec2(nextPoint.x*uvScale.x, nextPoint.y*uvScale.y) + uvCenter);
		faceNormalBufferData.push_back(zaxis);
		faceNormalBufferData.push_back(zaxis);
		faceNormalBufferData.push_back(zaxis);
		
		// Sometimes we only update the face
		if (!faceOnly) {
			vec3 normal = normalize(vec3(thisPoint.x, thisPoint.y, 0) + vec3(thisPoint.x, thisPoint.y, 0));

			// First half of side
			sideVertexBufferData.push_back(vec3(thisPoint));
			sideVertexBufferData.push_back(vec3(thisPoint.x, thisPoint.y, -1));
			sideVertexBufferData.push_back(vec3(nextPoint));
			sideUvBufferData.push_back(vec2(1, 1));
			sideUvBufferData.push_back(vec2(1, 0));
			sideUvBufferData.push_back(vec2(0, 1));
			sideNormalBufferData.push_back(normal);
			sideNormalBufferData.push_back(normal);
			sideNormalBufferData.push_back(normal);

			// Second half of side
			sideVertexBufferData.push_back(vec3(nextPoint));
			sideVertexBufferData.push_back(vec3(thisPoint.x, thisPoint.y, -1));
			sideVertexBufferData.push_back(vec3(nextPoint.x, nextPoint.y, -1));
			sideUvBufferData.push_back(vec2(0, 1));
			sideUvBufferData.push_back(vec2(1, 0)); 
			sideUvBufferData.push_back(vec2(0, 0));
			sideNormalBufferData.push_back(normal);
			sideNormalBufferData.push_back(normal);
			sideNormalBufferData.push_back(normal);

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


// Set the program, get uniform IDs
void prismTop::setProgramId(GLuint inProgramId) {
	programId = inProgramId;
	mvpId = glGetUniformLocation(programId, "MVP");
	textureId = glGetUniformLocation(programId, "standardShadingTexture");
	modelMatrixId = glGetUniformLocation(programId, "M");
	viewMatrixId = glGetUniformLocation(programId, "V");
	lightPositionId = glGetUniformLocation(programId, "LightPosition_worldspace");
	lightColorId = glGetUniformLocation(programId, "LightColor");
	lightPowerId = glGetUniformLocation(programId, "LightPower"); 
   ambientRatioId = glGetUniformLocation(programId, "AmbientColorFactor");
	specularRatioId = glGetUniformLocation(programId, "SpecularColorFactor");
}

