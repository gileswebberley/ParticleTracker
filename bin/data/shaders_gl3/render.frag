#version 150

//sparkTex is the image texture that is attached to the particles
uniform sampler2DRect sparkTex;

in vec2 vTexCoord;
//gColor is set with mesh.addColor(r,g,b) passed on from geom shader
in vec4 gColor;

out vec4 vFragColor;

void main() {
//    vec3 myColor = texture(vColor, vTexCoord).rgb;
    vec4 myTex = texture(sparkTex, vTexCoord).rgba;
    myTex.rgb *= gColor.rgb;
    vFragColor = myTex;
}
