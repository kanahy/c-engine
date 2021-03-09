#version 450 core

out vec4 frag_color;

in vec2 tex_coord;
in vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;

void main() {
    frag_color = mix(
        texture(texture_diffuse1, tex_coord),
        texture(texture_diffuse2, tex_coord),
        0.75
    ) * color;
}
