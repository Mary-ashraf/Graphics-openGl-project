#version 330 core

//TODO: (Light) Implement Lit Textured Fragment Shader

in Varyings {
    vec4 color;
    vec2 tex_coord;
    // We will need the vertex position in the world space,
    vec3 world;
    // the view vector (vertex to eye vector in the world space),
    vec3 view;
    // and the surface normal in the world space.
    vec3 normal;
} fsin;

// These type constants match their peers in the C++ code.
#define TYPE_POINT              0
#define TYPE_DIRECTIONAL        1
#define TYPE_SPOT               2

// This will define the maximum number of lights we can receive.
#define MAX_LIGHT_COUNT 16

// Now we will use a single struct for all light types.
struct Light {
    // This will hold the light type.
    int type;
    
    // This defines the color and intensity of the light.
   vec3 diffuse;
   vec3 specular;
   vec3 ambient;

    // Position is used for point and spot lights. Direction is used for directional and spot lights.
    vec3 position,
    vec3 direction;

    // Attentuation factors are used for point and spot lights.
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;

    // Cone angles are used for spot lights.
    float inner_angle;
    float outer_angle;
};

// Now we receive light array and the actual number of lights sent from the cpu.
uniform Light lights[MAX_LIGHT_COUNT];
uniform int light_count;

uniform sampler2D albedo_map; 
uniform vec4 albedo_tint;

uniform sampler2D specular_map; 
uniform vec4 specular_tint;

uniform sampler2D emissive_map; 
uniform vec4 emissive_tint;

uniform sampler2D ambient_occlusion_map; 
uniform vec4 ambient_tint;

uniform sampler2D roughness_map;
uniform vec2 roughness_range;
uniform float shininess;

out vec4 frag_color;

void main() {
    // We normalize the normal and the view. These are done once and reused for every light type.
    vec3 normal = normalize(fsin.normal); // Although the normal was already normalized, it may become shorter during interpolation.
    vec3 view = normalize(fsin.view);

    // Make sure that the actual light count never exceeds the maximum light count.
    int count = min(light_count, MAX_LIGHT_COUNT);

    // Roughness is used to compute the shininess (specular power).
    float roughness = mix(roughness_range.x, roughness_range.y, texture(roughness_map, fsin.tex_coord).r);
    // We are using a formula designed the Blinn-Phong model which is a popular approximation of the Phong model.
    // The source of the formula is http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    // It is noteworthy that we clamp the roughness to prevent its value from ever becoming 0 or 1 to prevent lighting artifacts.
    shininess = 2.0f/pow(clamp(roughness, 0.001f, 0.999f), 4.0f) - 2.0f;

    // Initially the accumulated light will be zero.
    vec3 accumulated_light = emissive_tint * texture(emissive_map, fsin.tex_coord).rgb;

    // Now we will loop over all the lights.
    for(int index = 0; index < count; index++){
        Light light = lights[index];
        vec3 light_direction;
        float attenuation = 1;
        if(light.type == TYPE_DIRECTIONAL)
            light_direction = light.direction; // If light is directional, use its direction as the light direction
        else {
            // If not directional, compute the direction from the position.
            light_direction = fsin.world - light.position;
            float distance = length(light_direction);
            light_direction /= distance;

            // And compute the attenuation.
            attenuation *= 1.0f / (light.attenuation_constant +
            light.attenuation_linear * distance +
            light.attenuation_quadratic * distance * distance);

            if(light.type == TYPE_SPOT){
                // If it is a spot light, comput the angle attenuation.
                float angle = acos(dot(light.direction, light_direction));
                attenuation *= smoothstep(light.outer_angle, light.inner_angle, angle);
            }
        }

        // This will be used to compute the diffuse factor.
        vec3 lambert = max(0.0f, dot(fsin.normal, -light_direction));

        // This will be used to compute the phong specular. 
        vec3 phong = pow(max(0.0f, dot(fsin.view, reflect(light_direction, fsin.normal))), shininess);

        // Now we compute the components of the light separately.
        vec3 diffuse = albedo_tint * light.diffuse * texture(albedo_map, fsin.tex_coord).rgb * lambert;
        vec3 specular = specular_tint * light.specular * texture(specular_map, fsin.tex_coord).rgb * phong;
        vec3 ambient = ambient_tint * light.ambient * texture(ambient_occlusion_map, fsin.tex_coord).r;

        // Then we accumulate the light components additively.
        accumulated_light += (diffuse + specular) * attenuation + ambient;
    }

    frag_color = fsin.color * vec4(accumulated_light, 1.0f);
}