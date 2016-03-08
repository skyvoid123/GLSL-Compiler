// For extra credit, compiler needs to pass this shader correctly,
// without reporting any error.
//
vec3 gv3;
vec4 gv4;
vec2 gv2;

mat2 gm2;
mat3 gm3;
mat4 gm4;

vec3 foo(float f) {
    float myVal;
    vec3 tv3;
    vec4 tv4;
    mat3 tm3;

    // scalar multiply
    myVal = ((gm2 + f) * gv2).x;

    // matrix + float
    tm3 = (gm3 + myVal) * f;
    tm3 += gm3;
    tv3 = tm3 * gv3 + f;
 
    tv4 = ((gm4 + gm4) / myVal) * gv4;

    tv3 += gv2.yyx * f + tv4.xzw;

    return tv3 * gm3;
}   
    

