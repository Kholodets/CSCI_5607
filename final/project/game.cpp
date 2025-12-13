//For Visual Studios
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>


#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>


bool saveOutput = true;
float timePast = 0;

// Shader sources
const GLchar* vertexSource =
"#version 150 core\n"
"in vec3 position;"
"const vec3 inColor = vec3(0.f,0.7f,0.f);"
"in vec3 inNormal;"
"out vec3 Color;"
"out vec3 normal;"
"out vec3 pos;"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"
"void main() {"
"   Color = inColor;"
"   gl_Position = proj * view * model * vec4(position,1.0);"
"   pos = (view * model * vec4(position,1.0)).xyz;"
"   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);" //Or Just: normal = normalize(view*model* vec4(inNormal,0.0)).xyz; //faster, but wrong if skew or non-uniform scale in model matrix
"   normal = normalize(norm4.xyz);"
"}";

const GLchar* fragmentSource =
"#version 150 core\n"
"in vec3 Color;"
"in vec3 normal;"
"in vec3 pos;"
//"const vec3 lightDir = normalize(vec3(1,1,1));"
"out vec4 outColor;"
"const float ambient = .3;"
"uniform mat4 view;"
"uniform vec3 mcol;"
"void main() {"
"   vec3 lightDir = mat3(view) * normalize(vec3(1,1,1));"
"   vec3 diffuseC = mcol*max(dot(lightDir,normal),0.0);"
"   vec3 ambC = mcol*ambient;"
"   vec3 viewDir = normalize(-pos);" //We know the eye is at (0,0)! [Q: Why?]
"   vec3 reflectDir = reflect(viewDir,normal);"
"   float spec = max(dot(reflectDir,-lightDir),0.0);"
"   if (dot(lightDir,normal) <= 0.0) spec = 0;"
"   vec3 specC = vec3(1.0,1.0,1.0)*pow(spec,4);"
"   outColor = vec4(diffuseC+ambC+specC, 1.0);"
"}";

bool fullscreen = false;
int screen_width = 800;
int screen_height = 600;

char window_title[] = "My OpenGL Program";

float avg_render_time = 0;

void Win2PPM(int width, int height);
int check_map(char *map, bool *keys, int h, int w, int x, int y);

// main function updated for SDL3 and with debugging checks

