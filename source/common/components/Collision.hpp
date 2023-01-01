#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>

namespace our {
    
    struct CollisionBoundary {
        glm::vec2 x_Boundary; 
        glm::vec2 y_Boundary; 
        float z_position;
    };

    // This component holds the boundaries of the game, number of walls, and the boundaries of the openings in the walls
    class CollisionComponent : public Component {
    public:
        std::vector<CollisionBoundary> Walls;
        //Game Boundaries
        glm::vec2 x_Boundary = {0, 0}; 
        glm::vec2 y_Boundary = {0, 0}; 
        glm::vec2 z_Boundary = {0, 0}; 
        //Number of walls
        int WallsNumber = 0;
        // The ID of this component type is "Collision"
        static std::string getID() { return "Collision"; }

        // Receives the data by the values given in the json object
        void deserialize(const nlohmann::json& data) override;
    };

}