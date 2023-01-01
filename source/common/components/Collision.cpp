#include "Collision.hpp"
#include "../deserialize-utils.hpp"

//TODO: (Game) Implement Collision Component cpp

namespace our {

    void CollisionComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        //read the number of walls 
        WallsNumber = data.value("WallsNumber", WallsNumber);
        //read x, y and z boundaries of the game
        x_Boundary = data.value("x_Boundary", x_Boundary);
        y_Boundary = data.value("y_Boundary", y_Boundary);
        z_Boundary = data.value("z_Boundary", z_Boundary);
        //read the x and y boundaries of the wall openings and the position of each wall
        for (int i = 0; i < WallsNumber; i++)
        {
            CollisionBoundary Wall;
            Wall.x_Boundary = data.value("x_Boundary"+std::to_string(i) , Wall.x_Boundary);
            Wall.y_Boundary = data.value("y_Boundary"+std::to_string(i) , Wall.y_Boundary);
            Wall.z_position = data.value("z_position"+std::to_string(i) , Wall.z_position);
            Walls.push_back(Wall);
        }       
    }
}