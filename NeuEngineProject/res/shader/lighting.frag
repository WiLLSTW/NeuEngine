#version 450

struct DirLight{
	vec3 dir;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight{
	vec3 pos;
	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
    vec3 pos;
    vec3 dir;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};



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
	PointLight pointLight[4];
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


	outColor=vec4(1.0);
}