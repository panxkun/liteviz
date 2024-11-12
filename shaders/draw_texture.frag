#version 400

uniform sampler2D Texture;
in vec2 Frag_TexCoords;
in vec3 Frag_Position;
out vec4 Out_Color;

void main(){
    vec2 texCoord = vec2(1.0 - Frag_TexCoords.y, 1.0 - Frag_TexCoords.x);
    Out_Color = texture(Texture, texCoord); 
}