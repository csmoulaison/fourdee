#include "mode_pathtrace.c"
#include "mode_holograph.c"
#include "mode_waves.c"

#define MODE_PATHTRACE 0
#define MODE_HOLOGRAPH 1
#define MODE_WAVE 2

#define MODES_COUNT 256
#define MAX_DIMENSIONS 32

typedef struct
{
	char compute_filename[32];
	uint8_t grid_length; // NOW path 16   holo 8   wave 16

	void (*init)(float* data);
	void (*update)(float* data, Input* input, float dt);
} Mode;

typedef struct
{
	float time_since_init;

	Vec3f cam_position;
	float cam_phi;
	float cam_theta;
	float cam_distance;
	float cam_target_distance;

	Mode modes[MODES_COUNT];
	uint8_t current_mode;
	float mode_data[MAX_DIMENSIONS];
} Game;

void game_init(Game* game)
{
	game->time_since_init = 0;

	game->cam_position= (Vec3f){ 0, 0, 0 };
	game->cam_phi = 1.1;
	game->cam_theta = 1.2;
	game->cam_distance = 15;
	game->cam_target_distance = 2;

	game->current_mode = 0;
	game->modes[0] = (Mode) { .compute_filename = "shaders/wave.comp",      .grid_length = 16, .init = wave_mode_init,      .update = wave_mode_update },
	game->modes[1] = (Mode) { .compute_filename = "shaders/holograph.comp", .grid_length = 8,  .init = holograph_mode_init, .update = holograph_mode_update },
	game->modes[2] = (Mode) { .compute_filename = "shaders/trace.comp",     .grid_length = 16, .init = pathtrace_mode_init, .update = pathtrace_mode_update },
	game->modes[game->current_mode].init(game->mode_data);
}

void game_loop(Game* game, Input* input, float dt)
{
	game->time_since_init += dt;

	float speed = 0.005;
	if(input->mouse_left.held)
	{
		game->cam_phi += input->mouse_delta_y * speed;
		game->cam_theta -= input->mouse_delta_x * speed;
	}

	if(game->cam_phi < 0.01)
	{
		game->cam_phi = 0.01;
	}
	if(game->cam_phi > 3.14)
	{
		game->cam_phi = 3.14;
	}

	if(game->cam_theta < 0)
	{
		game->cam_theta += 3.14159 * 2;
	}
	if(game->cam_theta > 3.14159 * 2)
	{
		game->cam_theta -= 3.14159 * 2;
	}
	if(game->cam_theta > 10)
	{
		game->cam_theta = 0;
	}

	float min_dist = 0.8f;
	float scroll_speed = 0.2;
	if(input->mouse_scroll_up == true)
	{
		game->cam_target_distance -= scroll_speed;
	}
	if(input->mouse_scroll_down == true)
	{
		game->cam_target_distance += scroll_speed;
	}
	if(game->cam_target_distance < min_dist)
	{
		game->cam_target_distance = min_dist;
	}
	if(game->cam_target_distance > 8.0f)
	{
		game->cam_target_distance = 8.0f;
	}

	float zoom_lerp_speed = 0.10f;
	if(game->cam_distance != game->cam_target_distance)
	{
		game->cam_distance = lerp(game->cam_distance, game->cam_target_distance, zoom_lerp_speed);
	}

	game->cam_position.x = game->cam_distance * sin(game->cam_phi) * cos(game->cam_theta);
	game->cam_position.y = game->cam_distance * cos(game->cam_phi);
	game->cam_position.z = game->cam_distance * sin(game->cam_phi) * sin(game->cam_theta);

	game->modes[game->current_mode].update(game->mode_data, input, dt);

	if(input->change_mode.held)
	{
		uint8_t tmp_wrap = 2;
		if(input->move_left.pressed)
		{
			if(game->current_mode == 0)
			{
				game->current_mode = tmp_wrap;
			}
			else
			{
				game->current_mode--;
			}
			game->modes[game->current_mode].init(game->mode_data);
		}
		if(input->move_right.pressed)
		{
			if(game->current_mode == tmp_wrap)
			{
				game->current_mode = 0;
			}
			else
			{
				game->current_mode++;
			}
			game->modes[game->current_mode].init(game->mode_data);
		}
	}
}
