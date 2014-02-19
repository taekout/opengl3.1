// 2D preintegrated transfer function for voltrace volume renderer
// requires "--integration preint" for preintegrated integration mode

// volumeLUT(s,t) gives the color and opacity for going from scalar
// value s to scalar value t over a unit-length segment of volume
uniform sampler2D volumeLUT;	

vec4 transfer(vec3 p, vec4 val, inout vec4 pVal)
{
    vec4 xfer = texture2D(volumeLUT, vec2(val.r,pVal.r));
    pVal = val;
    return xfer;
}
