

#ifndef PRISMTOP_HPP
#define PRISMTOP_HPP

#include <texture.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;


class prismTop {
public:

	// Constructor: number of sides are fixed, so we always want to know how many up-front
	prismTop(int nSidesIn);

	// Desctructor
	~prismTop() { delete[] g_vertex_buffer_data; delete[] g_uv_buffer_data; }

	// How many triangles are in this object?
	const int nTriangles() { return nSides * 4; }

	// Setters and getters for transformations in the model matrix
	void setScale(vec3 inScaling) { scaling = inScaling; }
	void setTranslation(vec3 inTranslation) { translation = inTranslation; }
	void setCamera(mat4 *inCamera) { camera = inCamera; }
	void setProjection(mat4 *inProjection) { projection = inProjection; }
	void setRotation(GLfloat inRadians, vec3 inAxis) { rotationRadians = inRadians;  rotationAxis = inAxis; }
	void setUvScale(vec2 uvScaleIn) { if (uvScaleIn == uvScale) return;  uvScale = uvScaleIn; imageMovedFlag = true; }
	void setUvCenter(vec2 uvCenterIn) { if (uvCenterIn == uvCenter) return;  uvCenter = uvCenterIn; imageMovedFlag = true; }
	mat4 & getCamera() { return *camera; }
	mat4 & getProjection() { return *projection; }
	mat4 & getModelMatrix() { return modelMatrix; }
	vec3 & getMaxCoords() { return maxCoords; }
	vec3 & getMinCoords() { return minCoords; }
	vec2 & getUvScale() { return uvScale; }
	vec2 & getUvCenter() { return uvCenter; }

	// Pass the buffers to GLM only once: after you generate them
	void passBuffersToGLM(GLuint uvStaticOrDynamic);

	// Update the model matrix whenever you change the position of the object
	void updateModelMatrix();
	void updateMVP() { MVP = (*projection) * (*camera) * modelMatrix; }

	// Image loaders (from opengl tutorial's texture and shader files)
	void loadBMP(const char * imagepath) { texture = loadBMP_custom(imagepath); }
	void loadDDS(const char * imagepath) { texture = ::loadDDS(imagepath); }

	// Update the face image when you move it
	void upateImage();

	// Draw the object; call in main loop
	void draw(GLuint MatrixID, GLuint TextureID);

	// When we start dragging an image, mark its start location
	// As we're dragging it, use the start location as a basis for moving it
	void dragImageBegin() { startUvCenter = uvCenter; }
	void dragImage(vec2 shiftFromStart) { setUvCenter(startUvCenter+ shiftFromStart); }

	// Action function pointers
	void (*glfwCursorPosCallback)(GLFWwindow* window, double x, double y);
	void (*doWhenSelected)(prismTop * prismTop);

private:
	int nSides; // number of sides in the polygon

	// Transformation information for the whole object
	vec3 scaling;
	vec3 translation;
	vec3 startTranslation;
	GLfloat rotationRadians;
	vec3 rotationAxis;
	mat4 *projection;
	mat4 *camera;
	mat4 modelMatrix; // model matrix
	mat4 MVP; // transformation matrix passed to GLM

	// UV (image mapping) transformation
	vec2 uvScale;
	vec2 uvCenter;
	vec2 startUvCenter;
	GLboolean imageMovedFlag;

	// Vertex and uv buffers
	GLuint vertexbuffer;
	GLfloat * g_vertex_buffer_data;
	GLuint uvbuffer;
	GLfloat * g_uv_buffer_data;

	// Texture ID
	GLuint texture;

	// For OBB, the minimum and maximum coordinates
	vec3 minCoords;
	vec3 maxCoords;

	// Fill vertices and uv coords
	void fillVerticesAndUVs();
};


#endif


