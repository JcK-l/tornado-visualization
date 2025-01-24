#version 460

out vec4 FragColor;
uniform bool isActive;
uniform int isoStateColor;

void main() {
 // Set the color "red", RGB = (1, 0, 0) to the fragment.
  if (isActive){
    FragColor = vec4(1, 0, 0, 1);
  } else {
    FragColor = vec4(1, 1, 1, 1);
  }
}
