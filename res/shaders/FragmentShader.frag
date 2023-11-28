#version 330 core
#define MAX_LIGHTS 128

out vec4 FragColor;

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec3 fragmentColor;
in vec2 textureCoordinates;

// materials and viewing
uniform vec3 cameraPosition;
uniform vec3 materialAmbient;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normal0;
uniform vec3 lightPosition;
uniform bool useTexture;

// lighting
uniform int lightCount;
uniform int lightType;
uniform float lightAmbience;
uniform vec4 lightColor;

// point lights
uniform vec3 pointLightAttenuation;

// directional lights
uniform vec3 directionalLightDirection;

// spot lights
uniform vec3 spotLightDirection;
uniform vec3 spotLightAttenuation;
uniform float spotLightInnerCone;
uniform float spotLightOuterCone;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

vec4 pointLight() {
	vec3 lightVector = lightPosition - fragmentPosition;

	// calculates intensity of light with respect to distance
	float dist = length(lightVector);
	float intensity = pointLightAttenuation.x / (pointLightAttenuation.z * dist * dist + pointLightAttenuation.y * dist + pointLightAttenuation.x);

	// diffuse lighting
	vec3 normal = normalize(fragmentNormal); // normalize(texture(normal0, textureCoordinates).xyz * 2.0f - 1.0f);
	vec3 lightDirection = normalize(lightVector);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specular = 0.0f;
	if (diffuse != 0.0f) {
		vec3 viewDirection = normalize(cameraPosition - fragmentPosition);
		vec3 halfwayVector = normalize(viewDirection + lightDirection);
		specular = pow(max(dot(normal, halfwayVector), 0.0f), materialShininess);
	}

	vec4 ambientColor, diffuseColor, specularColor;
	if (useTexture) {
		ambientColor = texture(diffuse0, textureCoordinates) * lightAmbience;
		diffuseColor = texture(diffuse0, textureCoordinates) * diffuse;
		specularColor = texture(specular0, textureCoordinates) * specular;
	} else {
		ambientColor = vec4(materialAmbient * lightAmbience, 1.0);
		diffuseColor = vec4(fragmentColor, 1.0) * diffuse;
		specularColor = vec4(materialSpecular * specular, 1.0);
	}

	return ambientColor + (diffuseColor * lightColor + specularColor) * intensity;
}

vec4 directionalLight() {
	// normalize light color
	float maxChannelColor = max(max(lightColor.r, lightColor.g), lightColor.b);
	float newRed = map(lightColor.r, 0.0f, maxChannelColor, 0.0f, 1.0f);
	float newGreen = map(lightColor.g, 0.0f, maxChannelColor, 0.0f, 1.0f);
	float newBlue = map(lightColor.b, 0.0f, maxChannelColor, 0.0f, 1.0f);
	float newAlpha = clamp(lightColor.a, 0.0f, 1.0f);
	vec4 newLightColor = vec4(newRed, newGreen, newBlue, newAlpha);

	// diffuse lighting
	vec3 normal = normalize(fragmentNormal);
	float diffuse = max(dot(normal, -directionalLightDirection), 0.0f);

	// specular lighting
	float specular = 0.0f;
	if (diffuse != 0.0f) {
		vec3 viewDirection = normalize(cameraPosition - fragmentPosition);
		vec3 halfwayVector = normalize(viewDirection - directionalLightDirection);
		specular = pow(max(dot(normal, halfwayVector), 0.0f), materialShininess);
	}

	vec4 ambientColor, diffuseColor, specularColor;
	if (useTexture) {
		ambientColor = texture(diffuse0, textureCoordinates) * lightAmbience;
		diffuseColor = texture(diffuse0, textureCoordinates) * diffuse;
		specularColor = texture(specular0, textureCoordinates) * specular;
	} else {
		ambientColor = vec4(materialAmbient * lightAmbience, 1.0);
		diffuseColor = vec4(fragmentColor, 1.0) * diffuse;
		specularColor = vec4(materialSpecular * specular, 1.0);
	}

	return ambientColor + diffuseColor * newLightColor + specularColor;
}

vec4 spotLight() {
	// normalize light color
	float maxChannelColor = max(max(lightColor.r, lightColor.g), lightColor.b);
	float newRed = map(lightColor.r, 0.0f, maxChannelColor, 0.0f, 1.0f);
	float newGreen = map(lightColor.g, 0.0f, maxChannelColor, 0.0f, 1.0f);
	float newBlue = map(lightColor.b, 0.0f, maxChannelColor, 0.0f, 1.0f);
	float newAlpha = clamp(lightColor.a, 0.0f, 1.0f);
	vec4 newLightColor = vec4(newRed, newGreen, newBlue, newAlpha);

	vec3 lightVector = lightPosition - fragmentPosition;

	// calculates intensity of light with respect to distance
	float dist = length(lightVector);
	float attenuationIntensity = spotLightAttenuation.x / (spotLightAttenuation.z * dist * dist + spotLightAttenuation.y * dist + spotLightAttenuation.x);

	// diffuse lighting
	vec3 normal = normalize(fragmentNormal); // normalize(texture(normal0, textureCoordinates).xyz * 2.0f - 1.0f);
	vec3 lightDirection = normalize(lightVector);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specular = 0.0f;
	if (diffuse != 0.0f) {
		vec3 viewDirection = normalize(cameraPosition - fragmentPosition);
		vec3 halfwayVector = normalize(viewDirection + lightDirection);
		specular = pow(max(dot(normal, halfwayVector), 0.0f), materialShininess);
	}

	// calculates intensity of the current position based on its angle to the center of the light cone
	float angle = dot(spotLightDirection, -lightDirection);
	float coneIntensity = clamp((cos(angle) - cos(spotLightOuterCone)) / (cos(spotLightInnerCone) - cos(spotLightOuterCone)), 0.0f, 1.0f);

	// combines the intensity of the light with the intensity of the current position
	float intensity = attenuationIntensity * (1.0 - attenuationIntensity * attenuationIntensity) + coneIntensity * (attenuationIntensity * attenuationIntensity);

	vec4 ambientColor, diffuseColor, specularColor;
	if (useTexture) {
		ambientColor = texture(diffuse0, textureCoordinates) * lightAmbience;
		diffuseColor = texture(diffuse0, textureCoordinates) * diffuse;
		specularColor = texture(specular0, textureCoordinates) * specular;
	} else {
		ambientColor = vec4(materialAmbient * lightAmbience, 1.0);
		diffuseColor = vec4(fragmentColor, 1.0) * diffuse;
		specularColor = vec4(materialSpecular, 1.0) * specular;
	}

	return ambientColor + (diffuseColor * newLightColor + specularColor) * intensity;
}

void main() {
	FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	switch (lightType) {
		case 1:
			FragColor += pointLight();
			break;
		case 2:
			FragColor += directionalLight();
			break;
		case 3:
			FragColor += spotLight();
			break;
		default:
			if (useTexture) {
				FragColor += texture(diffuse0, textureCoordinates);
			} else {
				FragColor += vec4(fragmentColor, 1.0);
			}
			break;
	}
}