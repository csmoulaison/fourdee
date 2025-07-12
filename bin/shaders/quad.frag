#version 430 core
out vec4 FragColor;

in float f_color;

void main()
{
    FragColor = vec4(f_color + 0.1, 0.0f, 1.0f, 1.0f);
} 
