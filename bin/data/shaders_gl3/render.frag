#version 150

// This fill the billboard made on the Geometry Shader with a texture

uniform sampler2DRect sparkTex;

in vec2 vTexCoord;
//gColor is set with mesh.addColor(r,g,b) passed on from geom shader
in vec4 gColor;

out vec4 vFragColor;

void main() {
//    vec3 myColor = texture(vColor, vTexCoord).rgb;
    vec4 myTex = texture(sparkTex, vTexCoord).rgba;
    myTex.rgb *= gColor.rgb;
    vFragColor = myTex;//vec4(myTex.rgb,0.5);
//    vFragColor = texture(vColor, vTexCoord);
}
