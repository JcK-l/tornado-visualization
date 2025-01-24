#version 460
uniform mat4 mvpMatrix;
uniform float currentStep;
in vec4 vertexPosition;
flat out int vertexID;

void main()
{
    gl_Position = mvpMatrix * vec4(vertexPosition.xy, currentStep, vertexPosition.w);
    vertexID = gl_VertexID;
}