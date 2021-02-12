#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <math.h>

using namespace glm;
//struct BoundingBox {
//	glm::vec3 min;
//	glm::vec3 max;
//}AABB;

//OBB Collision Detection
struct Shape {
	vec3 position;
	vec3 center_position;
	vec3 n_vec;
	vec3 max;
	vec3 min;
	vec3 point[4];
	vec3 scale;
	float rot;
};

class ObbCollision {
public:
	ObbCollision();
	~ObbCollision();
	bool OBB(Shape a, Shape b);

	//#######################################

	bool Collision_detect();
	void setHaptic(float* a);

private:
	
	Shape cube;
	Shape Haptic;
	Shape shape[2];
	vec3 getProj(vec3 a, vec3 axis);

	//#######################################

	vec3 getHeightVector(Shape a);
	vec3 getWidthVector(Shape a);
	vec3 getUnitVector(vec3 a);
	
	//Additional Part of Collision
	vec3 x_axis, y_axis, z_axis;
	bool x_collision, y_collision, z_collision;

};

//bool detectCollision(BoundingBox a, BoundingBox b) {
//	return (a.min <= b.max && b.min <= a.max);
//}

//###################################################################
//###################################################################
//###################################################################
class SphereToPlane {

public:
	SphereToPlane();
	~SphereToPlane();
	bool Collision_detect(Shape a);

private:
	vec3 sphere_center;
	float radius;

};