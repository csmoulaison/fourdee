typedef struct
{
	float position[3];
	Sphere sphere;
} HolographMode;

void holograph_mode_init(float* values)
{
	HolographMode* mode = (HolographMode*)values;

	v3_init(mode->position, -2.0f, -2.0f, -2.0f);
	v3_init(mode->sphere.center, 0.0f, 0.0f, 0.0f);
	mode->sphere.radius = 1.0f;
}

void holograph_mode_update(float* values, Input* input, float dt)
{
	HolographMode* mode = (HolographMode*)values;

	float speed = 0.1f;
	if(input->move_forward.held) 
		mode->position[2] += speed;
	if(input->move_left.held) 
		mode->position[0] += speed;
	if(input->move_back.held)
		mode->position[2] -= speed;
	if(input->move_right.held) 
		mode->position[0] -= speed;
	if(input->move_up.held)
		mode->position[1] += speed;
	if(input->move_down.held) 
		mode->position[1] -= speed;
}
