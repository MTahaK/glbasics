#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // Attempt to initialize GLFW
    if(!glfwInit()){
        std::cerr << "Failed to initialize GLFW. Exiting\n";
        return -1;
    }
    
    // Successful windowing initialization
    
    // Specify the OpenGL version and profile to request from the driver.
    // Must match (or be below) what GLAD was generated for (e.g., 3.3 Core).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Specifies OpenGL 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Specifies OpenGL x.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // OpenGL Core

    // GLFW will then ask the OS/driver to create a compatible OpenGL context.

    // Window creation
    // Create an 800x600 window with title "Gl Triangle Window"
    // last two args are for context sharing between windows - not needed here
    // Function call also creates an OpenGL context associated with the window.
    GLFWwindow* window = glfwCreateWindow(800, 600, "Gl Triangle Window", nullptr, nullptr);

    if(!window){
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate(); // shut down GLFW cleanly.
        return -1;
    }

    // Make OGL context active
    // Essentially signals to OGL: "all OpenGL function calls in this thread affect this specific window"
    glfwMakeContextCurrent(window);

    // Load OpenGL functions with GLAD
    // gladLoadGL uses the active context defined in the previous step
    if(!gladLoadGL()){
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window); // cleaner to explicitly call
        glfwTerminate(); // typically includes DestroyWindow
        return -1;
    }

    // Set default background colour of framebuffer. Tells OGL to use this colour next clear.
    // This ONLY sets the colour - it doesn't perform any colouring action.
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Dark teal


    // Main game/render loop, runs until close button is pressed or glfwSetWindowShouldClose(window, true) is called

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);     // Clear the screen to the background color
        // GL_COLOR_BUFFER_BIT is a bitmask constant that tells OpenGL to clear the color buffer
        // using the value previously set with glClearColor().
        glfwSwapBuffers(window);          // Present the frame (double buffering)
        glfwPollEvents();                 // Handle keyboard/mouse/input/window events
    }

    // GL_COLOR_BUFFER_BIT is not a color.
    // It is an instruction:
    // “Clear the color buffer”—i.e., erase the contents of the current framebuffer's pixel colors.
    // It tells glClear(...) what to erase. You can also clear:
    // GL_DEPTH_BUFFER_BIT → clears depth values
    // GL_STENCIL_BUFFER_BIT → clears stencil mask - not relevant now
    // You combine them using | when needed.
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
