// 1D transfer function for voltrace volume renderer
// For volume rendering with the gradient-based lighting model

// 1D transfer function mapping scalar values to color and opacity
uniform sampler2D volumeLUT;

vec4 transfer(vec3 p, vec4 val, inout vec4 pVal)
{
    vec4 xfer = texture2D(volumeLUT, val.rr);

    // this version only does lighting for opaque regions in the
    // transfer function
    if (xfer.a >= 1.) {
	// vector from current point in volume to light & viewer
	vec4 Lp = gl_ModelViewMatrixInverse*gl_LightSource[0].position;
	vec4 Vp = gl_ModelViewProjectionMatrixInverse[3];
	vec3 L = normalize(Lp.xyz-p*Lp.w);
	vec3 V = normalize(Vp.xyz-p*Vp.w);
	vec3 H = normalize(L+V);

	// compute normal from gradient: could preprocess to store in
	// volume (say val.xyz=normal, val.w=actual scalar value) for
	// now, do an incredibly inefficient extra six volume accesses.
	vec3 N = normalize(vec3(voldata(p-vec3(1./voxelSize.x,0.,0.)).r
				- voldata(p+vec3(1./voxelSize.x,0.,0.)).r,
				voldata(p-vec3(0.,1./voxelSize.y,0.)).r
				- voldata(p+vec3(0.,1./voxelSize.y,0.)).r,
				voldata(p-vec3(0.,0.,1./voxelSize.z)).r
				- voldata(p+vec3(0.,0.,1./voxelSize.z)).r));

	// compute diffuse & specular terms
	float diffuse = max(0.,dot(N,L));
	float specular = pow(max(0.,dot(N,H)), 64.);

	xfer.rgb = diffuse*(xfer.rgb + specular);
    }

    return xfer;
}
