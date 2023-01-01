#version 330 core

//TODO: (Light) Implement Lit Tinted Fragment Shader

in Varyings {
    vec4 color;
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
       int type; //0 point, 1 directional, 2 spot

    // Phong model (ambient, diffuse, specular)   
    // This defines the color and intensity of the light.
   vec3 diffuse;
   vec3 specular;
   vec3 ambient;

    // Position is used for point and spot lights. Direction is used for directional and spot lights.
    vec3 position;
    vec3 direction;

    // Attentuation factors are used for point and spot lights.
    //intensity of the light is affected by this equation -> 1/(a + b*d + c*d^2)
	//where a is attenuation_constant, b is attenuation_linear and c is attenuation_quadratic
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;

    // Cone angles are used for spot lights to define the inner and outer cones of spot light
	//For the space that lies between outer and inner cones, light intensity is interpolated
    float inner_angle;
    float outer_angle;
};

// Now we receive light array and the actual number of lights sent from the cpu.
//vector of all lights
uniform Light lights[MAX_LIGHT_COUNT];
uniform int light_count;

uniform vec4 albedo_tint;

uniform vec4 specular_tint;

uniform vec4 emissive_tint;

uniform vec4 ambient_tint;

uniform float shininess;

out vec4 frag_color;

void main() {
    // We normalize the normal and the view. These are done once and reused for every light type.
    //Normalize function returns a vector with the same direction as its parameter, v, but with length 1
    vec3 normal = normalize(fsin.normal); // Although the normal was already normalized, it may become shorter during interpolation.
    vec3 view = normalize(fsin.view);

    // Make sure that the actual light count never exceeds the maximum light count.
    int count = min(light_count, MAX_LIGHT_COUNT);

    // Initially the accumulated light will be the emissive.
    vec3 accumulated_light = emissive_tint.rgb;

    // Now we will loop over all the lights.
    for(int index = 0; index < count; index++){
        Light light = lights[index];
        vec3 light_direction;
        //set initial value for attenuation as no attenuation in directional light
        float attenuation = 1;
        if(light.type == TYPE_DIRECTIONAL)
            light_direction = light.direction; // If light is directional, use its direction as the light direction
        else {
            // If not directional, compute the direction from the position.
            //for point and spot lights we calculate the light direction
            light_direction = fsin.world - light.position;
            //length function returns sqrt(x[0]^2 + x[1]^2 + ......);
            float distance = length(light_direction);
            //getting unit vector that has the same direction of the light
            light_direction /= distance;

            // And compute the attenuation.
            //calculating flactuations in intensity due to the distance from light source
            attenuation *= 1.0f / (light.attenuation_constant +
            light.attenuation_linear * distance +
            light.attenuation_quadratic * distance * distance);

            if(light.type == TYPE_SPOT){
                // If it is a spot light, comput the angle attenuation.
                //for spot lights get inner and outer cone -> add their effect to the attenuation
                float angle = acos(dot(light.direction, light_direction));
                attenuation *= smoothstep(light.outer_angle, light.inner_angle, angle);
            }
        }

        // This will be used to compute the diffuse factor.
        float lambert = max(0.0f, dot(fsin.normal, -light_direction));

        // This will be used to compute the phong specular. 
        //reflect function takes (incident, normal) and returns the reflection direction calculated as I - 2.0 * dot(N, I) * N
        //For the function to work correctly normal vector must be normalized, thus initially we normalized the vector above
        float phong = pow(max(0.0f, dot(fsin.view, reflect(light_direction, fsin.normal))), shininess);

        // Now we compute the components of the light separately.
        //As color = M.ambient * I.ambient + M.diffuse * I.diffuse * lambert + M.specular * I.specular * phong
        //so color = ambient + diffuse + specular
        vec3 diffuse = albedo_tint.rgb * light.diffuse * lambert;
        vec3 specular = specular_tint.rgb * light.specular * phong;
        vec3 ambient = ambient_tint.rgb * light.ambient;

        // Then we accumulate the light components additively.
        //taking attenuation factor into consideration
        accumulated_light += (diffuse + specular) * attenuation + ambient;
    }

   //final light of the pixel
    frag_color = fsin.color * vec4(accumulated_light.rgb, albedo_tint.a);
}