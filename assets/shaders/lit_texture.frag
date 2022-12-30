#version 330 core

in Varyings {
   vec4 color;
   vec2 tex_coord;
   vec3 world;
   vec3 view;
   vec3 normal;
} fsin;

struct Material {
   vec3 diffuse;
   vec3 specular;
   vec3 ambient;
   vec3 emissive;
   float shininess;
};

struct TexturedMaterial{
   sampler2D albedo_map;
   sampler2D specular_map;
   sampler2D ambient_occlusion_map; 
   sampler2D roughness_map;
   sampler2D emissive_map;
   vec2 roughness_range; 
   vec3 albedo_tint;
   vec3 specular_tint;
   vec3 emissive_tint;
};

struct Light {
   int type;
   vec3 diffuse;
   vec3 specular;
   vec3 ambient;
   vec3 emissive;
   vec3 position, direction;
   float attenuation_constant;
   float attenuation_linear;
   float attenuation_quadratic;
   float inner_angle, outer_angle;
};

#define TYPE_POINT          0
#define TYPE_DIRECTIONAL    1
#define TYPE_SPOT           2
#define MAX_LIGHT_COUNT     16

uniform Light lights[MAX_LIGHT_COUNT];
uniform int light_count;
uniform TexturedMaterial tex_material;

out vec4 frag_color;

uniform vec4 tint;

void main(){
    ////////////////////////////////////////////////////////////////////////////////////////////
    vec3 normal = normalize(fsin.normal);
    vec3 view = normalize(fsin.view);

    int count = min(light_count, MAX_LIGHT_COUNT);
    vec3 accumulated_light = vec3(0.0);

    for(int index = 0; index < count; index++){
      Light light = lights[index];
      vec3 light_direction;
      float attenuation = 1;
      if(light.type == TYPE_DIRECTIONAL)
         light_direction = light.direction;
      else {
         light_direction = fsin.world - light.position;
         float distance = length(light_direction);
         light_direction /= distance;
         attenuation *= 1.0f / (light.attenuation_constant +
                        light.attenuation_linear * distance +
                        light.attenuation_quadratic * distance * distance);
         if(light.type == TYPE_SPOT){
            float angle = acos(dot(light.direction, light_direction));
            attenuation *= smoothstep(light.outer_angle, light.inner_angle, angle);
         }
      }
      vec3 reflected = reflect(light_direction, normal);
      float lambert = max(0.0f, dot(normal, -light_direction));
      Material material;
      material.diffuse = tex_material.albedo_tint * texture(tex_material.albedo_map, fsin.tex_coord).rgb;
      material.specular = tex_material.specular_tint * texture(tex_material.specular_map, fsin.tex_coord).rgb;
      material.emissive = tex_material.emissive_tint * texture(tex_material.emissive_map, fsin.tex_coord).rgb;
      material.ambient = material.diffuse * texture(tex_material.ambient_occlusion_map, fsin.tex_coord).r;
      float roughness = mix(tex_material.roughness_range.x, tex_material.roughness_range.y, 
                            texture(tex_material.roughness_map, fsin.tex_coord).r);
      material.shininess = 2.0f/pow(clamp(roughness, 0.001f, 0.999f), 4.0f) - 2.0f;
      float phong = pow(max(0.0f, dot(view, reflected)), material.shininess);
      vec3 diffuse = material.diffuse * light.diffuse * lambert;
      vec3 specular = material.specular * light.specular * phong;
      vec3 ambient = material.ambient * light.ambient;
      vec3 emissive = material.emissive * light.emissive;
      accumulated_light += (diffuse + specular + emissive) * attenuation + ambient;
      //accumulated_light = texture(tex_material.emissive_map, fsin.tex_coord).rgb;
      //accumulated_light += (diffuse + specular) * attenuation + ambient + emissive;
   }
   frag_color = fsin.color * vec4(accumulated_light, 1.0f);
   //frag_color = vec4(accumulated_light, 1.0f);
    ////////////////////////////////////////////////////////////////////////////////////////////
}