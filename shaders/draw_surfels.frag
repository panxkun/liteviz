#version 400

in vec3 vColor0;
in vec3 s;
in vec2 texcoord;

out vec4 Out_Color;

void main()
{  
    vec2 scaled_texcoord = vec2(texcoord.x / s.x, texcoord.y / s.y);
    if(dot(scaled_texcoord, scaled_texcoord) > 1)
        discard;

    Out_Color = vec4(vColor0.xyz, 1.0f);
    
    gl_FragDepth = gl_FragCoord.z;
}
