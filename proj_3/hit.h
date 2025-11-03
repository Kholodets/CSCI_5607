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


struct HitRec
{
	int hit;
	Material mat;
	Point3D p;
	Dir3D normd;
	Plane3D norm;
	float t;
	int ff;

	HitRec(int hit = 0, Material mat = Material(), Point3D p = Point3D(), Dir3D normd = Dir3D(), Plane3D norm = Plane3D(), float t = 0, int ff = 1) : hit(hit), mat(mat), p(p), normd(normd), norm(norm), t(t), ff(ff) {}
};

class Hittable
{
public:
	Hittable(){}
	virtual ~Hittable(){}

	virtual HitRec intersect(Point3D rayStart, Line3D rayLine) = 0;
};

class Sphere : public Hittable
{
	Point3D pos;
	float rad;
	Material mat;
public:
	Sphere(Point3D pos = Point3D(0,0,2), float rad = 1, Material mat = Material()) : pos(pos), rad(rad), mat(mat) {}
	~Sphere() {}
	HitRec intersect(Point3D rayStart, Line3D rayLine);

};

class Tri : public Hittable
{
public:
	Point3D v1, v2, v3;
	Dir3D n1, n2, n3;
	Material mat;
	Plane3D pl;
	int interp;
	~Tri() {}
	Tri(Point3D v1, Point3D v2, Point3D v3, Dir3D n1, Dir3D n2, Dir3D n3, Material mat) : v1(v1), v2(v2), v3(v3), n1(n1), n2(n2), n3(n3), mat(mat) {
		pl = vee(v1, v2, v3);
		interp = 1;
	}

	Tri(Point3D v1, Point3D v2, Point3D v3, Material mat) : v1(v1), v2(v2), v3(v3), mat(mat) {
		pl = vee(v1, v2, v3);
		interp = 0;
		Dir3D norm = (pl * Pseudoscalar()).normalized();
		n1 = norm;
		n2 = norm;
		n3 = norm;
	}

	HitRec intersect(Point3D rayStart, Line3D rayLine);
};

class Light
{
public:
	Light(){}
	virtual ~Light(){}

	virtual Color direct(HitRec hr, Line3D rl) = 0;
};

class PointLight : public Light
{
	Color col;
	Point3D pos;
public:
	~PointLight() {}
	PointLight(Color col = Color(0,0,0), Point3D pos = Point3D(0,0,0)): col(col), pos(pos) {}

	Color direct(HitRec hr, Line3D rl);
};

class DirLight : public Light
{
	Color col;
	Dir3D dir;
public:
	~DirLight() {}
	DirLight(Color col = Color(0,0,0), Dir3D dir = Dir3D(0,0,0)): col(col), dir(dir) {}

	Color direct(HitRec hr, Line3D rl);
};

class SpotLight : public Light
{
	Color col;
	Point3D pos;
	Dir3D dir;
	float ca1;
	float ca2;
public:
	~SpotLight() {}
	SpotLight(Color col, Point3D pos, Dir3D dir, float a1, float a2): col(col), dir(dir), pos(pos) {
		ca1 = std::cos(a1 * (3.1415 / 180.0));
		ca2 = std::cos(a2 * (3.1415 / 180.0));
	}

	Color direct(HitRec hr, Line3D rl);
};

Point3D pmin(Point3D a, Point3D b)
{
	return Point3D(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

Point3D pmax(Point3D a, Point3D b)
{
	return Point3D(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

class bvh_box
{
	Point3D min = Point3D(INFINITY, INFINITY, INFINITY);
	Point3D max = Point3D(-INFINITY, -INFINITY, -INFINITY);

public:
	bvh_box(){}
	~bvh_box(){}
	void grow(Point3D p) {
		min = pmin(p, min);
		max = pmax(p, max);
	}

	void grow(Tri *t) {
		grow(t->v1);
		grow(t->v2);
		grow(t->v3);
	}
};

class bvh_node
{
public:
	bvh_box box;
	//vector<Tri *> tris;
	int ca;
	int cb;
};

class bvh_root
{
public:
	//vector<bvh_node> nodes;
};


/*
typedef struct
{
	std::vector<Sphere> spheres;
	std::vector<PointLight> lights;
	Color ambient;
} Scene
*/
