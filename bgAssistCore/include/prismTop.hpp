

#ifndef PRISMTOP_HPP
#define PRISMTOP_HPP

#include <texture.hpp>
#include <glfwExt.hpp>

#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;


class prismTop {
public:

	// Constructor: number of sides are fixed, so we always want to know how many up-front
	prismTop(int nSidesIn);

	// Desctructor
	~prismTop();

	// Copy constructor
	prismTop(const prismTop &inPrismTop);

	// How many triangles are in this object?
	const int nTriangles() { return nSides * 3; }

	// Setters and getters for transformations in the model matrix
	void setScale(vec3 inScaling) { scaling = inScaling; updateModelMatrixFlag = true; }
	void setTranslation(vec3 inTranslation) { translation = inTranslation; updateModelMatrixFlag = true; }
	void setRotation(GLfloat inRadians, vec3 inAxis) { rotationRadians = inRadians;  rotationAxis = inAxis;  updateModelMatrixFlag = true; }
	void setCamera(timedMat4 *inCamera) { camera = inCamera; updateMVPFlag = true; }
	void setProjection(timedMat4 *inProjection) { projection = inProjection; updateMVPFlag = true; }
	void setUvScale(vec2 uvScaleIn) { if (uvScaleIn == uvScale) return;  uvScale = uvScaleIn; faceImageChangedFlag = true; }
	void setUvCenter(vec2 uvCenterIn) { if (uvCenterIn == uvCenter) return;  uvCenter = uvCenterIn; faceImageChangedFlag = true; }
	void setMatrixId(GLuint inMatrixId) { mvpId = inMatrixId; }
	void setTextureId(GLuint inTextureId) { textureId = inTextureId; }
	vec3 getScale() { return scaling; }
	vec3 getTranslation() { return translation; }
	timedMat4 & getCamera() { return *camera; }
	timedMat4 & getProjection() { return *projection; }
	mat4 & getModelMatrix() { return modelMatrix; }
	vec3 & getMaxCoords() { return maxCoords; }
	vec3 & getMinCoords() { return minCoords; }
	vec2 & getUvScale() { return uvScale; }
	vec2 & getUvCenter() { return uvCenter; }

	// Pass the buffers to GLM only once: after you generate them
	void passBuffersToGLM(GLuint uvStaticOrDynamicForFaceImage);

	// Image loaders (from opengl tutorial's texture and shader files)
	void loadFaceImage(const char * imagepath, GLboolean ddsFormatFlag);
	void loadSideImage(const char * imagepath, GLboolean ddsFormatFlag);

	// Copy face image uv vector from another prismTop
	void copyFaceImageUvs(const prismTop &inPrismTop);

	// Draw the object; call in main loop
	void draw();

	// When we start dragging an image, mark its start location
	// As we're dragging it, use the start location as a basis for moving it
	void dragFaceImageBegin() { startUvCenter = uvCenter; }
	void dragFaceImage(vec2 shiftFromStart) { setUvCenter(startUvCenter+ shiftFromStart); }

	// Action function pointers
	void (*glfwCursorPosCallback)(GLFWwindow* window, double x, double y);
	void (*glfwMouseButtonCallback)(GLFWwindow* window, int button, int action, int mods);

private:
	int nSides; // number of sides in the polygon

	// Transformation information for the whole object
	vec3 scaling;
	vec3 translation;
	vec3 startTranslation;
	GLfloat rotationRadians;
	vec3 rotationAxis;
	timedMat4 *projection;
	timedMat4 *camera;
	mat4 modelMatrix; // model matrix
	mat4 MVP; // transformation matrix passed to GLM
	GLboolean updateModelMatrixFlag;
	GLboolean updateMVPFlag;
	double timeMVPUpdated;
	GLuint mvpId; // glGetUniformLocation(programID, "MVP");

	// UV (image mapping) transformation
	vec2 uvScale;
	vec2 uvCenter;
	vec2 startUvCenter;
	GLboolean faceImageChangedFlag;
	GLboolean ddsFaceLoadedFlag;
	GLboolean ddsSideLoadedFlag;
	GLboolean copiedFaceImageFlag;
	GLboolean copiedSideImageFlag;

	// Vertex and uv buffers
	GLuint faceVertexBufferId;
	vector<vec3> faceVertexBufferData;
	GLuint faceUvBufferId;
	vector<vec2> faceUvBufferData;

	GLuint sideVertexBufferId;
	vector<vec3> sideVertexBufferData;
	GLuint sideUvBufferId;
	vector<vec2> sideUvBufferData;

	// Texture ID
	GLuint faceImageId; // from loading the face image
	GLuint sideImageId; // from loading the side image
	GLuint textureId; // glGetUniformLocation(programID, "prismTopTexture");

	// For OBB, the minimum and maximum coordinates
	vec3 minCoords;
	vec3 maxCoords;

	// Fill vertices and uv coords
	void fillVerticesAndUVs(GLboolean faceOnly);

	// Update the face image when you move it
	void upateFaceImage();

	// Update the model matrix whenever you change the position of the object
	void updateModelMatrix();
	void updateMVP();
};


#endif


