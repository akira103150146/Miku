#version 330

out vec4 outputColor;

uniform sampler2D furSampler;
uniform vec4 furColor = vec4(0.8, 0.8, 0.9, 1.0);

in GS_VERTEX {
	vec3 normal;
	vec2 texCoord;
	flat float furStrength;
} fragment_in;

void main()
{
	vec4 texColor = texture(furSampler, fragment_in.texCoord);
	float t = texColor.a;

	t *= fragment_in.furStrength;

	outputColor = furColor * vec4(1.0, 1.0, 1.0, t);
}