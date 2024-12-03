#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

out vec2 frag_texcoord;

uniform mat3 transform;
uniform mat3 projection;

void main() {
    gl_Position = vec4(projection * transform * vec3(in_position.xy, 1.0), 1.0);
    frag_texcoord = in_texcoord;
}
