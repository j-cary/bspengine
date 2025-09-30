/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"

inline void VecSet(vec3_t vec, float x, float y, float z);
inline void VecAdd(vec3_t accumulator, const vec3_t addend);
inline void VecNegate(vec3_t vec);
inline void VecScale(vec3_t out, const vec3_t in, const float scalar);
inline float VecLength(const  vec3_t vec);
inline void CrossProduct(const vec3_t vec1, const vec3_t vec2, vec3_t out);
inline float DotProduct(const vec3_t vec1, const vec3_t vec2);
inline void VecNormalize(vec3_t out, const vec3_t in);
inline void VecCopy(vec3_t dest, const vec3_t src);

inline float ToRadians(float degrees);

inline void GetAngleVectors(float pitch, float yaw, vec3_t forward, vec3_t right);
inline void GetForwardVector(float pitch, float yaw, vec3_t vec);
inline void GetRightVector(float pitch, float yaw, vec3_t vec);

int InAABB(const vec3_t point, const vec3_t mins, const vec3_t maxs);

//test every tick, if success, calling ent can be reversed according to its dir & velocity
int AABBTouch(const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
