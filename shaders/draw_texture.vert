#version 400

uniform mat4 ProjMat;
in vec2 TexCoords;
in vec3 Position;
out vec2 Frag_TexCoords;
out vec3 Frag_Position;
void main() {
    Frag_Position = Position;
    Frag_TexCoords = TexCoords;
    gl_Position = ProjMat * vec4(Position, 1);
}