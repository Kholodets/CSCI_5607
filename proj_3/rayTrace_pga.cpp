//To Compile: g++ -fsanitize=address -std=c++11 rayTrace_pga.cpp

//For Visual Studios
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // For fopen and sscanf
#define _USE_MATH_DEFINES 
#endif

//Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files
#include "image_lib.h" //Defines an image class and a color class

//#3D PGA
#include "PGA_3D.h"

//High resolution timer
#include <chrono>

#include <cmath>

//Scene file parser
#include "parse_pga.h"
#include "hit.h"

//shade forward declaration for recursion
Color shade(Point3D rayStart, Line3D rayLine, int depth);

//this is the provided code + modification to get the normal and t value back
HitRec raySphereIntersect_fast(Point3D rayStart, Line3D rayLine, Sphere sphere, int jt){
	Point3D sphereCenter = sphere.pos;
	float sphereRadius = sphere.rad;
	Dir3D dir = rayLine.dir();
	float a = dot(dir,dir); //TODO - Understand: What do we know about "a" if "rayLine" is normalized on creation?
	Dir3D toStart = (rayStart - sphereCenter);
	float b = 2 * dot(dir,toStart);
	float c = dot(toStart,toStart) - sphereRadius*sphereRadius;
	float discr = b*b - 4*a*c;
	if (discr < 0) return HitRec();
	else{
		float t0 = (-b + sqrt(discr))/(2*a);
		float t1 = (-b - sqrt(discr))/(2*a);
		if (t0 > 0.01 || t1 > 0.01) {
			float tmin = std::min(t0 > 0.01 ? t0 : INFINITY, t1 > 0.01 ? t1 : INFINITY);
			if (jt) return HitRec(1, Material(), Point3D(), Dir3D(), Plane3D(), tmin);
			Point3D hp = rayStart + dir*tmin;
			Dir3D normdir = (hp - sphereCenter).normalized();
			Plane3D normp = Plane3D(normdir.x, normdir.y, normdir.z, 0/*this is not correct, but works for now*/);
			return HitRec(1, sphere.mat, hp, normdir, normp, tmin);			
		}
	}
	return HitRec();
}

HitRec find_intersection(Point3D rayStart, Line3D rayLine, int jt)
{
	//TODO use better scene structure, probably BVH or BSP
	HitRec minty = HitRec(0,Material(),Point3D(),Dir3D(),Plane3D(),INFINITY);
	for (Sphere s : spheres) {
		HitRec hr = raySphereIntersect_fast(rayStart, rayLine, s, jt);
		if (hr.hit) {
			if (hr.t < minty.t) {
				minty = hr;
			}				
		}
	}
	return minty;
}

