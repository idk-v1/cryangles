#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/util/darkTheme.h>
#include <GLFW/util/sleep.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "gl_helper.h"

void resize(GLFWwindow* window, int width, int height)
{
	glh_setView(width, height);
}


void playerInput(GLFWwindow* window, Object* camera, float moveSpeed, float lookSpeed, bool* paused)
{
	static bool escLast = false;
	bool escape = glfwGetKey(window, GLFW_KEY_ESCAPE);
	if (escape && !escLast)
	{
		*paused = !*paused;
		if (*paused) // release cursor
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPos(window, glh_width / 2.f, glh_height / 2.f);
		}
		else // trap cursor, reset time
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(window, glh_width / 2.f, glh_height / 2.f);
		}
	}
	escLast = escape;

	camera->vel.y -= moveSpeed * 2.f;

	if (*paused)
		return;

	float forward = 0.f;
	float side = 0.f;
	float up = 0.f;

	if (glfwGetKey(window, GLFW_KEY_W))
		--forward;
	if (glfwGetKey(window, GLFW_KEY_S))
		++forward;
	if (glfwGetKey(window, GLFW_KEY_A))
		--side;
	if (glfwGetKey(window, GLFW_KEY_D))
		++side;
	if (glfwGetKey(window, GLFW_KEY_SPACE))
		++up;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		--up;

	float moveDist = fastSqrt(forward * forward + side * side);
	float moveAngle = fastAtan2(side, forward);
	forward = cosf(moveAngle) * moveDist;
	side = sinf(moveAngle) * moveDist;

	camera->vel.z += (
		cosf(toRad(camera->trans.rot.y)) * forward - 
		sinf(toRad(camera->trans.rot.y)) * side
		) * moveSpeed;

	camera->vel.x += (
		sinf(toRad(camera->trans.rot.y)) * forward +
		cosf(toRad(camera->trans.rot.y)) * side
		) * moveSpeed;

	camera->vel.y += up * moveSpeed * 3.f;


	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	camera->trans.rot.y -= (float)(mouseX - glh_width / 2.f) * lookSpeed;
	camera->trans.rot.x += (float)(glh_height / 2.f - mouseY) * lookSpeed;
	glfwSetCursorPos(window, glh_width / 2.f, glh_height / 2.f);

	if (camera->trans.rot.x > 90.f)
		camera->trans.rot.x = 90.f;
	if (camera->trans.rot.x < -90.f)
		camera->trans.rot.x = -90.f;
	camera->trans.rot.y = gls_wrapDeg(camera->trans.rot.y);
	camera->trans.rot.z = gls_wrapDeg(camera->trans.rot.z);
}

void setTitle(GLFWwindow* window, char* fmt, ...)
{
	char titleBuf[100];

	va_list args;
	va_start(args, fmt);
	vsnprintf(titleBuf, 100, fmt, args);
	va_end(args);

	glfwSetWindowTitle(window, titleBuf);
}

float fract(float x)
{
	float i = floorf(x);
	return x - i;
}

float random(float x, float z)
{
	return fract(sinf(x * 12.9898f + z * 78.233f) * 43758.5453123f);
}

float mix(float x, float y, float a)
{
	return x * (1.f - a) + y * a;
}

float noise(float x, float z)
{
	float ix = floorf(x);
	float iz = floorf(z);
	float fx = fract(x);
	float fz = fract(z);
	
	// Four corners in 2D of a tile
	float a = random(ix, iz);
	float b = random(ix + 1.f, iz);
	float c = random(ix, iz + 1.f);
	float d = random(ix + 1.f, iz + 1.f);

	float ux = fx * fx * (3.f - 2.f * fx);
	float uz = fz * fz * (3.f - 2.f * fz);

	return mix(a, b, ux) +
		(c - a) * uz * (1.f - ux) +
		(d - b) * ux * uz;
}

