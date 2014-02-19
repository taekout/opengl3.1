// color and opacity from data value; no texture

vec4 transfer(vec3 p, vec4 val, vec4 prev) 
{
    return vec4(0,0,0,val.x);
}
