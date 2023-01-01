#version 330

// The texture holding the scene pixels
uniform sampler2D tex;

// Read "assets/shaders/fullscreen.vert" to know what "tex_coord" holds;
in vec2 tex_coord;
out vec4 frag_color;

// A function to generate noise
float noise(vec2 st) {
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    frag_color = texture(tex, tex_coord);

    // create yellow vector
    vec4 yellow = vec4(frag_color.r, frag_color.g, -1.0, frag_color.a);

    // Generate a random noise value based on the texture coordinates
    float n = noise(tex_coord);

    // Use the noise value to randomly alter the grayscale value of the pixel
    frag_color.rgb = vec3(yellow + n * 0.2 - 0.1);
}