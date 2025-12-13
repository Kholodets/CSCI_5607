#define E2 10000000.f
#define TILE_SIZE 1024

#include <cuda_runtime.h>
#include <stdio.h>

#include "nbody_cu.h"

__device__ float2 ai_from_j(float3 bi, float3 bj)
{
	float2 d;
	d.x = bj.x - bi.x;
	d.y = bj.y - bi.y;

	//TODO implement collisions?
	//this method ignores collisions and has a softening factor
	//for very close elements so force does not
	//approach infinity
	float d2 = d.x * d.x + d.y * d.y + E2;
	float d6 = d2 * d2 * d2;
	float rd = 1.0f / sqrtf(d6);

	float cont = rd * bj.z;

	float2 a;
	a.x = cont * d.x;
	a.y = cont * d.y;

	return a;
}

__global__ void find_forces(float3 *pos, float2 *accel, int n)
{
	__shared__ float3 positions[TILE_SIZE];
	int tid = threadIdx.x + blockDim.x * blockIdx.x;
	if (tid >= n) {
		return;
	}
	int stride = blockDim.x;

	float2 ai;
	ai.x = 0;
	ai.y = 0;

	float3 bi = pos[tid];

	for (int i = threadIdx.x; i < n; i += stride) {
		positions[threadIdx.x] = pos[i];
		__syncthreads();

		for (int j = 0; j < blockDim.x; j++) {
			float2 pa = ai_from_j(bi, positions[j]);
			ai.x += pa.x;
			ai.y += pa.y;
		}
	}

	accel[tid] = ai;
}

__global__ void update(float3 *pos, float2 *accel, float2 *vel, int n, float dt)
{
	int tid = threadIdx.x + blockDim.x * blockIdx.x;
	if (tid >= n) {
		return;
	}

	float3 posi = pos[tid];
	float2 veli = vel[tid];
	float2 acceli = accel[tid];

	veli.x += acceli.x * dt;
	veli.y += acceli.y * dt;

	posi.x += veli.x * dt;
	posi.y += veli.y * dt;

	pos[tid] = posi;
	vel[tid] = veli;
}

extern "C"
void init_bodies(float3 *d_pos, float2 **d_accel, float2 **d_vel, int n)
{
	//float *h_vel = (float *) malloc(sizeof(float) * n);
	/*for (int i = 0; i < n; i++) {

	}
	*/

	cudaMalloc(d_accel, sizeof(float2) * n);
	cudaMalloc(d_vel, sizeof(float2) * n);
	cudaMemset(*d_vel, 0, sizeof(float2) *n);
}

extern "C"
void process_bodies(float3 *d_pos, float2 *d_accel, float2 *d_vel, int n, float dt)
{
	int tiles = (n + TILE_SIZE - 1) / TILE_SIZE;
	find_forces<<<tiles, TILE_SIZE>>>(d_pos, d_accel, n);

	update<<<tiles, TILE_SIZE>>>(d_pos, d_accel, d_vel, n, dt);
}

__global__ void test_vbo_share_kernel(float3 *d_pos)
{
	int i = threadIdx.x;
	d_pos[i].x = 800 - d_pos[i].x;
}

extern "C"
void test_vbo_share(float3 *d_pos) {
	test_vbo_share_kernel<<<1, 9>>>(d_pos);
}