Color shade(Point3D rayStart, Line3D rayLine, int depth)
{
	if (depth >= max_depth) {
		return Color(0,0,0);
	}
	HitRec minty = find_intersection(rayStart, rayLine, 0); // fresh :)

	//TODO separate lighting model into functions, potentially another file
	if (minty.hit) {
		//color based on the normal, fun vizualization
		//return Color(std::abs(minty.norm.x),std::abs(minty.norm.y),std::abs(minty.norm.z));

		float r = 0, g = 0, b = 0;
		Line3D nline = dot(minty.p, minty.norm).normalized();

		//TODO implement different types of light
		//probably use an abstract class with virtual functions which check for 
		for (PointLight light : lights) {
			//check if blocked
			Dir3D ldir = light.pos - minty.p;
			Line3D lline = vee(minty.p, ldir).normalized();
			HitRec shade = find_intersection(minty.p, lline, 1);
			float hdist = minty.p.distToSqr(shade.p);
			float ldist = minty.p.distToSqr(light.pos);
			float insq = 1/ldist;
			if (shade.hit == 0 || ldist < hdist) {

				//diffuse contribution
				float dcont = insq * std::max(0.0f, dot(dot(minty.p,minty.norm), lline));
				r += minty.mat.diffuse.r * dcont * light.col.r;
				g += minty.mat.diffuse.g * dcont * light.col.g;
				b += minty.mat.diffuse.b * dcont * light.col.b;

				//specular contribution

				if (minty.mat.ns > 0.01) {
					//phong this works
					Line3D ref = Line3D(sandwhich(minty.norm, lline));
					float scont = insq * pow(std::max(0.0f, dot(rayLine, ref)), minty.mat.ns);
				
					//blinn phong this doesnt quite work :(
					//Line3D bis = (lline + rayLine).normalized();
					//float scont = insq * pow(std::max(0.0f, dot(nline, bis)), minty.mat.ns);
				
					r += minty.mat.specular.r * scont * light.col.r;
					g += minty.mat.specular.g * scont * light.col.g;
					b += minty.mat.specular.b * scont * light.col.b;
				}



			}
		}
		//reflective contribution
		Line3D mir = Line3D(sandwhich(minty.norm, rayLine));
		Color mir_col = shade(minty.p, mir, depth + 1);

		r += minty.mat.specular.r * mir_col.r;
		g += minty.mat.specular.g * mir_col.g;
		b += minty.mat.specular.b * mir_col.b;
			
		//TODO transmissive contribution


		// ambient contribution
		r += minty.mat.ambient.r * ambient.r;
		g += minty.mat.ambient.g * ambient.g;
		b += minty.mat.ambient.b * ambient.b;

		//printf("what %f %f %f\n", minty.mat.ambient.r, minty.mat.ambient.g, mintyambient.b);
		return Color(r, g, b);
		
	} else {
		return bgcol;
	}
}

//this is the provided code from HW3, modified slightly to use my shading
int main(int argc, char** argv){

	//Read command line parameters to get scene file
	if (argc != 2){
		std::cout << "Usage: ./a.out scenefile\n";
		return(0);
	}
	std::string sceneFileName = argv[1];

	//Parse Scene File
	parseSceneFile(sceneFileName);

	float imgW = img_width, imgH = img_height;
	float halfW = imgW/2, halfH = imgH/2;
	float d = halfH / tanf(halfAngleVFOV * (M_PI / 180.0f));

	//TODO add supersampling
	//TODO parallelize shading
	Image outputImg = Image(img_width,img_height);
	auto t_start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < img_width; i++){
		for (int j = 0; j < img_height; j++){
			float u = (halfW - (imgW)*((i+0.5)/imgW));
			float v = (halfH - (imgH)*((j+0.5)/imgH));
			Point3D p = eye - d*forward + u*right + v*up;
			Dir3D rayDir = (p - eye); 
			Line3D rayLine = vee(eye,rayDir).normalized();  //Normalizing here is optional
			Color color;
			color = shade(eye, rayLine, 0);
			outputImg.setPixel(i,j, color);
		}
	}
	auto t_end = std::chrono::high_resolution_clock::now();
	printf("Rendering took %.2f ms\n",std::chrono::duration<double, std::milli>(t_end-t_start).count());

	outputImg.write(imgName.c_str());
	return 0;
}

/* not using this, kept here for reference
bool raySphereIntersect(Point3D rayStart, Line3D rayLine, Point3D sphereCenter, float sphereRadius){
	Point3D projPoint = dot(rayLine,sphereCenter)*rayLine;      //Project to find closest point between circle center and line [proj(sphereCenter,rayLine);]
	float distSqr = projPoint.distToSqr(sphereCenter);          //Point-line distance (squared)
	float d2 = distSqr/(sphereRadius*sphereRadius);             //If distance is larger than radius, then...
	if (d2 > 1) return false;                                   //... the ray missed the sphere
	float w = sphereRadius*sqrt(1-d2);                          //Pythagorean theorem to determine dist between proj point and intersection points
	Point3D p1 = projPoint - rayLine.dir()*w;                   //Add/subtract above distance to find hit points
	Point3D p2 = projPoint + rayLine.dir()*w; 

	if (dot((p1-rayStart),rayLine.dir()) >= 0) return true;     //Is the first point in same direction as the ray line?
	if (dot((p2-rayStart),rayLine.dir()) >= 0) return true;     //Is the second point in same direction as the ray line?
	return false;
}
*/
