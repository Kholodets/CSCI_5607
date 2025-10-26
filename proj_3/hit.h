#pragma once

struct Material
{

	Color ambient;
	Color diffuse;
	Color specular;
	float ns;
	Color transmissive;
	float ior;

	Material(Color ambient = Color(0,0,0), Color diffuse = Color(1,1,1), Color specular = Color(0,0,0), float ns = 5, Color transmissive = Color(0,0,0), float ior = 1) : ambient(ambient), diffuse(diffuse), specular(specular), ns(ns), transmissive(transmissive), ior(ior) {}

};

struct Sphere
{
	Point3D pos;
	float rad;
	Material mat;
	Sphere(Point3D pos = Point3D(0,0,2), float rad = 1, Material mat = Material()) : pos(pos), rad(rad), mat(mat) {}

};

struct PointLight
{
	Color col;
	Point3D pos;

	PointLight(Color col = Color(0,0,0), Point3D pos = Point3D(0,0,0)): col(col), pos(pos) {}
};

struct HitRec
{
	int hit;
	Material mat;
	Point3D p;
	Dir3D normd;
	Plane3D norm;
	float t;

	HitRec(int hit = 0, Material mat = Material(), Point3D p = Point3D(), Dir3D normd = Dir3D(), Plane3D norm = Plane3D(), float t = 0) : hit(hit), mat(mat), p(p), normd(normd), norm(norm), t(t) {}
};

/*
typedef struct
{
	std::vector<Sphere> spheres;
	std::vector<PointLight> lights;
	Color ambient;
} Scene
*/
