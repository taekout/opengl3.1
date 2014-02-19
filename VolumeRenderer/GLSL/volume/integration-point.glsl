// integration step as a scaled constant color
// also used with preintegration transfer function, since that
// transfer function manages the value interpolation
// (but not step size scaling)

// return color and opacity after one ray step given:
//   p = position of this sample
//   step = size of this step
//   val = value at p
//   pVal, pColor, pAlpha = saved data from previous step
//   color = output color for this step
//   alpha = per-channel opacity for this step
vec4 integration(vec3 p, float step, vec4 val, 
		 inout vec4 pVal, inout vec4 pColor) 
{
    // compute transfer function for this step
    vec4 xfer = transfer(p, val, pVal);
    xfer.a = clamp(xfer.a, 0.,1.);

#if 1
    // re-scale integration to step size
    float alpha = 1 - pow(1-xfer.a,step);
    vec3 color = xfer.rgb*alpha/max(xfer.a,1e-9); // avoid divide by 0
    return vec4(color, alpha);
#else
    // linear approximation to step size
    float scale = mix(xfer.a,1,step);
    return xfer*scale;
#endif

}
