#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexcoord;
layout(location = 2) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

layout (binding=0)uniform UniformBufferObjectProj{
	mat4 view;
	mat4 proj;
}uboProjView;

layout(binding = 1)uniform sampler2D texSampler[8];
layout (binding=2) uniform UniformBufferPhongLighting{
	vec3 viewPos;
	vec3 pos;
	vec3 color;
	uint shiness;
	float ambientStrength;
	float specularStrength;
}uboLight;
layout (push_constant) uniform PushConstantSelector{
	mat4 model;
	uint selector;
}selector; 

void main() {
	vec3 norm = normalize(fragNormal);//法線向量

	vec3 ambient = uboLight.ambientStrength * uboLight.color;

	vec3 lightDir = normalize(uboLight.pos - fragPosition);//正規化計算光的向量
	float diff = max(dot(norm, lightDir), 0.0);//內積 如果小於0代表背對光源
	vec3 diffuse = diff * uboLight.color;


	vec3 viewDir = normalize(uboLight.viewPos - fragPosition);
	vec3 reflectDir = reflect(-lightDir, norm);  //計算反射方向
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), uboLight.shiness);//shiness:反射參數
	vec3 specular = uboLight.specularStrength * spec * uboLight.color;

    vec4 textureColor=texture(texSampler[selector.selector],fragTexcoord);
	vec3 result=(ambient+diffuse+specular)*textureColor.rgb;

	outColor=vec4(result.rgb,textureColor.a);
}