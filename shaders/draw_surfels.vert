#version 400

uniform mat4 ProjMat;
in vec4 Color;
in vec3 Position;
in vec3 Normal;
in vec3 Scale;

out vec3 vPosition;
out vec4 vColor;
out vec3 vNormal;
out vec3 vScale;
out mat4 vMVP;

void main() {
    vPosition = Position;
    vColor = Color;
    vNormal = Normal;
    vMVP = ProjMat;
    vScale = Scale;
    gl_Position = ProjMat * vec4(Position, 1.0);
}