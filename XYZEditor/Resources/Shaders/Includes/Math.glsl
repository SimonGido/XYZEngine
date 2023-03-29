vec4 EulerToQuat(in vec3 eulerAngles)
{
    vec4 result;
    vec3 c = vec3(cos(eulerAngles.x * 0.5),
                  cos(eulerAngles.y * 0.5),
                  cos(eulerAngles.z * 0.5));

    vec3 s = vec3(sin(eulerAngles.x * 0.5),
                  sin(eulerAngles.y * 0.5),
                  sin(eulerAngles.z * 0.5));

    result.w = c.x * c.y * c.z + s.x * s.y * s.z;
	result.x = s.x * c.y * c.z - c.x * s.y * s.z;
	result.y = c.x * s.y * c.z + s.x * c.y * s.z;
	result.z = c.x * c.y * s.z - s.x * s.y * c.z;
    return result;
}

mat4 QuatToMat4(in vec4 q)
{
    mat4 result;
    float qxx = q.x * q.x;
	float qyy = q.y * q.y;
	float qzz = q.z * q.z;
	float qxz = q.x * q.z;
	float qxy = q.x * q.y;
	float qyz = q.y * q.z;
	float qwx = q.w * q.x;
	float qwy = q.w * q.y;
	float qwz = q.w * q.z;

    result[0][0] = 1.0 - 2.0 * (qyy +  qzz);
	result[0][1] = 2.0 * (qxy + qwz);
	result[0][2] = 2.0 * (qxz - qwy);

	result[1][0] = 2.0 * (qxy - qwz);
	result[1][1] = 1.0 - 2.0 * (qxx +  qzz);
	result[1][2] = 2.0 * (qyz + qwx);

	result[2][0] = 2.0 * (qxz + qwy);
	result[2][1] = 2.0 * (qyz - qwx);
	result[2][2] = 1.0 - 2.0 * (qxx +  qyy);

    result[0][3] = result[1][3] = result[2][3] = result[3][0] = result[3][1] = result[3][2] = 0;
    result[3][3] = 1;
    return result;
}

mat4 TranslationMatrix(in vec3 translation)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(translation, 1.0));
}

mat4 TranslationMatrix(in vec3 translation, in vec3 scale)
{
    return mat4(
        vec4(scale.x, 0.0, 0.0, 0.0),
        vec4(0.0, scale.y, 0.0, 0.0),
        vec4(0.0, 0.0, scale.z, 0.0),
        vec4(translation, 1.0));
}

mat4 RotationMatrix(in vec3 eulerAngles)
{
    vec4 quat = EulerToQuat(eulerAngles);
    return QuatToMat4(quat);
}

mat4 RotationMatrix(in vec4 quat)
{
    return QuatToMat4(quat);
}

mat4 TransformFromRows(in vec4 transformRow0, in vec4 transformRow1, in vec4 transformRow2)
{
    return mat4(
		vec4(transformRow0.x, transformRow1.x, transformRow2.x, 0.0),
		vec4(transformRow0.y, transformRow1.y, transformRow2.y, 0.0),
		vec4(transformRow0.z, transformRow1.z, transformRow2.z, 0.0),
		vec4(transformRow0.w, transformRow1.w, transformRow2.w, 1.0)
	);
}

bool AABBCollisionAABB(vec3 min0, vec3 max0, vec3 min1, vec3 max1)
{
    float d1x = min1.x - max0.x;
    float d1y = min1.y - max0.y;
    float d2x = min0.x - max1.x;
    float d2y = min0.y - max1.y;
    
    if (d1x > 0.0 || d1y > 0.0)
    	return false;
    
    if (d2x > 0.0 || d2y > 0.0)
    	return false;
    
    return true;
}

float SQDistance(float v, float min, float max)
{
    float sqDist = 0.0;
    if (v < min) 
        sqDist += (min - v) * (min - v);

    if (v > max) 
        sqDist += (v - max) * (v - max);

    return sqDist;
}

bool AABBCollisionSphere(vec3 min, vec3 max, vec3 center, float radius)
{
    float sqDist = 0.0;

    sqDist += SQDistance(center.x, min.x, max.x);
    sqDist += SQDistance(center.y, min.y, max.y);
    sqDist += SQDistance(center.z, min.z, max.z);

    return sqDist <= radius * radius;
}

int ComputeBinominal(int n, int k)
{
    if (n == k)
        return 1;
 
    int value = 1;
    for (int i = 1; i <= k; i++)
    {
        value = value * ((n + 1 - i) / i);
    }     
    return value;
}


vec2 ComputeBezierCurveCubic(vec2 p[4], float t)
{
    vec2 result = 
            pow((1.0 - t), 3.0) * p[0] 
    + 3.0 * pow((1.0 - t), 2.0) * t * p[1] 
    + 3.0 * pow((1.0 - t), 1.0) * pow(t, 2.0) * p[2] 
    +       pow(t, 3.0) * p[3];

    return result;
}