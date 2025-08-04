typedef struct
{
	float position[3];
} Battleship3dMode;

void battleship_3d_mode_init(float* data)
{
	Battleship3dMode* mode = (Battleship3dMode*)data;

	v3_init(mode->position, 0.0f, 0.0f, 0.0f);
}

void battleship_3d_mode_update(float* data, Input* input, float dt)
{
	Battleship3dMode* mode = (Battleship3dMode*)data;

	if(input->move_back.pressed) 
	{
		if(mode->position[2] >= 3.0f)
		{
			mode->position[2] = 0.0f;
		}
		else
		{
			mode->position[2] += 1.0f;
		}
	}
	if(input->move_right.pressed) 
	{
		if(mode->position[0] >= 3.0f)
		{
			mode->position[0] = 0.0f;
		}
		else
		{
			mode->position[0] += 1.0f;
		}
	}
	if(input->move_forward.pressed)
	{
		if(mode->position[2] <= 0.0f)
		{
			mode->position[2] = 3.0f;
		}
		else
		{
			mode->position[2] -= 1.0f;
		}
	}
	if(input->move_left.pressed) 
	{
		if(mode->position[0] <= 0.0f)
		{
			mode->position[0] = 3.0f;
		}
		else
		{
			mode->position[0] -= 1.0f;
		}
	}
	if(input->move_up.pressed)
	{
		if(mode->position[1] >= 3.0f)
		{
			mode->position[1] = 0.0f;
		}
		else
		{
			mode->position[1] += 1.0f;
		}
	}
	if(input->move_down.pressed) 
	{
		if(mode->position[1] <= 0.0f)
		{
			mode->position[1] = 3.0f;
		}
		else
		{
			mode->position[1] -= 1.0f;
		}
	}

}
