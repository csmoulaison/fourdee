typedef struct
{
	float position[4];
	float scale;
	float constant;
	float multiplier;
} WaveMode;

void wave_mode_init(float* data)
{
	WaveMode* mode = (WaveMode*)data;

	v4_init(mode->position, 0.0f, 0.0f, 0.0f, 0.0f);
	mode->scale = 4.0f;
	mode->constant = 0.0f;
	mode->multiplier = 1.0f;
}

void wave_mode_update(float* data, Input* input, float dt)
{
	WaveMode* mode = (WaveMode*)data;

	float speed = 0.1;
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
	if(input->move_ana.held)
		mode->position[3] += speed;
	if(input->move_kata.held) 
		mode->position[3] -= speed;

	speed = 0.05;
	if(input->move_up_a.held)
		mode->scale += speed;
	if(input->move_down_a.held) 
		mode->scale -= speed;

	if(input->move_up_b.held)
		mode->constant += speed;
	if(input->move_down_b.held) 
		mode->constant -= speed;

	if(input->move_up_c.held)
		mode->multiplier += speed;
	if(input->move_down_c.held) 
		mode->multiplier -= speed;
}
