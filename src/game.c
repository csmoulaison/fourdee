#include "mode_pathtrace.c"
#include "mode_holograph.c"
#include "mode_waves.c"
#include "mode_battleship_3d.c"

#define MODES_COUNT 256
#define MAX_DIMENSIONS 16

typedef struct
{
	char compute_filename[32];
	uint8_t grid_length; // NOW path 16   holo 8   wave 16
	uint8_t visible_dimensions;

	void (*init)(float* data);
	void (*update)(float* data, Input* input, float dt);
} Mode;

typedef struct
{
	float time_since_init;

	float cam_position[3];
	float cam_phi;
	float cam_theta;
	float cam_distance;
	float cam_target_distance;

	Mode modes[MODES_COUNT];
	uint8_t current_mode;
	float mode_data[MAX_DIMENSIONS];
} Game;

void mode_init(Mode* mode, float data[MAX_DIMENSIONS])
{
	for(uint32_t i = 0; i < MAX_DIMENSIONS; i++)
	{
		data[i] = 0.0f;
	}
	mode->init(data);
}

void game_init(Game* game)
{
	game->time_since_init = 0.0f;

	game->cam_position[0] = 0.0f;
	game->cam_position[1] = 0.0f;
	game->cam_position[2] = 0.0f;
	game->cam_phi = 1.1f;
	game->cam_theta = 1.2f;
	game->cam_distance = 15.0f;
	game->cam_target_distance = 1.0f;

	game->current_mode = 3;
#define MODES_TMP_COUNT 4
	game->modes[0] = (Mode) { .compute_filename = "shaders/wave.comp",          .grid_length = 16, .visible_dimensions = 3, .init = wave_mode_init,          .update = wave_mode_update },
	game->modes[1] = (Mode) { .compute_filename = "shaders/holograph.comp",     .grid_length = 8,  .visible_dimensions = 3, .init = holograph_mode_init,     .update = holograph_mode_update },
	game->modes[2] = (Mode) { .compute_filename = "shaders/trace.comp",         .grid_length = 16, .visible_dimensions = 3, .init = pathtrace_mode_init,     .update = pathtrace_mode_update },
	game->modes[3] = (Mode) { .compute_filename = "shaders/battleship_3d.comp", .grid_length = 4,  .visible_dimensions = 3, .init = battleship_3d_mode_init, .update = battleship_3d_mode_update },
	mode_init(&game->modes[game->current_mode], game->mode_data);
}

void game_loop(Game* game, Input* input, float dt)
{
	game->time_since_init += dt;

	float speed = 0.005f;
	if(input->mouse_left.held)
	{
		game->cam_phi -= input->mouse_delta_y * speed;
		game->cam_theta += input->mouse_delta_x * speed;
	}

	if(game->cam_phi < 0.01f)
	{
		game->cam_phi = 0.01f;
	}
	if(game->cam_phi > 3.14f)
	{
		game->cam_phi = 3.14f;
	}

	if(game->cam_theta < 0.0f)
	{
		game->cam_theta += 3.14159f * 2.0f;
	}
	if(game->cam_theta > 3.14159f * 2.0f)
	{
		game->cam_theta -= 3.14159f * 2.0f;
	}
	if(game->cam_theta > 10.0f)
	{
		game->cam_theta = 0.0f;
	}

	float min_dist = 0.4f;
	float scroll_speed = 0.2f;
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

	game->cam_position[0] = game->cam_distance * sin(game->cam_phi) * cos(game->cam_theta);
	game->cam_position[1] = game->cam_distance * cos(game->cam_phi);
	game->cam_position[2] = game->cam_distance * sin(game->cam_phi) * sin(game->cam_theta);

	game->modes[game->current_mode].update(game->mode_data, input, dt);

	if(input->change_mode.held)
	{
		uint8_t tmp_wrap = MODES_TMP_COUNT - 1; // TODO - stop having to change this
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
			mode_init(&game->modes[game->current_mode], game->mode_data);
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
			mode_init(&game->modes[game->current_mode], game->mode_data);
		}
	}
}
