#version 330
layout(location = 0) out vec4 fragColor;
uniform bool isActive;

void main()
{
    // Set the color "red", RGB = (1, 0, 0) to the fragment.
    if (isActive){
        fragColor = vec4(0, 0, 1, 1);
    } else {
        fragColor = vec4(128.0 / 255.0, 172.0/ 255.0, 241.0/255.0, 1);
    }
}

