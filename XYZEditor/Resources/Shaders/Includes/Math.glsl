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