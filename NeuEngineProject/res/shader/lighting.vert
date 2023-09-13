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
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location=1) out vec2 fragTexCoord;
layout(location=2) out vec3 fragPosition;

void main(){
	
	gl_Position = uboProjView.proj * uboProjView.view * pcModel.model * vec4(inPosition, 1.0);
	fragColor=inColor;
}
