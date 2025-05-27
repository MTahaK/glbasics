#version 330 core // declares GLSL version being used: 3.30 -> 330

layout (location = 0) in vec2 aPos;
// declares input attribute to vertex shader
// location = 0 must match index in glVertexAttribPointer
// vec2: 2 components for (x,y) position
// `layout` tells GPU:
// “This input variable (e.g., aPos) should be connected to attribute index X as 
// configured by the application code using glVertexAttribPointer.”

// OpenGL will automatically read from the currently bound VAO, fetch the correct 
// vertex data from the linked VBO, and store it in aPos.

// For multiple attributes:
// layout (location = 0) in vec3 aPosition;
// layout (location = 1) in vec3 aColor;
// layout (location = 2) in vec2 aTexCoord;

// This would correspond to the following code in C++:
// // aPosition
// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset0);
// glEnableVertexAttribArray(0);

// // aColor
// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset1);
// glEnableVertexAttribArray(1);

// // aTexCoord
// glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset2);
// glEnableVertexAttribArray(2);

uniform mat4 model; // using GLM for transformations

// uniform float xOffset, yOffset;  // Allows for control of position of shape
// uniform - set once per frame or per object, value is same for all vertices
// or fragments in the draw call.
// Used for global offsets/shifts, time values in animation, transform matrices,
// lighting parameters, and anything that stays constant across a singel draw
void main(){
    // Entry point of vertex shader
    // Convert vec2 (2D position) to vec4 by setting z = 0, w = 1
    // w = 1 ensures proper division in the projection stage (rasterization)

    // gl_Position is a built-in output that all vertex shaders must write.
    // defines the position of the current vertex in clip space (NDC: Normalized Device 
    // Coordinates).
    // gl_Position = vec4(aPos.x + xOffset, aPos.y + yOffset, 0.0, 1.0);
    vec4 pos = vec4(aPos, 0.0, 1.0);
    gl_Position = model * pos;          // apply transform
}