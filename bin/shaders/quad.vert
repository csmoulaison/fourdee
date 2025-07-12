#version 430 core
layout (location = 0) in vec2 in_position;

layout(std430, binding = 0) buffer in_color_buffer
{
	float colors[];
} color_buffer;

out float f_color;

void main()
{
	vec2 offset = vec2(mod(gl_InstanceID, 32), ((gl_InstanceID) / 32));
	offset += vec2(-16, -16);
	offset /= 20.0f;
	offset += vec2(0.04, 0.04);

	gl_Position = vec4(in_position + offset, 0, 1.0f);
	f_color = color_buffer.colors[gl_InstanceID];
}
