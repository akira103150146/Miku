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

smooth out vec4 vColor;
smooth out vec3 vNormal;
smooth out vec2 vTexCoord;
smooth out vec3 vWorldPos;

smooth out vec4 vEyeSpacePos;

void main()
{
  mat4 mMV = matrices.viewMatrix*matrices.modelMatrix;  
  mat4 mMVP = matrices.projMatrix*matrices.viewMatrix*matrices.modelMatrix;
  
  vColor = inColor;
  vTexCoord = inCoord;

  mat4 boneTransform = gBones[boneIndices[0]] * boneWeights[0]
  + gBones[boneIndices[1]] * boneWeights[1]
  + gBones[boneIndices[2]] * boneWeights[2]
  + gBones[boneIndices[3]] * boneWeights[3];

  vEyeSpacePos = mMV * boneTransform * vec4(inPosition, 1.0);
  gl_Position = mMVP * boneTransform * vec4(inPosition, 1.0);

  vNormal = (matrices.normalMatrix * transpose(inverse(boneTransform)) * vec4(inNormal, 1.0)).xyz;
  vWorldPos = (matrices.modelMatrix * boneTransform * vec4(inPosition, 1.0)).xyz;
}