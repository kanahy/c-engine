#version 450 core

out vec4 frag_color;

in vec2 tex_coord;
in vec4 color;

uniform sampler2D texture_diffuse;

void main() {
    frag_color = texture(texture_diffuse, tex_coord) * color;
}
