struct DirLight {
	vec3 dir;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
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

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 _ambient, vec3 _diffuse, vec3 _specular,float _shiness) {
	vec3 lightDir = normalize(-light.dir);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), _shiness);

	vec3 ambient = light.ambient  * _ambient;
	vec3 diffuse= light.diffuse  * diff * _diffuse;
	vec3 specular = light.specular * spec * _specular;

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 _ambient, vec3 _diffuse, vec3 _specular, float _shiness) {
	float distance = length(light.pos - fragPos);
	float att = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));  //att計算


	vec3 lightDir = normalize(light.pos - fragPos);

	vec3 ambient = light.ambient * _ambient;

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * _diffuse;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), _shiness);
	vec3 specular = light.specular * spec * _specular;

	ambient *= att;
	diffuse *= att;
	specular *= att;
	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 _ambient, vec3 _diffuse, vec3 _specular, float _shiness) {
	vec3 lightDir = normalize(light.pos - fragPos);//正規化計算光的向量

	float thetaAngle = dot(lightDir, normalize(-light.dir));
	float episilonAngle = (light.cutOff - light.outerCutOff);
	float intensity = clamp((thetaAngle - light.outerCutOff) / episilonAngle, 0.0, 1.0);//聚光燈的邊緣柔和效果


	//att
	float distance = length(light.pos - fragPos);	//距離
	float att = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    //距離衰減常數

	vec3 ambient = light.ambient*_ambient;

	float diff = max(dot(normal, lightDir), 0.0);//內積 如果小於0代表背對光源
	vec3 diffuse = light.diffuse*diff*_diffuse;

	vec3 reflectDir = reflect(-lightDir, normal);  //計算反射方向
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), _shiness);//shiness:反射參數
	vec3 specular = light.specular*spec*_specular;

	ambient *= intensity;
	diffuse *= intensity;
	specular *= intensity;

	ambient *= att;
	diffuse *= att;
	specular *= att;
	return  (ambient + diffuse + specular);
}
