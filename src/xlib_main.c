#include <GL/gl3w.h>
#include <GL/glx.h>
#include <X11/extensions/Xfixes.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "cglm/cglm.h"

typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

#include "vector.c"
#include "primitives.c"
#include "render_group.c"
#include "input.c"
#include "game.c"

#define panic() printf("Panic at %s:%u\n", __FILE__, __LINE__); exit(1)

typedef struct
{
	uint32_t quad_program;
	uint32_t quad_vao;

	uint32_t trace_program;
} GlContext;

typedef struct
{
	mat4 projection;
} QuadUbo;

typedef struct
{
	Sphere sphere;
	Vec3f camera_position;
	float time;
} TraceUbo;

typedef struct 
{
	Display* display;
	Window window;
	uint32_t window_width;
	uint32_t window_height;
	uint32_t mouse_moved_yet;
	uint32_t mouse_just_warped;

	float time_since_start;
	struct timespec time_previous;

	void* game_memory;
	uint32_t game_memory_bytes;

	GlContext gl;
	RenderGroup render_group;
	QuadUbo quad_ubo;
	TraceUbo trace_ubo;
	Input input;
} XlibContext;

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

int32_t main(int32_t argc, char** argv)
{
	XlibContext xlib;

	xlib.display = XOpenDisplay(0);
	if(xlib.display == NULL) 
	{
		panic();
	}

	// GLX related code starts here.
	int32_t glx_version_major;
	int32_t glx_version_minor;
	if(glXQueryVersion(xlib.display, &glx_version_major, &glx_version_minor) == 0
		|| ((glx_version_major == 1) && (glx_version_minor < 3)) || (glx_version_major < 1))
	{
		panic();
	}

	int32_t desired_framebuffer_attributes[] = 
	{
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_DOUBLEBUFFER, True,
		GLX_SAMPLE_BUFFERS, 1,
		GLX_SAMPLES, 4,
		None
	};

	int32_t framebuffer_configs_len;
	GLXFBConfig* framebuffer_configs = glXChooseFBConfig(xlib.display, DefaultScreen(xlib.display), desired_framebuffer_attributes, &framebuffer_configs_len);
	if(framebuffer_configs == NULL) 
	{
		panic();
	}

	// Here we choose the framebuffer config with the most samples per pixel.
	int32_t best_framebuffer_config = -1;
	int32_t most_samples = -1;
	for(int32_t i = 0; i < framebuffer_configs_len; i++)
	{
		XVisualInfo* tmp_visual_info = glXGetVisualFromFBConfig(xlib.display, framebuffer_configs[i]);
		if(tmp_visual_info != NULL)
		{
			int32_t sample_buffers;
			int32_t samples;
			glXGetFBConfigAttrib(xlib.display, framebuffer_configs[i], GLX_SAMPLE_BUFFERS, &sample_buffers);
			glXGetFBConfigAttrib(xlib.display, framebuffer_configs[i], GLX_SAMPLES, &samples);
			if(best_framebuffer_config == -1 || (sample_buffers && samples > most_samples))
			{
				best_framebuffer_config = i;
				most_samples = samples;
			}
		}
		XFree(tmp_visual_info);
	}

	GLXFBConfig framebuffer_config = framebuffer_configs[best_framebuffer_config];
	XFree(framebuffer_configs);

	// The visual info returned from the chosen framebuffer config will be used for Xlib window creation.
	XVisualInfo* visual_info = glXGetVisualFromFBConfig(xlib.display, framebuffer_config);

	Window root_window = RootWindow(xlib.display, visual_info->screen);
	XSetWindowAttributes set_window_attributes =
	{
		.colormap = XCreateColormap(xlib.display, root_window, visual_info->visual, AllocNone),
		.background_pixmap = None,
		.border_pixel = 0,
		.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask
	};

	// Here's where our xlib window is created. This will be snipped out if/when we are graphics API independent.
	xlib.window_width = 100;
	xlib.window_height = 100;
	xlib.window = XCreateWindow(xlib.display, root_window, 0, 0, xlib.window_width, xlib.window_height, 0, visual_info->depth, InputOutput, visual_info->visual, CWBorderPixel | CWColormap | CWEventMask, &set_window_attributes);
	if(xlib.window == 0) 
	{
		panic();
	}

	XFree(visual_info);

	XStoreName(xlib.display, xlib.window, "fourdee");
	XMapWindow(xlib.display, xlib.window);

	// Check for required GL extensions.
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB;
	char* gl_extensions = (char*)glXQueryExtensionsString(xlib.display, DefaultScreen(xlib.display));
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

	char* extension = "GLX_ARB_create_context";
	char* start;
	char* where;
	char* terminator;

	// Extension names should not have spaces.
	where = strchr(extension, ' ');
	if (where || *extension == '\0') 
	{
		panic();
	}

	// It takes a bit of care to be fool-proof about parsing the OpenGL
	// extensions string. Don't be fooled by sub-strings, etc.
	bool found_extension = true;
	for (start = gl_extensions;;) 
	{
		where = strstr(start, extension);
		if (!where)
		{
			break;
		}

		terminator = where + strlen(extension);

		if (where == start || *(where - 1) == ' ')
		{
			if (*terminator == ' ' || *terminator == '\0')
			{
				found_extension = true;
			}

			start = terminator;
		}
	}
	if(found_extension == false) 
	{
		panic();
	}

	// Create GLX context and window
	int32_t glx_attributes[] = 
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 6,
		None
	};

	GLXContext glx = glXCreateContextAttribsARB(xlib.display, framebuffer_config, 0, 1, glx_attributes);
	if(glXIsDirect(xlib.display, glx) == false) 
	{
		panic();
	}

	// Bind GLX to window
	glXMakeCurrent(xlib.display, xlib.window, glx);

	// Lock and hide mouse cursor
	XGrabPointer(xlib.display, xlib.window, 1, PointerMotionMask, GrabModeAsync, GrabModeAsync, xlib.window, None, CurrentTime);
	XFixesHideCursor(xlib.display, xlib.window);
	XSync(xlib.display, 1);

	// Now we initialize gl3w. In contrast with the GLX code, which will be decoupled from xlib
	// initialization in the case we support another graphics API, The following code will be decoupled
	// also from the platform layer entirely. Put another way, the following code should be able to run
	// in our Win32 platform layer, for instance.
	if(gl3wInit() != 0) 
	{
		panic();
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Shader quad_program
	uint32_t vert_shader = gl_compile_shader("shaders/quad.vert", GL_VERTEX_SHADER);
	uint32_t frag_shader = gl_compile_shader("shaders/quad.frag", GL_FRAGMENT_SHADER);

	xlib.gl.quad_program = glCreateProgram();
	glAttachShader(xlib.gl.quad_program, vert_shader);
	glAttachShader(xlib.gl.quad_program, frag_shader);
	glLinkProgram(xlib.gl.quad_program);

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	// Vertex array/buffer
	float vertices[] =
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

	glGenVertexArrays(1, &xlib.gl.quad_vao);
	glBindVertexArray(xlib.gl.quad_vao);

	uint32_t vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	float colors[1024];
	for(uint32_t i = 0; i < 1024; i++)
	{
		colors[i] = 0.5;
	}

	uint32_t color_buffer;
	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, color_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, color_buffer);

	uint32_t trace_ubo_buffer;
	glGenBuffers(1, &trace_ubo_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, trace_ubo_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(xlib.render_group), &xlib.render_group, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t quad_ubo_buffer;
	glGenBuffers(1, &quad_ubo_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, quad_ubo_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(xlib.quad_ubo), &xlib.quad_ubo, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Compute shader
	uint32_t compute_shader = gl_compile_shader("shaders/trace.comp", GL_COMPUTE_SHADER);

	xlib.gl.trace_program = glCreateProgram();
	glAttachShader(xlib.gl.trace_program, compute_shader);
	glLinkProgram(xlib.gl.trace_program);

	glDeleteShader(compute_shader);

	XWindowAttributes window_attributes;
	XGetWindowAttributes(xlib.display, xlib.window, &window_attributes);
	xlib.window_width = window_attributes.width;
	xlib.window_height = window_attributes.height;
	glViewport(0, 0, xlib.window_width, xlib.window_height);

	Game game;
	game_init((void*)&game);

	// Initialize input to default
	xlib.input.mouse_delta_x = 0;
	xlib.input.mouse_delta_y = 0;
	for(uint32_t i = 0; i < INPUT_BUTTONS_LEN; i++) 
	{
		xlib.input.buttons[i].held = 0;
		xlib.input.buttons[i].pressed = 0;
		xlib.input.buttons[i].released = 0;
	}

	// Initialize time
    if(clock_gettime(CLOCK_REALTIME, &xlib.time_previous))
    {
        panic();
    }
    xlib.time_since_start = 0;

	bool should_quit = false;
	while(should_quit == false)
	{
		xlib.input.mouse_delta_x = 0;
		xlib.input.mouse_delta_y = 0;
		while(XPending(xlib.display))
		{
			input_reset_buttons(&xlib.input);

			XEvent event;
			XNextEvent(xlib.display,  &event);
			switch(event.type)
			{
				case Expose:
				{
					break;
				}
				case ConfigureNotify:
				{
					XWindowAttributes tmp_window_attributes;
					XGetWindowAttributes(xlib.display, xlib.window, &tmp_window_attributes);

					xlib.window_width = tmp_window_attributes.width;
					xlib.window_height = tmp_window_attributes.height;
					glViewport(0, 0, xlib.window_width, xlib.window_height);
					break;
				}
				case MotionNotify:
				{
					if(xlib.mouse_just_warped) 
					{
						xlib.mouse_just_warped = 0;
						break;
					}

					if(!xlib.mouse_moved_yet) 
					{
						xlib.mouse_moved_yet = 1;
						xlib.input.mouse_delta_x = 0;
						xlib.input.mouse_delta_y = 0;
						xlib.input.mouse_x = event.xmotion.x;
						xlib.input.mouse_y = event.xmotion.y;
						break;
					}

					xlib.input.mouse_delta_x = event.xmotion.x - xlib.input.mouse_x;
					xlib.input.mouse_delta_y = event.xmotion.y - xlib.input.mouse_y;
					xlib.input.mouse_x = event.xmotion.x;
					xlib.input.mouse_y = event.xmotion.y;

					int32_t bounds_x = xlib.window_width / 4;
					int32_t bounds_y = xlib.window_height / 4;
					if(xlib.input.mouse_x < bounds_x ||
						xlib.input.mouse_x > xlib.window_width - bounds_x ||
						xlib.input.mouse_y < bounds_y ||
						xlib.input.mouse_y > xlib.window_height - bounds_y)
					{
						xlib.mouse_just_warped = 1;
						xlib.input.mouse_x = xlib.window_width / 2;
						xlib.input.mouse_y = xlib.window_height / 2;

						XWarpPointer(
							xlib.display,
							None,
							event.xmotion.window,
							0, 0, 0, 0,
							xlib.window_width / 2, xlib.window_height / 2);
						XSync(xlib.display, 0);
					}
					break;
				}
				case KeyPress:
				{
					uint32_t keysym = XLookupKeysym(&(event.xkey), 0);
					switch(keysym)
					{
						case XK_w:
						{
							input_button_press(&xlib.input.move_forward);
							break;
						}
						case XK_a:
						{
							input_button_press(&xlib.input.move_left);
							break;
						}
						case XK_s:
						{
							input_button_press(&xlib.input.move_back);
							break;
						}
						case XK_d:
						{
							input_button_press(&xlib.input.move_right);
							break;
						}
						default: break;
					}
					break;
				}
				case KeyRelease:
				{
					uint32_t keysym = XLookupKeysym(&(event.xkey), 0);
					switch(keysym)
					{
						case XK_w:
						{
							input_button_release(&xlib.input.move_forward);
							break;
						}
						case XK_a:
						{
							input_button_release(&xlib.input.move_left);
							break;
						}
						case XK_s:
						{
							input_button_release(&xlib.input.move_back);
							break;
						}
						case XK_d:
						{
							input_button_release(&xlib.input.move_right);
							break;
						}
						default: break;
					}
					break;
				}
				default: break;
			}
		}

		// Update time value
        struct timespec time_cur;
        if(clock_gettime(CLOCK_REALTIME, &time_cur))
        {
    		panic();
        }
    	float dt = time_cur.tv_sec - xlib.time_previous.tv_sec + (float)time_cur.tv_nsec / 1000000000 - (float)xlib.time_previous.tv_nsec / 1000000000;
        xlib.time_previous = time_cur;
    	xlib.time_since_start += dt;

    	xlib.render_group = game_loop((void*)&game, &xlib.input, dt);

		// NOW - It should be relatively clear how to extend this to 4d pathtracing on a 3d cube group.
		// Likely, the best incremental approach would be to get the 3d working with camera orbit and such,
		// but just duplicate 3d pathtracing results across one axis, THEN do the 4d stuff.

		// Then - have only a certain number of pixels update at a time based on the delta time passed in
		// the frame, and reflect this in the color somehow via gl_InstanceID. The last updated pixel
		// should be a bit brighter than the next, and so on.

		// Gl render
		glClearColor(0.1, 0.0, 0.2, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update trace ubo
		xlib.trace_ubo.time = xlib.time_since_start;
		xlib.trace_ubo.camera_position = xlib.render_group.camera_position_4d;

		glBindBuffer(GL_UNIFORM_BUFFER, trace_ubo_buffer);
		void* p_trace_ubo = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		memcpy(p_trace_ubo, &xlib.trace_ubo, sizeof(xlib.trace_ubo));
		glUnmapBuffer(GL_UNIFORM_BUFFER);

		// Dispatch pathtracer
		glUseProgram(xlib.gl.trace_program);

		uint32_t trace_ubo_block_index = glGetUniformBlockIndex(xlib.gl.trace_program, "ubo");
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, trace_ubo_buffer);
		glUniformBlockBinding(xlib.gl.trace_program, trace_ubo_block_index, 1);

		glDispatchCompute(16, 16, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Update quad ubo
		mat4 perspective;
		glm_perspective(glm_rad(75.0f), (float)xlib.window_width / (float)xlib.window_height, 0.05f, 100.0f, perspective);

		mat4 view;
		glm_mat4_identity(view);
		Vec3f cam_target = {0, 0, 0};
		Vec3f up = {0, 1, 0};
		glm_lookat((float*)&xlib.render_group.camera_position_3d, (float*)&cam_target, (float*)&up, view);

		glm_mat4_mul(perspective, view, xlib.quad_ubo.projection);

		glBindBuffer(GL_UNIFORM_BUFFER, quad_ubo_buffer);
		void* p_quad_ubo = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		memcpy(p_quad_ubo, &xlib.quad_ubo, sizeof(xlib.quad_ubo));
		glUnmapBuffer(GL_UNIFORM_BUFFER);

		// Draw grid
		glUseProgram(xlib.gl.quad_program);

		uint32_t quad_ubo_block_index = glGetUniformBlockIndex(xlib.gl.quad_program, "ubo");
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, quad_ubo_buffer);
		glUniformBlockBinding(xlib.gl.quad_program, quad_ubo_block_index, 1);

		glBindVertexArray(xlib.gl.quad_vao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 4096);

		glXSwapBuffers(xlib.display, xlib.window);

		// Update debug HUD
		/*
		printf("\033[2J\033[H");
		printf("x: %f\nz: %f\n", 
			xlib.render_group.camera_position_3d.x,
			xlib.render_group.camera_position_3d.z
		);
		*/
	}
	
	return 0;
}
