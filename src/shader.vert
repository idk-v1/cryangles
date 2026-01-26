#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

out vec4 outColor;

uniform mat4 projMat;
uniform mat4 viewMat;
uniform vec3 camPos;

void main()
{
	gl_Position = projMat * viewMat * vec4(inPos - camPos, 1.0);

	if (camPos.y < 125.0)
	{
		float depthEffect = (1.0 - (camPos.y / 125.0)) * 0.8 + 0.2;
		vec3 waterColor = vec3(0.0, 0.0, 0.4);
	    outColor = vec4((inColor * (1.0 - depthEffect) + waterColor * depthEffect) * 0.5, 1.0);
	}
	else 
		outColor = vec4(inColor, 1.0);
}