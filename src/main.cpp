#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::string loadShaderSource(const char* filePath){
    std::ifstream file(filePath);
    std::stringstream buffer;

    if(!file.is_open()){
        std::cerr<< "Failed to open shader file: " << filePath <<". Load aborting...\n";
        return "";
    }

    buffer << file.rdbuf();
    return buffer.str();
}
// Note: TECHNICALLY, you could just directly hardcode shader code as a set of strings.
// This would work perfectly fine. However:
// External files let you:
//     Reload shaders on the fly (especially during development)
//     Swap shaders without touching your engine code
//     Compile tools that preprocess or validate your shader files

// compileShader(...)
// ------------------
// source:      Pointer to C-style string containing GLSL source code.
//              Usually passed as .c_str() from a std::string.
// shaderType:  Type of shader to compile (e.g. GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
//
// Purpose: Creates, attaches, and compiles a shader of the given type using the provided GLSL source.
// Returns: GLuint ID of the compiled shader object.

GLuint compileShader(const char* source, GLenum shaderType){
    // | `shaderType`         | Meaning                               |
    // | -------------------- | ------------------------------------- |
    // | `GL_VERTEX_SHADER`   | Vertex shader stage (runs per vertex) |
    // | `GL_FRAGMENT_SHADER` | Fragment (pixel) shader               |
    // | `GL_GEOMETRY_SHADER` | Optional stage (more advanced)        |
    // | etc.                 | (e.g., tessellation, compute)         |

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    // void glShaderSource(GLuint shader,
    //     GLsizei count,
    //     const GLchar* const* string,
    //     const GLint* length);

    // | Parameter | Meaning                                                      |
    // | --------- | ------------------------------------------------------------ |
    // | `shader`  | The shader object to attach source code to                   |
    // | `count`   | Number of strings passed (usually 1)                         |
    // | `string`  | An array of `char*` pointers (C-strings holding GLSL source) |
    // | `length`  | Optional: array of lengths for each string                   |

    // If you pass nullptr for length, OpenGL assumes:

    // “Each string in string[] is null-terminated. I’ll keep reading until I hit \0.”

    // | Question                         | Answer                                                                       |
    // | -------------------------------- | ---------------------------------------------------------------------------- |
    // | Why pass `nullptr` for `length`? | It tells OpenGL “just treat this as a null-terminated C-string.”             |
    // | Is that safe?                    | ✅ Yes—*if* the source string is cleanly null-terminated (as from `.c_str()`) |
    // | When would I pass real lengths?  | When using multiple strings or substrings, or embedded nulls                 |

    glCompileShader(shader);
    // Check for compile success
    int pass;
    // Checks status of a shader 
    glGetShaderiv(shader, GL_COMPILE_STATUS, &pass);

    // | Parameter | Type     | Purpose                                                |
    // | --------- | -------- | ------------------------------------------------------ |
    // | `shader`  | `GLuint` | The ID of the shader object you want to query          |
    // | `pname`   | `GLenum` | What property you want to query (e.g., compile status) |
    // | `params`  | `GLint*` | Pointer to an integer where the result will be stored  |

    // | `pname`                   | Meaning                                                                  |
    // | ------------------------- | ------------------------------------------------------------------------ |
    // | `GL_COMPILE_STATUS`       | Whether the shader compiled successfully (`GL_TRUE` or `GL_FALSE`)       |
    // | `GL_SHADER_TYPE`          | Whether it's a vertex, fragment, geometry shader, etc.                   |
    // | `GL_DELETE_STATUS`        | Whether the shader has been flagged for deletion                         |
    // | `GL_INFO_LOG_LENGTH`      | Number of characters in the shader’s info log (compile errors, warnings) |
    // | `GL_SHADER_SOURCE_LENGTH` | Length of the source code (if stored)                                    |

    if(!pass){
        // Compile failed
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << "\n";
    }

    return shader;
}

