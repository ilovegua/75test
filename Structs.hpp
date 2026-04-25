#pragma once

struct Vector2 {
	float x, y;
};

struct Vector3 {
	float x, y, z;
};

struct Matrix {
	float matrix[16];
};

inline struct Vector3 _WorldToScreen(const struct Vector3 pos, struct Matrix matrix, struct Vector2 resolution) {
	struct Vector3 out;

	float _x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
	float _y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
	out.z    = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];

	
	if (out.z < 0.001f) {
		out.z = -1.f;
		return out;
	}

	float invW = 1.f / out.z;
	_x *= invW;
	_y *= invW;

	float w = (float)resolution.x;
	float h = (float)resolution.y;

	out.x = w * 0.5f + 0.5f * _x * w + 0.5f;
	out.y = h * 0.5f - 0.5f * _y * h + 0.5f;

	
	
	const float marginX = w * 0.5f;
	const float marginY = h * 0.5f;
	if (out.x < -marginX || out.x > w + marginX ||
	    out.y < -marginY || out.y > h + marginY) {
		out.z = -1.f;
		return out;
	}

	return out;
}