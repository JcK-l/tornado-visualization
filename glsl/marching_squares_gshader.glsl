#version 460

layout (lines_adjacency) in;
layout (line_strip, max_vertices = 4) out;
uniform float currentStep;
uniform int component;
uniform float c;
uniform mat4 mvpMatrix;

in VS_OUT
{
    vec3 vData;
} gs_in[];

vec2 interpolation(int, int);
int rotationLeft(int); 
int rotationRight(int); 
int countSetBits(int); 
int power2(int); 
vec4 getPoint(int);
float getValue(int);
float getBetrag(vec3);

void main() {
    float p1v, p2v, p3v, p4v, asymptote;

    vec4 p1 = getPoint(1);
    vec4 p2 = getPoint(2);
    vec4 p3 = getPoint(4);
    vec4 p4 = getPoint(8);

    p1v = getValue(1);
    p2v = getValue(2);
    p3v = getValue(4);
    p4v = getValue(8);

    int bitMap = 0x0;

    bitMap = (p1v > c) ? bitMap | 0x01 : bitMap;
    bitMap = (p2v > c) ? bitMap | 0x02 : bitMap;
    bitMap = (p3v > c) ? bitMap | 0x04 : bitMap;
    bitMap = (p4v > c) ? bitMap | 0x08 : bitMap;

    vec2 iso1, iso2;

    if (bitMap == 0x5 || bitMap == 0xA) {
        vec2 iso3, iso4;

    iso1 = interpolation(rotationLeft(bitMap & 0x3),
    bitMap & 0x3);
    iso2 = interpolation(rotationRight(bitMap & 0x3),
    bitMap & 0x3);
    iso3 = interpolation(rotationLeft(bitMap & 0xC),
    bitMap & 0xC);
    iso4 = interpolation(rotationRight(bitMap & 0xC),
    bitMap & 0xC);
    asymptote =
    ((p4v * p2v) - (p3v * p1v)) / 
    (p1v - p3v - p4v + p2v);
        if (asymptote < c) {
            gl_Position = mvpMatrix * vec4(iso1.x, iso1.y, currentStep, 1);
            EmitVertex();
            gl_Position = mvpMatrix * vec4(iso2.x, iso2.y, currentStep, 1);
            EmitVertex();
            EndPrimitive();
            gl_Position = mvpMatrix * vec4(iso3.x, iso3.y, currentStep, 1);
            EmitVertex();
            gl_Position = mvpMatrix * vec4(iso4.x, iso4.y, currentStep, 1);
            EmitVertex();
            EndPrimitive();
        } else {
            gl_Position = mvpMatrix * vec4(iso2.x, iso2.y, currentStep, 1);
            EmitVertex();
            gl_Position = mvpMatrix * vec4(iso3.x, iso3.y, currentStep, 1);
            EmitVertex();
            EndPrimitive();
            gl_Position = mvpMatrix * vec4(iso1.x, iso1.y, currentStep, 1);
            EmitVertex();
            gl_Position = mvpMatrix * vec4(iso4.x, iso4.y, currentStep, 1);
            EmitVertex();
            EndPrimitive();
        }
        return;
    }

    switch (countSetBits(bitMap)) {
        case 1:
            iso1 = interpolation(rotationLeft(bitMap), bitMap);
            iso2 = interpolation(rotationRight(bitMap), bitMap);
            break;
        case 2:
      iso1 = interpolation(rotationLeft(bitMap) & ~bitMap,
      rotationLeft(bitMap) & bitMap);
      iso2 = interpolation(rotationRight(bitMap) & ~bitMap,
      rotationRight(bitMap) & bitMap);
            break;
        case 3:
            iso1 = interpolation(bitMap ^ 0xF, rotationLeft(bitMap ^ 0xF));
            iso2 = interpolation(bitMap ^ 0xF, rotationRight(bitMap ^ 0xF));
            break;
        default:
            return;
    }
    gl_Position = mvpMatrix * vec4(iso1.x, iso1.y, currentStep, 1);
    EmitVertex();
    gl_Position = mvpMatrix * vec4(iso2.x, iso2.y, currentStep, 1);
    EmitVertex();
    EndPrimitive();
}

vec4 getPoint(int inp) {
    switch(inp) {
        case 0x1:
            return gl_in[0].gl_Position;
        case 0x2:
            return gl_in[1].gl_Position;
        case 0x4:
            return gl_in[2].gl_Position;
        case 0x8:
            return gl_in[3].gl_Position;
        default:
            return vec4(0, 0, 0, 0);
    }
}

float getBetrag(vec3 inp){
  float betrag;
  float xc2 = pow(inp.x, 2);
  float yc2 = pow(inp.y, 2);
  float zc2 = pow(inp.z, 2);
  betrag = sqrt(xc2 + yc2 + zc2);
  return betrag;
}

vec2 interpolation(int point1, int point2)
{ 
  vec4 p, pd;
  float pv, pdv;
  p = getPoint(point1);
  pd = getPoint(point2);
  pv = getValue(point1);
  pdv = getValue(point2);

    if(p.x != pd.x) {
    return vec2(((c - pv) * (pd.x - p.x) / (pdv -pv)) + p.x , p.y);
    } else {
        return vec2(p.x, ((c - pv) * (pd.y - p.y) / (pdv - pv)) + p.y);
    }
}

int rotationLeft(int inp){
  int result = inp << 1;
  result = ((result & 0x10) > 0) ? result - 16 + 1 : result;
  return result;
}

int rotationRight(int inp){
  int result = ((inp & 0x01) > 0) ? inp + 16 : inp;
  return result >> 1;
}

int countSetBits(int inp){
    int count = 0;
  for (int i = 0; i < floor(log2(inp) + 1); i++) {
    count = ((power2(i) & inp) > 0) ? count + 1 : count;
    }
    return count;
}

int power2(int inp){
  int result = 1;
  for (int i = 0; i < inp; i++) {
    result *= 2;
}
  return result;
}

float getValue(int inp){
  float p1v, p2v, p3v, p4v;
  switch(component){
        case 0:
      p1v = gs_in[0].vData.x;
      p2v = gs_in[1].vData.x;
      p3v = gs_in[2].vData.x;
      p4v = gs_in[3].vData.x;
      break;
        case 1:
      p1v = gs_in[0].vData.y;
      p2v = gs_in[1].vData.y;
      p3v = gs_in[2].vData.y;
      p4v = gs_in[3].vData.y;
      break;
        case 2:
      p1v = gs_in[0].vData.z;
      p2v = gs_in[1].vData.z;
      p3v = gs_in[2].vData.z;
      p4v = gs_in[3].vData.z;
      break;
        case 3:
      p1v = getBetrag(gs_in[0].vData);
      p2v = getBetrag(gs_in[1].vData);
      p3v = getBetrag(gs_in[2].vData);
      p4v = getBetrag(gs_in[3].vData);
      break;
  }
  switch(inp){
    case 0x1:
    return p1v;
    case 0x2:
    return p2v;
    case 0x4:
    return p3v;
    case 0x8:
    return p4v;
        default:
            return 0;
    }
}