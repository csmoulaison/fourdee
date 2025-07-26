typedef struct
{
	Sphere sphere;
	Vec3f position;
} HolographMode;

void holograph_mode_loop(HolographMode* holograph, Input* input, float dt)
{
	float speed = 0.1;
	if(input->move_forward.held) 
		holograph->position.z += speed;
	if(input->move_left.held) 
		holograph->position.x += speed;
	if(input->move_back.held)
		holograph->position.z -= speed;
	if(input->move_right.held) 
		holograph->position.x -= speed;
	if(input->move_up.held)
		holograph->position.y += speed;
	if(input->move_down.held) 
		holograph->position.y -= speed;
}
