namespace glsl {
	const char reflFrag[] = R"(
#version 330

uniform mat4 _IP;
uniform vec2 screenSize;
uniform sampler2D inColor;
uniform sampler2D inNormal;
uniform sampler2D inDepth;

uniform sampler2D inSky;
uniform sampler2D inSkyE;
uniform float skyStrength;
uniform float skyStrDecay;
uniform float skyStrDecayOff;
uniform float specStr;
uniform float glass;
uniform float ior;
uniform int bgType;
uniform vec4 bgCol;
uniform vec4 fogCol;
uniform bool isOrtho;

out vec4 fragCol;

float length2(vec3 v) {
    return v.x*v.x + v.y*v.y + v.z*v.z;
}

vec3 refract2(vec3 i, vec3 n, float ior) {
    ior = 1 / ior;
    float ni = dot(n, i);
    float k = 1 - ior*ior*(1 - ni*ni);
    if (k < 0) return vec3(0, 0, 0);
    else return ior*i - (ior*ni + sqrt(k))*n;
}

vec4 skyColAt(sampler2D sky, vec3 dir) {
    vec2 refla = normalize(-vec2(dir.x, dir.z));
    float cx = acos(refla.x)/(3.14159 * 2);
    cx = mix(1-cx, cx, sign(refla.y) * 0.5 + 0.5);
    float sy = asin(dir.y)/(3.14159);

    return texture(sky, vec2(cx + 0.125, sy + 0.5));
}

void main () {
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec4 dCol = texture(inColor, uv);
    vec4 nCol = texture(inNormal, uv);
    float z = texture(inDepth, uv).x;
    float nClip = 0.01;
    float fClip = 500.0;

    float zLinear;
    if (isOrtho) zLinear = z;// * fClip;
    else zLinear = (2 * nClip) / (fClip + nClip - z * (fClip - nClip));

    vec4 dc = vec4(uv.x*2-1, uv.y*2-1, z*2-1, 1);
    vec4 wPos = _IP*dc;
    wPos /= wPos.w;
    vec4 wPos2 = _IP*vec4(uv.x*2-1, uv.y*2-1, -1, 1);
    wPos2 /= wPos2.w;
    vec3 fwd = normalize(wPos.xyz - wPos2.xyz);
	
    if (z >= 1) {
        if (bgType == 0)
            fragCol = bgCol;
        else {
            if (bgType == 1)
                fragCol.rgb = skyColAt(inSkyE, fwd).rgb;
            else
                fragCol.rgb = skyColAt(inSky, fwd).rgb;
            fragCol *= skyStrength * bgCol.a;
            fragCol.a = 1;
        }
        return;
    }
    else if (length2(nCol.xyz) == 0) {
        fragCol.rgb = dCol.rgb;
    }
    else {
        vec3 skycol = skyColAt(inSkyE, nCol.xyz).rgb * dCol.rgb;
        vec3 refl = reflect(fwd, nCol.xyz);
        vec3 refr = refract2(fwd, nCol.xyz, ior);
        vec3 skycol2 = skyColAt(inSky, refl.xyz).rgb;
        vec3 skycolr = skyColAt(inSky, refr.xyz).rgb;
        float fres = pow(1-dot(-fwd, nCol.xyz), 5);
        skycol2 = skycol2 * mix(vec3(1, 1, 1), dCol.rgb, specStr);
        skycol = mix(skycol, skycol2, specStr);
        skycol = mix(skycol, skycolr, glass);
        fragCol.rgb = mix(skycol, skycol2, fres) * skyStrength;
    }
    if (fogCol.a > 0) {
        fragCol.rgb = mix(fragCol.rgb, fogCol.rgb, clamp(skyStrDecay * (zLinear - skyStrDecayOff), 0, 1));
        fragCol.a = 1;
    }
    else
        fragCol.a = clamp(1 - skyStrDecay * (zLinear - skyStrDecayOff), 0, 1);
}
)";
}