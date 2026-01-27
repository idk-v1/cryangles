#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

out vec4 outColor;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform vec3 camPos;
uniform float viewDist;

void main()
{
	gl_Position = projMat * viewMat * vec4(inPos - camPos, 1.0);

	float camDist = length(inPos.xz - camPos.xz);

	vec3 color = inColor;

	float seaLevel = 125.0;
	if (camPos.y < seaLevel)
	{
		float depthEffect = clamp((1.0 - (camPos.y / seaLevel)) * 0.8 + 0.2, 0.0, 1.0);
		vec3 waterColor = vec3(0.0, 0.0, 0.4);
		color.r = mix(waterColor.r, inColor.r, depthEffect);
		color.g = mix(waterColor.g, inColor.g, depthEffect);
		color.b = mix(waterColor.b, inColor.b, depthEffect);
	}

	float a = 1.0;
	float viewDistMod = viewDist - 100.0;
	if (camDist > viewDistMod)
		a = 1.0 - clamp((camDist - viewDistMod) * 0.01, 0.0, 1.0);

	outColor = vec4(color, a);
}