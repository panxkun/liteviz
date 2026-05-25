#version 430

uniform sampler2D TexSampler;
uniform float Alpha;
in vec2 Frag_TexCoord;
out vec4 Out_Color;

void main() {
    vec4 texColor = texture(TexSampler, Frag_TexCoord);
    Out_Color = vec4(texColor.rgb, texColor.a * Alpha);
}
