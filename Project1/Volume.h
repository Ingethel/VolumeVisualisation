#ifndef VOLUME_H
#define VOLUME_H

#pragma once

#include "Vector.h"

typedef struct{
	bool intersection;
	Vector3 point;
	Vector3 step;
	float time;
} INTERSECTION;

typedef struct{
	int x, y, z;
} INDEX_3;

class Ray{
public:
	Vector3 origin;
	Vector3 direction;
	Ray(void){};
	Ray(Vector3 &origin, Vector3 &direction);
	~Ray(void){};
	Vector3 operator() (float t);
};

class Plane {
public:
	Plane(void){};
	Plane(Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector3 &v4);
	~Plane(void){};
	void Intersect(Ray &ray, INTERSECTION &result, INDEX_3 &V_size);
private:
	Vector3 vertices[4];
	Vector3 centroid;
	Vector3 normal;
	float dist;
};

class Volume {
public:
	enum Sampling {NearestNeighbor, Trilinear};
	Volume(std::string filename);
	~Volume();
	int GetWidth();
	int GetHeight();
	int GetDepth();
	void Set(int x, int y, int z, unsigned char amount);

	unsigned char Get(int, int, int);
	unsigned char Get(INDEX_3);
	unsigned char Get(Vector3);

	bool Validate(int, int, int);
	bool Validate(INDEX_3);
	bool Validate(Vector3);

	INTERSECTION Intersect(Ray &ray);
	void SetSamplingMethod(Sampling s){ samplingMethod = s; }
private:
	Vector3 vertices[8];
	Plane plane[6];
	INDEX_3 size;
	unsigned char* m_data;
	int ConvertIndex(int x, int y, int z);
	Sampling samplingMethod;
};

#endif
