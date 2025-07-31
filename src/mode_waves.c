typedef struct
{
	Vec4f position;
	float scale;
	float constant;
	float multiplier;
} WaveMode;

void wave_mode_init(float* data)
{
	WaveMode* mode = (WaveMode*)data;

	mode->position = (Vec4f){ 0, 0, 0, 0 };
	mode->scale = 4.0f;
	mode->constant = 0.0f;
	mode->multiplier = 1.0f;
}

void wave_mode_update(float* data, Input* input, float dt)
{
	WaveMode* mode = (WaveMode*)data;

	float speed = 0.1;
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
	if(input->move_ana.held)
		mode->position.w += speed;
	if(input->move_kata.held) 
		mode->position.w -= speed;

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
