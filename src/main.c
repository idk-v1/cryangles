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

#include "perlin.h"

void resize(GLFWwindow* window, int width, int height)
{
	glh_setView(width, height);
}


float noiseMod(float height)
{
	return height * 250.f;
}

float noise(float x, float z)
{
	float perlin = Perlin_Get2d(x, z, 0.0001f, 5);
	float pow = powf(perlin * 2.f - 1.f, 3.f) / 2.f + 0.5f;
	if (perlin <= 0.5f)
	{
		return lerp(pow, perlin, clampf(0.f, 0.75f, 1.f));
	}
	else
	{
		return pow;
	}
}


void playerInput(GLFWwindow* window, Object* camera, float moveSpeed, 
	float sprintSpeed, float lookSpeed, float jumpHeight, float gravity, bool* paused)
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

	static bool lastOnGround = false;
	float ground = noiseMod(noise(camera->trans.pos.x * 100.f, camera->trans.pos.z * 100.f));
	bool onGround = (camera->trans.pos.y == ground);

	if (!*paused)
	{
		float forward = 0.f;
		float side = 0.f;
		float up = 0.f;

		bool sprint = false;

		if (glfwGetKey(window, GLFW_KEY_W))
		{
			--forward;
			sprint = true;
		}
		if (glfwGetKey(window, GLFW_KEY_S))
			++forward;
		if (glfwGetKey(window, GLFW_KEY_A))
			--side;
		if (glfwGetKey(window, GLFW_KEY_D))
			++side;
		if (glfwGetKey(window, GLFW_KEY_SPACE))
		{
			if (onGround || lastOnGround || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
				++up;
		}
		sprint = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) && sprint);

		float moveDist = fastSqrt(forward * forward + side * side);
		float moveAngle = fastAtan2(side, forward);
		forward = cosf(moveAngle) * moveDist;
		side = sinf(moveAngle) * moveDist;

		camera->vel.z += (
			cosf(toRad(camera->trans.rot.y)) * forward -
			sinf(toRad(camera->trans.rot.y)) * side
			) * (sprint ? sprintSpeed : moveSpeed);

		camera->vel.x += (
			sinf(toRad(camera->trans.rot.y)) * forward +
			cosf(toRad(camera->trans.rot.y)) * side
			) * (sprint ? sprintSpeed : moveSpeed);

		camera->vel.y += up * jumpHeight;


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

	camera->vel.y += gravity;

	camera->trans.pos.x += camera->vel.x;
	camera->trans.pos.y += camera->vel.y;
	camera->trans.pos.z += camera->vel.z;

	camera->vel.x *= onGround ? 0.9f : 0.95f;
	camera->vel.y *= 0.95f;
	camera->vel.z *= onGround ? 0.9f : 0.95f;

	ground = noiseMod(noise(camera->trans.pos.x * 100.f, camera->trans.pos.z * 100.f));
	if (camera->trans.pos.y < ground)
	{
		camera->vel.y = 0.f;
		camera->trans.pos.y = ground;
	}

	lastOnGround = onGround;
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


// 0.2 is min ocean
// 0.4 is avg min ocean
// 0.5 is max ocean
RGB colorFromHeight(float height)
{
	RGB color = { 0 };

	const RGB deepColor = {0.f, 0.f, 0.05f};
	const RGB waterColor = {0.f, 0.f, 0.25f};
	const RGB sandColor = {0.5f, 0.5f, 0.35f};
	const RGB grassColor = {0.35f, 0.5f, 0.15f};
	const RGB coldColor = {0.f, 0.25f, 0.15f};
	const RGB stoneColor = {0.25f, 0.25f, 0.25f};
	const RGB snowColor = {0.7f, 0.7f, 0.8f};

	const float deepHeight = 0.45f;
	const float waterHeight = 0.499f;
	const float sandHeight = 0.5f;
	const float grassHeight = 0.501;
	const float coldHeight = 0.52f;
	const float stoneHeight = 0.58f;
	const float snowHeight = 0.64f;

	if (height <= waterHeight)
	{
		float value = (height - deepHeight) / (waterHeight - deepHeight);
		color = lerpRGB(waterColor, deepColor, value);
	}
	else if (height <= sandHeight)
	{
		float value = (height - waterHeight) / (sandHeight - waterHeight);
		color = lerpRGB(sandColor, waterColor, value);
	}
	else if (height <= grassHeight)
	{
		float value = (height - sandHeight) / (grassHeight - sandHeight);
		color = lerpRGB(grassColor, sandColor, value);
	}
	else if (height <= coldHeight)
	{
		float value = (height - grassHeight) / (coldHeight - grassHeight);
		color = lerpRGB(coldColor, grassColor, value);
	}
	else if (height <= stoneHeight)
	{
		float value = (height - coldHeight) / (stoneHeight - coldHeight);
		color = lerpRGB(stoneColor, coldColor, value);
	}
	else if (height <= snowHeight)
	{
		float value = (height - stoneHeight) / (snowHeight - stoneHeight);
		color = lerpRGB(snowColor, stoneColor, value);
	}
	else
	{
		color = snowColor;
	}

	return color;
}

