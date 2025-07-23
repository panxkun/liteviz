#version 430

uniform mat4 ProjMat;
uniform float PointSize;
in vec3 Position;
in vec4 Color;
out vec3 Frag_Position;
out vec4 Frag_Color;

void main() {
    Frag_Position = Position;
    Frag_Color = Color;
    gl_Position = ProjMat * vec4(Position, 1);
    gl_PointSize = PointSize;
}