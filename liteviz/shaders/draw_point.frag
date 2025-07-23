#version 430

uniform float Alpha;
in vec3 Frag_Position;
in vec4 Frag_Color;
out vec4 Out_Color;

void main() {
    Out_Color = vec4(Frag_Color.rgb, Frag_Color.a * Alpha);
}