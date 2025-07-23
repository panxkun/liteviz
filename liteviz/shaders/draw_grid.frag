#version 430

in vec3 Frag_Position;
in vec4 Frag_Color;
out vec4 Out_Color;

void main(){
    float distance = length(Frag_Position);
    float transparency = Frag_Color.a * (1.0 - smoothstep(10, 100, distance));
    Out_Color = vec4(Frag_Color.rgb, transparency);
}