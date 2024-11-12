#version 400

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec4 vColor[];
in vec4 vPosition[];
in vec3 vNormal[];
in mat4 vMVP[];
in vec3 vScale[];

out vec3 vColor0;
out vec3 v;
out vec3 s;
out vec2 texcoord;

void main() 
{
    vColor0 = vColor[0].xyz;

    s = vScale[0];

    float scale_x = vScale[0].x;
    float scale_y = vScale[0].y;

    vec3 x = normalize(vec3((vNormal[0].y - vNormal[0].z), -vNormal[0].x, vNormal[0].x)) * scale_x * 1.41421356;
    vec3 y = cross(vNormal[0].xyz, x) * scale_y;

    texcoord = vec2(-1.0, -1.0);
    gl_Position = vMVP[0] * vec4(vPosition[0].xyz + x, 1.0);
    v = vPosition[0].xyz + x;
    EmitVertex();

    texcoord = vec2(1.0, -1.0);
    gl_Position = vMVP[0] * vec4(vPosition[0].xyz + y, 1.0);
    v = vPosition[0].xyz + y;
    EmitVertex();

    texcoord = vec2(-1.0, 1.0);
    gl_Position = vMVP[0] * vec4(vPosition[0].xyz - y, 1.0);
    v = vPosition[0].xyz - y;
    EmitVertex();

    texcoord = vec2(1.0, 1.0);
    gl_Position = vMVP[0] * vec4(vPosition[0].xyz - x, 1.0);
    v = vPosition[0].xyz - x;
    EmitVertex();
    EndPrimitive();
}
