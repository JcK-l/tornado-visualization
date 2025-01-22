#version 330
const int MAX_ISO_AMOUNT = 5;

layout(location = 0) out vec4 fragColor;
flat in int vertexID;
uniform int isoStateColor;
uniform int currentActiveIso;
uniform int IsoSizes[MAX_ISO_AMOUNT];

void main()
{
    // Set the color "red", RGB = (1, 0, 0) to the fragment.
    bool isActiveIso = (currentActiveIso == 0) ? vertexID < IsoSizes[0] :
    IsoSizes[currentActiveIso - 1] <= vertexID && vertexID < IsoSizes[currentActiveIso];

    if (isActiveIso){
        fragColor = vec4(isoStateColor & 8, isoStateColor & 4, isoStateColor & 2, isoStateColor & 1);
    } else {
        fragColor = vec4(1, 1, 1, 1);
    }
}

