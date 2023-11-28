#version 330 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec3 vertexBitangent;
layout (location = 4) in vec3 vertexColor;
layout (location = 5) in vec2 vertexUV;

uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

out DATA {
	vec3 normal;
	vec3 color;
	vec2 textureCoordinates;
	mat4 projection;
} dataOut;

void main() {
	gl_Position = modelMatrix * vec4(vertexPosition, 1.0f);
	dataOut.normal = vertexNormal;
	dataOut.color = vertexColor;
	dataOut.textureCoordinates = vertexUV;
	dataOut.projection = cameraMatrix;
}