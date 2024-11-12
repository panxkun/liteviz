#version 400

uniform mat4 ProjMat;
in vec3 Position;
in vec3 Normal;
in vec4 Color;
out vec3 Frag_Position;
out vec4 Frag_Color;
out vec3 Frag_Normal;

void main() {
    Frag_Position = Position;
    Frag_Color = Color;
    Frag_Normal = Normal;
    gl_Position = ProjMat * vec4(Position, 1);
}