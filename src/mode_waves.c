// NOW we are adding a third mode simply to brush up on the process as it stands at the moment,
// after which we will bake all of this into a struct, making all mode specific logic in the
// routines. Some light OOP.

typedef struct
{
	Vec4f position;
	float scale;
	float constant;
	float multiplier;
} WaveMode;

void wave_mode_loop(WaveMode* wave, Input* input, float dt)
{
	float speed = 0.1;
	if(input->move_forward.held) 
		wave->position.z += speed;
	if(input->move_left.held) 
		wave->position.x += speed;
	if(input->move_back.held)
		wave->position.z -= speed;
	if(input->move_right.held) 
		wave->position.x -= speed;
	if(input->move_up.held)
		wave->position.y += speed;
	if(input->move_down.held) 
		wave->position.y -= speed;

	speed = 0.05;
	if(input->move_ana.held)
		wave->position.w += speed;
	if(input->move_kata.held) 
		wave->position.w -= speed;

	speed = 0.05;
	if(input->move_up_a.held)
		wave->scale += speed;
	if(input->move_down_a.held) 
		wave->scale -= speed;

	if(input->move_up_b.held)
		wave->constant += speed;
	if(input->move_down_b.held) 
		wave->constant -= speed;

	if(input->move_up_c.held)
		wave->multiplier += speed;
	if(input->move_down_c.held) 
		wave->multiplier -= speed;
}
