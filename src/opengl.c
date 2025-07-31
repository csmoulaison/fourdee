// NOTE: On Platform, Game, and GPU Interfacing:
// 
// The platform_specific_main.c orchestrates the modules which are
// combinatoric, being the root dimension, if you will.
//
// The two other dimensions are the game and the graphics API. The graphics API,
// however, respects the contract of the game, while the game has no knowledge
// of the API. The game updates its state, the platform sends this state to the
// active graphics API, and the graphics API renders accordingly.
//
// To restate: the platform is agnostic to both the API and the game, while the
// API is agnostic to the platform but specific to the game. The game is
// agnostic to both the API and the platform.
//
// Finally, commonalities between the APIs are handled by shared procedures
// called by each specific API.
 
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_BMP
#include "stb/stb_image.h"

#include "voxel_sort.c"

#define GRID_MAX_VOLUME 32768
#define TEXT_MAX_CHARS 2048

typedef struct
{
	int32_t index;
	float x;
	float y;
	float size;
} TextChar;

#include "fill_text.c"

typedef struct
{
	mat4 projection;
	int32_t grid_length;
} VoxelUbo;

typedef struct
{
	// this is a mat2, but must be separated like this for padding requirements.
	alignas(16) Vec2f transform_a;
	alignas(16) Vec2f transform_b;
} TextUbo;

typedef struct
{
	int32_t map[GRID_MAX_VOLUME];
} InstanceToVoxelSsbo;

typedef struct
{
	// Textures
	uint32_t font_texture;
	
	// UBOs
	uint32_t voxel_ubo_buffer;
	uint32_t text_ubo_buffer;
	uint32_t mode_data_ubo_buffer;

	// SSBOs
	uint32_t text_buffer;
	uint32_t instance_to_voxel_buffer;

	// VAOs
	uint32_t voxel_vao;
	uint32_t text_vao;

	// Raster programs
	uint32_t voxel_program;
	uint32_t text_program;

	// Compute programs
	uint32_t mode_programs[MAX_DIMENSIONS];
} GlContext;

