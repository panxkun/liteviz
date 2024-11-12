    #version 400
    
    in vec3 Frag_Position;
    in vec4 Frag_Color;
    in vec3 Frag_Normal;
    out vec4 Out_Color;

    const vec3 ambientLightColor = vec3(0.2, 0.2, 0.2);
    vec3 lightPosition = vec3(0.0, 10.0, 10.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    void main(){  
        vec3 norm = normalize(Frag_Normal);  
        vec3 lightDir = normalize(lightPosition - Frag_Position);  
        float diff = max(dot(norm, lightDir), 0.0);  
        vec3 ambient = ambientLightColor * Frag_Color.rgb;
        vec3 diffuse = lightColor * diff * Frag_Color.rgb; 
        vec3 result = ambient + diffuse;  
        Out_Color = vec4(result, Frag_Color.a);
    }  