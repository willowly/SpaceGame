#define GL_SILENCE_DEPRECATION

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>

#include <engine/loader.hpp>
#include <engine/world.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <graphics/text.hpp>

using glm::vec3;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SpaceGame", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Registry registry;
    Loader loader;
    loader.loadAll(registry);

    Material material(&registry.litShader,&registry.textures.at("cow"));

    // World world;

    Text text("fonts/courier-new.ttf",30);
    text.text = "hello world";

    Camera camera;
    camera.move(vec3(0,0,5));

    camera.setAspect(800,600);
    
    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
    glEnable(GL_DEPTH_TEST);
    // for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float angle;

    while(!glfwWindowShouldClose(window))
    {

        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        angle += 0.1;
        auto matrix = glm::ortho(0.0f,800.0f,0.0f,600.0f);
        text.render(registry.textShader,matrix);
        // auto matrix = glm::mat4(1.0f);
        // matrix = glm::toMat4(glm::quat(glm::vec3(0,angle,0))) * matrix;
        // registry.models.at("cube").render(matrix,camera,material);
        
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}