Model generateWorld(float x, float z, int lod)
{
	Model model = { 0 };

	lod = min(lod, 8);

	float step = 5.f * powf(1.75f, lod);
	float size = 1000.f;

	float xx = x * size * 2.f;
	float zz = z * size * 2.f;
	float scale = 0.01f;
	//printf("Gen Chunk (%f, %f)\n", scale * xx, scale * zz);

	// I do not care how bad this is
	for (float z = -size; z < size; z += step)
		for (float x = -size; x < size; x += step)
			model.count += 6;

	Vertex* verts = malloc(sizeof(Vertex) * model.count);
	if (!verts)
	{
		model.count = 0;
		return model;
	}

	size_t pos = 0;
	for (float z = -size; z < size; z += step)
	{		
		float y00;
		float y01;
		float y10;
		float y11;

		float oldZ = z - step;
		if (z + step >= size)
			z = size;

		y00 = 0.f;
		y01 = 0.f;
		y10 = noise(xx - size - step, zz + oldZ);
		y11 = noise(xx - size - step, zz + z);

		for (float x = -size; x < size; x += step)
		{
			float oldX = x - step;
			if (x + step >= size)
				x = size;

			y00 = y10;
			y01 = y11;
			y10 = noise(xx + x, zz + oldZ);
			y11 = noise(xx + x, zz + z);

			verts[pos + 0].pos = vec3f(scale * (xx + oldX), noiseMod(y01), scale * (zz + z));
			verts[pos + 0].rgb = colorFromHeight(y01);
			verts[pos + 1].pos = vec3f(scale * (xx + x), noiseMod(y11), scale * (zz + z));
			verts[pos + 1].rgb = colorFromHeight(y11);
			verts[pos + 2].pos = vec3f(scale * (xx + oldX), noiseMod(y00), scale * (zz + oldZ));
			verts[pos + 2].rgb = colorFromHeight(y00);

			verts[pos + 3].pos = vec3f(scale * (xx + oldX), noiseMod(y00), scale * (zz + oldZ));
			verts[pos + 3].rgb = colorFromHeight(y00);
			verts[pos + 4].pos = vec3f(scale * (xx + x), noiseMod(y11), scale * (zz + z));
			verts[pos + 4].rgb = colorFromHeight(y11);
			verts[pos + 5].pos = vec3f(scale * (xx + x), noiseMod(y10), scale * (zz + oldZ));
			verts[pos + 5].rgb = colorFromHeight(y10);

			pos += 6;
		}
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

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

	float viewDist = 750.f;

	int worldSize = 125;
	Model* world = malloc(sizeof(Model) * worldSize * worldSize);
	for (int z = 0; z < worldSize; z++)
		for (int x = 0; x < worldSize; x++)
		{
			int lod = max(abs(x - worldSize / 2), abs(z - worldSize / 2));
			world[x + z * worldSize] = generateWorld(x - worldSize / 2, z - worldSize / 2, lod);
		}

	Object camera = { 0 };
	float moveSpeed = 0.0075f;
	float sprintSpeed = 0.02f;
	float jumpHeight = 0.3f;
	float gravity = -0.03f;
	float lookSpeed = 0.1f;
	float fov = 120.f;

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

			playerInput(window, &camera, moveSpeed, sprintSpeed, lookSpeed, jumpHeight, gravity, &paused);

			if (glfwGetKey(window, GLFW_KEY_DOWN))
				viewDist -= 5.f;
			if (glfwGetKey(window, GLFW_KEY_UP))
				viewDist += 5.f;
		}
		timeLast = timeNow;
		glh_updateCamera(shader, &camera, fov, viewDist);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader);

		size_t triCount = 0;
		for (int z = 0; z < worldSize; z++)
			for (int x = 0; x < worldSize; x++)
			{
				float cX = (x - worldSize / 2) * 20.f;
				float cZ = (z - worldSize / 2) * 20.f;

				float pX = camera.trans.pos.x;
				float pZ = camera.trans.pos.z;

				float dist = pow2f(cX - pX) + pow2f(cZ - pZ);
				if (dist > pow2f(viewDist))
					continue;
				
				pX += sinf(toRad(camera.trans.rot.y)) * 20.f;
				pZ += cosf(toRad(camera.trans.rot.y)) * 20.f;
				
				float angleDiff = fmodf(camera.trans.rot.y - 
					toDeg(fastAtan2(pX - cX, pZ - cZ)) + 
					180.f + 360.f, 360.f) - 180.f;
				
				if (angleDiff <= fov * 0.667f && angleDiff >= -fov * 0.667f)
				{
					gls_drawModel(world[x + z * worldSize]);
					triCount += world[x + z * worldSize].count;
				}
			}

		glfwSwapBuffers(window);

		setTitle(window, "FPS:%4u | #Tri: %llu | Pos(%.2f, %.2f, %.2f) | Rot(%.2f, %.2f) | ViewDist: %f", 
			fps, triCount / 3,
			camera.trans.pos.x, camera.trans.pos.y, camera.trans.pos.z,
			camera.trans.rot.x, camera.trans.rot.y,
			viewDist);

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

	for (int z = 0; z < worldSize; z++)
		for (int x = 0; x < worldSize; x++)
			glh_deleteModel(world[x + z * worldSize]);
	free(world);

	glDeleteProgram(shader);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}