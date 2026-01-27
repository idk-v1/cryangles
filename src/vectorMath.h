#pragma once
#include <math.h>

static const float PI = 3.1415f;

static float fastSqrt(float x)
{
	union
	{
		int i;
		float x;
	} u;
	u.x = x;
	u.i = (1 << 29) + (u.i >> 1) - (1 << 22);

	// Two Babylonian Steps (simplified from:)
	// u.x = 0.5f * (u.x + x/u.x);
	// u.x = 0.5f * (u.x + x/u.x);
	u.x = u.x + x / u.x;
	u.x = 0.25f * u.x + x / u.x;

	return u.x;
}

static float fastAtan2(float y, float x)
{
	//http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
	//Volkan SALMA

	const float ONEQTR_PI = PI / 4.f;
	const float THRQTR_PI = 3.f * PI / 4.f;
	float r, angle;
	float abs_y = fabsf(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if (x < 0.f)
	{
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	}
	else
	{
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;
	if (y < 0.f)
		return(-angle);     // negate if in quad III or IV
	else
		return(angle);
}

typedef struct Vec3f
{
	float x, y, z;
} Vec3f;

static Vec3f vec3f(float x, float y, float z)
{
	Vec3f vec;
	vec.x = x;
	vec.y = y;
	vec.z = z;
	return vec;
}

static float pow2f(float x)
{
	return x * x;
}

static Vec3f normalize(Vec3f vec)
{
	float dist = fastSqrt(pow2f(vec.x) + pow2f(vec.y) + pow2f(vec.z));
	vec.x /= dist;
	vec.y /= dist;
	vec.z /= dist;
	return vec;
}

static Vec3f cross(Vec3f l, Vec3f r)
{
	Vec3f vec = { 0 };
	vec.x = l.y * r.z - r.y * l.z;
	vec.y = l.z * r.x - r.z * l.x;
	vec.z = l.x * r.y - r.x * l.y;
	return vec;
}

static float dot(Vec3f l, Vec3f r)
{
	return l.x * r.x + l.y * r.y + l.z * r.z;
}


static float toRad(float deg)
{
	return deg * PI / 180.f;
}

static float toDeg(float rad)
{
	return rad * 180.f / PI;
}

static float gls_wrapDeg(float deg)
{
	deg = fmodf(deg, 360.f);
	if (deg < 0.f)
		deg += 360.f;
	return deg;
}


static float clampf(float min, float value, float max)
{
	return fmaxf(fminf(value, max), min);
}

static float lerp(float x, float y, float mix)
{
	//mix = clampf(0.f, mix, 1.f);
	return x * mix + y * (1.f - mix);
}

typedef struct RGB
{
	float r, g, b;
} RGB;

static RGB lerpRGB(RGB a, RGB b, float mix)
{
	RGB color;
	color.r = lerp(a.r, b.r, mix);
	color.g = lerp(a.g, b.g, mix);
	color.b = lerp(a.b, b.b, mix);
	return color;
}

static RGB rgb(float r, float g, float b)
{
	RGB color;
	color.r = r; // clampf(0.f, r, 1.f);
	color.g = g; // clampf(0.f, g, 1.f);
	color.b = b; // clampf(0.f, b, 1.f);
	return color;
}