#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "Volume.h"

#pragma region Helper Geometry Funcs
/*
* Calculate area of triangle with Heron's Formula
*/
float TriangleArea(Vector3 &p1, Vector3 &p2, Vector3 &p3)
{
	// compute sides of triangle
	float a = Vector3::Distance(p2, p1);
	float b = Vector3::Distance(p3, p2);
	float c = Vector3::Distance(p3, p1);
	// apply Heron's formula
	float s = (a + b + c) / 2;
	float area = sqrt(s*(s - a)*(s - b)*(s - c));

	return area;
}

/*
* Check if a point is inside a given triangle by comparing subsequent areas
*/
bool checkPointInArea(Vector3 &p1, Vector3 &p2, Vector3 &p3, Vector3 &p){
	float triangle_area = TriangleArea(p1, p2, p3);
	float error_margin = triangle_area * 0.01;
	float area_diff = abs(triangle_area - (TriangleArea(p1, p2, p) + TriangleArea(p1, p, p3) + TriangleArea(p, p2, p3)));
	if (area_diff > error_margin)
		return false;
	return true;
}

INDEX_3 VectorToIndex(Vector3 point){
	INDEX_3 index;
	index.x = roundf(point.x);
	index.y = roundf(point.y);
	index.z = roundf(point.z);
	return index;
}
#pragma endregion

#pragma region Ray
Ray::Ray(Vector3 &origin, Vector3 &direction) :
origin(origin), direction(direction)
{};

Vector3 Ray::operator() (float t) {
	return origin + direction * t;
}
#pragma endregion

#pragma region Plane
Plane::Plane(Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector3 &v4) {
	vertices[0] = v1; vertices[1] = v2; vertices[2] = v3; vertices[3] = v4;

	float a = Vector3::Distance(v1, v3);
	float b = Vector3::Distance(v2, v4);
	dist = a > b ? a / 2 : b / 2;

	centroid = (v1 + v2 + v3 + v4) / 4;

	Vector3 u = vertices[1] - vertices[0];
	Vector3 v = vertices[2] - vertices[0];
	normal = Vector3::Normalize(Vector3::Cross(u, v));
}

void Plane::Intersect(Ray &ray, INTERSECTION &result, INDEX_3 &V_size){
	if (Vector3::Dot(ray.direction, this->normal) != 0.f){
		// calculate t coefficient of line being axis aligned with polygon
		float t = Vector3::Dot((vertices[0] - ray.origin), this->normal) / Vector3::Dot(ray.direction, this->normal);
		Vector3 point = ray(t);

		// check if point is behind of camera or further from previous intersection
		if (t < 0.0f || t > result.time){
			return;
		}
		
		// check by index
		if ((point.x >= 0 && point.x < V_size.x) && (point.y >= 0 && point.y < V_size.y) && (point.z >= 0 && point.z < V_size.z)){
			result.step = ray.direction * (Vector3::Dot(Vector3::Normalize(point - ray.origin), normal) * .75);
			result.time = t;
			result.point = point;
			result.intersection = true;
		}
		
		/* check by geometry

		// check bounding circle
		if (Vector3::Distance(centroid, point) > dist){
			return;
		}

		//check if its actually inside polygon
		if (checkPointInArea(vertices[0], vertices[1], vertices[2], point) || (checkPointInArea(vertices[0], vertices[2], vertices[3], point)))
			// check if it is closer than any previous intersection
			if (t < result.time){
				result.step = ray.direction * (Vector3::Dot(Vector3::Normalize(point - ray.origin), normal) * .75);
				result.time = t;
				result.point = point;
				result.intersection = true;
			}
*/
	}
}
#pragma endregion