// linkProgram(...)
// ----------------
// vertexShader:   ID of a compiled vertex shader (from glCreateShader + glCompileShader)
// fragmentShader: ID of a compiled fragment shader
//
// Purpose:
//   - Creates a shader program object.
//   - Attaches the provided shaders.
//   - Links them into a single executable GPU pipeline.
//   - Checks for link errors and logs them.
//   - Deletes the individual shaders (they're no longer needed after linking).
//
// Returns: GLuint ID of the final linked shader program.
//          Pass to glUseProgram(...) to activate it before drawing.

GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    // Create a program object to link shaders into.
    // Think of this like allocating a "pipeline object" that will represent your final shader program.
    GLuint program = glCreateProgram();

    // Attach the already compiled vertex and fragment shaders to the program.
    // These are just attachments at this stage—the actual linking hasn't happened yet.
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    // This doesn’t copy its contents—it’s still a reference to the original shader.

    // Link the attached shader stages into a final shader program.
    // This performs validation, matches inputs/outputs, resolves locations, and finalizes GPU-side code.
    glLinkProgram(program);
    // If linking fails, it's usually because:

    // Vertex shader outputs don’t match fragment shader inputs
    // Multiple main() definitions
    // Using variables or functions inconsistently

    // | Term                        | Summary 
    // | --------------------------- | -------------------------------------------------------------------------------------------- |
    // | **Attaching**               | Adds compiled shader stages to a program object. No validation.                              |
    // | **Linking**                 | Finalizes the GPU pipeline by stitching stages together. Must be done before `glUseProgram`. |
    // | When is the program usable? | **Only after linking succeeds**                                                              |

    // Shader Program Stages Summary:
    // ------------------------------
    // glAttachShader → Stage compiled shader(s) for linking (not usable yet)
    // glLinkProgram  → Validate & finalize the shader pipeline (now usable)
    // Must glUseProgram(...) after linking to activate


    // Check whether the program linking was successful.
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success); // similar to Shaderiv

    // If linking failed, retrieve the error log from the GPU and output it to the console.
    if (!success) {
        char infoLog[512]; // Stores any linker output (errors, warnings, etc.)
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed:\n" << infoLog << '\n';
    }

    // Once linked, the individual shaders are no longer needed.
    // The GPU retains the compiled logic inside the program, and the original shader objects can be deleted.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Return the program ID so it can be used with glUseProgram(...) in rendering.
    return program;
}


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
    // Create an 800x600 window with title "GL Triangle Window"
    // last two args are for context sharing between windows - not needed here
    // Function call also creates an OpenGL context associated with the window.
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "GL Triangle Window", nullptr, nullptr);

    if(!window){
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate(); // shut down GLFW cleanly.
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
    });
    
    // Callback Function (Resize Handling)
    // -----------------------------------
    // What is a callback?
    // - A function that YOU write, but GLFW (or any system) calls LATER when an event happens.
    // - You don’t call this function manually—it’s run automatically when GLFW detects a matching event.
    //
    // Who decides when the callback runs?
    // - GLFW does. You simply *register* the function (with glfwSetFramebufferSizeCallback),
    //   and GLFW calls it every time the framebuffer size changes (e.g., on window resize).
    //
    // What does this callback do?
    // - It receives the new width and height of the framebuffer.
    // - We then call glViewport(...) to tell OpenGL to match the new window size.
    // - Without this, OpenGL would keep drawing to the old viewport and not scale with the window.

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

    // Set up triangle, shader objects, and load and link shaders

    float vertices[] = {
        // x, y coords
        // -0.5f, -0.5f, // bottom left
        // 0.5f, 0.5f,   // bottom right
        // 0.0f, 0.5f    // top center

        -0.25f, -0.25f, 
        0.0f, 0.25f,   
        0.25f, -0.25f    
    };

    // For use with glDrawElements (more useful for my game):
    unsigned int indices[] = {
        0, 1, 2  // draw triangle using vertex 0 → 1 → 2
    };

    float xOffset, yOffset;
    
    // Create VAO (vertex array object)
    // Create and bind VAO BEFORE VBO bind, binding VAO "starts recording state"
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); 

    unsigned int VBO;                       // vertex buffer object
    glGenBuffers(1, &VBO);                  // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);     // Bind it as a vertex buffer
    // Binding a VBO to GL_ARRAY_BUFFER sets it as the source of vertex data for upcoming attribute configuration calls, like glVertexAttribPointer.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // This does the actual upload of vertex data to the GPU.
    // GL_STATIC_DRAW -> “This data will be set once and drawn many times”
    // "I'm not going to update this often—just use it for drawing"

    // | Function       | Purpose                                      |
    // | -------------- | -------------------------------------------- |
    // | `glGenBuffers` | Ask OpenGL to allocate a buffer              |
    // | `glBindBuffer` | Make it the *current* buffer for vertex data |
    // | `glBufferData` | Copy your CPU-side array into the GPU buffer |

    // | Usage Hint        | Meaning                                                        |
    // | ----------------- | -------------------------------------------------------------- |
    // | `GL_STATIC_DRAW`  | Data will not change often; used for static models, UI, etc.   |
    // | `GL_DYNAMIC_DRAW` | Data will change often; used for frequently updated animations |
    // | `GL_STREAM_DRAW`  | Data will change every frame; used for real-time streams       |

    // What happens internally:
    // Your float[] array in RAM:
    // [-0.5, -0.5, 0.5, -0.5, 0.0, 0.5]

    // ↓ glBufferData

    // GPU Memory (VRAM):
    //     [same data now lives here]

    // For glDrawElements:
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,                  // attribute location (in vertex shader)
        2,                  // number of components (x, y)
        GL_FLOAT,           // data type
        GL_FALSE,           // normalize?
        2 * sizeof(float),  // stride (bytes between vertices)
        (void*)0            // offset into data
    );

    // One can think of the VAO as a lookup table of mappings for the data in the VBO.

    // glVertexAttribPointer(
    //     index,          // attribute location in the vertex shader
    //     size,           // number of components (e.g., 2 for vec2, 3 for vec3)
    //     type,           // usually GL_FLOAT
    //     normalized,     // whether to normalize (typically false for floats)
    //     stride,         // total byte size of one vertex
    //     pointer         // byte offset to this attribute within each vertex
    // );
    // “Attribute index in the vertex shader should read from the currently bound VBO
    //  using this layout. Also, store this association in the currently bound VAO.”
    glEnableVertexAttribArray(0);

    // Effectively, each call of glVertexAttribPointer sets up the mapping for a single
    // attribute in the VBO. The index given corresponds to the 'attribute index' that
    // is then activated by glEnableVertexAttribArray(index). The index has nothing
    // to do with the byte offset in the VBO. The index is the location in the vertex
    // shader.
    
    // GLSL example:
    // layout (location = 0) in vec3 aPosition;
    // layout (location = 1) in vec3 aColor;
    // layout (location = 2) in vec2 aTexCoord;
    
    // After this point, the VAO, which is 'recording state', understands that:
    // Attribute 0 uses VBO X, reads 2 floats...
    
    // Load shader sources
    std::string vertexSrc = loadShaderSource("shaders/vertex.glsl");
    std::string fragmentSrc = loadShaderSource("shaders/fragment.glsl");
    
    // Compile individual shaders
    GLuint vertexShader = compileShader(vertexSrc.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSrc.c_str(), GL_FRAGMENT_SHADER);

    // Link them into a program
    GLuint shaderProgram = linkProgram(vertexShader, fragmentShader);

    // Everything is now ready, enter draw loop.
    // Main game/render loop, runs until close button is pressed or glfwSetWindowShouldClose(window, true) is called

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);     // Clear the screen to the background color
        // GL_COLOR_BUFFER_BIT is a bitmask constant that tells OpenGL to clear the color buffer
        // using the value previously set with glClearColor().
        
        // Handle horizontal input polling
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
            // left key pressed
            xOffset = xOffset - 0.01f;
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
            // right key pressed
            xOffset = xOffset + 0.01f;
        }

        // Handle vertical input polling
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            // up key pressed
            yOffset = yOffset + 0.01f;
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            // down key pressed
            yOffset = yOffset - 0.01f;
        }

        glUseProgram(shaderProgram);

        // int offsetLoc = glGetUniformLocation(shaderProgram, "xOffset");
        // Asks OpenGL:
        // “In the currently linked shader program, where is the uniform variable named xOffset?”
        // OpenGL will:
        // Look into the compiled+linked shader program
        // Find the memory location for the variable xOffset
        // Return an integer handle that refers to that location
        // OpengGL doesn't expose actual variable pointers in GLSL - uses opaque location IDs
        glUniform1f(glGetUniformLocation(shaderProgram, "xOffset"), xOffset);
        glUniform1f(glGetUniformLocation(shaderProgram, "yOffset"), yOffset);

        // Sends the value of xOffset(C++) to the GPU at the location where xOffset is found
        // in the GLSL shader program. 
        // * THIS IS THE BASIC METHOD TO UPDATE VALUES USED IN SHADER CODE!

        glBindVertexArray(VAO);           // ✅ Reactivate the same VAO for drawing
        // Render Loop Setup Notes
        // ------------------------
        // Why glBindVertexArray() is called again:
        // - The first glBindVertexArray(...) earlier in the code was for *configuration*.
        //   → It told OpenGL how to interpret the vertex data (via glVertexAttribPointer).
        //
        // - This glBindVertexArray(...) is for *use* during drawing.
        //   → It reactivates the VAO so the GPU knows where to fetch vertex data from,
        //     and how to match it to shader inputs.
        //
        // OpenGL does not retain the VAO binding between frames or state changes,
        // so you must bind the VAO (and shader program) again before issuing draw calls.

        // glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // void glDrawArrays(GLenum mode, GLint first, GLsizei count);
        // -----------------
        // mode:   GL_TRIANGLES = interpret every group of 3 vertices as a triangle
        // first:  Index of the first vertex in the VBO to use (usually 0)
        // count:  Total number of vertices to process (e.g., 3 for one triangle)
        //
        // Other common modes include:
        //   GL_POINTS           → render vertices as dots
        //   GL_LINES            → pairs of vertices form lines
        //   GL_LINE_STRIP       → connected line segments
        //   GL_TRIANGLE_STRIP   → shared-vertex triangle chain
        //   GL_TRIANGLE_FAN     → radial triangles sharing the first vertex

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        // | Argument          | Meaning                                         |
        // | ----------------- | ----------------------------------------------- |
        // | `GL_TRIANGLES`    | Draws one triangle per 3 indices                |
        // | `3`               | Number of indices to use                        |
        // | `GL_UNSIGNED_INT` | Type of the indices in the EBO (`unsigned int`) |
        // | `0`               | Offset in the EBO (start at beginning)          |

        // ! NOTE: glDrawElements would be better suited for my game.
        // ! glDrawElements allows you to draw objects that share vertices w/o
        // ! having to duplicate the shared vertices.
        
        // glDrawArrays
        // float vertices[] = {
        //     triangle 1
        //     -0.5f, -0.5f,  // bottom left
        //      0.5f, -0.5f,  // bottom right
        //      0.5f,  0.5f,  // top right
        
        //      triangle 2
        //      0.5f,  0.5f,  // top right (again)
        //     -0.5f,  0.5f,  // top left
        //     -0.5f, -0.5f   // bottom left (again)
        // };

        // glDrawElements
        // float vertices[] = {
        //     -0.5f, -0.5f,  // 0: bottom left
        //      0.5f, -0.5f,  // 1: bottom right
        //      0.5f,  0.5f,  // 2: top right
        //     -0.5f,  0.5f   // 3: top left
        // };
        // unsigned int indices[] = {
        //     0, 1, 2,  // first triangle
        //     2, 3, 0   // second triangle
        // };

        // for glDrawElements, add during setup after binding VAO:
        // unsigned int EBO;
        // glGenBuffers(1, &EBO);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
    
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
