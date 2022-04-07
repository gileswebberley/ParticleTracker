#version 150

uniform sampler2DRect backbuffer;   // previous velocity texture
uniform sampler2DRect posData;      // position texture
uniform sampler2DRect originalPos;//the original position that the particles started
uniform vec2 screen;//the target point
uniform float timestep;

in vec2 vTexCoord;//I think this is almost like the i and j variables in a 2d array loop

out vec4 vFragColor;
    
void main(void){
    // Get the position and velocity from the pixel color.
    vec3 pos = texture( posData, vTexCoord).xyz;
    vec3 vel = texture( backbuffer, vTexCoord).xyz;
    vec2 orig = texture( originalPos, vTexCoord).xy;
    vec2 targetV;

    float elasticity;
    float resistance;
    float attraction;

    if(screen.x <= 0 && screen.y <= 0){
        //find the vector that goes from position to orig.xy
        targetV = orig.xy - pos.xy;
        //make it a unit vector by dividing by it's magnitude |v|
        targetV /= distance(orig.xy,pos.xy);
        elasticity = pos.z;//0.7;
        resistance = vel.z*distance(orig.xy,pos.xy);//0.1;
        //divide by 10 so it doesn't follow too closely, this has a huge affect
        attraction = (1-resistance)/1000;
    }else{
        //find the vector that goes from position to screen.xy
        targetV = screen.xy - pos.xy;
        //make it a unit vector by dividing by it's magnitude |v|
        targetV /= distance(screen.xy,pos.xy);
        elasticity = pos.z;
        resistance = vel.z;//*abs(distance(screen.xy,pos.xy));
        //divide by 10 so it doesn't follow too closely, this has a huge affect
        attraction = (1-resistance)/10;
    }

    //slowly turn to face the direction of the target
    vel.xy += attraction*targetV;
        
    // Calculate what´s going to be the next position without updating it,
    // to see if it collide with the borders of the FBO texture.
    vec2 nextPos = pos.xy;
    //this is what is done in the posUpdate.frag
    nextPos += (vel.xy * resistance) * timestep;
        
    // If it´s going to collide with the screen edges, change the velocity course.

    if ( nextPos.x < 0)
        vel.x = elasticity * abs(vel.x);

    if ( nextPos.x > 1)
        vel.x = -elasticity * abs(vel.x);

    if (nextPos.y < 0)
        vel.y = elasticity * abs(vel.y);

    if ( nextPos.y > 1)
        vel.y = -elasticity * abs(vel.y);
    
    // Then save the vel data into the velocity FBO.
    vFragColor = vec4(vel.x,vel.y,vel.z,1.0);
}
