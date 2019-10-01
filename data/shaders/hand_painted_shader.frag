#version 330

smooth in vec2 vTexCoord;
smooth in vec3 vNormal;
smooth in vec4 vColor;
smooth in vec4 vEyeSpacePos;
smooth in vec3 vWorldPos;
out vec4 outputColor;

uniform sampler2D gSampler;
uniform vec3 ambient;
uniform vec3 diffuse;

#include "dirLight.frag"

uniform DirectionalLight sunLight;
uniform int nbins = 8;
uniform int img_size = 1024;

uniform float sigma_e = 1.3;
uniform float sigma_r = 3.5;
uniform float phi = 3.4;
uniform float tau = 0.99;

void main()
{
	vec3 vNormalized = normalize(vNormal);
	
	vec4 vTexColor = texture2D(gSampler, vTexCoord);

	float twoSigmaESquared = 2.0 * sigma_e * sigma_e;
	float twoSigmaRSquared = 2.0 * sigma_r * sigma_r;

	int halfWidth = int(ceil(2.0 * sigma_r));
	vec2 sum = vec2(0.0);
	vec2 norm = vec2(0.0);
	for(int i = -halfWidth ; i <= halfWidth ; i++){
		for(int j = -halfWidth ; j <= halfWidth ; j++){
			float d = length(vec2(i, j));
			vec2 kernel = vec2(exp(-d * d / twoSigmaESquared), exp(-d * d / twoSigmaRSquared));
			vec4 c = texture2D(gSampler, vTexCoord + vec2(i, j) / float(img_size));
			vec2 l = vec2(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
			norm += 2.0 * kernel;
			sum += kernel * l;
		}
	}
	sum /= norm;
	float h = 100.0 * (sum.x - tau * sum.y);
	float edge = (h > 0.0) ? 1.0 : 2.0 * smoothstep(-2.0, 2.0, phi * h);
	vec3 edge_color = vec3(0.0);

	vec4 color = vec4(0);
	int n = 0;
	for(int i = -halfWidth ; i <= halfWidth ; i++){
		for(int j = -halfWidth ; j <= halfWidth ; j++){
			vec4 c = texture2D(gSampler, vTexCoord + vec2(i, j) / float(img_size));
			color += c;
			n++;
		}
	}
	color /= n;
	float r = floor(color.r * float(nbins)) / float(nbins);
	float g = floor(color.g * float(nbins)) / float(nbins);
	float b = floor(color.b * float(nbins)) / float(nbins);
	color = vec4(r, g, b, color.a);
    vec4 vDirLightColor = GetDirectionalLightColor(sunLight, vNormalized);

	vec3 img_color = color.xyz;
	outputColor = vec4(mix(edge_color, img_color, edge), color.a);
}