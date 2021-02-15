#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;
layout (location = 3) in vec4 a_color;
layout (location = 4) in vec3 a_tangent;
layout (location = 5) in vec3 a_bitangent;

out vec2 tex_coord;
out vec4 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(a_position, 1.0);
    tex_coord = a_tex_coord;
    color = a_color;
}