float fbm(float x, float z)
{
	float scale = 0.0005f;
	x *= scale;
	z *= scale;

	float dist = 0.f;
	//dist = powf(fastSqrt(powf(x, 2.f) + powf(z, 2.f)) * 0.1f, 1.05f);

	// Initial values
	float value = 0;
	float amplitude = 0.5f;
	float frequency = 0.f;
	int octaves = 6;
	//
	// Loop of octaves
	for (int i = 0; i < octaves; i++)
	{
		value += amplitude * noise(x, z);
		x *= 2.f;
		z *= 2.f;
		amplitude *= 0.5f;
	}

	return max(value - dist, 0.f);
}

float noiseMod(float height)
{
	if (height > 0.7f)
		height = 0.6f + log10f(1.f + (height - 0.6f) * 0.75f) * 10.f + (height - 0.7f) * 1.5f;
	else if (height > 0.6f)
		height = 0.6f + log10f(1.f + (height - 0.6f) * 0.75f) * 10.f;
	else if (height < 0.25f)
		height = 0.25f - powf(0.25f - height, 0.9f);

	return height * 10.f;
}

RGB colorFromHeight(float height)
{
	RGB rgb = { 0 };
	if (height >= 0.75f)
	{
		rgb.r = (height - 0.65f) * 2.f + 0.15f;
		rgb.g = (height - 0.65f) * 2.f + 0.15f;
		rgb.b = (height - 0.65f) * 2.f + 0.15f;
	}
	else if (height >= 0.65f)
	{
		rgb.r = (height - 0.65f + 0.2f);
		rgb.g = (height - 0.65f + 0.2f);
		rgb.b = (height - 0.65f + 0.2f);
	}
	else if (height >= 0.5f)
	{
		rgb.r = (height - 0.5f) * 1.333f;
		rgb.g = (height - 0.5f) * -2.65f + 0.6f;
		rgb.b = (height - 0.5f) * 1.333f;
	}
	else if (height >= 0.35f)
	{
		rgb.r = 0.f;
		rgb.g = 1.f - ((height - 0.35f) * 2.f);
		rgb.b = 0.f;
	}
	else if (height >= 0.3f)
	{
		rgb.r = 1.f - ((height - 0.3f) * 2.f);
		rgb.g = 1.f - ((height - 0.3f) * 2.f);
		rgb.b = 0.25f;
	}
	else if (height <= 0.3f)
	{
		rgb.r = 0.f;
		rgb.g = 0.f;
		rgb.b = height * 2.f;
	}

	return rgb;
}

