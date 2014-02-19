// 1D transfer function for voltrace volume renderer
// For volume rendering with the emission/absorption model, in which
// each voxel emits light according to the transfer function and absorbs
// light passing through it according to the transfer alpha


// 1D transfer function mapping scalar values to color and opacity
uniform sampler2D volumeLUT;

vec4 transfer(vec3 p, vec4 val, inout vec4 pVal)
{
    return texture2D(volumeLUT, val.rr);
}
