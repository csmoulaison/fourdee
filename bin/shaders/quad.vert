#version 430 core
layout (location = 0) in vec3 in_position;

layout(std430, binding = 0) buffer in_color_buffer
{
	float colors[];
} color_buffer;

out float f_color;

layout(std140, binding = 1) uniform in_ubo
{
	mat4 projection;
} ubo;

void main()
{
	vec3 offset = vec3((gl_InstanceID / 16) % 16, (gl_InstanceID / 16) / 16, mod(gl_InstanceID, 16));
	offset += vec3(-8.0f, -8.0f, -8.0f);
	offset /= 16.0f;

	gl_Position = ubo.projection * vec4((in_position / 48) + offset, 1.0f);
	f_color = color_buffer.colors[gl_InstanceID / 16];
}
