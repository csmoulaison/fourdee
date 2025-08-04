typedef struct
{
	float position[3];
	Sphere sphere;
} PathtraceMode;

void pathtrace_mode_init(float* data)
{
	PathtraceMode* mode = (PathtraceMode*)data;

	v3_init(mode->position, 0.0f, 0.0f, 0.0f);
	v3_init(mode->sphere.center, 0.0f, 0.0f, -3.0f);
	mode->sphere.radius = 1.0f;
}

void pathtrace_mode_update(float* data, Input* input, float dt)
{
	PathtraceMode* mode = (PathtraceMode*)data;

	float speed = 0.1;
	if(input->move_forward.held) 
		mode->position[2] -= speed;
	if(input->move_left.held) 
		mode->position[0] -= speed;
	if(input->move_back.held)
		mode->position[2] += speed;
	if(input->move_right.held) 
		mode->position[0] += speed;
}
