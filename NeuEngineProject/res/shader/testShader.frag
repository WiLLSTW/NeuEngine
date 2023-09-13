#version 450
#define POINTLIGHTCOUNT 4
#include "lightCalc.h"

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexcoord;
layout(location = 2) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

layout (binding=0)uniform UniformBufferObjectProj{
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
}uboProjView;

layout(binding = 1)uniform sampler2D texSampler[8];

layout(binding = 2) uniform UniformBufferObjectLight{
	DirLight dirLight;
	PointLight pointLight[POINTLIGHTCOUNT];
	SpotLight spotLight;
}uboLight;

layout (binding=3) uniform UniformBufferMaterial{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shiness;
}uboMaterial;

layout (push_constant) uniform PushConstantSelector{
	mat4 model;
	uint selector;
}selector; 

void main() {
	vec3 viewDir=normalize(uboProjView.cameraPos-fragPosition);
	vec3 result=CalcDirLight(uboLight.dirLight,fragNormal,viewDir,uboMaterial.ambient,uboMaterial.diffuse,uboMaterial.specular,uboMaterial.shiness);

	for(int i=0;i<4;i++)
		result+=CalcPointLight(uboLight.pointLight[i],fragNormal,fragPosition,viewDir,uboMaterial.ambient,uboMaterial.diffuse,uboMaterial.specular,uboMaterial.shiness);

	result+=CalcSpotLight(uboLight.spotLight,fragNormal,fragPosition,viewDir,uboMaterial.ambient,uboMaterial.diffuse,uboMaterial.specular,uboMaterial.shiness);

	vec4 textureColor=texture(texSampler[selector.selector],fragTexcoord);
	result=result*textureColor.rgb;

	float dist=length(fragPosition-uboProjView.cameraPos);
	outColor=vec4(result.rgb,textureColor.a);
	

}