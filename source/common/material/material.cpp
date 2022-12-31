#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"

#include <glm/vec2.hpp>

namespace our {

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const {
        //TODO: (Req 7) Write this function
        pipelineState.setup(); // setup the pipline
        shader->use(); // to use the shader
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;

        if(data.contains("pipelineState")){
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    void LitMaterial::setup() const 
    {
        Material::setup();

        //TODO: (Light) SEND NEEDED DATA TO SHADER
    }

    void LitMaterial::deserialize(const nlohmann::json& data)
    {
        Material::deserialize(data);
        if(!data.is_object()) return;
        diffuse = data.value("diffuse", glm::vec4(1.0f));
        specular = data.value("specular", glm::vec4(1.0f));
        ambient = data.value("ambient", glm::vec4(1.0f));
        emissive = data.value("emissive", glm::vec4(1.0f));

        shininess = data.value("shininess", 1.0f);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint 
    void TintedMaterial::setup() const {
        //TODO: (Req 7) Write this function
        Material::setup(); //call the setup of its parent 
        shader->set("tint", tint); //set the tint
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json& data){
        Material::deserialize(data);
        if(!data.is_object()) return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    void LitTintedMaterial::setup() const 
    {
        LitMaterial::setup();

        //TODO: (Light) SEND NEEDED DATA TO SHADER
    }

    void LitTintedMaterial::deserialize(const nlohmann::json& data)
    {
        LitMaterial::deserialize(data);
        if(!data.is_object()) return;
        albedo_tint = data.value("albedo_tint", glm::vec4(1.0f));
        specular_tint = data.value("specular_tint", glm::vec4(1.0f));
        emissive_tint = data.value("emissive_tint", glm::vec4(1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex" 
    void TexturedMaterial::setup() const {
        //TODO: (Req 7) Write this function
        TintedMaterial::setup(); // call the setup of its parent
        shader->set("alphaThreshold", alphaThreshold); // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
        if (texture) texture->bind(); // check if the texture is not null then bind it 
        if (sampler) sampler->bind(0); // check if sampler is not null then bind it
        shader->set("tex", 0); // send the unit number to the uniform variable "tex" 
    
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json& data){
        TintedMaterial::deserialize(data);
        if(!data.is_object()) return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    void LitTexturedMaterial::setup() const 
    {
        LitTintedMaterial::setup();

        //TODO: (Light) SEND NEEDED DATA TO SHADER
    }

    void LitTexturedMaterial::deserialize(const nlohmann::json& data)
    {
        LitTintedMaterial::deserialize(data);
        if(!data.is_object()) return;

        albedo_map = AssetLoader<Texture2D>::get(data.value("albedo_map", ""));
        albedo_sampler = AssetLoader<Sampler>::get(data.value("albedo_sampler", ""));
        specular_map = AssetLoader<Texture2D>::get(data.value("specular_map", ""));
        specular_sampler = AssetLoader<Sampler>::get(data.value("specular_sampler", ""));
        roughness_map = AssetLoader<Texture2D>::get(data.value("roughness_map", ""));
        roughness_sampler = AssetLoader<Sampler>::get(data.value("roughness_sampler", ""));
        roughness_range = data.value("roughness_range", glm::vec2(0.0f, 1.0f)); 
        ambient_occlusion_map = AssetLoader<Texture2D>::get(data.value("ambient_occlusion_map", ""));
        ambient_occlusion_sampler = AssetLoader<Sampler>::get(data.value("ambient_occlusion_sampler", ""));      
        emissive_map = AssetLoader<Texture2D>::get(data.value("emissive_map", ""));
        emissive_sampler = AssetLoader<Sampler>::get(data.value("emissive_sampler", ""));

        alphaThreshold = data.value("alphaThreshold", 0.0f);
    }

}