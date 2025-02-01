const uint UINT_MAX = 0xFFFFFFFF;

const float pi = 3.1415926535897932384626433832795;
const float pi2 = pi*2.0;
const float halfPi = pi/2.0;
const float toDeg = 180.0/pi;
const float toRad = pi/180.0;

//-------------RANDOM NUMBER GENERATOR--------------
uint seed = 742796405u;
uint pcg_hash(uint val) {
    seed += val * 747796405u + 2891336453u;
    uint word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand(uvec2 v) { return float(pcg_hash(v.x + (v.y<<16)))/UINT_MAX; }
float rand() {return rand(gl_LaunchIDEXT.xy);};  // range  0 to 1
float rands() {return rand()*2.0-1.0;}           // range -1 to 1
//--------------------------------------------------

//-----------------Random direction-----------------
vec2 rand2d() { float a=rand()*pi2; return vec2(cos(a), sin(a)); }       // uniformly distributed point on 2d circle
vec3 rand3d() { float z=rand();  return vec3(rand2d()*sqrt(1-z*z),z); }  // uniformly distributed point on 3d hemisphere
vec3 rands3d(){ float z=rands(); return vec3(rand2d()*sqrt(1-z*z),z); }  // uniformly distributed point on 3d sphere
//--------------------------------------------------

//-----------------------VEC3-----------------------  
float AngleX(vec3 v) {return atan(v.z, v.y);}
float AngleY(vec3 v) {return atan(v.x, v.z);}
float AngleZ(vec3 v) {return atan(v.x, v.y);}  // not used

vec3 RotateX(vec3 v, float rad) {
    rad = mod(rad, pi2);
    float cs = cos(-rad);
    float sn = sin(-rad);
    float z = v.z*cs - v.y*sn;
    float y = v.y*cs + v.z*sn;
    return vec3(v.x, y, z);
}

vec3 RotateY(vec3 v, float rad) {
    rad = mod(rad, pi2);
    float cs = cos(-rad);
    float sn = sin(-rad);
    float x = v.x*cs - v.z*sn;
    float z = v.z*cs + v.x*sn;
    return vec3(x, v.y, z);
}

vec3 RotateZ(vec3 v, float rad) {
    rad = mod(rad, pi2);
    float cs = cos(-rad);
    float sn = sin(-rad);
    float x = v.x*cs - v.y*sn;
    float y = v.y*cs + v.x*sn;
    return vec3(x, y, v.z);
}

mat3 look(vec3 v, vec3 up) {
    vec3 zAxis = normalize(v);
    vec3 xAxis = cross(up, zAxis);
    xAxis = (length(xAxis)<0.001) ? vec3(1,0,0) : normalize(xAxis);
    vec3 yAxis = normalize(cross(zAxis, xAxis));
    return mat3(xAxis, yAxis, zAxis);
}

mat3 look(vec3 v) {return look(v, vec3(0,1,0));}  // y=up
//--------------------------------------------------
