#include "gl_helper.h"

unsigned glh_width = 0;
unsigned glh_height = 0;

void glh_setView(unsigned width, unsigned height)
{
	glh_width = width;
	glh_height = height;
	glViewport(0, 0, width, height);
}


Model glh_loadModel(Vertex* verts, size_t count)
{
	Model model = { 0 };
	model.count = count;

	glGenVertexArrays(1, &model.vao);
	glBindVertexArray(model.vao);

	glGenBuffers(1, &model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), verts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Vec3f));
	glEnableVertexAttribArray(1);

	return model;
}

void glh_deleteModel(Model model)
{
	glDeleteVertexArrays(1, &model.vao);
	glDeleteBuffers(1, &model.vbo);
}

void gls_drawModel(Model model)
{
	glBindVertexArray(model.vao);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)model.count);
}


char* readFile(const char* filename)
{
	FILE* file = fopen(filename, "r");
	if (!file)
	{
		printf("Unable to open read file \"%s\"\n", filename);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buf = malloc(size + 1);
	if (!buf)
	{
		fclose(file);
		return NULL;
	}

	memset(buf, 0, size + 1);
	fread(buf, 1, size, file);
	
	fclose(file);

	return buf;
}

GLuint glh_loadShader(const char* vert, const char* frag)
{
	GLuint shader = 0;

	char* vertSrc = readFile(vert);
	if (vertSrc)
	{
		char* fragSrc = readFile(frag);
		if (fragSrc)
		{
			int status = 0;
			char info[512];

			unsigned vertShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertShader, 1, &vertSrc, NULL);
			glCompileShader(vertShader);
			glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);
			if (!status)
			{
				printf("Failed to compile vertex shader \"%s\"\n", vert);
				glGetShaderInfoLog(vertShader, 512, NULL, info);
				puts(info);
			}

			unsigned fragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragShader, 1, &fragSrc, NULL);
			glCompileShader(fragShader);
			glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
			if (!status)
			{
				printf("Failed to compile fragment shader \"%s\"\n", frag);
				glGetShaderInfoLog(fragShader, 512, NULL, info);
				puts(info);
			}

			shader = glCreateProgram();
			glAttachShader(shader, vertShader);
			glAttachShader(shader, fragShader);
			glLinkProgram(shader);
			glGetProgramiv(shader, GL_LINK_STATUS, &status);
			if (!status)
			{
				puts("Failed to link shader");
				glGetProgramInfoLog(shader, 512, NULL, info);
				puts(info);
			}

			glDeleteShader(vertShader);
			glDeleteShader(fragShader);

			free(fragSrc);
		}

		free(vertSrc);
	}

	return shader;
}



void glh_setUniformVec3(GLuint shader, const char* name, Vec3f value)
{
	glUniform3f(glGetUniformLocation(shader, name), value.x, value.y, value.z);
}

void glh_setUniformFloat(GLuint shader, const char* name, float value)
{
	glUniform1f(glGetUniformLocation(shader, name), value);
}

void glh_setUniformMat4(GLuint shader, const char* name, Matrix4* value)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, false, value->m);
}


void glh_updateCamera(GLuint shader, Object* camera, float fov, float viewDist)
{
	Matrix4 projMat = { 0 };
	Matrix4 viewMat = { 0 };

	float aspect = (float)glh_width / (float)glh_height;

	float far = 10000.f;
	float near = 0.01f;

	projMat.m[0 + 0 * 4] = 1.f / (aspect * tanf(toRad(fov) / 2.f));
	projMat.m[1 + 1 * 4] = 1.f / tanf(toRad(fov) / 2.f);
	projMat.m[2 + 2 * 4] = (far + near) / (near - far);
	projMat.m[3 + 2 * 4] = -1.f;
	projMat.m[2 + 3 * 4] = -(2.f * far * near) / (far - near);

	Vec3f up = { 0.f, 1.f, 0.f };
	Vec3f lookat = normalize(vec3f(
		-sinf(toRad(camera->trans.rot.y)) * cosf(toRad(camera->trans.rot.x)),
		 sinf(toRad(camera->trans.rot.x)),
		-cosf(toRad(camera->trans.rot.y)) * cosf(toRad(camera->trans.rot.x))));
	Vec3f eye = camera->trans.pos;
	eye.y += 0.5f;
	Vec3f s = normalize(cross(lookat, up));
	Vec3f u = cross(s, lookat);

	viewMat.m[0 + 0 * 4] = s.x;
	viewMat.m[0 + 1 * 4] = s.y;
	viewMat.m[0 + 2 * 4] = s.z;
	viewMat.m[1 + 0 * 4] = u.x;
	viewMat.m[1 + 1 * 4] = u.y;
	viewMat.m[1 + 2 * 4] = u.z;
	viewMat.m[2 + 0 * 4] = -lookat.x;
	viewMat.m[2 + 1 * 4] = -lookat.y;
	viewMat.m[2 + 2 * 4] = -lookat.z;
	viewMat.m[3 + 3 * 4] = 1.f;

	glh_setUniformMat4(shader, "projMat", &projMat);
	glh_setUniformMat4(shader, "viewMat", &viewMat);
	glh_setUniformVec3(shader, "camPos", eye);
	glh_setUniformFloat(shader, "viewDist", viewDist);
}
