#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;

out Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 world;
    vec3 view;
    vec3 normal;
} vs_out;

uniform mat4 transform;
uniform mat4 objectToInvTranspose;
uniform mat4 objectToWorld;
uniform mat4 view_projection;
uniform vec3 cameraPosition;

void main(){
    //TODO: (Req 7) Change the next line to apply the transformation matrix
    //gl_Position = vec4(position, 1.0);
    //vs_out.color = color;
    //vs_out.tex_coord = tex_coord;

    gl_Position = transform * vec4(position, 1.0); // apply transformation mat to the vec
    //gl_Position = view_projection * vec4(vs_out.world, 1.0);
    vs_out.world = (objectToWorld * vec4(position, 1.0f)).xyz;
    vs_out.tex_coord = tex_coord;
    vs_out.view = cameraPosition - vs_out.world;
    vs_out.color = color;
    vs_out.normal = normalize((objectToInvTranspose * vec4(normal, 0.0f)).xyz);
}