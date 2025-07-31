typedef struct
{
	Sphere sphere;
	Vec3f position;
} PathtraceMode;

void pathtrace_mode_init(float* data)
{
	PathtraceMode* mode = (PathtraceMode*)data;

	mode->sphere = (Sphere){ .center = { 0, 0, -3 }, .radius = 1 };
	mode->position = (Vec3f){ 0, 0, 0 };
}

void pathtrace_mode_update(float* data, Input* input, float dt)
{
	PathtraceMode* mode = (PathtraceMode*)data;

	float speed = 0.1;
	if(input->move_forward.held) 
		mode->position.z -= speed;
	if(input->move_left.held) 
		mode->position.x -= speed;
	if(input->move_back.held)
		mode->position.z += speed;
	if(input->move_right.held) 
		mode->position.x += speed;
}
