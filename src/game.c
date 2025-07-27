#include "mode_pathtrace.c"
#include "mode_holograph.c"

#define MODE_PATHTRACE 0
#define MODE_HOLOGRAPH 1

typedef struct
{
	uint8_t mode;
	float time_since_init;

	Vec3f position;
	float phi;
	float theta;
	float distance;
	float target_distance;

	union
	{
		PathtraceMode pathtrace;
		HolographMode holograph;
	};
} Game;

void mode_init(Game* game)
{
	switch(game->mode)
	{
		case MODE_PATHTRACE:
		{
			PathtraceMode* pathtrace = &game->pathtrace;
			pathtrace->sphere = (Sphere){ .center = { 0, 0, -3 }, .radius = 1 };
			pathtrace->position = (Vec3f){ 0, 0, 0 };
			break;
		}
		case MODE_HOLOGRAPH:
		{
			HolographMode* holograph = &game->holograph;
			holograph->sphere = (Sphere){ .center = { 0, 0, 0 }, .radius = 1 };
			holograph->position = (Vec3f){ -2, -2, -2 };
			break;
		}
		default: break;
	}
}

void game_init(Game* game)
{
	game->mode = MODE_HOLOGRAPH;
	game->time_since_init = 0;

	game->position= (Vec3f){ 0, 0, 0 };
	game->phi = 1.1;
	game->theta = 1.2;
	game->distance = 15;
	game->target_distance = 2;

	mode_init(game);
}

void game_loop(Game* game, Input* input, float dt)
{
	game->time_since_init += dt;

	float speed = 0.005;
	game->phi += input->mouse_delta_y * speed;
	game->theta -= input->mouse_delta_x * speed;

	if(game->phi < 0.01)
	{
		game->phi = 0.01;
	}
	if(game->phi > 3.14)
	{
		game->phi = 3.14;
	}

	if(game->theta < 0)
	{
		game->theta += 3.14159 * 2;
	}
	if(game->theta > 3.14159 * 2)
	{
		game->theta -= 3.14159 * 2;
	}
	if(game->theta > 10)
	{
		game->theta = 0;
	}

	float min_dist = 0;
	switch(game->mode)
	{
		case MODE_PATHTRACE:
		{
			min_dist = 1.4f;
			break;
		}
		case MODE_HOLOGRAPH:
		{
			min_dist = 0.4f;
			break;
		}
		default: break;
	}

	float scroll_speed = 0.2;
	if(input->mouse_scroll_up == true)
	{
		game->target_distance -= scroll_speed;
	}
	if(input->mouse_scroll_down == true)
	{
		game->target_distance += scroll_speed;
	}
	if(game->target_distance < min_dist)
	{
		game->target_distance = min_dist;
	}
	if(game->target_distance > 4.0f)
	{
		game->target_distance = 4.0f;
	}

	float distance_lerp_speed = 0.10f;
	if(game->distance != game->target_distance)
	{
		game->distance = lerp(game->distance, game->target_distance, distance_lerp_speed);
	}

	game->position.x = game->distance * sin(game->phi) * cos(game->theta);
	game->position.y = game->distance * cos(game->phi);
	game->position.z = game->distance * sin(game->phi) * sin(game->theta);

	switch(game->mode)
	{
		case MODE_PATHTRACE:
		{
			pathtrace_mode_loop(&game->pathtrace, input, dt);
			break;
		}
		case MODE_HOLOGRAPH:
		{
			holograph_mode_loop(&game->holograph, input, dt);
			break;
		}
		default: break;
	}

	if(input->change_mode.pressed)
	{
		printf("changing mode!\n");
		if(game->mode == MODE_PATHTRACE)
		{
			game->mode = MODE_HOLOGRAPH;
		}
		else
		{
			game->mode = MODE_PATHTRACE;
		}
		mode_init(game);
	}
}
