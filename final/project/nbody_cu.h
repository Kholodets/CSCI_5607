extern "C" void init_bodies (float3 *d_pos, float2 **d_accel, float2 **d_vel, int n);
extern "C" void process_bodies(float3 *d_pos, float2 *d_accel, float2 *d_vel, int n, float dt);
extern "C" void test_vbo_share(float3 *d_pos);
