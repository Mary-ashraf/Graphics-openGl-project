#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"

namespace our
{

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if (config.contains("sky"))
        {
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));

            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram *skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();

            // TODO: (Req 10) Pick the correct pipeline state to draw the sky
            //  Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            //  We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{
                skyPipelineState.faceCulling.enabled = true,
                skyPipelineState.faceCulling.frontFace = GL_CCW,
                skyPipelineState.faceCulling.culledFace = GL_FRONT,
                skyPipelineState.depthTesting.enabled = true,
                skyPipelineState.depthTesting.function = GL_LEQUAL};

            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D *skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky
            Sampler *skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if (config.contains("postprocess"))
        {
            // TODO: (Req 11) Create a framebuffer
            glGenFramebuffers(1, &postprocessFrameBuffer);

            // TODO: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            //  Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            //  The depth format can be (Depth component with 24 bits).
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);

            colorTarget = texture_utils::empty(GL_RGBA, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT, windowSize);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            // TODO: (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler *postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram *postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy()
    {
        // Delete all objects related to the sky
        if (skyMaterial)
        {
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if (postprocessMaterial)
        {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World *world)
    {
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent *camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        lights.clear();
        for (auto entity : world->getEntities())
        {
            // If we hadn't found a camera yet, we look for a camera in this entity
            if (!camera)
                camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component
            if (auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer)
            {
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if (command.material->transparent)
                {
                    transparentCommands.push_back(command);
                }
                else
                {
                    // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
            if (auto light = entity->getComponent<LightComponent>(); light)
            {
                lights.push_back(light);
                //printf("%d \n",lights.size());
                //lights.push_back(std::make_pair(entity->localTransform.position, entity.))
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if (camera == nullptr)
            return;

        // TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        // HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        // glm::vec3 cameraForward = glm::vec3(0.0, 0.0, -1.0f);
        glm::mat4 VM = camera->getViewMatrix();
        glm::vec3 cameraForward = glm::vec3(VM[2][0], VM[2][1], VM[2][2]); // 3rd row
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second)
                  {
            //TODO: (Req 9) Finish this function
            // HINT: the following return should return true "first" should be drawn before "second". 
            return first.center.z < second.center.z; });

        // TODO: (Req 9) Get the camera ViewProjection matrix and store it in VP
        glm::mat4 VP = camera->getProjectionMatrix(windowSize) * camera->getViewMatrix();

        // TODO: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        glViewport(0, 0, windowSize.x, windowSize.y);

        // TODO: (Req 9) Set the clear color to black and the clear depth to 1
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClearDepth(1);

        // TODO: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        // If there is a postprocess material, bind the framebuffer
        if (postprocessMaterial)
        {
            // TODO: (Req 11) bind the framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);
        }

        // TODO: (Req 9) Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: (Req 9) Draw all the opaque commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render
        for (unsigned long int i = 0; i < opaqueCommands.size(); i++)
        {
            opaqueCommands[i].material->transparent = false;
            opaqueCommands[i].material->setup();
            opaqueCommands[i].material->shader->set("transform", VP * opaqueCommands[i].localToWorld);
            /*TODO (req Light): SEND THE LIST OF LIGHTS TO THE SHADER FOR LIGHTING SUPPORT*/
            for (int j = 0; j < lights.size(); j++)
            {
                opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "diffuse", lights[j]->diffuse);
                opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "specular", lights[j]->specular);
                opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "ambient", lights[j]->ambient);
                opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "emissive", lights[j]->emissive);
                opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "type", static_cast<int>(lights[j]->lightType));

                switch (lights[j]->lightType)
                {
                case LightType::DIRECTIONAL:
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "direction", glm::normalize(lights[j]->getOwner()->localTransform.rotation));
                    break;
                case LightType::POINT:
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "position", lights[j]->getOwner()->localTransform.position);
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_constant", lights[j]->attenuation_constant);
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_linear", lights[j]->attenuation_linear);
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_quadratic", lights[j]->attenuation_quadratic);
                    break;
                case LightType::SPOT:
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "position", lights[j]->getOwner()->localTransform.position);
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "direction", glm::normalize(lights[j]->getOwner()->localTransform.rotation));
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_constant", lights[j]->attenuation_constant);
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_linear", lights[j]->attenuation_linear);
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_quadratic", lights[j]->attenuation_quadratic);
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "inner_angle", lights[j]->inner_angle);
                    opaqueCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "outer_angle", lights[j]->outer_angle);
                    break;
                }
            }
            opaqueCommands[i].material->shader->set("light_count", (GLint)lights.size());
            //opaqueCommands[i].material->shader->set("light_count", 1);
            opaqueCommands[i].mesh->draw();
            //Texture2D::unbind();
        }

        // If there is a sky material, draw the sky
        if (this->skyMaterial)
        {
            // TODO: (Req 10) setup the sky material
            skyMaterial->setup();
            // TODO: (Req 10) Get the camera position
            glm::vec3 cameraPosition = glm::vec3(VM[3][0], VM[3][1], VM[3][2]); // 4th row
            // TODO: (Req 10) Create a model matrix for the sky such that it always follows the camera (sky sphere center = camera position)
            glm::mat4 view = glm::mat4(1.0f);
            view[0][3] = cameraPosition[0];
            view[1][3] = cameraPosition[1];
            view[2][3] = cameraPosition[2];
            // TODO: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            //  We can acheive this by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 projection = camera->getProjectionMatrix(windowSize);
            float far = camera->far;
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f);
            // TODO: (Req 10) set the "transform" uniform
            skyMaterial->shader->set("transform", alwaysBehindTransform * projection * view);
            // TODO: (Req 10) draw the sky sphere
            skySphere->draw();
        }
        // TODO: (Req 9) Draw all the transparent commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (unsigned long int i = 0; i < transparentCommands.size(); i++)
        {
            transparentCommands[i].material->transparent = true;
            transparentCommands[i].material->setup();
            transparentCommands[i].material->shader->set("transform", VP * transparentCommands[i].localToWorld);
            /*TODO (req Light): SEND THE LIST OF LIGHTS TO THE SHADER FOR LIGHTING SUPPORT*/
            for (int j = 0; j < lights.size(); j++)
            {
                transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "diffuse", lights[j]->diffuse);
                transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "specular", lights[j]->specular);
                transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "ambient", lights[j]->ambient);
                transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "type", static_cast<int>(lights[j]->lightType));

                switch (lights[j]->lightType)
                {
                case LightType::DIRECTIONAL:
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "direction", glm::normalize(lights[j]->getOwner()->localTransform.rotation));
                    break;
                case LightType::POINT:
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "position", lights[j]->getOwner()->localTransform.position);
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_constant", lights[j]->attenuation_constant);
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_linear", lights[j]->attenuation_linear);
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_quadratic", lights[j]->attenuation_quadratic);
                    break;
                case LightType::SPOT:
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "position", lights[j]->getOwner()->localTransform.position);
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "direction", glm::normalize(lights[j]->getOwner()->localTransform.rotation));
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_constant", lights[j]->attenuation_constant);
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_linear", lights[j]->attenuation_linear);
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "attenuation_quadratic", lights[j]->attenuation_quadratic);
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "inner_angle", lights[j]->inner_angle);
                    transparentCommands[i].material->shader->set("lights[" + std::to_string(j) + "]." + "outer_angle", lights[j]->outer_angle);
                    break;
                }
            }
            transparentCommands[i].material->shader->set("light_count", (GLint)lights.size());
            //transparentCommands[i].material->shader->set("light_count", 1);
            transparentCommands[i].mesh->draw();
        }

        // If there is a postprocess material, apply postprocessing
        if (postprocessMaterial)
        {
            // TODO: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // TODO: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            postprocessMaterial->setup();
            glBindVertexArray(postProcessVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }
    }

}