#pragma region Volume
Volume::Volume(std::string filename)
{

	std::ifstream f(filename.c_str());
	if (!f) {
		printf("Failed to read file %s\n", filename.c_str()); fflush(stdout);
		exit(EXIT_FAILURE);
	}

	f >> size.x; f >> size.y; f >> size.z;

	printf("Volume Width: %i Height: %i Depth: %i\n", size.x, size.y, size.z);

	m_data = new unsigned char[size.x * size.y * size.z];

	int val = 0;

	for (int z = 0; z < size.z; z++)
		for (int x = 0; x < size.x; x++)
			for (int y = 0; y < size.y; y++) {
				f >> val;
				Set(x, y, z, val);
			}

	f.close();

	vertices[0] = Vector3(0, 0, 0);
	vertices[1] = Vector3(size.x - 1, 0, 0);
	vertices[2] = Vector3(size.x - 1, size.y - 1, 0);
	vertices[3] = Vector3(0, size.y - 1, 0);

	vertices[4] = Vector3(size.x - 1, 0, size.z - 1);
	vertices[5] = Vector3(0, 0, size.z - 1);
	vertices[6] = Vector3(0, size.y - 1, size.z - 1);
	vertices[7] = Vector3(size.x - 1, size.y - 1, size.z - 1);

	plane[0] = Plane(vertices[0], vertices[1], vertices[2], vertices[3]);
	plane[1] = Plane(vertices[1], vertices[4], vertices[7], vertices[2]);
	plane[2] = Plane(vertices[4], vertices[5], vertices[6], vertices[7]);
	plane[3] = Plane(vertices[5], vertices[0], vertices[3], vertices[6]);
	plane[4] = Plane(vertices[3], vertices[2], vertices[7], vertices[6]);
	plane[5] = Plane(vertices[5], vertices[4], vertices[1], vertices[0]);

}

INTERSECTION Volume::Intersect(Ray &ray){
	INTERSECTION result;
	result.intersection = false;
	result.time = std::numeric_limits<float>::max();
	for (int i = 0; i < 6; i++){
		Vector3 temp_step;
		plane[i].Intersect(ray, result, size);
	}
	return result;
}

Volume::~Volume() {
	delete[] m_data;
}

int Volume::GetDepth() {
	return size.z;
}

int Volume::GetWidth() {
	return size.x;
}

int Volume::GetHeight() {
	return size.y;
}

void Volume::Set(int x, int y, int z, unsigned char val) {
	m_data[ConvertIndex(x, y, z)] = val;
}

unsigned char Volume::Get(int x, int y, int z) {
	return m_data[ConvertIndex(x, y, z)];
}

unsigned char Volume::Get(INDEX_3 point){
	return Get(point.x, point.y, point.z);
}

unsigned char Volume::Get(Vector3 point){
	if (samplingMethod == Trilinear){
		int x0 = floor(point.x), x1 = ceil(point.x);
		int y0 = floor(point.y), y1 = ceil(point.y);
		int z0 = floor(point.z), z1 = ceil(point.z);

		if(! (Validate(x0, y0, z0) && Validate(x1, y0, z0) && Validate(x0, y0, z1) && Validate(x1, y0, z1) && Validate(x0, y1, z0) && Validate(x1, y1, z0) && Validate(x0, y1, z1) && Validate(x1, y1, z1))){
			return 0;
		}
		
		float xd = (point.x - x0) / (x1 - x0);
		float yd = (point.y - y0) / (y1 - y0);
		float zd = (point.z - z0) / (z1 - z0);

		float c00 = Get(x0, y0, z0)*(1 - xd) + Get(x1, y0, z0)*xd;
		float c01 = Get(x0, y0, z1)*(1 - xd) + Get(x1, y0, z1)*xd;
		float c10 = Get(x0, y1, z0)*(1 - xd) + Get(x1, y1, z0)*xd;
		float c11 = Get(x0, y1, z1)*(1 - xd) + Get(x1, y1, z1)*xd;

		float c0 = c00*(1 - yd) + c10*yd;
		float c1 = c01*(1 - yd) + c11*yd;

		return c0*(1 - zd) + c1*zd;
	}
	else{
		return Get(VectorToIndex(point));
	}
}

int Volume::ConvertIndex(int x, int y, int z){
	return x + y * size.x + z * size.x * size.y;
}

bool Volume::Validate(int x, int y, int z){
	if (x >= 0 && x < size.x && y >= 0 && y < size.y && z >= 0 && z < size.z)
		return true;
	return false;
}

bool Volume::Validate(INDEX_3 point){
	return Validate(point.x, point.y, point.z);
}

bool Volume::Validate(Vector3 point){
	return Validate(VectorToIndex(point));
}

#pragma endregion
