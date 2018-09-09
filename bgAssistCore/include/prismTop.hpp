

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
	prismTop(); // just to have it

	// Desctructor
	~prismTop();

	// Copy constructor
	prismTop(const prismTop &inPrismTop);

	// How many triangles are in this object?
	const int nTriangles() { return nSides * 3; }

	// Setters and getters for transformations in the model matrix
	void setNSides(int inNSides) { if (buffsPassedFlag) return; nSides = inNSides; }
	void setProgramId(GLuint inProgramId);
	void setScale(vec3 inScaling) { scaling = inScaling; updateModelMatrixFlag = true; }
	void setTranslation(vec3 inTranslation) { translation = inTranslation; updateModelMatrixFlag = true; }
	void setRotation(GLfloat inRadians, vec3 inAxis) { rotationRadians = inRadians;  rotationAxis = inAxis;  updateModelMatrixFlag = true; }
	void setCamera(timedMat4 *inCamera) { camera = inCamera; updateMVPFlag = true; }
	void setProjection(timedMat4 *inProjection) { projection = inProjection; updateMVPFlag = true; }
	void setLight(lightSource *inLight) { light = inLight; }
	void setAmbientRatio(GLfloat inAmbientRatio) { ambientRatio = max(inAmbientRatio, 0.0f); }
	void setSpecularRatio(GLfloat inSpecularRatio) { specularRatio = max(inSpecularRatio, 0.0f); }
	void setUvScale(vec2 uvScaleIn) { if (uvScaleIn == uvScale) return;  uvScale = uvScaleIn; faceImageChangedFlag = true; }
	void setUvCenter(vec2 uvCenterIn) { if (uvCenterIn == uvCenter) return;  uvCenter = uvCenterIn; faceImageChangedFlag = true; }
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
	lightSource *light;
	GLfloat ambientRatio; // ratio to apply to ambient light (e.g. 0.1)
	GLfloat specularRatio; // ratio to apply to specular light (e.g. 0.3)

	mat4 modelMatrix; // model matrix
	mat4 MVP; // transformation matrix passed to GLM
	GLboolean updateModelMatrixFlag;
	GLboolean updateMVPFlag;
	double timeMVPUpdated;

	// GL program (shader) and uniform IDs
	GLuint programId; // from LoadShaders
	GLuint mvpId; // glGetUniformLocation(programId, "MVP");
	GLuint textureId; // glGetUniformLocation(programId, "standardShadingTexture");
	GLuint modelMatrixId; // glGetUniformLocation(programId, "M");
	GLuint viewMatrixId; // glGetUniformLocation(programId, "V");
	GLuint lightPositionId; // glGetUniformLocation(programId, "LightPosition_worldspace");
	GLuint lightPowerId; // glGetUniformLocation(programId, "LightPower");
	GLuint lightColorId; // glGetUniformLocation(programId, "LightColor");
	GLuint ambientRatioId; // glGetUniformLocation(programId, "AmbientColorFactor");
	GLuint specularRatioId; // glGetUniformLocation(programId, "SpecularColorFactor");

	// UV (image mapping) transformation
	vec2 uvScale;
	vec2 uvCenter;
	vec2 startUvCenter;
	GLboolean faceImageChangedFlag;
	GLboolean ddsFaceLoadedFlag;
	GLboolean ddsSideLoadedFlag;
	GLboolean copiedFaceImageFlag;
	GLboolean copiedSideImageFlag;

	// Vertex, uv, and normal buffers
	GLuint faceVertexBufferId;
	vector<vec3> faceVertexBufferData;
	GLuint faceUvBufferId;
	vector<vec2> faceUvBufferData;
	GLuint faceNormalBufferId;
	vector<vec3> faceNormalBufferData;

	GLuint sideVertexBufferId;
	vector<vec3> sideVertexBufferData;
	GLuint sideUvBufferId;
	vector<vec2> sideUvBufferData;
	GLuint sideNormalBufferId;
	vector<vec3> sideNormalBufferData;

	GLboolean buffsPassedFlag;

	// Image IDs
	GLuint faceImageId; // from loading the face image
	GLuint sideImageId; // from loading the side image

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

	// Actual constructor function
	void constructPrismTop(int nSidesIn);
};


#endif


