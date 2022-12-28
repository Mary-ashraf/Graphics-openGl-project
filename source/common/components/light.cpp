include "light.hpp"
#include "../ecs/entity.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

    namespace our
{

    // Reads light parameters from the given json object
    void LightComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;

        std::string lightTypeStr = data.value("lightType", "directional");

        switch (lightTypeStr)
        {
        case "point" lightType = LightType::POINT;
            break;
            case "directional" lightType = LightType::DIRECTIONAL;
            break;
            case "spot" lightType = LightType::SPOT;
            break;

            default:
            lightType = LightType::DIRECTIONAL;
            break;
        }

        diffuse = data.value("diffuse", glm::vec3(1, 1, 1));
        specular = data.value("specular", glm::vec3(1, 1, 1));
        ambient = data.value("ambient", glm::vec3(1, 1, 1));

        attenuation_constant = data.value("attenuation_constant", 1.0f);
        attenuation_linear = data.value("attenuation_linear", 1.0f);
        attenuation_quadratic = data.value("attenuation_quadratic", 1.0f);

        inner_angle = data.value("inner_angle", 10.0f);
        outer_angle = data.value("outer_angle", 40.0f);
    }
}