#version 330 core

in Varyings {
   vec4 color;
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
uniform Material material;

out vec4 frag_color;

uniform vec4 tint;
uniform sampler2D tex;

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
      float phong = pow(max(0.0f, dot(view, reflected)), material.shininess);
      vec3 diffuse = material.diffuse * light.diffuse * lambert;
      vec3 specular = material.specular * light.specular * phong;
      vec3 ambient = material.ambient * light.ambient;
      vec3 emissive = material.emissive * light.emissive;
      accumulated_light += (diffuse + specular + emissive) * attenuation + ambient;
      //accumulated_light += (diffuse + specular) * attenuation + ambient + emissive;
   }
   frag_color = fsin.color * vec4(accumulated_light, 1.0f);
   //frag_color = vec4(material.ambient, 1.0f);
   
   // switch(light_count){
   //    case 0: 
   //       frag_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
   //       break;
   //    case 1:
   //       frag_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
   //       break;
   //    case 2:
   //       frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
   //       break;
   //    case -1:
   //       frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
   //       break;
   // }
    ////////////////////////////////////////////////////////////////////////////////////////////
}