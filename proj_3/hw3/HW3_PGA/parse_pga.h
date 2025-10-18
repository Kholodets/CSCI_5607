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

//Camera & Scene Parameters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parameters
int img_width = 800, img_height = 600;
std::string imgName = "raytraced.png";

//Camera Parameters
Point3D eye = Point3D(0,0,0); 
Dir3D forward = Dir3D(0,0,-1).normalized();
Dir3D up = Dir3D(0,1,0).normalized();
Dir3D right = Dir3D(-1,0,0).normalized();
float halfAngleVFOV = 35; 

//Scene (Sphere) Parameters
Point3D spherePos = Point3D(0,0,2);
float sphereRadius = 1; 

void parseSceneFile(std::string fileName){
	
	std::ifstream scene_file(fileName);
	if (scene_file.is_open()) {
		std::string line_str;
		int orthonormalize = 0;
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
					spherePos = Point3D(x,y,z);
					sphereRadius = r;
				} else {
					printf("malformed command in scene file\n");
				}
			}
			
			if (!cmd.compare("image_resolution:")) {
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
