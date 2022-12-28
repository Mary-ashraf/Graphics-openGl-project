#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our {

    // An enum that defines the types of lights
    enum class LightType {
        POINT,
        DIRECTIONAL,
        SPOT
    };

    class LightComponent : public Component {

        public:
        LightType lightType;

        glm::vec3 diffuse;
        glm::vec3 specular;
        glm::vec3 ambient;

        float attenuation_constant;
        float attenuation_linear;
        float attenuation_quadratic;


        float inner_angle;
        float outer_angle;

        // The ID of this component type is "Light"
        static std::string getID() { return "Light"; }

        // Reads light parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };
}