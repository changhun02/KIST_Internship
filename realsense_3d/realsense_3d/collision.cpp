#include "collision.h"

//#############################
ObbCollision::ObbCollision() {
	x_axis = vec3(1.0f, 0.0f, 0.0f);
	y_axis = vec3(0.0f, 1.0f, 0.0f);
	z_axis = vec3(0.0f, 0.0f, 1.0f);

	cube.center_position = vec3(0.0f, 0.0f, 0.0f);
	cube.max = vec3(0.5f, 0.5f, 0.5f);
	cube.min = vec3(-0.5f, -0.5f, -0.5f);
}
ObbCollision::~ObbCollision() {}

vec3 ObbCollision::getHeightVector(Shape a) {
	vec3 ret;
	ret.x =	  a.scale.x * sinf(a.rot) / 2;
	ret.y = - a.scale.y * cosf(a.rot) / 2;
	return ret;
}

vec3 ObbCollision::getWidthVector(Shape a) {
	vec3 ret;
	ret.x =  a.scale.x * cosf(a.rot) / 2;
	ret.y = -a.scale.y * sinf(a.rot) / 2;
	return ret;
}

vec3 ObbCollision::getUnitVector(vec3 a) {
	vec3 ret;
	float size = sqrt(a.x * a.x + a.y * a.y);
	ret.x = a.x / size;
	ret.y = a.y / size;
	return ret;
}

bool ObbCollision::OBB(Shape a, Shape b) {
	vec3 dist = a.position - b.position;
	vec3 vec[4] = { getHeightVector(a), getHeightVector(b), getWidthVector(a), getWidthVector(b) };
	vec3 unit;
	for (int i = 0; 0 < 4; i++) {
		double sum = 0;
		unit = getUnitVector(vec[i]);
		for (int j = 0; j < 4; j++) {
			sum += abs(vec[j].x * unit.x + vec[j].y * unit.y);
		}
		if (abs(dist.x * unit.x + dist.y * unit.y) < sum) {
			return false;
		}
	}
	return true;
}

//#############################

vec3 ObbCollision::getProj(vec3 a, vec3 axis) {
	vec3 proj(0.0f);
	proj = (dot(a, axis) / dot(a, a))*axis; // 정사영을 이용한 vector연산
	return proj;
}

void ObbCollision::setHaptic(float* a) {
	vec3 total(0.0f);
	
	Haptic.point[0] = vec3(a[0], a[1],  a[2] );
	Haptic.point[1] = vec3(a[3], a[4],  a[5] );
	Haptic.point[2] = vec3(a[6], a[7],  a[8] );
	Haptic.point[3] = vec3(a[9], a[10], a[11]);

	for (int i = 0; i < 4; i++) {
		total += Haptic.point[i];
	}
	Haptic.center_position = total / 4.0f;
}

bool ObbCollision::Collision_detect() {
	
}