Model generateWorld(float x, float z, int lod)
{
	Model model = { 0 };

	float step = 5.f * powf(1.75f, lod);
	float size = 1000.f;

	float xx = x * size * 2.f;
	float zz = z * size * 2.f;

	// I do not care how bad this is
	for (float z = -size; z <= size + step; z += step)
		for (float x = -size; x <= size + step; x += step)
			model.count += 6;

	Vertex* verts = malloc(sizeof(Vertex) * model.count);
	if (!verts)
	{
		model.count = 0;
		return model;
	}

	float scale = 0.01f;

	size_t pos = 0;
	for (float z = -size; z <= size + step; z += step)
		for (float x = -size; x <= size + step; x += step)
		{
			float y00 = fbm(xx + x - step, zz + z - step);
			float y01 = fbm(xx + x - step, zz + z       );
			float y10 = fbm(xx + x       , zz + z - step);
			float y11 = fbm(xx + x       , zz + z       );

			verts[pos + 0].pos = vec3f(scale * (xx + x - step), noiseMod(y01), scale * (zz + z       ));
			verts[pos + 1].pos = vec3f(scale * (xx + x       ), noiseMod(y11), scale * (zz + z       ));
			verts[pos + 2].pos = vec3f(scale * (xx + x - step), noiseMod(y00), scale * (zz + z - step));

			verts[pos + 3].pos = vec3f(scale * (xx + x - step), noiseMod(y00), scale * (zz + z - step));
			verts[pos + 4].pos = vec3f(scale * (xx + x       ), noiseMod(y11), scale * (zz + z       ));
			verts[pos + 5].pos = vec3f(scale * (xx + x       ), noiseMod(y10), scale * (zz + z - step));

			verts[pos + 0].rgb = colorFromHeight(y01);
			verts[pos + 1].rgb = colorFromHeight(y11);
			verts[pos + 2].rgb = colorFromHeight(y00);

			verts[pos + 3].rgb = colorFromHeight(y00);
			verts[pos + 4].rgb = colorFromHeight(y11);
			verts[pos + 5].rgb = colorFromHeight(y10);

			pos += 6;
		}

	model = glh_loadModel(verts, model.count);
	free(verts);

	return model;
}


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1000, 600, "test", NULL, NULL);
	glfwSetWindowTheme(window, glfwGetSystemTheme());
	glfwSetWindowSizeCallback(window, resize);
	
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((void*)glfwGetProcAddress);
	glfwSwapInterval(0);

	glh_setView(1000, 600);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	//glEnable(GL_POLYGON_OFFSET_LINE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	double timeNow = glfwGetTime();
	double timeLast = glfwGetTime();
	double deltaTime = 0.0;
	double updateTime = 1.0 / 60.0;

	unsigned fps = 0;
	unsigned fpsCount = 0;
	double deltaFPS = 0.0;
	double timeFPSLast = glfwGetTime();
	
	GLuint shader = glh_loadShader("src/shader.vert", "src/shader.frag");

	int worldSize = 21;
	Model* world = malloc(sizeof(Model) * worldSize * worldSize);
	for (int x = 0; x < worldSize; x++)
		for (int z = 0; z < worldSize; z++)
		{
			int lod = max(abs(x - worldSize / 2), abs(z - worldSize / 2));
			world[x + z * worldSize] = generateWorld(x - worldSize / 2, z - worldSize / 2, lod);
		}

	Object camera = { 0 };
	float moveSpeed = 0.0075f;
	float lookSpeed = 0.1f;

	bool paused = false;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, glh_width / 2.f, glh_height / 2.f);

	while (!glfwWindowShouldClose(window))
	{
		timeNow = glfwGetTime();
		deltaTime += timeNow - timeLast;
		while (deltaTime >= updateTime)
		{
			deltaTime -= updateTime;

			playerInput(window, &camera, moveSpeed, lookSpeed, &paused);

			camera.trans.pos.x += camera.vel.x;
			camera.trans.pos.y += camera.vel.y;
			camera.trans.pos.z += camera.vel.z;

			camera.vel.x *= 0.9f;
			camera.vel.y *= 0.9f;
			camera.vel.z *= 0.9f;

			float ground = noiseMod(fbm(camera.trans.pos.x * 100.f, camera.trans.pos.z * 100.f));

			if (camera.trans.pos.y < ground)
			{
				camera.vel.y = 0.f;
				camera.trans.pos.y = ground;
			}
		}
		timeLast = timeNow;
		glh_updateCamera(shader, &camera);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader);

		for (int x = 0; x < worldSize; x++)
			for (int z = 0; z < worldSize; z++)
				gls_drawModel(world[x + z * worldSize]);

		glfwSwapBuffers(window);

		setTitle(window, "FPS:%4u | #Tri: %llu | Pos(%.2f, %.2f, %.2f)", fps,
			triCount / 3,
			camera.trans.pos.x, camera.trans.pos.y, camera.trans.pos.z);

		double timeFPSNow = glfwGetTime();
		deltaFPS += timeFPSNow - timeFPSLast;
		timeFPSLast = timeFPSNow;
		if (deltaFPS >= 1.0)
		{
			deltaFPS = 0.0;
			fps = fpsCount;
			fpsCount = 0;
		}
		++fpsCount;

		glfwPollEvents();
	}

	for (int x = 0; x < worldSize; x++)
		for (int z = 0; z < worldSize; z++)
			glh_deleteModel(world[x + z * worldSize]);

	glDeleteProgram(shader);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}