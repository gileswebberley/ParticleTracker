#version 150

uniform sampler2DRect backbuffer;   // previous velocity texture
uniform sampler2DRect posData;      // position texture
uniform vec2 screen;
uniform float timestep;

in vec2 vTexCoord;

out vec4 vFragColor;
    
void main(void){
    // Get the position and velocity from the pixel color.
    vec3 pos = texture( posData, vTexCoord).xyz;
    vec3 vel = texture( backbuffer, vTexCoord).xyz;

    float elasticity = pos.z;
    float resistance = vel.z;
    //divide by 10 so it doesn't follow too closely
    float attraction = (1-resistance)/10;
    //find the vector that goes from position to screen.xy
    vec2 targetV = screen.xy - pos.xy;
    //make it a unit vector by dividing by it's magnitude |v|
    targetV /= distance(screen.xy,pos.xy);
    //slowly turn to face the direction of the target
    vel.xy += attraction*targetV;
        
    // Calculate what´s going to be the next position without updating it,
    // to see if it collide with the borders of the FBO texture.
    vec2 nextPos = pos.xy;
    nextPos += (vel.xy * resistance) * timestep;
        
    // If it´s going to collide, change the velocity course.
    if ( nextPos.x < attraction)
        vel.x = elasticity * abs(vel.x);
        
    if ( nextPos.x > 1-attraction)
        vel.x = -elasticity * abs(vel.x);
        
    if (nextPos.y < attraction)
        vel.y = elasticity * abs(vel.y);
    
    if ( nextPos.y > 1-attraction)
        vel.y = -elasticity * abs(vel.y);
    
    // Then save the vel data into the velocity FBO.
    vFragColor = vec4(vel.x,vel.y,vel.z,1.0);
}
