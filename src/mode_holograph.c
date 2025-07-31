typedef struct
{
	Sphere sphere;
	Vec3f position;
} HolographMode;

void holograph_mode_init(float* values)
{
	HolographMode* mode = (HolographMode*)values;

	mode->sphere = (Sphere){ .center = { 0, 0, 0 }, .radius = 1 };
	mode->position = (Vec3f){ -2, -2, -2 };
}

void holograph_mode_update(float* values, Input* input, float dt)
{
	HolographMode* mode = (HolographMode*)values;

	float speed = 0.1f;
	if(input->move_forward.held) 
		mode->position.z += speed;
	if(input->move_left.held) 
		mode->position.x += speed;
	if(input->move_back.held)
		mode->position.z -= speed;
	if(input->move_right.held) 
		mode->position.x -= speed;
	if(input->move_up.held)
		mode->position.y += speed;
	if(input->move_down.held) 
		mode->position.y -= speed;
}
