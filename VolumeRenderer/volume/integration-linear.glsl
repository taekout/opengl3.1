// approximation for linear interpolation of color & alpha
// from Ken Morelands PhD dissertation, UNM 2004
// "Fast High Accuracy Volume Rendering"

// return color and opacity after one ray step given:
//   p = position of this sample
//   step = size of this step
//   val = value at p
//   pVal, pColor = saved value, color and alpha from previous step
vec4 integration(vec3 p, float step, vec4 val, 
		 inout vec4 pVal, inout vec4 pColor)
{
    // compute transfer function for this step
    vec4 xfer = transfer(p, val, pVal);
    xfer.a = clamp(xfer.a, 0.,1.);

    // integrate along the step
    float avg = .5*(xfer.a+pColor.a), diff=xfer.a-pColor.a;
    float alpha = 1 - pow(1.-avg-.108165*diff*diff, step);

    float beta = 1 - pow(1.-.27*xfer.a-.73*pColor.a, step);
    vec3 color = xfer.rgb*alpha + (pColor.rgb-xfer.rgb)*beta;

    // save point sampled colors for next step
    pColor = xfer;
    return vec4(color,alpha);
}
