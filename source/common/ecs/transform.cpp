#include "entity.hpp"
#include "../deserialize-utils.hpp"

#include <glm/gtx/euler_angles.hpp>

namespace our {

    // This function computes and returns a matrix that represents this transform
    // Remember that the order of transformations is: Scaling, Rotation then Translation
    // HINT: to convert euler angles to a rotation matrix, you can use glm::yawPitchRoll
    glm::mat4 Transform::toMat4() const {
        
        glm::mat4 Scaling_Matrix = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 Rotation_Matrix = glm::yawPitchRoll(rotation.y, rotation.x, rotation.z);
        glm::mat4 Translation_Matrix = glm::translate(glm::mat4(1.0f), position);

        glm::mat4 Transformation_Matrix = Translation_Matrix * Rotation_Matrix * Scaling_Matrix;
        return Transformation_Matrix; 
    }

     // Deserializes the entity data and components from a json object
    void Transform::deserialize(const nlohmann::json& data){
        position = data.value("position", position);
        rotation = glm::radians(data.value("rotation", glm::degrees(rotation)));
        scale    = data.value("scale", scale);
    }

}