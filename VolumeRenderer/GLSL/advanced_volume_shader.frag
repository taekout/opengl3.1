@#version 120
@#extension GL_ARB_texture_rectangle : enable
@#extension all : warn
// volume rendering ray caster, supporting clipping and embedded objects

// shader options
#undef SPACESKIP
#define RANDSTEP
#undef DEPTH
#undef SLICES
#define BOX
#undef CLIP
#undef DEBUGPLANE
#undef DEBUGEXIT

// maximum steps through volume
#define MAXSTEPS 500

uniform vec3 volumeSize; // volume is centered from -volumeSize/2 .. volumeSize/2
uniform vec3 voxelSize;	 // dimensions in voxels in each direction

// opticalScale scales between geometric size and optical depth
//   increasing opticalScale makes the object more opaque
//   decreasing opticalScale makes the object more transparent
uniform float opticalScale;

varying vec4 Eo;		// eye location in object space
varying vec4 Vo;		// position in object space
varying vec4 DebugColor;	// color each plane for debugging

//////////////////////////////
// get volume data
// should define voldata() returning a vec4 data value
// vec4 voldata(vec4 p)
//    p = position of this sample
#include "volume/voldata-lookup.glsl"

//////////////////////////////
// transfer function
// should define transfer() returning sample color & opacity
// vec4 transfer(vec3 p, vec4 val, vec4 prev)
//    p = position of this sample
//    val = current sample value
//    prev = previous sample
#include "volume/transfer-lit.glsl"

//////////////////////////////
// integration function
// should define integration() returning color & opacity of a step
// vec4 integration(vec3 p, float step, vec4 val, 
//                  inout vec4 pVal, inout vec4 pColor)
//    p = position of this sample
//    step = size of the integration step
//    val = current sample value
//    pVal, pColor = previous sample value 
// integration() should call transfer if necessary and update previous values.
// The previous values are for integration() use only, only set if needed
const float raystep=1;		// minimum ray step to take (in voxels)
#include "volume/integration-linear.glsl"

// convert volume result to step (for optimized volumes)
float stepSize(vec4 volume) {
#ifdef SPACESKIP
    // the optimizer never uses alpha of 1, so if alpha is exactly 1
    // this must be an unprocessed volume, so use unit step; otherwise
    // convert step of (0..1) into an integer step size multiplier
    return max(raystep, volume.a == 1. ? raystep : volume.a*255.-.5);
#else
    return raystep;
#endif
}

#ifdef DEPTH
uniform bool fxcam;		// is the fxcamDepth valid?
uniform sampler2DRect fxcamDepth;	// previous z-buffer
#endif

void main() {
#ifdef DEBUGPLANE
    // vertex shader sets each type of plane to a different color
    gl_FragColor = DebugColor;
    return;
#endif

    // perspective-safe ray direction
    vec3 p = Vo.xyz/Vo.w;
    vec3 d = normalize(p*Eo.w - Eo.xyz);

    // find ray exit point from next slicing plane (or far plane)
    vec4 backPlane = vec4(0.,0.,-1.,1.)*gl_ModelViewProjectionMatrix;
#ifdef SLICES
    // fraction from back to front corner (0 to 1)
    float z = dot(normalize(backPlane.xyz),p)/length(volumeSize) + .5;

    // convert into slice number
    int slice = int(float(SLICES)*z-.5+1e-4);

    // move the back plane up if were in front of the backmost slice
    if (slice > 0) {
	float offset = (float(slice)-.5)/float(SLICES)-.5;
	backPlane.w = -offset*length(backPlane.xyz)*length(volumeSize);
    }
#endif
    float exit = -(dot(backPlane.xyz,p)+backPlane.w)/dot(backPlane.xyz,d);

#ifdef BOX
    // intersect ray with box edges
    vec3 box = (sign(d)*volumeSize/2. - p)/d;
    exit = min(exit,min(box.x,min(box.y,box.z)));
#endif

#ifdef CLIP
    // also intersect with user clipping planes
    for(int i=0; i<gl_MaxClipPlanes; ++i) {
	vec4 plane = gl_ClipPlane[i]*gl_ModelViewMatrix;
	float p_d = dot(plane.xyz,d);
	if (p_d<0.)
	    exit = min(exit, -(dot(plane.xyz,p)+plane.w)/p_d);
    }
#endif

#ifdef DEPTH
    // finally, check against previous z-buffer
    if (fxcam) {
	float zbuf = texture2DRect(fxcamDepth,gl_FragCoord.xy).r;
	vec4 plane = vec4(0.,0.,gl_DepthRange.diff,
			  (gl_DepthRange.near+gl_DepthRange.far-2.*zbuf)) 
	    * gl_ModelViewProjectionMatrix;
	exit = min(exit, -(dot(plane.xyz,p)+plane.w)/dot(plane.xyz,d));
    }
#endif

#ifdef DEBUGEXIT
    // show exit distance
    gl_FragColor = vec4(vec3(exit/length(volumeSize)),1.);
    return;
#endif

#ifdef RANDSTEP
    // shrink factor to take a random partial first step
    // based on Hammersley sequence for interleaved bits of x&y
    uvec2 v = uvec2(gl_FragCoord.xy);
    unsigned int ham = 0u; float hamMax = 1;
    ham = ham<<2u | v.x<<1u&2u | v.y&1u; v>>=1u; hamMax *= 4.;
    ham = ham<<2u | v.x<<1u&2u | v.y&1u; v>>=1u; hamMax *= 4.;
    ham = ham<<2u | v.x<<1u&2u | v.y&1u; v>>=1u; hamMax *= 4.;
    float start = float(ham+1u)/hamMax;
#else
    float start=1.;
#endif

    // scale size of ray step to volume
    float scale = length(volumeSize)/length(voxelSize);
    d *= scale; exit /= scale;

    // start value on ray and first step
    vec4 vol = voldata(p), pVal=vec4(0);
    vec4 pColor = vec4(0);
    vec4 color = integration(p, 0., vol, pVal, pColor);
    float step = stepSize(vol)*start;
    float s=step; p += d*step;

    scale *= opticalScale;	// factor in optical scaling

    // loop through volume
    //   i loop count provides a constant max to avoid NVIDIA compiler bug
    vec4 result = vec4(0);
    float end = exit-step;
    int i;
    for(i=0; i<MAXSTEPS && s<end; ++i) {
	// look up in volume, pass through transfer function, and accumulate
	vol = voldata(p);
	vec4 color = integration(p, scale*step, vol, pVal, pColor);
	result += (1-result.a)*color;

	// advance ray location
	step=stepSize(vol);
	s += step; p += d*step;

	// exit if opaque
 	if (result.a > 254./255.) i=MAXSTEPS;
    }

    // last partial step
    if (i < MAXSTEPS) {
	step = exit-s;
	p += d*step;

	// look up in volume, pass through transfer function, and accumulate
	vol = voldata(p);
	vec4 color = integration(p, scale*step, vol, pVal, pColor);
	result += (1-result.a)*color;
    }

    gl_FragColor = result;
}
