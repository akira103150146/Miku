#version 330

uniform struct Matrices
{
	mat4 projMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;                                                                           
	mat4 normalMatrix;
} matrices;

layout(triangles) in;
layout(triangle_strip, max_vertices = 30) out;

uniform int furLayers = 15;
uniform float furDepth = 5.0;

in VS_VERTEX {
	vec3 normal;
	vec2 texCoord;
} vertex_in[];

out GS_VERTEX {
	vec3 normal;
	vec2 texCoord;
	flat float furStrength;
} vertex_out;

void main(){
	int i, layer;
	float dispDelta = 1.0 / float(furLayers);
	float d = 0.0;

	mat4 mMVP = matrices.projMatrix*matrices.viewMatrix*matrices.modelMatrix;

	for(layer = 0 ; layer < furLayers ; layer++){
		for(i = 0 ; i < gl_in.length() ; i++){
			vec3 n = vertex_in[i].normal;
			vertex_out.normal = n;
			vertex_out.texCoord = vertex_in[i].texCoord;
			vertex_out.furStrength = 1.0 - d;
			vec4 position = gl_in[i].gl_Position + vec4(n * d * furDepth, 0.0);
			gl_Position = mMVP * position;
			EmitVertex();
		}
		d += dispDelta;
		EndPrimitive();
	}
}