int main(int argc, char *argv[]){
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

	//Print the version of SDL we are using (should be 3.x or higher)
	const int sdl_linked = SDL_GetVersion();
	printf("\nCompiled against SDL version %d.%d.%d ...\n", SDL_VERSIONNUM_MAJOR(SDL_VERSION), SDL_VERSIONNUM_MINOR(SDL_VERSION), SDL_VERSIONNUM_MICRO(SDL_VERSION));
	printf("Linking against SDL version %d.%d.%d.\n", SDL_VERSIONNUM_MAJOR(sdl_linked), SDL_VERSIONNUM_MINOR(sdl_linked), SDL_VERSIONNUM_MICRO(sdl_linked));

	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (title, width, height, flags)
	SDL_Window* window = SDL_CreateWindow(window_title, screen_width, screen_height, SDL_WINDOW_OPENGL);
	float aspect = screen_width/(float)screen_height; //aspect ratio (needs to be updated if the window is resized)
							  //The above window cannot be resized which makes some code slightly easier.
							  //Below we show how to make a full screen window or allow resizing
							  //SDL_Window* window = SDL_CreateWindow(window_title, screen_width, screen_height, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
							  //SDL_Window* window = SDL_CreateWindow(window_title, screen_width, screen_height, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
	if (!window) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)){
		printf("\nOpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n\n", glGetString(GL_VERSION));
	}
	else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}

	// --- Model Loading with Error Checking ---
	std::string file_name = "models/cube.txt";
	std::ifstream modelFile(file_name);
	if (!modelFile.is_open()) {
		printf("ERROR: Model file '%s' not found or could not be opened.\n", file_name.c_str());
		printf("Ensure the 'models' folder is next to your executable.\n");
		SDL_GL_DestroyContext(context);
		SDL_Quit();
		return 1;
	}

	int numModelParams = 0;
	modelFile >> numModelParams;
	if (numModelParams <= 0) {
		printf("ERROR: Model file is empty or does not start with a valid float count.\n");
		SDL_GL_DestroyContext(context);
		SDL_Quit();
		return 1;
	}

	float* modelData = new float[numModelParams];
	for (int i = 0; i < numModelParams; i++){
		modelFile >> modelData[i];
	}
	modelFile.close();




	printf("Model total float count: %d\n", numModelParams);
	const int paramsPerVertex = 8; // Assumes X,Y,Z, U,V, Nx,Ny,Nz
	int numVertices = numModelParams / paramsPerVertex;

	//Load the vertex Shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); 
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	//Let's double check the shader compiled 
	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (!status){
		char buffer[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
		printf("Vertex Shader Compile Failed. Info:\n\n%s\n",buffer);
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	//Double check the shader compiled 
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (!status){
		char buffer[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
		printf("Fragment Shader Compile Failed. Info:\n\n%s\n",buffer);
	}

	//Join the vertex and fragment shaders together into one program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	//Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
	GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context

	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo[1];
	glGenBuffers(1, vbo);  //Create 1 buffer called vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
	glBufferData(GL_ARRAY_BUFFER, numModelParams*sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
												//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
												//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used

												// --- Vertex Attribute Setup ---
												// This setup assumes the data in our file is formatted as:
												// 8 floats per vertex: [X, Y, Z, U, V, Nx, Ny, Nz]

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, paramsPerVertex * sizeof(float), 0);
	//Attribute, vals/attrib., type, is_normalized, stride, offset
	//Binds to VBO current GL_ARRAY_BUFFER 
	glEnableVertexAttribArray(posAttrib);

	// E.g., if you had color data per vertex, you would do this:
	//GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, paramsPerVertex*sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(colAttrib);

	GLint normAttrib = glGetAttribLocation(shaderProgram, "inNormal");
	// Normal data starts after 5 floats (X,Y,Z,U,V)
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, paramsPerVertex * sizeof(float), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);

	glBindVertexArray(0);  //Unbind the VAO


	//If you need a second VAO (e.g., if some of the models are stored in a second format)
	//Here is what that looks like--
	//GLuint vao2;  
	//glGenVertexArrays(1, &vao2); //Create the VAO
	//glBindVertexArray(vao2); //Bind the above created VAO to the current context
	//  Creat VBOs ... 
	//  Set-up attributes ...
	//glBindVertexArray(0); //Unbind the VAO



	glEnable(GL_DEPTH_TEST);  

	float cx = 0.f, cy = 0.f, cz = 0.f;
	int mw;
	int mh;
	std::cin >> mw >> mh;
	char game_map[mw * mh];
	for (int i = 0; i < mw * mh; ++i) {
		char ch;
		std::cin.get(ch);
		if (ch == '\n') {
			--i;
			continue;
		}
		game_map[i] = ch;
		//printf("read %c\n", ch);
		if (ch == 'S') {
			cx = (i % mw);
			cy = (i / mw);
		}
	}

	float keycol[15] = 
	{
		0.f, 1.f, 0.f,
		1.f, 1.f, 0.f,
		1.f, 0.f, 1.f,
		0.f, 1.f, 1.f,
		0.f, 0.f, 1.f
	};


	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	SDL_SetWindowRelativeMouseMode(window, 1);
	bool keys[5] = {false, false, false, false, false};
	bool quit = false;
	float vx = 0.f, vy = 0.f, vz = 0.f;
	float pitch = 0, yaw = 0;
	while (!quit){
		float t_start = SDL_GetTicks();

		while (SDL_PollEvent(&windowEvent)){
			// List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can get events from many special keys
			// Scancode refers to a keyboard position, keycode refers to the letter (e.g., EU keyboards)
			if (windowEvent.type == SDL_EVENT_QUIT) quit = true;
			if (windowEvent.type == SDL_EVENT_KEY_UP && windowEvent.key.key == SDLK_ESCAPE)
				quit = true;
			if (windowEvent.type == SDL_EVENT_KEY_UP && windowEvent.key.key == SDLK_F) { // Toggle fullscreen with 'F' key
				fullscreen = !fullscreen;
				SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
			}

			if (windowEvent.type == SDL_EVENT_MOUSE_MOTION) {
				yaw += windowEvent.motion.xrel / 1000;
				pitch -= windowEvent.motion.yrel / 1000;

				if (pitch > 1.5708f) {
					pitch = 1.5707f;
				}

				if (pitch < -1.5708f) {
					pitch = -1.5707f;
				}
			}

			/*
			   if (windowEvent.type == SDL_EVENT_KEY_DOWN) {
			   switch(windowEvent.key.key) {
			   case SDLK_W:
			   vz += -0.005f;
			   break;
			   case SDLK_S:
			   vz += 0.005f;
			   break;
			   }
			   }

			   if (windowEvent.type == SDL_EVENT_KEY_UP) {
			   switch(windowEvent.key.key) {
			   case SDLK_W:
			   vz -= -0.01f;
			   break;
			   case SDLK_S:
			   vz -= 0.01f;
			   break;
			   }
			   }
			   */
		}

		const bool* keyStates = SDL_GetKeyboardState(NULL);

		vx = 0.f, vy = 0.f, vz = 0.f;
		if (keyStates[SDL_SCANCODE_W]) {
			vx += -0.0051f;
		}

		if (keyStates[SDL_SCANCODE_S]) {
			vx += 0.005f;
		}

		if (keyStates[SDL_SCANCODE_A]) {
			vy += -0.005f;
		}

		if (keyStates[SDL_SCANCODE_D]) {
			vy += 0.005f;
		}


		float dx = (vx*cos(yaw) + vy*sin(yaw)) * avg_render_time;
		int t1 = -1 + (int) (dx*2 + cx + 1.f);
		int t2 = -1 + (int) (cy + 1.f);
		int res;
		if (res = check_map(game_map, keys, mh, mw, t1, t2)) {
			cx += dx;
		}
		//printf("\rcx %f, cy %f, t1 %d, t2 %d    ", cx, cy, t1, t2);

		if (res == 2) {
			printf("you win!\n");
			quit = 1;
		}

		float dy = (vy*cos(yaw) - vx*sin(yaw)) * avg_render_time;
		t1 = -1 + (int) (cx + 1.f);
		t2 = -1 + (int) (cy + dy*2 + 1.f);
		if (res = check_map(game_map, keys, mh, mw, t1, t2)) {
			cy += dy;
		}

		if (res == 2) {
			printf("you win!\n");
			quit = 1;
		}

		//cz += vz * avg_render_time;

		float mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);



		// Clear the screen to target color and clear depth buffer
		glClearColor(.2f, 0.4f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!saveOutput) timePast = SDL_GetTicks()/1000.f; 
		else timePast += .07;

		glm::mat4 model = glm::mat4(1);
		//model = glm::rotate(model,timePast * glm::radians(90.0f),glm::vec3(0.0f, 1.0f, 1.0f));
		//model = glm::rotate(model,timePast * glm::radians(45.0f),glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 view = glm::lookAt(
				glm::vec3(cx, cy, cz),    //Cam Position
				glm::vec3(cx - (cos(yaw) * cos(pitch)), cy+(sin(yaw) * cos(pitch)), cz + sin(pitch)),  //Look At Point
				glm::vec3(0.0f, 0.0f, 1.0f)); //Up Vector
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f); //FOV, aspect, near, far
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

		for (int i = 0; i < mh; ++i) {
			for (int j = 0; j < mw; ++j) {
				glUniform3f(glGetUniformLocation(shaderProgram,"mcol"), 0.8f, 0.25f, 0.1f);
				glm::mat4 floor = glm::translate(model, glm::vec3(((float) j) + 0.5f, ((float) i) + 0.5f, -1.0f));
				//wall = glm::scale(wall, glm::vec3(0.75f, 0.75f, 0.75f));
				glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(floor));
				glBindVertexArray(vao); //Bind the VAO for the shader(s) we are using
				glDrawArrays(GL_TRIANGLES, 0, numVertices); //Primitives, Which VBO, Number of vertices
				int noth = 0;
				char c = game_map[i*mw + j];
				if (c == 'W') {
					glUniform3f(glGetUniformLocation(shaderProgram,"mcol"), 1.f, 1.f, 1.f);
					glm::mat4 wall = glm::translate(model, glm::vec3(((float) j) + 0.5f, ((float) i) + 0.5f, 0.f));
					//wall = glm::scale(wall, glm::vec3(0.75f, 0.75f, 0.75f));
					glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(wall));
				}
				if (c == 'G') {
					glUniform3f(glGetUniformLocation(shaderProgram,"mcol"), 1.f, 0.6f, 0.1f);
					glm::mat4 goal = glm::translate(model, glm::vec3(((float) j) + 0.5f, ((float) i) + 0.5f, 0.f));
					goal = glm::scale(goal, glm::vec3(0.6f, 0.6f, 5.f));
					glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(goal));
				}
				if (c >= 'A' && c <= 'E') {
					int ci = c - 'A';
					glUniform3f(glGetUniformLocation(shaderProgram,"mcol"), keycol[ci * 3 + 0], keycol[ci * 3 + 1], keycol[ci * 3 + 2]);
					glm::mat4 door = glm::translate(model, glm::vec3(((float) j) + 0.5f, ((float) i) + 0.5f, 0.f));
					door = glm::scale(door, glm::vec3(0.75f, 0.75f, 0.75f));
					glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(door));
				}
				if (c >= 'a' && c <= 'e') {

					int ci = c - 'a';
					glUniform3f(glGetUniformLocation(shaderProgram,"mcol"), keycol[ci * 3 + 0], keycol[ci * 3 + 1], keycol[ci * 3 + 2]);
					glm::mat4 key = glm::translate(model, glm::vec3(((float) j) + 0.5f, ((float) i) + 0.5f, 0.f));
					key = glm::scale(key, glm::vec3(0.25f, 0.25f, 0.25f));
					glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(key));
				}
				if (c == '0') {
					noth = 1;
				}
				if (!noth) {
					//glBindVertexArray(vao); //Bind the VAO for the shader(s) we are using
					glDrawArrays(GL_TRIANGLES, 0, numVertices); //Primitives, Which VBO, Number of vertices
				}

			}
		}

		for (int i = 0; i < 5; ++i) {
			if (keys[i]) {
				float np = pitch + 0.3;
				float ny = yaw - 0.4f + (0.2f * (float) i);
				glUniform3f(glGetUniformLocation(shaderProgram,"mcol"), keycol[i * 3 + 0], keycol[i * 3 + 1], keycol[i * 3 + 2]);
				glm::vec3 kp = glm::vec3(cx - 0.2*(cos(ny) * cos(np)), cy+0.2*(sin(ny) * cos(np)), cz + 0.2*sin(np));
				glm::mat4 key = glm::translate(model, kp);
				key = glm::scale(key, glm::vec3(0.02f, 0.02f, 0.02f));
				glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(key));
				glDrawArrays(GL_TRIANGLES, 0, numVertices); //Primitives, Which VBO, Number of vertices
			}

		}


		//glBindVertexArray(vao); //Bind the VAO for the shader(s) we are using
		//glDrawArrays(GL_TRIANGLES, 0, numVertices); //Primitives, Which VBO, Number of vertices

		//if (saveOutput) Win2PPM(screen_width,screen_height);
		//if (saveOutput) timePast += .05; //Fix framerate at 20 FPS
		SDL_GL_SwapWindow(window); //Double buffering

		float t_end = SDL_GetTicks();
		char update_title[100];
		float time_per_frame = t_end-t_start;
		avg_render_time = .98f*avg_render_time + .02f*time_per_frame;  //Weighted average for smoothing
		snprintf(update_title, 100, "%s [Update: %3.0f ms]",window_title,avg_render_time);
		SDL_SetWindowTitle(window, update_title);
	}

	delete[] modelData;

	//Clean Up
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, vbo);
	glDeleteVertexArrays(1, &vao);
	SDL_GL_DestroyContext(context);
	SDL_Quit();
	return 0;
}

