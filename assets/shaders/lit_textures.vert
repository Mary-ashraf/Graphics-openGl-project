#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 world;
layout(location = 4) in vec3 view;
layout(location = 5) in vec3 normal;

out Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 world;
    vec3 view;
    vec3 normal;
} vs_out;

uniform mat4 transform;

void main(){
    //TODO: (Req 7) Change the next line to apply the transformation matrix
    //gl_Position = vec4(position, 1.0);
    //vs_out.color = color;
    //vs_out.tex_coord = tex_coord;

    gl_Position = transform * vec4(position, 1.0); // apply transformation mat to the vec
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
    vs_out.world = world;
    vs_out.view = view;
    vs_out.normal = normal;
}