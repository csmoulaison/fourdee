typedef struct
{
	Sphere sphere;
	Vec3f position;
} PathtraceMode;

void pathtrace_mode_loop(PathtraceMode* pathtrace, Input* input, float dt)
{
	float speed = 0.1;
	if(input->move_forward.held) 
		pathtrace->position.z -= speed;
	if(input->move_left.held) 
		pathtrace->position.x -= speed;
	if(input->move_back.held)
		pathtrace->position.z += speed;
	if(input->move_right.held) 
		pathtrace->position.x += speed;
}
