
#ifndef GLFWEXT_HPP
#define GLFWEXT_HPP

#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
using namespace glm;

class timedMat4 {
public:
	timedMat4() { matrix = mat4(1.0f); noteTime(); }
	timedMat4(mat4 inMatrix) { setMatrix(inMatrix); }

	void setMatrix(mat4 inMatrix) { matrix = inMatrix; noteTime();}
	mat4 getMatrix() { return matrix; }
	double timeUpdated() { return timeWasUpdated; }

private:
	mat4 matrix;
	double timeWasUpdated;

	void noteTime() { timeWasUpdated = glfwGetTime(); }
};

void screenPosToWorldRay(
	int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
	int screenWidth, int screenHeight,  // Window size, in pixels
	mat4 ViewMatrix,               // Camera position and orientation
	mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
	vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
	vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
);

bool testRayOBBIntersection(
	vec3 ray_origin,        // Ray origin, in world space
	vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
	vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
	vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
	float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
);


#endif
