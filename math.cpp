#include "cmath" //cos and sin
#include "math.h"

extern inline void VecSet(vec3_t vec, float x, float y, float z)
{
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
}

extern inline void VecAdd(vec3_t accumulator, const vec3_t addend)
{
	accumulator[0] += addend[0];
	accumulator[1] += addend[1];
	accumulator[2] += addend[2];
}

extern inline void VecNegate(vec3_t vec)
{
	vec[0] = -vec[0];
	vec[1] = -vec[1];
	vec[2] = -vec[2];
}

extern inline void VecScale(vec3_t out, const vec3_t in, const float scalar)
{
	out[0] = in[0] * scalar;
	out[1] = in[1] * scalar;
	out[2] = in[2] * scalar;
}

extern inline float VecLength(const vec3_t vec)
{
	return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

extern inline void CrossProduct(const vec3_t vec1, const vec3_t vec2, vec3_t out)
{
	out[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
	out[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
	out[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

extern inline float DotProduct(const vec3_t vec1, const vec3_t vec2)
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

extern inline void VecNormalize(vec3_t out, const vec3_t in)
{
	float l = VecLength(in);

	if (!l)
		return;
	l = 1 / l;
	out[0] = in[0] * l;
	out[1] = in[1] * l;
	out[2] = in[2] * l;
}

extern inline void VecCopy(vec3_t dest, const vec3_t src)
{
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}


extern inline float ToRadians(float degrees)
{
	return (float)(degrees * (PI / 180));
}


extern inline void GetAngleVectors(float pitch, float yaw, vec3_t forward, vec3_t right)
{
	forward[0] = cos(ToRadians(pitch)) * -sin(ToRadians(yaw));
	forward[1] = -sin(ToRadians(pitch));
	forward[2] = cos(ToRadians(pitch)) * cos(ToRadians(yaw));

	//FIXME: this is not the correct way to do this
	VecNormalize(forward, forward);
	CrossProduct(forward, upvec, right);
	VecNormalize(right, right);
}

extern inline void GetForwardVector(float pitch, float yaw, vec3_t vec)
{
	vec[0] = cos(ToRadians(pitch)) * -sin(ToRadians(yaw));
	vec[1] = -sin(ToRadians(pitch));
	vec[2] = cos(ToRadians(pitch)) * cos(ToRadians(yaw));
}

extern inline void GetRightVector(float pitch, float yaw, vec3_t vec)
{
	vec[0] = cos(ToRadians(yaw));
	vec[1] = 0;
	vec[2] = -sin(ToRadians(yaw));
}

int InAABB(const vec3_t point, const vec3_t mins, const vec3_t maxs)
{
	for (int i = 0; i < 3; i++)
	{
		if ((point[i] < mins[i]) || (point[i] > maxs[i]))
			return 0;
		//todo: catch near equal to zero values
	}

	//If all elements of point - mins are > 0 & all elements of maxs - point > 0
	return 1;
}