

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint Hash( uint x ) 
{
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}



// Compound versions of the hashing algorithm I whipped together.
uint Hash( uvec2 v ) { return Hash( v.x ^ Hash(v.y)                         ); }
uint Hash( uvec3 v ) { return Hash( v.x ^ Hash(v.y) ^ Hash(v.z)             ); }
uint Hash( uvec4 v ) { return Hash( v.x ^ Hash(v.y) ^ Hash(v.z) ^ Hash(v.w) ); }



// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float FloatConstruct( uint m ) 
{
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}



// Pseudo-random value in half-open range [0:1].
float Random( float x ) { return FloatConstruct(Hash(floatBitsToUint(x))); }
float Random( vec2  v ) { return FloatConstruct(Hash(floatBitsToUint(v))); }
float Random( vec3  v ) { return FloatConstruct(Hash(floatBitsToUint(v))); }
float Random( vec4  v ) { return FloatConstruct(Hash(floatBitsToUint(v))); }


float Random(float minimum, float maximum, float seed)
{
    float randRatio = Random(seed);
    return mix(minimum, maximum, randRatio); 
}


vec3 Random(vec3 minimum, vec3 maximum, in vec3 seed0, in vec3 seed1, float seed3)
{
    vec3 randMin = vec3(
        Random(seed0.x) * minimum.x,
        Random(seed0.y) * minimum.y,
        Random(seed0.z) * minimum.z
    );

    vec3 randMax = vec3(
        Random(seed1.x) * maximum.x,
        Random(seed1.y) * maximum.y,
        Random(seed1.z) * maximum.z
    );

    float randRatio = Random(seed3);

    return mix(randMin, randMax, randRatio);
}

vec4 Random(in vec4 minimum, in vec4 maximum, in vec4 seed0, in vec4 seed1, float seed3)
{
    vec4 randMin = vec4(
        Random(seed0.x) * minimum.x,
        Random(seed0.y) * minimum.y,
        Random(seed0.z) * minimum.z,
        Random(seed0.w) * minimum.w
    );

    vec4 randMax = vec4(
        Random(seed1.x) * maximum.x,
        Random(seed1.y) * maximum.y,
        Random(seed1.z) * maximum.z,
        Random(seed1.w) * maximum.w
    );

    float randRatio = Random(seed3);



    return mix(randMin, randMax, randRatio);
}


vec4 Random(in vec4 minimum, in vec4 maximum, float seed)
{
    float randRatio = Random(seed);
    return mix(minimum, maximum, randRatio);
}