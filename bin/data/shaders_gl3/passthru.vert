#version 150

uniform mat4 modelViewProjectionMatrix;
//in means it's per vertex or fragment(pixel)
//position is the xyz of the individual vertex
in vec4 position;
//texcoord is the associated texture coordinate
in vec2 texcoord;

out vec2 vTexCoord;

void main() {
    //pass on the texcoord through the pipeline
    vTexCoord = texcoord;
    //every vertex shader must output the gl_Position
    gl_Position = modelViewProjectionMatrix * position;
}