int check_map(char *map, bool *keys, int h, int w, int x, int y)
{
	if (!(x >= 0 && x < w && y >= 0 && y < h)) {
		return 0;
	}

	char c = map[y * w + x];

	if (c >= 'a' && c <= 'e') { //key
		keys[c - 'a'] = true;
		map[y * w + x] = '0';
		return 1;
	}

	if (c >= 'A' && c <= 'E') {
		if (keys[c - 'A']) {
			keys[c - 'A'] = false;
			map[y * w + x] = '0';
			return 1;
		} else {
			return 0;
		}
	}

	if (c == 'W') {
		return 0;
	}

	if (c == 'G') {
		return 2;
	}

	return 1;
}



//Write out PPM image from screen
void Win2PPM(int width, int height){
	char outdir[10] = "out/"; //Must be defined!
	int i,j;
	FILE* fptr;
	static int counter = 0;
	char fname[32];
	unsigned char *image;

	/* Allocate our buffer for the image */
	image = (unsigned char *)malloc(3*width*height*sizeof(char));
	if (image == NULL) {
		fprintf(stderr,"ERROR: Failed to allocate memory for image\n");
	}

	/* Open the file */
	snprintf(fname, sizeof(fname), "%simage_%04d.ppm",outdir,counter);
	if ((fptr = fopen(fname,"w")) == NULL) {
		fprintf(stderr,"ERROR: Failed to open file to write image\n");
	}

	/* Copy the image into our buffer */
	glReadBuffer(GL_BACK);
	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

	/* Write the PPM file */
	fprintf(fptr,"P6\n%d %d\n255\n",width,height);
	for (j=height-1;j>=0;j--) {
		for (i=0;i<width;i++) {
			fputc(image[3*j*width+3*i+0],fptr);
			fputc(image[3*j*width+3*i+1],fptr);
			fputc(image[3*j*width+3*i+2],fptr);
		}
	}

	free(image);
	fclose(fptr);
	counter++;
}
