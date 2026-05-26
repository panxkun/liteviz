#version 430

uniform mat4 ProjMat;
in vec3 Position;
in vec2 TexCoord;
out vec2 Frag_TexCoord;

void main() {
    Frag_TexCoord = TexCoord;
    gl_Position = ProjMat * vec4(Position, 1.0);
}
