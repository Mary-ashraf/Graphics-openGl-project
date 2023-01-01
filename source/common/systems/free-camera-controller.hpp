#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"
#include "../components/Collision.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace our
{

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem {
        Application* app; // The application in which the state runs
        bool mouse_locked = false; // Is the mouse locked

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent 
        void update(World* world, float deltaTime) {
            
            CameraComponent* camera = nullptr;
            FreeCameraControllerComponent *controller = nullptr;

            //Collision component variables
            std::vector<CollisionBoundary> Walls;
            glm::vec2 x_Boundary = glm::vec2(1.0f);
            glm::vec2 y_Boundary = glm::vec2(1.0f);
            glm::vec2 z_Boundary = glm::vec2(1.0f);
            int WallsNumber;

            for(auto entity : world->getEntities()){
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            if(!camera) camera = entity->getComponent<CameraComponent>();
            if(!controller) controller = entity->getComponent<FreeCameraControllerComponent>();
            // If this entity has a collision compnent
            if(auto Collisions = entity->getComponent<CollisionComponent>(); Collisions){
                x_Boundary = Collisions->x_Boundary;
                y_Boundary = Collisions->y_Boundary;
                z_Boundary = Collisions->z_Boundary;
                WallsNumber = Collisions->WallsNumber;
                Walls = Collisions->Walls;                
            }
        }

            // If there is no entity with both a CameraComponent and a FreeCameraControllerComponent, we can do nothing so we return
            if(!(camera && controller)) return;
            // Get the entity that we found via getowner of camera (we could use controller->getOwner())
            Entity* entity = camera->getOwner();
            

            // We get a reference to the entity's position and rotation
            glm::vec3& position = entity->localTransform.position;
            glm::vec3& rotation = entity->localTransform.rotation;
            
            // We prevent the pitch from exceeding a certain angle from the XZ plane to prevent gimbal locks
            if(rotation.x < -glm::half_pi<float>() * 0.99f) rotation.x = -glm::half_pi<float>() * 0.99f;
            if(rotation.x >  glm::half_pi<float>() * 0.99f) rotation.x  = glm::half_pi<float>() * 0.99f;
            // This is not necessary, but whenever the rotation goes outside the 0 to 2*PI range, we wrap it back inside.
            // This could prevent floating point error if the player rotates in single direction for an extremely long time. 
            rotation.y = glm::wrapAngle(rotation.y);
            

            // We get the camera model matrix (relative to its parent) to compute the front, up and right directions
            glm::mat4 matrix = entity->localTransform.toMat4();

            glm::vec3 front = glm::vec3(matrix * glm::vec4(0, 0, -1, 0)),
                      up = glm::vec3(matrix * glm::vec4(0, 1, 0, 0)), 
                      right = glm::vec3(matrix * glm::vec4(1, 0, 0, 0));

            glm::vec3 current_sensitivity = controller->positionSensitivity;
            // If the LEFT SHIFT key is pressed, we multiply the position sensitivity by the speed up factor
            if(app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT)) current_sensitivity *= controller->speedupFactor;
           
           //Collision Detection
            if(WallsNumber == 4)
            {
                //First Wall:
                //check if first wall is reached (using the z position)
                if(position.z <= (Walls[0].z_position+1) && position.z >Walls[0].z_position)
                {
                    //check if the position is within the x & y boundaries 
                    if(position.x > Walls[0].x_Boundary.x && position.x < Walls[0].x_Boundary.y && position.y > Walls[0].y_Boundary.x && position.y < Walls[0].y_Boundary.y)
                    {
                        //if yes move forward
                        position += front * (deltaTime * current_sensitivity.z);
                    }
                    else
                    {
                        //else restart the game
                        //this is only for the first wall
                        app->changeState("play");
                    }
                }
                //Second Wall:
                //check if second wall is reached (using the z position)
                else if(position.z <= (Walls[1].z_position+1) && position.z >Walls[1].z_position)
                {
                    //check if the position is within the x & y boundaries 
                    if(position.x > Walls[1].x_Boundary.x && position.x < Walls[1].x_Boundary.y && position.y > Walls[1].y_Boundary.x && position.y < Walls[1].y_Boundary.y)
                    {
                        //if yes move forward
                        position += front * (deltaTime * current_sensitivity.z);
                    }
                    else
                    {
                        //else go to lose state
                        app->changeState("lose");
                    }
                }
                //Third Wall:
                //check if third wall is reached (using the z position)
                else if(position.z <= (Walls[2].z_position+1) && position.z >Walls[2].z_position)
                {
                    //check if the position is within the x & y boundaries 
                    if(position.x > Walls[2].x_Boundary.x && position.x < Walls[2].x_Boundary.y && position.y > Walls[2].y_Boundary.x && position.y < Walls[2].y_Boundary.y)
                    {
                        //if yes move forward
                        position += front * (deltaTime * current_sensitivity.z);
                    }
                    else
                    {
                        //else go to lose state
                        app->changeState("lose");
                    }
                }
                //Fourth Wall:
                //check if fourth wall is reached (using the z position)
                else if(position.z <= (Walls[3].z_position+1) && position.z >Walls[3].z_position)
                {
                    //check if the position is within the x & y boundaries 
                    if(position.x > Walls[3].x_Boundary.x && position.x < Walls[3].x_Boundary.y && position.y > Walls[3].y_Boundary.x && position.y < Walls[3].y_Boundary.y)
                    {
                        //if yes move forward
                        position += front * (deltaTime * current_sensitivity.z);
                    }
                    else
                    {
                        //else go to lose state
                        app->changeState("lose");
                    }
                }
                //if the position is not at any wall, move forward
                else
                {
                    position += front * (deltaTime * current_sensitivity.z);
                }
            }
            //if the position passed the z boundary, go to win state
            if(position.z < z_Boundary.y)
            {
                app->changeState("win");
            }
            

        // W & S moves the player up and down
        //Check for y boundaries, so the player doesn't go past them
        if(app->getKeyboard().isPressed(GLFW_KEY_W) && position.y <y_Boundary.x)
        {
            position += up * (deltaTime * current_sensitivity.y);
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_S) && position.y > y_Boundary.y)
        {
            position -= up * (deltaTime * current_sensitivity.y);
        }
        // A & D moves the player left or right 
        //Check for x boundaries, so the player doesn't go past them
        if(app->getKeyboard().isPressed(GLFW_KEY_D) && position.x <x_Boundary.x)
        {
            position += right * (deltaTime * current_sensitivity.x);
        }
        if(app->getKeyboard().isPressed(GLFW_KEY_A) && position.x >x_Boundary.y) 
        {
            position -= right * (deltaTime * current_sensitivity.x);
        }
    }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit(){
            if(mouse_locked) {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }

    };

}
