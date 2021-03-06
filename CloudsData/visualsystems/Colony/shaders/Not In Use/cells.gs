#version 120
#extension all : warn
#define RESOLUTION 6
#define PI 3.14159265359
#define SPREAD_RADIUS 100.


uniform vec2 screenResolution;
void main() {
    //IN
    vec3 p_in = gl_PositionIn[0].xyz;
    vec2 data = gl_PositionIn[1].xy;     //size = data.x, angle = data.y;
    
    //FLAT PLACEMENT
    vec4 p = vec4(vec3(p_in.xy - screenResolution/2.,100.),1.);
    
    //POINT GENERATION
    float opacity = 0.;
    float magnitude = 20.;
    for (int i = 0; i < RESOLUTION ; i++) {
        /*
         b
         |\
         | \
         a--c
         */
        float f = data.y + (float(i) / float(RESOLUTION)) * 2. * PI;
        
        //a
        gl_Position = gl_ModelViewProjectionMatrix  * (p + vec4(data.x * magnitude * vec3(cos(f), sin(f), 0.), 0.));
        gl_FrontColor = vec4(1., 1., 1., opacity);
        EmitVertex();
        
        //b
        gl_Position =  gl_ModelViewProjectionMatrix * p;
        gl_FrontColor = vec4(1., 1., 1., 1.);
        EmitVertex();
        
        //c
        f += (1. / float(RESOLUTION)) * 2. * PI;
        gl_Position =  gl_ModelViewProjectionMatrix * (p + vec4(data.x * magnitude * vec3(cos(f), sin(f), 0.), 0.));
        gl_FrontColor = vec4(1., 1., 1., opacity);
        EmitVertex();
        EndPrimitive();
    }
    
}