#pragma once
#include <glad/glad.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "vectorMath.h"

extern unsigned glh_width;
extern unsigned glh_height;

void glh_setView(unsigned width, unsigned height);


typedef struct Vertex
{
	Vec3f pos;
	RGB rgb;
} Vertex;

typedef struct Transform
{
	Vec3f pos;
	Vec3f rot;
} Transform;

typedef struct Object
{
	Transform trans;
	Vec3f vel;
	bool onGround;
	bool inWater;
} Object;

typedef struct Model
{
	size_t count;
	GLuint vao, vbo;
} Model;

extern size_t triCount;

Model glh_loadModel(Vertex* verts, size_t count);

void glh_deleteModel(Model model);

void gls_drawModel(Model model);


char* readFile(const char* filename);

GLuint glh_loadShader(const char* vert, const char* frag);


typedef struct Matrix4
{
	float m[4 * 4];
} Matrix4;


void glh_setUniformVec3(GLuint shader, const char* name, Vec3f value);
void glh_setUniformFloat(GLuint shader, const char* name, float value);
void glh_setUniformMat4(GLuint shader, const char* name, Matrix4* value);

void glh_updateCamera(GLuint shader, Object* camera, float fov, float viewDist);