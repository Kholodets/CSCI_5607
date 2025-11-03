/*
 * CSCI_5607 Homework 3
 * Lexi MacLean
 * Ray tracer scene file parsing
 */


//Set the global scene parameter variables

#ifndef PARSE_PGA_H
#define PARSE_PGA_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>

#include "hit.h"

//Camera & Scene Parameters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parameters
int img_width = 640, img_height = 480;
std::string imgName = "raytraced.png";

//TODO move all these global parameters into a scene definition struct
//Camera Parameters
Point3D eye = Point3D(0,0,0); 
Dir3D forward = Dir3D(0,0,-1).normalized();
Dir3D up = Dir3D(0,1,0).normalized();
Dir3D right = Dir3D(-1,0,0).normalized();
float halfAngleVFOV = 35; 

//Scene (Sphere) Parameters
std::vector<Hittable *> hits;
std::vector<Light *> lights;
std::vector<Point3D> verts;
std::vector<Dir3D> norms;

Point3D spherePos = Point3D(0,0,2);
float sphereRadius = 1;
Color bgcol = Color(0,0,0);
Color ambient = Color(0,0,0);
int max_depth = 5;

void parseSceneFile(std::string fileName){
	
	std::ifstream scene_file(fileName);
	if (scene_file.is_open()) {
		std::string line_str;
		int orthonormalize = 0;
		Material mat = Material();
		while (std::getline(scene_file, line_str)) {
			std::stringstream line(line_str);
			
			std::string cmd;
			line >> cmd;
			if (cmd[0] == '#') {
				continue;
			}

			if (!cmd.compare("sphere:")) {
				float x, y, z, r;
				if (line >> x >> y >> z >> r) {
					Sphere *ns = new Sphere(Point3D(x,y,z), r, mat);
					hits.push_back(ns);
				} else {
					printf("malformed command in scene file\n");
				}
			}
			
			if (!cmd.compare("vertex:")) {
				float x, y, z;
				if (line >> x >> y >> z) {
					verts.push_back(Point3D(x,y,z));
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("normal:")) {
				float x, y, z;
				if (line >> x >> y >> z) {
					norms.push_back(Dir3D(x,y,z));
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("triangle:")) {
				int v1, v2, v3;
				if (line >> v1 >> v2 >> v3) {
					Tri *t = new Tri(verts[v1], verts[v2], verts[v3], mat);
					hits.push_back(t);
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("normal_triangle:")) {
				int v1, v2, v3, n1, n2, n3;
				if (line >> v1 >> v2 >> v3 >> n1 >> n2 >> n3) {
					Tri *t = new Tri(verts[v1], verts[v2], verts[v3], norms[n1], norms[n2], norms[n3], mat);
					hits.push_back(t);
				} else {
					printf("malformed command in scene file\n");
				}
			}
			

			if (!cmd.compare("max_depth:")) {
				int n;
				if (line >> n) {
					max_depth = n;
				} else {
					printf("malformed command in scene file\n");
				}
			}


			if (!cmd.compare("point_light:")) {
				float r, g, b, x, y, z;
				if (line >> r >> g >> b >> x >> y >> z) {
					PointLight *l = new PointLight(Color(r,g,b), Point3D(x,y,z));
					lights.push_back(l);
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("directional_light:")) {
				float r, g, b, x, y, z;
				if (line >> r >> g >> b >> x >> y >> z) {
					DirLight *l = new DirLight(Color(r,g,b), Dir3D(x,y,z));
					lights.push_back(l);
				} else {
					printf("malformed command in scene file\n");
				}
			}
			
			if (!cmd.compare("spot_light:")) {
				float r, g, b, px, py, pz, dx, dy, dz, a1, a2;
				if (line >> r >> g >> b >>
						px >> py >> pz >>
						dx >> dy >> dz >> 
						a1 >> a2
						) {
					SpotLight *l = new SpotLight(Color(r,g,b), Point3D(px,py,pz), Dir3D(dx,dy,dz), a1, a2);
					lights.push_back(l);
				} else {
					printf("malformed command in scene file\n");
				}
			}
			

			if (!cmd.compare("material:")) {
				float ar, ag, ab, dr, dg, db, sr, sg, sb, ns, tr, tg, tb, ior;
				if (line >> ar >> ag >> ab >>
						dr >> dg >> db >>
						sr >> sg >> sb >> 
						ns >>
						tr >> tg >> tb >>
						ior
						) {
					mat = Material(
							Color(ar, ag, ab),
							Color(dr, dg, db),
							Color(sr, sg, sb),
							ns,
							Color(tr, tg, tb),
							ior
							);
				} else {
					printf("malformed command in scene file\n");
				}
			}
			
			if (!cmd.compare("ambient_light:")) {
				float r, g, b;
				if (line >> r >> g >> b) {
					ambient = Color(r, g, b);
				} else {
					printf("malformed command in scene file\n");
				}
			}

	
			if (!cmd.compare("background:")) {
				float r, g, b;
				if (line >> r >> g >> b) {
					bgcol = Color(r, g, b);
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("image_resolution:") || !cmd.compare("film_resolution:")) {
				int w, h;
				if (line >> w >> h) {
					img_width = w;
					img_height = h;
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("output_image:")) {
				std::string fname;
				if (line >> fname) {
					imgName = fname;
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("camera_pos:")) {
				float x, y, z;
				if (line >> x >> y >> z) {
					eye = Point3D(x, y, z);
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("camera_fwd:")) {
				orthonormalize = 1;
				float fx, fy, fz;
				if (line >> fx >> fy >> fz) {
					forward = Dir3D(fx, fy, fz).normalized();
				} else {
					printf("malformed command in scene file\n");
				}
			}

			if (!cmd.compare("camera_up:")) {
				orthonormalize = 1;
				float ux, uy, uz;
				if (line >> ux >> uy >> uz) {
					up = Dir3D(ux, uy, uz).normalized();
				} else {
					printf("malformed command in scene file\n");
				}
			}
			
			if (!cmd.compare("camera_fov_ha:")) {
				float ha;
				if (line >> ha) {
					halfAngleVFOV = ha;
				} else {
					printf("malformed command in scene file\n");
				}
			}
		}

		if (orthonormalize) {
			//check orthogonality
			MultiVector fmv = MultiVector(forward);
			MultiVector umv = MultiVector(up);

			//orthonormalize up
			MultiVector umvon = umv - ( fmv.dot(umv) * fmv );
			up = Dir3D(umvon).normalized();

			//generate right
			MultiVector rmv = -1 * fmv.times(umvon.dual());
			//im pretty sure for right hand rule i shouldnt be negating here
			//but the images are mirrored if i dont do this, so
		
			right = Dir3D(rmv.wx, rmv.wy, rmv.wz).normalized();

			//assert that these bases are in fact orthogonal
			assert(
				std::abs(fmv.dot(MultiVector(right)).s) < 0.001 &&
				std::abs(fmv.dot(umvon).s) < 0.001 &&
				std::abs(MultiVector(right).dot(umvon).s) < 0.001
			);
		}

	} else {
		perror("ifstream");
		printf("could not open provided scene file\n");
		return;
	}



	//TODO: Create an orthogonal camera basis, based on the provided up and right vectors
	printf("Orthogonal Camera Basis:\n");
	forward.print("forward");
	right.print("right");
	up.print("up");
}

#endif
