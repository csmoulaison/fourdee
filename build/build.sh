cp ../src/shaders/ ../bin/ -r

gcc ../src/xlib_main.c ../external/GL/gl3w.c \
	-o ../bin/fourdee \
	-Wall \
	-I ../external \
	-lX11 -lX11-xcb -lGL -lm -lxcb -lXfixes
