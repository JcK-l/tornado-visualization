#version 330
uniform mat4 mvpMatrix;
uniform float currentStep;
in vec4 vertexPosition;
flat out int vertexID;


void main()
{
    // Calculate vertex position in screen space.
    gl_Position = mvpMatrix * vec4(vertexPosition.x, vertexPosition.y, currentStep, vertexPosition.w);
    vertexID = gl_VertexID;
}
