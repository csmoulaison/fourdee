float* v2_init(float* v, float x, float y)
{
	v[0] = x;
	v[1] = y;
	return v;
}

float* v2_copy(float* v, float* res)
{
	res[0] = v[0];
	res[1] = v[1];
	return res;
}

float* v2_add(float* va, float* vb, float* res)
{
	res[0] = va[0] + vb[0];
	res[1] = va[1] + vb[1];
	return res;
}

float* v2_sub(float* va, float* vb, float* res)
{
	res[0] = va[0] - vb[0];
	res[1] = va[1] - vb[1];
	return res;
}

float* v2_scale(float* v, float s, float* res)
{
	res[0] = v[0] * s;
	res[1] = v[1] * s;
	return res;
}

float* v2_pair_mult(float* va, float* vb, float* res)
{
	res[0] = va[0] * vb[0];
	res[1] = va[1] * vb[1];
	return res;
}

float* v2_div(float* va, float* vb, float* res)
{
	res[0] = va[0] / vb[0];
	res[1] = va[1] / vb[1];
	return res;
}

float* v3_init(float* v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
	return v;
}

float* v3_copy(float* v, float* res)
{
	res[0] = v[0];
	res[1] = v[1];
	res[2] = v[2];
	return res;
}

float* v3_add(float* va, float* vb, float* res)
{
	res[0] = va[0] + vb[0];
	res[1] = va[1] + vb[1];
	res[2] = va[2] + vb[2];
	return res;
}

float* v3_sub(float* va, float* vb, float* res)
{
	res[0] = va[0] - vb[0];
	res[1] = va[1] - vb[1];
	res[2] = va[2] - vb[2];
	return res;
}

float* v3_scale(float* v, float s, float* res)
{
	res[0] = v[0] * s;
	res[1] = v[1] * s;
	res[2] = v[2] * s;
	return res;
}

float* v3_pair_mult(float* va, float* vb, float* res)
{
	res[0] = va[0] * vb[0];
	res[1] = va[1] * vb[1];
	res[2] = va[2] * vb[2];
	return res;
}

float* v3_div(float* va, float* vb, float* res)
{
	res[0] = va[0] / vb[0];
	res[1] = va[1] / vb[1];
	res[2] = va[2] / vb[2];
	return res;
}

float v3_dot(float* va, float* vb)
{
	// TODO - implement
	return 0;
}

float v3_cross(float* va, float* vb)
{
	// TODO - implement
	return 0;
}

float* v4_init(float* v, float x, float y, float z, float w)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = w;
	return v;
}

float* v4_copy(float* v, float* res)
{
	res[0] = v[0];
	res[1] = v[1];
	res[2] = v[2];
	res[3] = v[3];
	return res;
}

float* v4_add(float* va, float* vb, float* res)
{
	res[0] = va[0] + vb[0];
	res[1] = va[1] + vb[1];
	res[2] = va[2] + vb[2];
	res[3] = va[3] + vb[3];
	return res;
}

float* v4_sub(float* va, float* vb, float* res)
{
	res[0] = va[0] - vb[0];
	res[1] = va[1] - vb[1];
	res[2] = va[2] - vb[2];
	res[3] = va[3] - vb[3];
	return res;
}

float* v4_scale(float* v, float s, float* res)
{
	res[0] = v[0] * s;
	res[1] = v[1] * s;
	res[2] = v[2] * s;
	res[3] = v[3] * s;
	return res;
}

float* v4_pair_mult(float* va, float* vb, float* res)
{
	res[0] = va[0] * vb[0];
	res[1] = va[1] * vb[1];
	res[2] = va[2] * vb[2];
	res[3] = va[3] * vb[3];
	return res;
}

float* v4_div(float* va, float* vb, float* res)
{
	res[0] = va[0] / vb[0];
	res[1] = va[1] / vb[1];
	res[2] = va[2] / vb[2];
	res[3] = va[3] / vb[3];
	return res;
}
