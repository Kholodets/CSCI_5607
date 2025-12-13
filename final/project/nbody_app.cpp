#include "glad/glad.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <cstdio>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#include "nbody_cu.h"

#define N_BODIES 5

float timePast = 0;
int screen_width = 800;
int screen_height = 600;

char window_title[] = "nbody problem";

const GLchar *vertexSource = 
"#version 330 core\n"
"in vec3 pos;"
"uniform mat4 proj;"
"void main() {"
"	gl_Position = proj * vec4(pos.x, pos.y, 1.f, 1.f);"
"	gl_PointSize = pos.z;"
"}"
;

const GLchar *fragmentSource =
"#version 330 core\n"
"out vec4 fragcol;"
"void main() {"
"	fragcol = vec4(1,1,1,1);"
"}"
;

int main()
{
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
	float aspect = screen_width/(float)screen_height;

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
		exit(1);
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
		exit(1);
	}

	//Join the vertex and fragment shaders together into one program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	printf("shader program compiled\n");

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);

	
	int paramsPerVertex = 3;
	float verticies[] = {
		200.f, 200.f, 10.f,
		250.f, 250.f, 10.f,
		300.f, 300.f, 10.f
	};
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo[1];
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_DYNAMIC_DRAW);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "pos");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, paramsPerVertex * sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);


	glBindVertexArray(0);

	cudaGraphicsResource_t vbo_cr;
	cudaGraphicsGLRegisterBuffer(&vbo_cr, vbo[0], cudaGraphicsRegisterFlagsNone);
	float3 *d_pos;
	size_t pos_size;
	cudaGraphicsMapResources(1, &vbo_cr);
	cudaGraphicsResourceGetMappedPointer((void**) &d_pos, &pos_size, vbo_cr);

	test_vbo_share(d_pos);

	float2 *d_accel, *d_vel;

	init_bodies(d_pos, &d_accel, &d_vel, 3);

	int quit = 0;
	float avg_render_time = 0;
	float avg_cuda_time = 0;
	float avg_draw_time = 0;

	glBindVertexArray(vao);
	printf("entering main loop\n");
	while (!quit) {
		float t_start = SDL_GetTicks();
		SDL_Event windowEvent;

		while (SDL_PollEvent(&windowEvent)){
			// List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can get events from many special keys
			// Scancode refers to a keyboard position, keycode refers to the letter (e.g., EU keyboards)
			if (windowEvent.type == SDL_EVENT_QUIT) quit = true;
			if (windowEvent.type == SDL_EVENT_KEY_UP && windowEvent.key.key == SDLK_ESCAPE)
				quit = true;
		}
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		timePast = SDL_GetTicks() / 1000.f;
		glm::mat4 proj = glm::ortho(0.0f, (float) screen_width, 0.f, (float) screen_height, -100.f, 100.f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

		/*
		float cuda_time;
		cudaEvent_t start, stop;
		cudaEventCreate(&start);
		cudaEventCreate(&stop);
		cudaEventRecord(start, 0);
		
		process_bodies(d_pos, d_accel, d_vel, 3, avg_render_time);
		cudaDeviceSynchronize();

		cudaEventRecord(stop, 0);
		cudaEventSynchronize(stop);
		cudaEventElapsedTime(&cuda_time, start, stop);
		*/


		float t_draw_start  = SDL_GetTicks();

		//glBindVertexArray(vao);
		glDrawArrays(GL_POINTS, 0, 3);
		//glBindVertexArray(0);


		SDL_GL_SwapWindow(window);

		float t_draw_end = SDL_GetTicks();
		float t_end = SDL_GetTicks();
		char update_title[100];
		float time_per_frame = t_end-t_start;
		avg_render_time = 0.98f * avg_render_time + 0.2f * time_per_frame;
		//avg_cuda_time = 0.98f * avg_cuda_time + 0.2f * cuda_time;
		avg_draw_time = 0.98f * avg_draw_time + 0.2f * (t_draw_end - t_draw_start);
		snprintf(update_title, 100, "%s, [Update: %3.0f ms]", window_title, avg_render_time);
		printf("\r total update time: %3.0f ms, cuda function time: %3.0f ms, draw time: %3.0f ms", avg_render_time, avg_cuda_time, avg_draw_time);
		SDL_SetWindowTitle(window, update_title);

	}

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, vbo);
	glDeleteVertexArrays(1, &vao);
	SDL_GL_DestroyContext(context);
	SDL_Quit();

	/*
	GLuint vertexArray;
	glGenBuffers(N_BODIES, &vertexArray);
	glBindBuffer( GL_ARRAY_BUFFER, vertexArray);
	glBufferData(GL_ARRAY_BUFFER, 
	*/
	return 0;
}


