#version 450

layout (binding=0)uniform UniformBufferObjectProj{
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
}uboProjView;


layout (push_constant) uniform PushConstantModel{
	mat4 model;
	uint selector;
}pcModel;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location=1) out vec2 fragTexCoord;
layout(location=2) out vec3 fragPosition;

void main(){
	
	gl_Position = uboProjView.proj * uboProjView.view * pcModel.model * vec4(inPosition, 1.0);
	//gl_Position=vec4(inPosition,0.0,1.0);
	fragNormal=mat3(transpose(inverse(pcModel.model)))*inNormal;
	fragNormal=normalize(fragNormal);//ñ@ê¸å¸ó ê≥ãKâª
	fragTexCoord=inTexCoord;
	fragPosition=vec3(pcModel.model*vec4(inPosition,1.0));
}