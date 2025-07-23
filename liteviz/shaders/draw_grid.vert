#version 430

uniform mat4 ProjMat;
in vec4 Color;
in vec3 Position;
out vec3 Frag_Position;
out vec4 Frag_Color;

void main() {
    Frag_Position = Position;
    Frag_Color = Color;
    gl_Position = ProjMat * vec4(Position, 1);
}