uint32_t gl_compile_shader(char* filename, GLenum type)
{
	// Read file
	FILE* file = fopen(filename, "r");
	if(file == NULL) 
	{
		panic();
	}
	fseek(file, 0, SEEK_END);
	uint32_t fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	char src[fsize];

	char c;
	uint32_t i = 0;
	while((c = fgetc(file)) != EOF)
	{
		src[i] = c;
		i++;
	}
	src[i] = '\0';
	fclose(file);

	// Compile shader
	uint32_t shader = glCreateShader(type);
	const char* src_ptr = src;
	glShaderSource(shader, 1, &src_ptr, 0);
	glCompileShader(shader);

	int32_t success;
	char info[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(success == false)
	{
		glGetShaderInfoLog(shader, 512, NULL, info);
		printf(info);
		panic();
	}

	printf("compiled %s\n", filename);

	return shader;
}


void gl_init(GlContext* gl, Game* game)
{
	if(gl3wInit() != 0) 
	{
		panic();
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Voxel program
	// TODO - factor out program creation
	uint32_t voxel_vert_shader = gl_compile_shader("shaders/voxel.vert", GL_VERTEX_SHADER);
	uint32_t voxel_frag_shader = gl_compile_shader("shaders/voxel.frag", GL_FRAGMENT_SHADER);

	gl->voxel_program = glCreateProgram();
	glAttachShader(gl->voxel_program, voxel_vert_shader);
	glAttachShader(gl->voxel_program, voxel_frag_shader);
	glLinkProgram(gl->voxel_program);

	glDeleteShader(voxel_vert_shader);
	glDeleteShader(voxel_frag_shader);

	uint32_t text_vert_shader = gl_compile_shader("shaders/text.vert", GL_VERTEX_SHADER);
	uint32_t text_frag_shader = gl_compile_shader("shaders/text.frag", GL_FRAGMENT_SHADER);

	gl->text_program = glCreateProgram();
	glAttachShader(gl->text_program, text_vert_shader);
	glAttachShader(gl->text_program, text_frag_shader);
	glLinkProgram(gl->text_program);

	glDeleteShader(text_vert_shader);
	glDeleteShader(text_frag_shader);

	// Mode programs
	for(uint8_t i = 0; i < 3; i++) // TODO - change to MODES_COUNT
	{
		Mode* mode = &game->modes[i];

		uint32_t mode_shader = gl_compile_shader(mode->compute_filename, GL_COMPUTE_SHADER);
		gl->mode_programs[i] = glCreateProgram();
		glAttachShader(gl->mode_programs[i], mode_shader);
		glLinkProgram(gl->mode_programs[i]);
		glDeleteShader(mode_shader);
		printf("moder\n");
	}

	// Vertex arrays/buffers
	float voxel_vertices[] =
	{
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,

		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,

		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,	
	};

	// TODO - factor out vbo/vao creation
	glGenVertexArrays(1, &gl->voxel_vao);
	glBindVertexArray(gl->voxel_vao);

	uint32_t voxel_vertex_buffer;
	glGenBuffers(1, &voxel_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, voxel_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(voxel_vertices), voxel_vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	float text_quad_vertices[] =
	{
		 1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,

		 1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f,  1.0f
	};

	glGenVertexArrays(1, &gl->text_vao);
	glBindVertexArray(gl->text_vao);

	uint32_t text_vertex_buffer;
	glGenBuffers(1, &text_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, text_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(text_quad_vertices), text_quad_vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	// Font atlas texture
	glGenTextures(1, &gl->font_texture);
	glBindTexture(GL_TEXTURE_2D, gl->font_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int32_t w, h, channels;
	unsigned char* tex_data = stbi_load("fonts/plex_mono.bmp", &w, &h, &channels, 3);
	if(tex_data == NULL)
	{
		panic();
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(tex_data);

	// SSBOs
	// TODO - factor out ssbo creation
	float colors[GRID_MAX_VOLUME];

	uint32_t color_ssbo;
	glGenBuffers(1, &color_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, color_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(colors), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, color_ssbo);

	glGenBuffers(1, &gl->text_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl->text_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TextChar[TEXT_MAX_CHARS]), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gl->text_buffer);

	glGenBuffers(1, &gl->instance_to_voxel_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl->instance_to_voxel_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int32_t[GRID_MAX_VOLUME]), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gl->instance_to_voxel_buffer);

	// UBOs
	// TODO - factor out ubo creation
	glGenBuffers(1, &gl->voxel_ubo_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, gl->voxel_ubo_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(VoxelUbo), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &gl->text_ubo_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, gl->text_ubo_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(TextUbo), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &gl->mode_data_ubo_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, gl->mode_data_ubo_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float[MAX_DIMENSIONS]), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void gl_loop(GlContext* gl, Game* game, float window_width, float window_height)
{
	// Gl render
	glClearColor(0.84, 0.84, 0.84, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Game mode specific settings
	Mode* mode = &game->modes[game->current_mode];
	uint32_t mode_program = gl->mode_programs[game->current_mode];

	uint32_t grid_length = mode->grid_length;
	uint32_t grid_area = grid_length * grid_length;
	uint32_t grid_volume = grid_length * grid_area;

	// Update buffer
	glBindBuffer(GL_UNIFORM_BUFFER, gl->mode_data_ubo_buffer);
	void* p_mode_data_ubo = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p_mode_data_ubo, &game->mode_data, sizeof(float[MAX_DIMENSIONS]));
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	// Dispatch compute program
	glUseProgram(mode_program);
	uint32_t mode_data_ubo_block_index = glGetUniformBlockIndex(mode_program, "ubo");
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, gl->mode_data_ubo_buffer);
	glUniformBlockBinding(mode_program, mode_data_ubo_block_index, 0);

	glDispatchCompute(grid_length / 4, grid_length / 4, grid_length / 4);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Update voxel ubo
	VoxelUbo voxel_ubo;
	voxel_ubo.grid_length = grid_length;

	mat4 perspective;
	glm_perspective(glm_rad(75.0f), window_width / window_height, 0.05f, 100.0f, perspective);

	mat4 view;
	glm_mat4_identity(view);
	Vec3f cam_target = {0, 0, 0};
	Vec3f up = {0, 1, 0};
	glm_lookat((float*)&game->cam_position, (float*)&cam_target, (float*)&up, view);

	glm_mat4_mul(perspective, view, voxel_ubo.projection);

	glBindBuffer(GL_UNIFORM_BUFFER, gl->voxel_ubo_buffer);
	void* p_voxel_ubo = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p_voxel_ubo, &voxel_ubo, sizeof(voxel_ubo));
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	// Update instance to voxel map ssbo
	int32_t instance_to_voxel_map[grid_volume];
	Vec3f cam_pos = game->cam_position;

	sort_voxels(instance_to_voxel_map, grid_length, grid_area, grid_volume, cam_pos);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl->instance_to_voxel_buffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(instance_to_voxel_map), instance_to_voxel_map);

	// Draw grid
	glUseProgram(gl->voxel_program);

	uint32_t voxel_ubo_block_index = glGetUniformBlockIndex(gl->voxel_program, "ubo");
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, gl->voxel_ubo_buffer);
	glUniformBlockBinding(gl->voxel_program, voxel_ubo_block_index, 0);

	glBindVertexArray(gl->voxel_vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, grid_volume);

	// Update text ubo
	TextUbo text_ubo;	
	float text_scale_x = 27.0f / window_width;
	float text_scale_y = 46.0f / window_height;
	
	text_ubo.transform_a = (Vec2f) {  text_scale_x,  0 };
	text_ubo.transform_b = (Vec2f) {  0, -text_scale_y };

	glBindBuffer(GL_UNIFORM_BUFFER, gl->text_ubo_buffer);
	void* p_text_ubo = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p_text_ubo, &text_ubo, sizeof(text_ubo));
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	// Update text ssbo buffer
	// 
	// TODO - 1. Improve text rendering API, moving some of it to game code.
	//        2. Reason about where different calculations should be made between
	//           GPU and host. Probably a lot more here, obviously.
	//        3. Keep in mind any additional features such as text color and things.
	//           
	// Then, we should probably move on to formalizing the way we are keeping track
	// of level-specific things, and how they will end up in an asset.
	//
	// As a part of that, we will want to make the flow between levels and
	// implement the level selector.
	TextChar text_buffer[TEXT_MAX_CHARS];

	char* desc_str = "Fun with sine waves.";
	uint32_t text_i = fill_text_buffer(desc_str, text_buffer, (Vec2f){2.35, 28.25}, 1.0f);

	char* sub_desc_str = "This is just a small showcase of what our world, nay, our universe, is capable of.";
	text_i += fill_text_buffer(sub_desc_str, &text_buffer[text_i], (Vec2f){4, 59}, 0.5f);

#define DIMLEN 7
	struct 
	{
		char control_down;
		char control_up;
	} dimensions[DIMLEN] = {
		{ 'A', 'D' },
		{ 'Q', 'E' },
		{ 'W', 'S' },
		{ 'R', 'F' },
		{ 'T', 'G' },
		{ 'Y', 'H' },
		{ 'U', 'J' }
	};

	for(uint8_t i = 0; i < DIMLEN ; i++)
	{
		char s[128];
		//sprintf(s, "[%c,%c] %.1f", dimensions[i].control_down, dimensions[i].control_up, game->mode_data[i]);
		sprintf(s, "[%i] %.1f", i, game->mode_data[i]);
		text_i += fill_text_buffer(s, &text_buffer[text_i], (Vec2f){4, 2.5 + i * 1.5}, 0.66f);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl->text_buffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(text_buffer), text_buffer);

	// Draw text
	glUseProgram(gl->text_program);

	uint32_t text_ubo_block_index = glGetUniformBlockIndex(gl->text_program, "ubo");
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, gl->text_ubo_buffer);
	glUniformBlockBinding(gl->text_program, text_ubo_block_index, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl->font_texture);
	glBindVertexArray(gl->text_vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, text_i);
}
