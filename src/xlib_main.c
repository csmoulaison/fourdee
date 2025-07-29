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

#define panic() printf("Panic at %s:%u\n", __FILE__, __LINE__); exit(1)

#include "lerp.c"
#include "vector.c"
#include "primitives.c"
#include "input.c"
#include "game.c"
#include "opengl.c"

typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

typedef struct 
{
	Display* display;
	Window window;
	uint32_t window_width;
	uint32_t window_height;
	uint32_t mouse_moved_yet;
	uint32_t mouse_just_warped;

	struct timespec time_previous;

	Game game;

	GlContext gl; // this will become a union if multiple APIs are introduced.
	Input input;
} XlibContext;

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
		.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask
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

	gl_init(&xlib.gl);

	XWindowAttributes window_attributes;
	XGetWindowAttributes(xlib.display, xlib.window, &window_attributes);
	xlib.window_width = window_attributes.width;
	xlib.window_height = window_attributes.height;
	glViewport(0, 0, xlib.window_width, xlib.window_height);

	game_init(&xlib.game);

	// Initialize input to default
	xlib.input.mouse_delta_x = 0;
	xlib.input.mouse_delta_y = 0;
	for(uint32_t i = 0; i < INPUT_BUTTONS_LEN; i++) 
	{
		xlib.input.buttons[i].held = 0;
		xlib.input.buttons[i].pressed = 0;
		xlib.input.buttons[i].released = 0;
	}
	xlib.mouse_moved_yet = false;

	// Initialize time
    if(clock_gettime(CLOCK_REALTIME, &xlib.time_previous))
    {
        panic();
    }

	bool should_quit = false;
	while(should_quit == false)
	{
		Input* input = &xlib.input;
		input->mouse_scroll_up = false;
		input->mouse_scroll_down = false;
		input->mouse_delta_x = 0;
		input->mouse_delta_y = 0;
		input_reset_buttons(input);

		while(XPending(xlib.display))
		{
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
						input->mouse_delta_x = 0;
						input->mouse_delta_y = 0;
						input->mouse_x = event.xmotion.x;
						input->mouse_y = event.xmotion.y;
						break;
					}

					input->mouse_delta_x = event.xmotion.x - input->mouse_x;
					input->mouse_delta_y = event.xmotion.y - input->mouse_y;
					input->mouse_x = event.xmotion.x;
					input->mouse_y = event.xmotion.y;

					int32_t bounds_x = xlib.window_width / 4;
					int32_t bounds_y = xlib.window_height / 4;
					if(input->mouse_x < bounds_x ||
						input->mouse_x > xlib.window_width - bounds_x ||
						input->mouse_y < bounds_y ||
						input->mouse_y > xlib.window_height - bounds_y)
					{
						xlib.mouse_just_warped = 1;
						input->mouse_x = xlib.window_width / 2;
						input->mouse_y = xlib.window_height / 2;

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
				case ButtonPress:
				{
					switch(event.xbutton.button)
					{
						case 4:
						{
							input->mouse_scroll_up = true;
							break;
						}
						case 5:
						{
							input->mouse_scroll_down = true;
							break;
						}
						default: break;
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
							input_button_press(&input->move_forward);
							break;
						}
						case XK_a:
						{
							input_button_press(&input->move_left);
							break;
						}
						case XK_s:
						{
							input_button_press(&input->move_back);
							break;
						}
						case XK_d:
						{
							input_button_press(&input->move_right);
							break;
						}
						case XK_q:
						{
							input_button_press(&input->move_down);
							break;
						}
						case XK_e:
						{
							input_button_press(&input->move_up);
							break;
						}
						case XK_r:
						{
							input_button_press(&input->move_ana);
							break;
						}
						case XK_f:
						{
							input_button_press(&input->move_kata);
							break;
						}
						case XK_t:
						{
							input_button_press(&input->move_up_a);
							break;
						}
						case XK_g:
						{
							input_button_press(&input->move_down_a);
							break;
						}
						case XK_y:
						{
							input_button_press(&input->move_up_b);
							break;
						}
						case XK_h:
						{
							input_button_press(&input->move_down_b);
							break;
						}
						case XK_u:
						{
							input_button_press(&input->move_up_c);
							break;
						}
						case XK_j:
						{
							input_button_press(&input->move_down_c);
							break;
						}
						case XK_i:
						{
							input_button_press(&input->move_up_d);
							break;
						}
						case XK_k:
						{
							input_button_press(&input->move_down_d);
							break;
						}
						case XK_Tab:
						{
							input_button_press(&input->change_mode);
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
							input_button_release(&input->move_forward);
							break;
						}
						case XK_a:
						{
							input_button_release(&input->move_left);
							break;
						}
						case XK_s:
						{
							input_button_release(&input->move_back);
							break;
						}
						case XK_d:
						{
							input_button_release(&input->move_right);
							break;
						}
						case XK_q:
						{
							input_button_release(&input->move_down);
							break;
						}
						case XK_e:
						{
							input_button_release(&input->move_up);
							break;
						}
						case XK_r:
						{
							input_button_release(&input->move_ana);
							break;
						}
						case XK_f:
						{
							input_button_release(&input->move_kata);
							break;
						}
						case XK_t:
						{
							input_button_release(&input->move_up_a);
							break;
						}
						case XK_g:
						{
							input_button_release(&input->move_down_a);
							break;
						}
						case XK_y:
						{
							input_button_release(&input->move_up_b);
							break;
						}
						case XK_h:
						{
							input_button_release(&input->move_down_b);
							break;
						}
						case XK_u:
						{
							input_button_release(&input->move_up_c);
							break;
						}
						case XK_j:
						{
							input_button_release(&input->move_down_c);
							break;
						}
						case XK_i:
						{
							input_button_release(&input->move_up_d);
							break;
						}
						case XK_k:
						{
							input_button_release(&input->move_down_d);
							break;
						}
						case XK_Tab:
						{
							input_button_release(&input->change_mode);
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

    	game_loop(&xlib.game, &xlib.input, dt);

    	gl_loop(&xlib.gl, &xlib.game, (float)xlib.window_width, (float)xlib.window_height);
    	// TODO - deal with GlX stuff once we get another API. Too speculative as is.
		glXSwapBuffers(xlib.display, xlib.window);

		// Update debug HUD
		if(1==1) // DBG hud
		{
			printf("\033[2J\033[H");
			printf("x: %f\ny: %f\nz: %f\nphi: %f\ntheta: %f\n", 
				xlib.game.position.x,
				xlib.game.position.y,
				xlib.game.position.z,
				xlib.game.phi,
				xlib.game.theta
			);
		}
	}
	
	return 0;
}
