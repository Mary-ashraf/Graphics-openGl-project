#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>

namespace our {
    
    struct CollisionBoundary {
        glm::vec2 x_Boundary; 
        glm::vec2 y_Boundary; 
        float z_position;
    };

    // This component denotes that any renderer should draw the given mesh using the given material at the transformation of the owning entity.
    class CollisionComponent : public Component {
    public:
        std::vector<CollisionBoundary> Walls;
        //Game Boundaries
        glm::vec2 x_Boundary = {0, 0}; 
        glm::vec2 y_Boundary = {0, 0}; 
        glm::vec2 z_Boundary = {0, 0}; 
        int WallsNumber = 0;
        // The ID of this component type is "Collision"
        static std::string getID() { return "Collision"; }

        // Receives the mesh & material from the AssetLoader by the names given in the json object
        void deserialize(const nlohmann::json& data) override;
    };

}