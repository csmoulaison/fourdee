typedef struct
{
	Sphere sphere;
	Vec3f camera_position;
} World4d;

typedef struct
{
	Vec3f camera_position;
	float phi;
	float theta;
} World3d;

typedef struct
{
	World3d world_3d;
	World4d world_4d;
} Game;

void game_init(void* memory)
{
	Game* game = (Game*)memory;
	game->world_4d.sphere = (Sphere){ .center = { 0, 0, -3 }, .radius = 1 };
	game->world_4d.camera_position = (Vec3f){ 0, 0, 0 };
	game->world_3d.camera_position = (Vec3f){ 0, 0, 0 };

	game->world_3d.phi = 3.14159 / 2;
	game->world_3d.theta = 0;
}

RenderGroup game_loop(void* memory, Input* input, float dt)
{
	Game* game = (Game*)memory;

	float speed_4d = 0.1;
	if(input->move_forward.held) 
		game->world_4d.camera_position.z -= speed_4d;
	if(input->move_left.held) 
		game->world_4d.camera_position.x -= speed_4d;
	if(input->move_back.held)
		game->world_4d.camera_position.z += speed_4d;
	if(input->move_right.held) 
		game->world_4d.camera_position.x += speed_4d;

	float speed = 0.005;
	game->world_3d.phi += input->mouse_delta_y * speed;
	game->world_3d.theta -= input->mouse_delta_x * speed;

	if(game->world_3d.phi < 0.01)
	{
		game->world_3d.phi = 0.01;
	}
	if(game->world_3d.phi > 3.14)
	{
		game->world_3d.phi = 3.14;
	}

	float distance = 2;
	game->world_3d.camera_position.x = distance * sin(game->world_3d.phi) * cos(game->world_3d.theta);
	game->world_3d.camera_position.y = distance * cos(game->world_3d.phi);
	game->world_3d.camera_position.z = distance * sin(game->world_3d.phi) * sin(game->world_3d.theta);
	
	RenderGroup render_group =
	{
		.camera_position_3d = game->world_3d.camera_position,
		.camera_position_4d = game->world_4d.camera_position,
		.sphere_4d = game->world_4d.sphere
	};
	return render_group;
}
