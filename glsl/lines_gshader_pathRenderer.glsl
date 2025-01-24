#version 460

layout (lines) in;
layout (triangle_strip, max_vertices = 9) out;

uniform int maxSteps;
uniform mat4 mvpMatrix;

    /* QVector3D direction = currentLocation - prevLocation; */
    /* QVector3D tempValue = (currentLocation + QVector3D(1, 0, 0)) - prevLocation; */
    /* direction.normalize(); */
    /* QVector3D base = prevLocation;  //+ (0.12 * direction); */
    /* QVector3D direction2 = QVector3D::crossProduct(direction, tempValue); */
    /* direction2.normalize(); */
    /* // QVector3D base2 = base + (0.04 * direction2); */
    /* QVector3D base3 = base + (-0.04 * direction2); */
    /* QVector3D direction3 = QVector3D::crossProduct(direction, direction2); */
    /* direction3.normalize(); */
    /* QVector3D base4 = base + (0.04 * direction3); */
    /* QVector3D base5 = base + (-0.04 * direction3); */

    // pathLinesResult.push_back(base2 / maxSteps);
    /* temp.push_back(base3 / maxSteps); */
    /* temp.push_back(prevLocation / maxSteps); */

    /* temp.push_back(base4 / maxSteps); */
    /* temp.push_back(base5 / maxSteps); */

    // pathLinesResult.push_back(base2 / maxSteps);
    // pathLinesResult.push_back(currentLocation / maxSteps);

    /* temp.push_back(base3 / maxSteps); */
    /* temp.push_back(currentLocation / maxSteps); */

    /* temp.push_back(base4 / maxSteps); */
    /* temp.push_back(currentLocation / maxSteps); */

    /* temp.push_back(base5 / maxSteps); */
    /* temp.push_back(currentLocation / maxSteps); */

void helperFunc(vec3, vec3);

void main(){
  helperFunc(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz);
}

void helperFunc(vec3 start, vec3 end){
  vec3 direction = end - start;
  vec3 tempValue = (end + vec3(1,0,0)) - start;
  direction = normalize(direction);
  vec3 base = start;
  vec3 direction2 = cross(direction, tempValue);
  direction2 = normalize(direction2);
  vec3 direction3 = cross(direction, direction2);
  direction3 = normalize(direction3);

  vec3 base3 = (base + (-0.04 * direction2) / maxSteps);
  vec3 base4 = (base + (0.04 * direction3) / maxSteps);
  vec3 base5 = (base + (-0.04 * direction3) / maxSteps);

  gl_Position = mvpMatrix * vec4(start.xyz, 1);
  EmitVertex();
  gl_Position = mvpMatrix * vec4(base3.xyz, 1);
  EmitVertex();
  gl_Position = mvpMatrix * vec4(end.xyz, 1);
  EmitVertex();
  EndPrimitive();

  gl_Position = mvpMatrix * vec4(start.xyz, 1);
  EmitVertex();
  gl_Position = mvpMatrix * vec4(base4.xyz, 1);
  EmitVertex();
  gl_Position = mvpMatrix * vec4(end.xyz, 1);
  EmitVertex();
  EndPrimitive();

  gl_Position = mvpMatrix * vec4(start.xyz, 1);
  EmitVertex();
  gl_Position = mvpMatrix * vec4(base5.xyz, 1);
  EmitVertex();
  gl_Position = mvpMatrix * vec4(end.xyz, 1);
  EmitVertex();
  EndPrimitive();

}
