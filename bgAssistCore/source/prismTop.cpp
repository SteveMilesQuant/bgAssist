
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
	ddsLoadedFlag = false;
	copiedTextureFlag = false;

	g_vertex_buffer_data = NULL;
	g_uv_buffer_data = NULL;

	doWhenSelected = NULL;
	glfwCursorPosCallback = NULL;

	imageChangedFlag = true;
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
	doWhenSelected = inPrismTop.doWhenSelected;
	glfwCursorPosCallback = inPrismTop.glfwCursorPosCallback;
	ddsLoadedFlag = inPrismTop.ddsLoadedFlag;
	texture = inPrismTop.texture;
	copiedTextureFlag = true;

	// Let passBuffersToGLM generate the vertices
	g_vertex_buffer_data = NULL;
	g_uv_buffer_data = NULL;

	imageChangedFlag = true;
	updateModelMatrixFlag = true;
	updateMVPFlag = true;
	timeMVPUpdated = 0;
}

// Desctuctor
prismTop::~prismTop() {
	delete[] g_vertex_buffer_data;
	delete[] g_uv_buffer_data;
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	if (!copiedTextureFlag) glDeleteTextures(1, &texture);
}

// Pass the buffers to GLM only once: after you generate them
// uvStaticOrDynamic should be GL_DYNAMIC_DRAW if you wish to update
//    the image on the face of the prism, or GL_STATIC_DRAW otherwise.
void prismTop::passBuffersToGLM(GLuint uvStaticOrDynamic) {
	int nCoordinates = nTriangles() * 9;
	int size = sizeof(GLfloat)*nCoordinates;
	g_vertex_buffer_data = new GLfloat[nCoordinates * 3];
	g_uv_buffer_data = new GLfloat[nCoordinates * 2];
	fillVerticesAndUVs();
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, size, g_vertex_buffer_data, GL_STATIC_DRAW);
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, size, g_uv_buffer_data, uvStaticOrDynamic);
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
void prismTop::upateImage() {
	if (!imageChangedFlag) return;

	int nCoordinates = nTriangles() * 9;
	int size = sizeof(GLfloat)*nCoordinates;
	fillVerticesAndUVs();
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, g_uv_buffer_data);
	imageChangedFlag = false;
}

// Draw the object in the main loop
void prismTop::draw(GLuint MatrixID, GLuint TextureID) {
	// Update the image, model matrix and MVP
	// These routines will check if this is needed
	upateImage();
	updateModelMatrix();
	updateMVP();

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(TextureID, 0);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
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
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw our object(s)
	glDrawArrays(GL_TRIANGLES, 0, nTriangles() * 3);
	glDisableVertexAttribArray(0);
}


// Fill the vertices and UV
void prismTop::fillVerticesAndUVs() {
	GLfloat * vertices = g_vertex_buffer_data;
	GLfloat * uvs = g_uv_buffer_data;
	int i, j, vertexIdx = 0, uvIdx = 0;
	vec3 v3RotAxis(0.0f, 0.0f, 1.0f); // Rotate about z+
	mat4x4 matRot = rotate(2.0f * pi<float>() / nSides, v3RotAxis);
	vec4 startPoint = rotate(pi<float>() / nSides, v3RotAxis) * vec4(1, 0, 0, 0);
	vec4 thisPoint = startPoint;
	vec4 nextPoint = matRot * thisPoint;

	for (i = 0; i < nSides; i++) {
		if (i == nSides - 1) nextPoint = startPoint;

		// Update OBB (min and max coordinates used for picking and collision)
		minCoords[0] = min(minCoords[0], thisPoint[0]);
		minCoords[1] = min(minCoords[1], thisPoint[1]);
		maxCoords[0] = max(maxCoords[0], thisPoint[0]);
		maxCoords[1] = max(maxCoords[1], thisPoint[1]);

		// Front triangle
		vertices[vertexIdx] = 0.0f;
		uvs[uvIdx++] = vertices[vertexIdx++] * uvScale[0] + uvCenter[0];
		vertices[vertexIdx] = 0.0f;
		uvs[uvIdx++] = vertices[vertexIdx++] * uvScale[1] + uvCenter[1];
		vertices[vertexIdx++] = 1.0f;

		vertices[vertexIdx] = thisPoint[0];
		uvs[uvIdx++] = vertices[vertexIdx++] * uvScale[0] + uvCenter[0];
		vertices[vertexIdx] = thisPoint[1];
		uvs[uvIdx++] = vertices[vertexIdx++] * uvScale[1] + uvCenter[1];
		vertices[vertexIdx++] = 1.0f;

		vertices[vertexIdx] = nextPoint[0];
		uvs[uvIdx++] = vertices[vertexIdx++] * uvScale[0] + uvCenter[0];
		vertices[vertexIdx] = nextPoint[1];
		uvs[uvIdx++] = vertices[vertexIdx++] * uvScale[1] + uvCenter[1];
		vertices[vertexIdx++] = 1.0f;

		// Sides will have a single color using the picture
		for (j = 0; j < 12; j++) {
			uvs[uvIdx++] = 0.0f;
		}

		// First half of side
		vertices[vertexIdx++] = thisPoint[0];
		vertices[vertexIdx++] = thisPoint[1];
		vertices[vertexIdx++] = 1.0f;

		vertices[vertexIdx++] = thisPoint[0];
		vertices[vertexIdx++] = thisPoint[1];
		vertices[vertexIdx++] = -1.0f;

		vertices[vertexIdx++] = nextPoint[0];
		vertices[vertexIdx++] = nextPoint[1];
		vertices[vertexIdx++] = 1.0f;

		// Second half of side
		vertices[vertexIdx++] = nextPoint[0];
		vertices[vertexIdx++] = nextPoint[1];
		vertices[vertexIdx++] = 1.0f;

		vertices[vertexIdx++] = nextPoint[0];
		vertices[vertexIdx++] = nextPoint[1];
		vertices[vertexIdx++] = -1.0f;

		vertices[vertexIdx++] = thisPoint[0];
		vertices[vertexIdx++] = thisPoint[1];
		vertices[vertexIdx++] = -1.0f;

		// Move to next set
		thisPoint = nextPoint;
		nextPoint = matRot * thisPoint;
	}

	// DDS inverts the Y coordinates
	if (ddsLoadedFlag) {
		uvIdx = 1;
		for (i = 0; i < nTriangles()*3; i++, uvIdx+=2) {
			uvs[uvIdx] = 1.0f - uvs[uvIdx];
		}
	}
}


void prismTop::loadBMP(const char * imagepath) {
	texture = loadBMP_custom(imagepath);
	imageChangedFlag = true;
	ddsLoadedFlag = false;
	copiedTextureFlag = false;
}

void prismTop::loadDDS(const char * imagepath) {
	texture = ::loadDDS(imagepath);
	imageChangedFlag = true;
	ddsLoadedFlag = true;
	copiedTextureFlag = false;
}




