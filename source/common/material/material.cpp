#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"

#include <glm/vec2.hpp>

namespace our
{

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const
    {
        // TODO: (Req 7) Write this function
        pipelineState.setup(); // setup the pipline
        shader->use();         // to use the shader
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;
        printf("%s \n", "Material deserialized");

        if (data.contains("pipelineState"))
        {
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    void LitMaterial::setup() const
    {
        Material::setup();

        /*TODO (req Light): SEND NEEDED DATA TO SHADER*/
    }

    void LitMaterial::deserialize(const nlohmann::json &data)
    {
        Material::deserialize(data);
        if (!data.is_object())
            return;
        printf("%s \n", "Lit material deserialized");
        diffuse = data.value("diffuse", glm::vec4(1.0f));
        specular = data.value("specular", glm::vec4(1.0f));
        ambient = data.value("ambient", glm::vec4(1.0f));
        emissive = data.value("emissive", glm::vec4(1.0f));

        shininess = data.value("shininess", 1.0f);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint
    void TintedMaterial::setup() const
    {
        // TODO: (Req 7) Write this function
        Material::setup();         // call the setup of its parent
        shader->set("tint", tint); // set the tint
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json &data)
    {
        Material::deserialize(data);
        if (!data.is_object())
            return;
        printf("%s \n", "tinted material deserialized");
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    void LitTintedMaterial::setup() const
    {
        LitMaterial::setup();
        shader->set("material.diffuse", glm::vec3(albedo_tint.r, albedo_tint.g, albedo_tint.b));
        shader->set("material.specular", glm::vec3(specular.r, specular.g, specular.b));
        shader->set("material.ambient", glm::vec3(ambient.r, ambient.g, ambient.b));
        printf("%f, %f, %f, %f \n", ambient.r, ambient.g, ambient.b, ambient.a);
        shader->set("material.emissive", glm::vec3(emissive_tint.r, emissive_tint.g, emissive_tint.b));
        shader->set("material.shininess", shininess);

        /*TODO (req Light): SEND NEEDED DATA TO SHADER*/
    }

    void LitTintedMaterial::deserialize(const nlohmann::json &data)
    {
        LitMaterial::deserialize(data);
        if (!data.is_object())
            return;
        printf("%s \n", "Lit tinted material deserialized");
        albedo_tint = data.value("albedo_tint", glm::vec4(1.0f));
        specular_tint = data.value("specular_tint", glm::vec4(1.0f));
        emissive_tint = data.value("emissive_tint", glm::vec4(1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex"
    void TexturedMaterial::setup() const
    {
        // TODO: (Req 7) Write this function
        TintedMaterial::setup();                       // call the setup of its parent
        shader->set("alphaThreshold", alphaThreshold); // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
        glActiveTexture(GL_TEXTURE0);
        if (texture)
            texture->bind(); // check if the texture is not null then bind it
        else Texture2D::unbind();
        if (sampler)
            sampler->bind(0);  // check if sampler is not null then bind it
        else sampler->unbind(0);
        shader->set("tex", 0); // send the unit number to the uniform variable "tex"
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json &data)
    {
        TintedMaterial::deserialize(data);
        if (!data.is_object())
            return;
        printf("%s \n", "textured material deserialized");
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    void LitTexturedMaterial::setup() const
    {
        LitTintedMaterial::setup();
        shader->set("tex_material.roughness_range", roughness_range);
        shader->set("tex_material.albedo_tint", glm::vec3(albedo_tint.r, albedo_tint.g, albedo_tint.b));
        shader->set("tex_material.specular_tint", glm::vec3(specular_tint.r, specular_tint.g, specular_tint.b));
        shader->set("tex_material.emissive_tint", glm::vec3(emissive_tint.r, emissive_tint.g, emissive_tint.b));
        shader->set("alphaThreshold", alphaThreshold); // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
        glActiveTexture(GL_TEXTURE0);
        if (albedo_map)
        {
            albedo_map->bind(); // check if the texture is not null then bind it
        }
        else
        {
            Texture2D::unbind();        
        }
        if (albedo_sampler)
        {
            albedo_sampler->bind(0); // check if sampler is not null then bind it
        }
        else
        {
            albedo_sampler->unbind(0);
        }
        shader->set("tex_material.albedo_map", 0);
        glActiveTexture(GL_TEXTURE0 + 1);
        if (specular_map)
        {
            specular_map->bind(); // check if the texture is not null then bind it
        }
        else
        {
            Texture2D::unbind();
        }
        if (specular_sampler)
        {
            specular_sampler->bind(1); // check if sampler is not null then bind it
        }
        else
        {
            specular_sampler->unbind(1);
        }
        shader->set("tex_material.specular_map", 1);
        
        glActiveTexture(GL_TEXTURE0 + 2);
        if (ambient_occlusion_map)
        {
            ambient_occlusion_map->bind(); // check if the texture is not null then bind it
        }
        else
        {
            Texture2D::unbind();
        }
        if (ambient_occlusion_sampler)
            ambient_occlusion_sampler->bind(2); // check if sampler is not null then bind it
        else ambient_occlusion_sampler->unbind(2);
        shader->set("tex_material.ambient_occlusion_map", 2);
        glActiveTexture(GL_TEXTURE0 + 3);
        if (roughness_map)
        {
            roughness_map->bind(); // check if the texture is not null then bind it
        }
        else Texture2D::unbind();
        if (roughness_sampler)
            roughness_sampler->bind(3); // check if sampler is not null then bind it
        else roughness_sampler->unbind(3);
        shader->set("tex_material.roughness_map", 3);
        glActiveTexture(GL_TEXTURE0 + 4);
        if (emissive_map)
        {
            emissive_map->bind(); // check if the texture is not null then bind it
        }
        else Texture2D::unbind();
        if (emissive_sampler)
            emissive_sampler->bind(4); // check if sampler is not null then bind it
        else emissive_sampler->unbind(4);
        shader->set("tex_material.emissive_map", 4);
        /*shader->set("material.diffuse", );
        shader->set("material.specular", specular);
        shader->set("material.ambient", ambient);
        shader->set("material.emissive", emissive);
        shader->set("material.shininess", shininess);*/

        /*TODO (req Light): SEND NEEDED DATA TO SHADER*/
    }

    void LitTexturedMaterial::deserialize(const nlohmann::json &data)
    {
        LitTintedMaterial::deserialize(data);
        if (!data.is_object())
            return;
        printf("%s \n", "Lit textured material deserialized");

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