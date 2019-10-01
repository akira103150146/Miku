#version 330

uniform struct Matrices
{
	mat4 projMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;                                                                           
	mat4 normalMatrix;
} matrices;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inCoord;
layout (location = 4) in ivec4 boneIndices;
layout (location = 5) in vec4 boneWeights;

const int MAX_BONES = 200;

uniform mat4 gBones[MAX_BONES];

out VS_VERTEX {
	vec3 normal;
	vec2 texCoord;
} vertex_out;

void main()
{
  vertex_out.texCoord = inCoord;

  mat4 boneTransform = gBones[boneIndices[0]] * boneWeights[0]
  + gBones[boneIndices[1]] * boneWeights[1]
  + gBones[boneIndices[2]] * boneWeights[2]
  + gBones[boneIndices[3]] * boneWeights[3];

  gl_Position = boneTransform * vec4(inPosition, 1.0);

  vertex_out.normal = inNormal;
}