#version 330 core

out vec4 FragColor;
// declares main output of fragment shader
// Every fragment (a pixel candidate generated from rasterizing the triangle) will call 
// main(), and your shader must assign a final RGBA color to this output.

void main(){
    FragColor = vec4(1.0, 0.5, 0.2, 1.0); // Orange
    // This simply outputs a constant color for every pixel in the triangle
    // There’s no lighting, no textures, no per-fragment computation yet—this is a 
    // minimal shader.
}