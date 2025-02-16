/*
*--------------------------------------------------------------------------
* Copyright (c) 2016-2019 Rene Lindsay
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Author: Rene Lindsay <rjklindsay@gmail.com>
*
*--------------------------------------------------------------------------
*
* This unit provides basic types for 3d graphics:
*
* vec2    : 2 element vector of floats
* vec3    : 3 element vector of floats
* vec4    : 4 element vector of floats (for homogenous computations)
* mat4    : 4x4 matrix in column major order
* euler   : represents an orientation as (yaw, pitch, roll)
* quat    : quaternion (WIP)
*
* The types were named to match equivalent types in GLSL.
*
*--------------------------------------------------------------------------
*/

// TODO:
//   Finish quat class
//   Use SIMD

#pragma once
#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
//#include <stdint.h>

//#include <intrin.h>    //for avx on windows
//include <x86intrin.h>  //for avx on linux

const double PI = 3.14159265358979323;
const double toDEG = 180.0/PI;
const double toRAD = PI/180.0;

const float pi = 3.141592653f;
const float toDeg = 180.0f/pi;
const float toRad = pi/180.0f;
typedef unsigned int uint;

#define Y_UP    // remove this for Z_UP

//#define FASTER  // but less accurate

#undef repeat
#define repeat(COUNT) for(uint i = 0; i < (COUNT); ++i)

//static float pyth(float x, float y)   {return sqrtf(x*x+y*y);}
//static double pyth(double x, double y) {return sqrt(x*x+y*y);}

//#define CLAMP(VAL, MIN, MAX) (((VAL)<(MIN))?(MIN):((VAL)>(MAX))?(MAX):(VAL))

template <class T> T clampT(T val, T min, T max) {return (val<min ? min : val > max ? max : val); }
static auto clampd = clampT<double>;
static auto clampf = clampT<float>;
static auto clampi = clampT<int>;

static float invSqrt(float x) {   // fast inverse sqrt
    //assert(x>0);
    float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;           // initial guess
    x = x*(1.5f - xhalf*x*x);  // refine with newtons method
    //x = x*(1.5f - xhalf*x*x);  // refine again for more accuracy (optional)
    return x;
}
/*
inline float sqrt_fast(const float x) {
    static union {
        int i;
        float x;
    } u;
    u.x = x;
    u.i = (1<<29) + (u.i >> 1) - (1<<22); 
    u.x = 0.5f * (u.x + x/u.x);
    return u.x;
}
*/

#ifdef _WIN32
    inline void sincosf(float ang, float* s, float* c) {
        *s = sinf(ang);
        *c = cosf(ang);
    }
#endif

//----------------vec2---------------
struct ivec2 {
    int x, y;
    ivec2() {}
    ivec2(int x, int y) : x(x), y(y) {}
    //vec2i(float x, float y) : x((int)x), y((int)y) {}
};

struct vec2 {
    float x, y;
    vec2() {}
    vec2(float* f) { x=f[0]; y=f[1]; }
    vec2(float x, float y) : x(x), y(y) {}
    vec2& operator += (const vec2& v) { x+=v.x;  y+=v.y;  return *this; }
    vec2& operator -= (const vec2& v) { x-=v.x;  y-=v.y;  return *this; }
    vec2& operator *= (const vec2& v) { x*=v.x;  y*=v.y;  return *this; }
    vec2& operator /= (const vec2& v) { x/=v.x;  y/=v.y;  return *this; }
    vec2 operator + (const vec2& v) const { return {x+v.x, y+v.y}; }
    vec2 operator - (const vec2& v) const { return {x-v.x, y-v.y}; }
    vec2 operator * (const vec2& v) const { return {x*v.x, y*v.y}; }
    vec2 operator / (const vec2& v) const { return {x/v.x, y/v.y}; }
    vec2 operator * (const float f) const { return {x*f, y*f}; }
    vec2 operator / (const float f) const { return {x/f, y/f}; }
    operator ivec2 () const { return {int(x), int(y)}; }
    vec2 lerp(vec2 v, float ratio) { return *this + ((v-*this)*ratio); }
    void Rotate(const float angle) {
        float c, s;
        sincosf(angle * toRad, &s, &c);
        const float X = x;
        x = X*c - y*s;
        y = y*c + X*s;
    }
};
//-----------------------------------

//----------------vec3---------------
struct vec3 {
    float x, y, z;
    vec3() {}
    vec3(float* f) { x=f[0]; y=f[1]; z=f[2]; }
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3(const vec2& v2) { x=v2.x; y=v2.y; z=0; }
    vec3(const vec2& v2, float _z) { x=v2.x; y=v2.y; z=_z; }
    void set(float _x, float _y, float _z) { x = _x;  y = _y; z = _z; }
    vec3& operator += (const vec3& v) { x+=v.x;  y+=v.y;  z+=v.z;  return *this; }
    vec3& operator -= (const vec3& v) { x-=v.x;  y-=v.y;  z-=v.z;  return *this; }
    vec3& operator *= (const vec3& v) { x*=v.x;  y*=v.y;  z*=v.z;  return *this; }
    vec3& operator /= (const vec3& v) { x/=v.x;  y/=v.y;  z/=v.z;  return *this; }
    vec3& operator *= (const float f) { x*=f;   y*=f;   z*=f;   return *this; }
    vec3& operator /= (const float f) { x/=f;   y/=f;   z/=f;   return *this; }
    vec3 operator + (const vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    vec3 operator - (const vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    vec3 operator * (const vec3& v) const { return {x*v.x, y*v.y, z*v.z}; }
    vec3 operator / (const vec3& v) const { return {x/v.x, y/v.y, z/v.z}; }
    vec3 operator * (const float f) const { return {x*f, y*f, z*f}; }
    vec3 operator / (const float f) const { float rf=1.f/f;  return *this*rf; }
    vec3 operator - () const { return {-x,-y,-z}; }
    bool operator ==(const vec3& v) const { return (x==v.x && y==v.y && z==v.z); }
    operator float* () const { return (float*)this; }
    vec2  xy() {return *(vec2*)this;}
    vec3  cross(const vec3& v) const { return{ y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x }; }  // cross product
    float dot  (const vec3& v) const { return x*v.x + y*v.y + z*v.z; }                     // dot product

    void normalize_fast() { *this *= invSqrt(x*x + y*y + z*z); }
#ifdef FASTER
    void normalize() { *this *= invSqrt(x*x + y*y + z*z); }
#else
    void normalize() { *this *= 1.f / length(); }
#endif
    vec3 normalized() const { return *this * (1.f / length()); }
    float length_squared() const { return (x*x + y*y + z*z); }
    float length() const { return sqrtf(x*x + y*y + z*z); }
    vec3 lerp(vec3 v, float ratio) { return *this + ((v-*this)*ratio); }
    vec3 reflect(const vec3& norm) const { return *this - (norm * dot(norm)) * 2.f; }  // reflect vector around suface normal
    //TODO: refract...
  private:
    inline void _rotate(const float ang, float &X, float &Y) {
        const float a = -ang * toRad;
        const float cs = cosf(a);
        const float sn = sinf(a);
        const float Z = X;
        X = Z*cs - Y*sn;
        Y = Y*cs + Z*sn;
    }
  public:
    void RotateX(const float angle) { _rotate( angle, z, y); }  // rotate vector around X-axis (clockwise)
    void RotateY(const float angle) { _rotate( angle, x, z); }  // rotate vector around Y-axis (clockwise)
    void RotateZ(const float angle) { _rotate(-angle, x, y); }  // rotate vector around Z-axis (clockwise)
    float AngleX(){ return  (float)atan2(z, y) * toDeg; }       // get vector's angle around X-axis  (PITCH  : positive is up       )  // y-axis=0
    float AngleY(){ return  (float)atan2(x, z) * toDeg; }       // get vector's angle around Y-axis  (ROLL   : clockwise from top   )  // z-axis=0
    float AngleZ(){ return -(float)atan2(x, y) * toDeg; }       // get vector's angle around Z-axis  (HEADING: clockwise from north )  // x=axis-0

    void Print() const { printf("(%+7.4f,%+7.4f,%+7.4f)\n", x ,y ,z); }
};

static vec3 normalize(const vec3& v) { return v / v.length(); }
//-----------------------------------

//----------------vec4---------------
struct alignas(16) vec4 {
    float x, y, z, w;

    vec4() {}
    vec4(float* f) { x=f[0]; y=f[1]; z=f[2]; w=f[3]; }
    vec4(float _x, float _y, float _z, float _w) { x=_x;  y=_y; z=_z; w=_w; }
    vec4(const vec3& v3) { x=v3.x; y=v3.y; z=v3.z; w=1; }
    vec4(const vec3& v3, float _w) { x=v3.x; y=v3.y; z=v3.z; w=_w; }

    void set(float _x, float _y, float _z, float _w=1.0) { x=_x;  y=_y; z=_z; w =_w; }
    vec4& operator += (const vec3& v) { x+=v.x;  y+=v.y;  z+=v.z;  return *this; }
    vec4& operator -= (const vec3& v) { x-=v.x;  y-=v.y;  z-=v.z;  return *this; }
    vec4& operator *= (const vec3& v) { x*=v.x;  y*=v.y;  z*=v.z;  return *this; }
    vec4& operator /= (const vec3& v) { x/=v.x;  y/=v.y;  z/=v.z;  return *this; }
    vec4& operator *= (const float f) { x*=f;   y*=f;   z*=f;      return *this; }
    vec4& operator /= (const float f) { x/=f;   y/=f;   z/=f;      return *this; }
    vec4& operator  = (const vec3& v) { set(v.x, v.y, v.z, 1);     return *this; }
    vec4 operator   + (const vec3& v) const { return {x+v.x, y+v.y, z+v.z, w}; }
    vec4 operator   - (const vec3& v) const { return {x-v.x, y-v.y, z-v.z, w}; }
    vec4 operator   * (const float f) const { return {x*f, y*f, z*f, w};  }
    vec4 operator   / (const float f) const { float rf=1.f/f;  return *this*rf;  }
    vec4 operator   - () const { return {-x,-y,-z, w}; }
    operator vec3& () {return *(vec3*)this;}                                               // (ignores w)
    operator vec3 () const {if(w) return vec3(x/w, y/w, z/w); else return {x,y,z};}
    vec2  xy() {return *(vec2*)this;}
    vec3  xyz(){return *(vec3*)this;}
    vec3  cross(const vec3& v) const { return{ y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x }; }  // cross product
    float dot  (const vec3& v) const { return x*v.x + y*v.y + z*v.z; }                     // dot product

    float length() const { return sqrtf(x*x + y*y + z*z); }
    void normalize() { *this *= 1.f / length(); }
    void normalize_fast() { *this *= invSqrt(x*x + y*y + z*z); }
    void Print() const { printf("(%+7.4f,%+7.4f,%+7.4f,%+7.4f)\n", x ,y ,z, w); }
};

//-----------------------------------
struct alignas(16) ivec4 {
    int x, y, z, w;
    ivec4() {}
    ivec4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
};
//-----------------------------------

//-------------- dvec3 --------------
struct alignas(16) dvec3 {
    double x, y, z;
    dvec3() {}
    dvec3(double x, double y, double z) : x(x), y(y), z(z) {}
    dvec3(const vec3& v) { x=v.x; y=v.y; z=v.z; }
    void set(double _x, double _y, double _z) { x = _x;  y = _y; z = _z; }
    dvec3& operator += (const dvec3& v) { x+=v.x;  y+=v.y;  z+=v.z;  return *this; }
    dvec3& operator -= (const dvec3& v) { x-=v.x;  y-=v.y;  z-=v.z;  return *this; }
    dvec3& operator *= (const dvec3& v) { x*=v.x;  y*=v.y;  z*=v.z;  return *this; }
    dvec3& operator /= (const dvec3& v) { x/=v.x;  y/=v.y;  z/=v.z;  return *this; }
    dvec3& operator *= (const double f) { x*=f;   y*=f;   z*=f;   return *this; }
    dvec3& operator /= (const double f) { x/=f;   y/=f;   z/=f;   return *this; }
    dvec3 operator + (const dvec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    dvec3 operator - (const dvec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    dvec3 operator * (const dvec3& v) const { return {x*v.x, y*v.y, z*v.z}; }
    dvec3 operator / (const dvec3& v) const { return {x/v.x, y/v.y, z/v.z}; }
    dvec3 operator * (const double d) const { return {x*d, y*d, z*d}; }
    dvec3 operator / (const double d) const { double r=1.0/d;  return (*this)*r; }
    dvec3 operator - () const { return {-x,-y,-z}; }

    dvec3  cross(const dvec3& v) const { return{ y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x }; }  // cross product
    double dot  (const dvec3& v) const { return x*v.x + y*v.y + z*v.z; }                     // dot product

    operator vec3 () const {return vec3((float)x, (float)y, (float)z);}

    double length() const { return sqrt(x*x + y*y + z*z); }
    void normalize() { *this *= 1.0 / length(); }
    dvec3 lerp(dvec3 v, double ratio) { return *this + ((v-*this)*ratio); }
    dvec3 reflect(const dvec3& norm) const { return *this - (norm * dot(norm)) * 2.0; }  // reflect vector around suface normal

  private:
    inline void _rotate(double ang, double &X, double &Y) {
        double a = -ang * toRAD;
        double cs = cos(a);
        double sn = sin(a);
        double Z = X;
        X = Z*cs - Y*sn;
        Y = Y*cs + Z*sn;
    }
  public:
    void RotateX(double angle) { _rotate( angle, z, y); }  // rotate vector around X-axis (clockwise)
    void RotateY(double angle) { _rotate( angle, x, z); }  // rotate vector around Y-axis (clockwise)
    void RotateZ(double angle) { _rotate(-angle, x, y); }  // rotate vector around Z-axis (clockwise)
    double AngleX(){ return  atan2(z, y) * toDEG; }        // get vector's angle around X-axis  (PITCH  : positive is up       )  // y-axis=0
    double AngleY(){ return  atan2(x, z) * toDEG; }        // get vector's angle around Y-axis  (ROLL   : clockwise from top   )  // z-axis=0
    double AngleZ(){ return -atan2(x, y) * toDEG; }        // get vector's angle around Z-axis  (HEADING: clockwise from north )  // x=axis-0

    void Print() const { printf("(%+7.4f,%+7.4f,%+7.4f)\n", x ,y ,z); }
};
static dvec3 normalize(const dvec3& v) { return v / v.length(); }
//-----------------------------------

//-------------- dvec4 --------------
struct alignas(16) dvec4 {
    double x, y, z, w;
    dvec4() {}
    dvec4(double* d) { x=d[0]; y=d[1]; z=d[2]; w=d[3]; }
    dvec4(double _x, double _y, double _z, double _w) { x=_x;  y=_y; z=_z; w=_w; }
    dvec4(const dvec3& v3) { x=v3.x; y=v3.y; z=v3.z; w=1; }
    dvec4(const dvec3& v3, double _w) { x=v3.x; y=v3.y; z=v3.z; w=_w; }
    dvec4(const vec4& v) { x=v.x; y=v.y; z=v.z; w=v.w; }

    void set(double _x, double _y, double _z, double _w=1.0) { x=_x;  y=_y; z=_z; w =_w; }
    dvec4& operator += (const dvec3& v) { x+=v.x;  y+=v.y;  z+=v.z;  return *this; }
    dvec4& operator -= (const dvec3& v) { x-=v.x;  y-=v.y;  z-=v.z;  return *this; }
    dvec4& operator *= (const dvec3& v) { x*=v.x;  y*=v.y;  z*=v.z;  return *this; }
    dvec4& operator /= (const dvec3& v) { x/=v.x;  y/=v.y;  z/=v.z;  return *this; }
    dvec4& operator *= (const double f) { x*=f;   y*=f;   z*=f;      return *this; }
    dvec4& operator /= (const double f) { x/=f;   y/=f;   z/=f;      return *this; }
    dvec4& operator  = (const dvec3& v) { set(v.x, v.y, v.z, 1);     return *this; }
    dvec4 operator   + (const dvec3& v) const { return {x+v.x, y+v.y, z+v.z, w}; }
    dvec4 operator   - (const dvec3& v) const { return {x-v.x, y-v.y, z-v.z, w}; }
    dvec4 operator   * (const double f) const { return {x*f, y*f, z*f, w};  }
    dvec4 operator   / (const double f) const { double rf=1.f/f;  return *this*rf;  }
    dvec4 operator   - () const { return {-x,-y,-z, w}; }

    operator dvec3 () const {if(w) return dvec3(x/w, y/w, z/w); else return {x,y,z};}
    operator vec4 () const {return vec4((float)x, (float)y, (float)z, (float)w);}

    dvec3  xyz(){return *(dvec3*)this;}
    dvec3  cross(const dvec3& v) const { return{ y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x }; }  // cross product
    double dot  (const dvec3& v) const { return x*v.x + y*v.y + z*v.z; }                     // dot product
    double length() const { return sqrt(x*x + y*y + z*z); }
    void normalize() { double l = length(); x*=l; y*=l; z*=l; }
    void Print() const { printf("(%+7.4f,%+7.4f,%+7.4f,%+7.4f)\n", x ,y ,z, w); }
};
//-----------------------------------

//--------------EULER----------------
struct euler {
    float yaw, pitch, roll;
    euler() {}
    euler(float yaw, float pitch, float roll) : yaw(yaw), pitch(pitch), roll(roll) {}

#ifdef Y_UP
    euler(vec3 fwd) {         //Get a vector's yaw and pitch. (no roll)
        vec3 v=fwd;
        yaw =v.AngleY();  v.RotateY(-yaw);   // get and eliminate yaw
        pitch=v.AngleX();                    // get pitch
        roll =0;                             // no roll
    }

    euler(vec3 fwd, vec3 up) {    // Convert forward and up vectors to yaw/pitch/roll
        yaw  =180+fwd.AngleY(); fwd.RotateY(-yaw);   up.RotateY(-yaw);
        pitch= 90+fwd.AngleX(); fwd.RotateX(-pitch); up.RotateX(-pitch);
        roll =     up.AngleZ();

        if(pitch<-89.9999 && roll>179.9999) { yaw+=180; roll+=180; }  // prevent yaw&roll 180 at singularity

        if(yaw   > 180) yaw  -=360;
        if(pitch > 180) pitch-=360;
        if(roll  > 180) roll -=360;
    }
#else
    euler(vec3 fwd) {         //Get a vector's yaw and pitch. (no roll)
        vec3 v=fwd;
        yaw =v.AngleZ();  v.RotateZ(-yaw);   // get and eliminate yaw
        pitch=v.AngleX();                    // get pitch
        roll =0;                             // no roll
    }

    euler(vec3 fwd, vec3 up) {    // Convert up and forward vectors to yaw/pitch/roll
        yaw  = fwd.AngleZ(); fwd.RotateZ(-yaw);   up.RotateZ(-yaw);
        pitch= fwd.AngleX(); fwd.RotateX(-pitch); up.RotateX(-pitch);
        roll =  up.AngleY();
    }
#endif

    void Print(){ printf("Roll=%f  Pitch=%f  Yaw=%f\n", roll, pitch, yaw); }
    //void Print(){ printf("Yaw=%f  Pitch=%f  Roll=%f\n", yaw, pitch, roll); }
};
//-----------------------------------

//------------QUATERNION-------------
struct quat { //(UNTESTED)
    float w, x, y, z;
    vec3& vec() const {return *(vec3*)&x;}    // xyz as vec3
    quat() : w(1), x(0), y(0), z(0) {}  // identity
    quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
    quat(vec3 axis, float angle) { float a=angle*toRad/2.f; vec()=axis.normalized()*sinf(a); w=cosf(a); }  // axis-angle to quat
    quat Inverted() const {return quat(w, -x, -y, -z); }

    quat operator*(const quat& q) const {  // multiply quaternions, to combine rotations
        vec3 v = vec();
        vec3 qv = q.vec();
        float rw = w*q.w - v.dot(qv);
        vec3  rv = v*q.w + qv*w + v.cross(qv);
        return quat(rw, rv.x, rv.y, rv.z);
    };

    vec3 operator*(const vec3& v) const {
        quat p(0, v.x, v.y, v.z);
        vec3 c = vec().cross(v);
        return v + c*(2*w) + vec().cross(c)*2;
    }

    void toAxisAngle(vec3& axis, float& angle) {
        float len = vec().length();
        axis  = (len < 0.0001f) ? vec3(1,0,0) : vec()/len;
        angle = acosf(w)*2 * toDeg;
    }

    quat slerp(quat& r, float s) {
        vec3& v = vec();
        vec3 rv = r.vec();
        //quat& q = *this;

        float cosOmega = w*r.w + rv.dot(v);
        if (cosOmega < 0) {  // Avoid going the long way around.
            r.w = -r.w;
            rv = -rv;
            cosOmega = -cosOmega;
        }

        float k0, k1;
        if (cosOmega > 0.9999f) {  // Very close, use a linear interpolation.
            k0 = 1-s;
            k1 = s;
        } else {
            // Trig identity, sin^2 + cos^2 = 1
            float sinOmega = sqrtf(1 - cosOmega*cosOmega);

            // Compute the angle omega
            float Omega = atan2f(sinOmega, cosOmega);
            float OneOverSinOmega = 1/sinOmega;

            k0 = sinf((1-s)*Omega) * OneOverSinOmega;
            k1 = sinf(s*Omega) * OneOverSinOmega;
        }

        // Interpolate
        quat result;
        result.w     = w * k0 + r.w * k1;
        result.vec() = v * k0 + rv * k1;
        return result;
    }
};
//-----------------------------------

static float Identity4x4[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

//--------------MATRIX---------------
class alignas(16) mat4 {
public:
    union {
        float m[16];
        struct {
            float m00, m10, m20, m30;  // x axis       ,0
            float m01, m11, m21, m31;  // y axis       ,0
            float m02, m12, m22, m32;  // z axis       ,0
            float m03, m13, m23, m33;  // translation  ,ScaleInverse
        };
        struct {
            vec4 xAxis4;
            vec4 yAxis4;
            vec4 zAxis4;
            vec4 position4;
        } row;
    };

    void Set(const float* f){ memcpy(m, f, sizeof(m)); }
    vec3& xAxis()    {return (vec3&)row.xAxis4;}
    vec3& yAxis()    {return (vec3&)row.yAxis4;}
    vec3& zAxis()    {return (vec3&)row.zAxis4;}
    vec3& position() {return (vec3&)row.position4;}

    inline bool isIdentity(){ return memcmp(this, Identity4x4, 64)==0; }
    inline void SetIdentity(){Set(Identity4x4);}  // Reset this matrix to Identity
    inline void Clear()      {Set(Identity4x4);}  // Reset this matrix to Identity
    inline void ClearRot() { vec3 pos = position(); Clear(); position() = pos; }

    float& operator [] (uint i) { return m[i]; }                         // array operator
    mat4(float* f) { Set(&(*f)); }                                       // convert float[16] to mat4
    mat4(quat q) { from_quat(q);}                                        // convert quaternion to mat4
    mat4(euler e) { SetRotation(e.roll, e.pitch, e.yaw); }
    mat4(){ SetIdentity(); }                       // default constructor

    //mat4(const mat4& other) { Set(other.m); }     // copy constructor
    //mat4& operator = (const mat4& other){ Set(other.m); return *this; }  // copy assignment operator

    operator euler () {return Euler();}

#ifdef Y_UP
    euler Euler() { return euler(-zAxis(), yAxis()); }                 // Get Euler angles from matrix (y-up)

    void SetRotation(float roll, float pitch, float yaw) {             // Apply euler rotations.
        vec3 pos = position();
        Clear();
        RotateY(yaw);
        RotateX(pitch);
        RotateZ(roll);
        position() = pos;
    }
#else
    euler Euler() { return euler( yAxis(), zAxis()); }                 // Get Euler angles from matrix (z-up)

    void SetRotation(float roll, float pitch, float yaw) {             // Apply euler rotations.
        vec3 pos = position();
        Clear();
        RotateZ(yaw);
        RotateX(pitch);
        RotateY(-roll);
        position() = pos;
    }

#endif

    inline void Translate(const float x, const float y, const float z) {
        position() += vec3(x, y, z);
    }

    inline void RotateX(const float angle) {  // Rotate around x-axis. (Pitch)
        float c, s; 
        sincosf(angle * toRad, &s, &c);
        mat4 M(*this);
        m01=c*M.m01+s*M.m02;  m02=-s*M.m01+c*M.m02;  // 1  0  0  0
        m11=c*M.m11+s*M.m12;  m12=-s*M.m11+c*M.m12;  // 0  c -s  0
        m21=c*M.m21+s*M.m22;  m22=-s*M.m21+c*M.m22;  // 0  s  c  0
        m31=c*M.m31+s*M.m32;  m32=-s*M.m31+c*M.m32;  // 0  0  0  1
    }

    inline void RotateY(const float angle) {  // Rotate clockwise around y-axis. (Roll)
        float c, s; 
        sincosf(angle * toRad, &s, &c);
        mat4 M(*this);
        m00=c*M.m00-s*M.m02;  m02=s*M.m00+c*M.m02;  // c  0  s  0
        m10=c*M.m10-s*M.m12;  m12=s*M.m10+c*M.m12;  // 0  1  0  0
        m20=c*M.m20-s*M.m22;  m22=s*M.m20+c*M.m22;  //-s  0  c  0
        m30=c*M.m30-s*M.m32;  m32=s*M.m30+c*M.m32;  // 0  0  0  1
    }

    inline void RotateZ(const float angle) {  // Rotate clockwise around z-axis. (-Heading)
        float c, s; 
        sincosf(angle * toRad, &s, &c);
        mat4 M(*this);
        m00=c*M.m00+s*M.m01;  m01=-s*M.m00+c*M.m01;  // c -s  0  0
        m10=c*M.m10+s*M.m11;  m11=-s*M.m10+c*M.m11;  // s  c  0  0
        m20=c*M.m20+s*M.m21;  m21=-s*M.m20+c*M.m21;  // 0  0  1  0
        m30=c*M.m30+s*M.m31;  m31=-s*M.m30+c*M.m31;  // 0  0  0  1
    }

    //======Quaternion-derived rotation======
    void Rotate(const float angle,const vec3 axis){Rotate(angle,axis.x,axis.y,axis.z);} // http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToMatrix/index.htm
    void Rotate(const float angle, float x, float y, float z) {                // Rotate clockwise around a vector. (see 'glRotatef')
#ifdef FASTER
        const float a = invSqrt(x*x + y*y + z*z); x *= a; y *= a; z *= a;  // Normalize the axis vector
#else
        const float a = 1.f / sqrtf(x*x + y*y + z*z); x *= a; y *= a; z *= a;  // Normalize the axis vector
#endif
        const float Rad = angle*toRad;
        const float c = cosf(Rad);
        const float s = sinf(Rad);
        const float t = 1 - c;
        mat4 M;
        M.m00 = x*x*t + c;    M.m10 = x*y*t - z*s;   M.m20 = x*z*t + y*s;
        M.m01 = y*x*t + z*s;  M.m11 = y*y*t + c;     M.m21 = y*z*t - x*s;
        M.m02 = z*x*t - y*s;  M.m12 = z*y*t + x*s;   M.m22 = z*z*t + c;
        Rotate(M);
    }

    //http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToAngle/index.htm
    void toAxisAngle(vec3 &axis, float &angle) {                              // Convert a rotation matrix to Axis/angle form. (Eigenvector)
        angle = (float)acos((m00 + m11 + m22 - 1) / 2.f) * toDeg;
        vec3 a(m21 - m12, m02 - m20, m10 - m01); a *= a;
        const float s = 1 / (a.x + a.y + a.z);
        axis.x = (m21 - m12) * s;
        axis.y = (m02 - m20) * s;
        axis.z = (m10 - m01) * s;
    }
    //========================================

    inline void Scale(const float s) { Scale(s, s, s); }
    inline void Scale(const float x, const float y, const float z) {
        m00*=x;  m01*=y;  m02*=z;                 // x 0 0 0
        m10*=x;  m11*=y;  m12*=z;                 // 0 y 0 0
        m20*=x;  m21*=y;  m22*=z;                 // 0 0 z 0
    }

    inline void Normalize() {
        xAxis().normalize();
        yAxis().normalize();
        zAxis().normalize();
        row.position4 = {0,0,0,1};
    }

    inline mat4 Transpose() const {
        float f[16] = { m00,m01,m02,m03,  m10,m11,m12,m13,  m20,m21,m22,m23,  m30,m31,m32,m33 };
        return (mat4)f;
    }

    inline void TransposeSelf() {
        float f[16] = { m00,m01,m02,m03,  m10,m11,m12,m13,  m20,m21,m22,m23,  m30,m31,m32,m33 };
        Set(f);
    }

    inline mat4 Transpose3x3() const {  // Transpose 3x3 part of 4x4 matrix
        mat4 R;
        R.xAxis().set(m00, m01, m02);
        R.yAxis().set(m10, m11, m12);
        R.zAxis().set(m20, m21, m22);
        return R;
    }

    inline void Rotate(const mat4& M) {  // Matrix * Matrix_3x3  :Rotate this matrix by the rotation part of matrix M.
        const mat4 t(*this);
        m00 = t.m00*M.m00 + t.m01*M.m10 + t.m02*M.m20;
        m01 = t.m00*M.m01 + t.m01*M.m11 + t.m02*M.m21;
        m02 = t.m00*M.m02 + t.m01*M.m12 + t.m02*M.m22;

        m10 = t.m10*M.m00 + t.m11*M.m10 + t.m12*M.m20;
        m11 = t.m10*M.m01 + t.m11*M.m11 + t.m12*M.m21;
        m12 = t.m10*M.m02 + t.m11*M.m12 + t.m12*M.m22;

        m20 = t.m20*M.m00 + t.m21*M.m10 + t.m22*M.m20;
        m21 = t.m20*M.m01 + t.m21*M.m11 + t.m22*M.m21;
        m22 = t.m20*M.m02 + t.m21*M.m12 + t.m22*M.m22;
    }
 
    mat4 operator * (const mat4& M) const {  // Matrix * Matrix
        float f[16]{
            m00 * M.m00 + m01 * M.m10 + m02 * M.m20 + m03 * M.m30,  // m00
            m10 * M.m00 + m11 * M.m10 + m12 * M.m20 + m13 * M.m30,  // m10
            m20 * M.m00 + m21 * M.m10 + m22 * M.m20 + m23 * M.m30,  // m20
            m30 * M.m00 + m31 * M.m10 + m32 * M.m20 + m33 * M.m30,  // m30

            m00 * M.m01 + m01 * M.m11 + m02 * M.m21 + m03 * M.m31,  // m01
            m10 * M.m01 + m11 * M.m11 + m12 * M.m21 + m13 * M.m31,  // m11
            m20 * M.m01 + m21 * M.m11 + m22 * M.m21 + m23 * M.m31,  // m21
            m30 * M.m01 + m31 * M.m11 + m32 * M.m21 + m33 * M.m31,  // m31

            m00 * M.m02 + m01 * M.m12 + m02 * M.m22 + m03 * M.m32,  // m02
            m10 * M.m02 + m11 * M.m12 + m12 * M.m22 + m13 * M.m32,  // m12
            m20 * M.m02 + m21 * M.m12 + m22 * M.m22 + m23 * M.m32,  // m22
            m30 * M.m02 + m31 * M.m12 + m32 * M.m22 + m33 * M.m32,  // m32

            m00 * M.m03 + m01 * M.m13 + m02 * M.m23 + m03 * M.m33,  // m03
            m10 * M.m03 + m11 * M.m13 + m12 * M.m23 + m13 * M.m33,  // m13
            m20 * M.m03 + m21 * M.m13 + m22 * M.m23 + m23 * M.m33,  // m23
            m30 * M.m03 + m31 * M.m13 + m32 * M.m23 + m33 * M.m33   // m33
        };
        return *((mat4*) f);
    }

/*
    //-----------------Matrix * Matrix using SSE -------------------
    mat4 operator * ( const mat4& M) {  // Matrix * Matrix  USING SSE2
        float f[16] alignas(32);
        __m128 row1 = _mm_load_ps(&m[0]);
        __m128 row2 = _mm_load_ps(&m[4]);
        __m128 row3 = _mm_load_ps(&m[8]);
        __m128 row4 = _mm_load_ps(&m[12]);

        for(int i=0; i<4; ++i){
            __m128 brod1 = _mm_set1_ps(M.m[4*i + 0]);
            __m128 brod2 = _mm_set1_ps(M.m[4*i + 1]);
            __m128 brod3 = _mm_set1_ps(M.m[4*i + 2]);
            __m128 brod4 = _mm_set1_ps(M.m[4*i + 3]);
            __m128 row = _mm_add_ps(
                         _mm_add_ps(
                             _mm_mul_ps(brod1, row1),
                             _mm_mul_ps(brod2, row2)),
                         _mm_add_ps(
                             _mm_mul_ps(brod3, row3),
                             _mm_mul_ps(brod4, row4)
                         ));
            _mm_store_ps(&f[4*i], row);
        }
        return *((mat4*) f);
    }
    //--------------------------------------------------------------
*/
/*
    //-----------------Matrix * Matrix using AVX -------------------
    // dual linear combination using AVX instructions on YMM regs
    static inline __m256 twolincomb_AVX_8(__m256 A01, const mat4 &B) {
        __m256 result;
        result = _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x00), _mm256_broadcast_ps((__m128*)&B.m00));
        result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x55), _mm256_broadcast_ps((__m128*)&B.m01)));
        result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xaa), _mm256_broadcast_ps((__m128*)&B.m02)));
        result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xff), _mm256_broadcast_ps((__m128*)&B.m03)));
        return result;
    }

    mat4 operator * ( const mat4& M) {  // Matrix * Matrix  USING AVX
        float f[16] alignas(32);
        __m256 A01 = _mm256_loadu_ps(&M.m00);
        __m256 A23 = _mm256_loadu_ps(&M.m02);
        __m256 out01x = twolincomb_AVX_8(A01, m);
        __m256 out23x = twolincomb_AVX_8(A23, m);
        _mm256_storeu_ps(&f[0], out01x);
        _mm256_storeu_ps(&f[8], out23x);
        return *((mat4*) f);
    }
    //--------------------------------------------------------------
*/

    inline void operator *= (const mat4& b){ *this = *this * b; }  // mat4 *= mat4

    inline vec3 operator * (const vec3& v) const {  // mat4 * vec3
        return vec3{ m00*v.x + m01*v.y + m02*v.z + m03,
                     m10*v.x + m11*v.y + m12*v.z + m13,
                     m20*v.x + m21*v.y + m22*v.z + m23 };
    }

    inline vec4 operator * (const vec4& v) const {  // mat4 * vec4
        return vec4{ m00*v.x + m01*v.y + m02*v.z + m03,
                     m10*v.x + m11*v.y + m12*v.z + m13,
                     m20*v.x + m21*v.y + m22*v.z + m23,
                     m30*v.x + m31*v.y + m32*v.z + m33 };
    }

    // OpenGL style
    void SetPerspective(float aspect, float fovy, float nearplane, float farplane) {
        float f = 1.f / tanf(fovy * toRad / 2.f);
        float d = nearplane - farplane;
        m00 = -f/aspect;  m01 = 0;        m02 = 0;                        m03 = 0;
        m10 = 0;          m11 = f;        m12 = 0;                        m13 = 0;
        m20 = 0;          m21 = 0;        m22 = (farplane+nearplane)/d;   m23 = (2*farplane*nearplane)/d;
        m30 = 0;          m31 = 0;        m32 = -1;                       m33 = 0;
    }

/*
    // Vulkan style Reverse depth
    void SetPerspective(float aspect, float fovy, float nearplane, float farplane) {
        float f = 1.f / tanf(fovy * toRad / 2.f);
        float d = farplane - nearplane;
        m00 = f/aspect;   m01 = 0;        m02 = 0;                        m03 = 0;
        m10 = 0;          m11 =-f;        m12 = 0;                        m13 = 0;
        m20 = 0;          m21 = 0;        m22 = nearplane/d;              m23 = farplane * m22;
        m30 = 0;          m31 = 0;        m32 = -1;                       m33 = 0;
    }
*/
    void SetOrtho(float left, float right, float bottom, float top, float nearplane, float farplane){
        float w  = right - left;
        float h  = top - bottom;
        float d  = farplane - nearplane;
        float tx = (right + left) / w;
        float ty = (top + bottom) / h;
        float tz = (farplane + nearplane) / d;
        m00=2/w;    m01=0;      m02=0;      m03=-tx;
        m10=0;      m11=2/h;    m12=0;      m13=-ty;
        m20=0;      m21=0;      m22=-2/d;   m23=-tz;
        m30=0;      m31=0;      m32=0;      m33=1;
    }
/*
    void SetCAHV(vec3 center, vec3 axis, vec3 horizontal, vec3 vertical, int width, int height) {
        //printf("Perspective from CAHV\n");
        mat4 translation; //translate camera to the position given by \param "center"
        mat4 changeBasis; //change basis from world space to camera space (A | H' | V')
        mat4 scale;       //scale according to fov angle and farplane (calculated from H')

        //extract focal length params
        float hs = (axis.cross(horizontal)).length();   //horizontal focal length (in pixels) This quantity is also the focal length
        float hc = axis.dot(horizontal);                //center pixel, x axis
        float vs = (axis.cross(vertical)).length();     //vertical focal length (in pixels)
        float vc = axis.dot(vertical);                  //center pixel, y axis
        //printf("Params foc (%f, %f), center (%f, %f)\n", hs, vs, hc, vc);
        //calculate H' and V'
        horizontal = (horizontal - axis * hc) / hs;
        vertical = (vertical - axis * vc) / vs;

        //calculate horizontal/vertical fov (aka tan(theta_h / 2)), theta_h being horizontal fov in radians
        float hfov = (width - hc) / hs;
        float vfov = (height - vc) / vs;
        //set scale matrix
        scale.row.xAxis4 = vec4(1000.f / (hs * hfov), 0.f, 0.f, 0.f);
        scale.row.yAxis4 = vec4(0.f, 1000.f / (hs * vfov), 0.f, 0.f);
        scale.row.zAxis4 = vec4(0.f, 0.f, 1000.f / hs, 0.f);
        scale.row.position4 = vec4(0.f, 0.f, 0.f, 1.f);

        //set basis change matrix
        changeBasis.row.xAxis4 = vec4(horizontal.x, -vertical.x, -axis.x, 0.f);
        changeBasis.row.yAxis4 = vec4(horizontal.y, -vertical.y, -axis.y, 0.f);
        changeBasis.row.zAxis4 = vec4(horizontal.z, -vertical.z, -axis.z, 0.f);
        changeBasis.row.position4 = vec4(0.f, 0.f, 0.f, 1.f);

        //set translation matrix
        translation.row.xAxis4 = vec4(1.f, 0.f, 0.f, 0.f);
        translation.row.yAxis4 = vec4(0.f, 1.f, 0.f, 0.f);
        translation.row.zAxis4 = vec4(0.f, 0.f, 1.f, 0.f);
        translation.row.position4 = vec4(-center, 1);


        *this = changeBasis * translation; //apply the CAHV model and assign it back into the perspective projection matrix
        this->Scale(4.f);
    }
*/
//    void LookAt(vec3 target, vec3 up = { 0, 0, 1 }) {  // z-up
    void LookAt(vec3 target, vec3 up = { 0, 1, 0 }) {  // y-up
        //if(position()==target) return;        // prevent NAN error
        if(position().x==target.x && position().z==target.z) return;  // prevent NAN error
        zAxis() = position() - target;    zAxis().normalize();
        xAxis() = up.cross(zAxis());      xAxis().normalize();
        yAxis() = zAxis().cross(xAxis()); yAxis().normalize();
    }

    // Look at direction (direction must be normailzed)
    void Look(vec3 direction, vec3 up = { 0, 1, 0 }) {        // y-up
        zAxis() = - direction;        //zAxis.normalize();      // assume direction is normalized
        if(fabs(zAxis().y) < 0.99f)                             // prevent NAN if fwd==up/down
            {xAxis() = up.cross(zAxis()); xAxis().normalize_fast();}
        yAxis() = zAxis().cross(xAxis()); yAxis().normalize_fast();
    }

    void Print() {  // for debugging
        printf("%9f %9f %9f %9f\n", m00,m10,m20,m30);
        printf("%9f %9f %9f %9f\n", m01,m11,m21,m31);
        printf("%9f %9f %9f %9f\n", m02,m12,m22,m32);
        printf("%9f %9f %9f %9f\n", m03,m13,m23,m33);
        printf(" \n");
    }

    inline mat4 Inverse() const {
        mat4 R;
        float s[6];
        float c[6];
        s[0] = m00*m11 - m10*m01;
        s[1] = m00*m12 - m10*m02;
        s[2] = m00*m13 - m10*m03;
        s[3] = m01*m12 - m11*m02;
        s[4] = m01*m13 - m11*m03;
        s[5] = m02*m13 - m12*m03;

        c[0] = m20*m31 - m30*m21;
        c[1] = m20*m32 - m30*m22;
        c[2] = m20*m33 - m30*m23;
        c[3] = m21*m32 - m31*m22;
        c[4] = m21*m33 - m31*m23;
        c[5] = m22*m33 - m32*m23;

        float idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );

        R.m00 = ( m11 * c[5] - m12 * c[4] + m13 * c[3]) * idet;
        R.m01 = (-m01 * c[5] + m02 * c[4] - m03 * c[3]) * idet;
        R.m02 = ( m31 * s[5] - m32 * s[4] + m33 * s[3]) * idet;
        R.m03 = (-m21 * s[5] + m22 * s[4] - m23 * s[3]) * idet;

        R.m10 = (-m10 * c[5] + m12 * c[2] - m13 * c[1]) * idet;
        R.m11 = ( m00 * c[5] - m02 * c[2] + m03 * c[1]) * idet;
        R.m12 = (-m30 * s[5] + m32 * s[2] - m33 * s[1]) * idet;
        R.m13 = ( m20 * s[5] - m22 * s[2] + m23 * s[1]) * idet;

        R.m20 = ( m10 * c[4] - m11 * c[2] + m13 * c[0]) * idet;
        R.m21 = (-m00 * c[4] + m01 * c[2] - m03 * c[0]) * idet;
        R.m22 = ( m30 * s[4] - m31 * s[2] + m33 * s[0]) * idet;
        R.m23 = (-m20 * s[4] + m21 * s[2] - m23 * s[0]) * idet;

        R.m30 = (-m10 * c[3] + m11 * c[1] - m12 * c[0]) * idet;
        R.m31 = ( m00 * c[3] - m01 * c[1] + m02 * c[0]) * idet;
        R.m32 = (-m30 * s[3] + m31 * s[1] - m32 * s[0]) * idet;
        R.m33 = ( m20 * s[3] - m21 * s[1] + m22 * s[0]) * idet;

        return R;
    }

    inline mat4 Inverse_fast() const {  // Assumes matrix is orthogonal
        mat4 R = Transpose3x3();
        vec3 pos = R * row.position4;
        R.row.position4 = -pos;
        return R;
    }

    inline mat4 WorldToView() const {
        mat4 R = Transpose3x3();
        R.xAxis().normalize();
        R.yAxis().normalize();
        R.zAxis().normalize();
        vec3 pos = R * row.position4;
        R.row.position4 = -pos;
        return R;
    }

    void from_quat(quat q) {  // convert quaternion to matrix (untested.. may need transpose)
        float w = q.w;
        float x = q.x;
        float y = q.y;
        float z = q.z;
        float w2 = w * w;
        float x2 = x * x;
        float y2 = y * y;
        float z2 = z * z;

        m00 = w2 + x2 - y2 - z2;
        m01 = 2.f * (x*y + w*z);
        m02 = 2.f * (x*z - w*y);
        m03 = 0.f;

        m10 = 2.f * (x*y - w*z);
        m11 = w2 - x2 + y2 - z2;
        m12 = 2.f * (y*z + w*x);
        m13 = 0.f;

        m20 = 2.f * (x*z + w*y);
        m21 = 2.f * (y*z - w*x);
        m22 = w2 - x2 - y2 + z2;
        m23 = 0.f;

        m30 = m31 = m32 = 0.f;
        m33 = 1.f;

        //*this = Transpose();
    }
};

//---------------------------------------------------------
//--------------------------dmat4--------------------------

static const double D_Identity4x4[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

class dmat4 {
public:
    union {
        double m[16];
        struct {
            double m00, m10, m20, m30;  // x axis       ,0
            double m01, m11, m21, m31;  // y axis       ,0
            double m02, m12, m22, m32;  // z axis       ,0
            double m03, m13, m23, m33;  // translation  ,ScaleInverse
        };
        struct {
            dvec4 xAxis4;
            dvec4 yAxis4;
            dvec4 zAxis4;
            dvec4 position4;
        } row;
    };

    void Set(const double* d){ memcpy(m, d, sizeof(m)); }
    dvec3& xAxis()    {return *(dvec3*)&m00;}
    dvec3& yAxis()    {return *(dvec3*)&m01;}
    dvec3& zAxis()    {return *(dvec3*)&m02;}
    dvec3& position() {return *(dvec3*)&m03;}

    double& operator [] (uint i) { return m[i]; }                                       // array operator
    dmat4& operator = (const mat4& other){ repeat(16) m[i]=other.m[i]; return *this; }  // mat4 to dmat4
    //operator mat4 () { mat4 f; repeat(16) f.m[i]=m[i];  return f; }                     // dmat4 to mat4  (compiler bug)

    operator mat4 () {
        mat4 f;
        f.row.xAxis4    = row.xAxis4;
        f.row.yAxis4    = row.yAxis4;
        f.row.zAxis4    = row.zAxis4;
        f.row.position4 = row.position4;
        return f;
    }

    inline bool isIdentity(){ return memcmp(this, D_Identity4x4, 128)==0; }
    inline void SetIdentity(){Set(D_Identity4x4);}  // Reset this matrix to Identity
    inline void Clear()      {Set(D_Identity4x4);}  // Reset this matrix to Identity

    dmat4(){ SetIdentity(); }                       // default constructor
    dmat4(double* d) { Set(&(*d)); }                // convert double[16] to dmat4
    dmat4(quat q) { from_quat(q);}                  // convert quaternion to dmat4
    //mat4(euler e) { SetRotation(e.roll, e.pitch, e.yaw); }

    //dmat4(const dmat4& other) { Set(other.m); }                            // copy constructor
    //dmat4& operator = (const dmat4& other){ Set(other.m); return *this; }  // copy assignment operator

    operator euler () {return Euler();}
    euler Euler() { return euler(-zAxis(), yAxis()); } // Get Euler angles from matrix (y-up)

    void SetRotation(double roll, double pitch, double yaw) {             // Apply euler rotations.
        dvec3 pos = position();
        Clear();
        RotateY(yaw);
        RotateX(pitch);
        RotateZ(roll);
        position() = pos;
    }

#ifdef _WIN32
    inline void sincos(double ang, double* s, double* c) {
        *s = sin(ang);
        *c = cos(ang);
    }
#endif

    inline void Translate(const double x, const double y, const double z) {
        position() += dvec3(x, y, z);
    }

    inline void RotateX(const double angle) {  // Rotate around x-axis. (Pitch)
        double c, s;
        sincos(angle * toRAD, &s, &c);
        dmat4 M(*this);
        m01=c*M.m01+s*M.m02;  m02=-s*M.m01+c*M.m02;  // 1  0  0  0
        m11=c*M.m11+s*M.m12;  m12=-s*M.m11+c*M.m12;  // 0  c -s  0
        m21=c*M.m21+s*M.m22;  m22=-s*M.m21+c*M.m22;  // 0  s  c  0
        m31=c*M.m31+s*M.m32;  m32=-s*M.m31+c*M.m32;  // 0  0  0  1
    }

    inline void RotateY(const double angle) {  // Rotate clockwise around y-axis. (Roll)
        double c, s;
        sincos(angle * toRAD, &s, &c);
        dmat4 M(*this);
        m00=c*M.m00-s*M.m02;  m02=s*M.m00+c*M.m02;  // c  0  s  0
        m10=c*M.m10-s*M.m12;  m12=s*M.m10+c*M.m12;  // 0  1  0  0
        m20=c*M.m20-s*M.m22;  m22=s*M.m20+c*M.m22;  //-s  0  c  0
        m30=c*M.m30-s*M.m32;  m32=s*M.m30+c*M.m32;  // 0  0  0  1
    }

    inline void RotateZ(const double angle) {  // Rotate clockwise around z-axis. (-Yaw)
        double c, s;
        sincos(angle * toRAD, &s, &c);
        dmat4 M(*this);
        m00=c*M.m00+s*M.m01;  m01=-s*M.m00+c*M.m01;  // c -s  0  0
        m10=c*M.m10+s*M.m11;  m11=-s*M.m10+c*M.m11;  // s  c  0  0
        m20=c*M.m20+s*M.m21;  m21=-s*M.m20+c*M.m21;  // 0  0  1  0
        m30=c*M.m30+s*M.m31;  m31=-s*M.m30+c*M.m31;  // 0  0  0  1
    }

    //======Quaternion-derived rotation======
    void Rotate(const double angle,const dvec3 axis){Rotate(angle,axis.x,axis.y,axis.z);} // http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToMatrix/index.htm
    void Rotate(const double angle, double x, double y, double z) {                // Rotate clockwise around a vector. (see 'glRotatef')
        const double a = 1.0 / sqrt(x*x + y*y + z*z); x *= a; y *= a; z *= a;  // Normalize the axis vector
        const double rad = angle*toRAD;
        const double c = cos(rad);
        const double s = sin(rad);
        const double t = 1 - c;
        dmat4 M;
        M.m00 = x*x*t + c;    M.m10 = x*y*t - z*s;   M.m20 = x*z*t + y*s;
        M.m01 = y*x*t + z*s;  M.m11 = y*y*t + c;     M.m21 = y*z*t - x*s;
        M.m02 = z*x*t - y*s;  M.m12 = z*y*t + x*s;   M.m22 = z*z*t + c;
        Rotate(M);
    }

    //http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToAngle/index.htm
    void toAxisAngle(dvec3 &axis, double &angle) {                              // Convert a rotation matrix to Axis/angle form. (Eigenvector)
        angle = acos((m00 + m11 + m22 - 1) / 2.0) * toDEG;
        dvec3 a(m21 - m12, m02 - m20, m10 - m01); a *= a;
        const double s = 1 / (a.x + a.y + a.z);
        axis.x = (m21 - m12) * s;
        axis.y = (m02 - m20) * s;
        axis.z = (m10 - m01) * s;
    }
    //========================================


    inline void Scale(const double s) { Scale(s, s, s); }
    inline void Scale(const double x, const double y, const double z) {
        m00*=x;  m01*=y;  m02*=z;                 // x 0 0 0
        m10*=x;  m11*=y;  m12*=z;                 // 0 y 0 0
        m20*=x;  m21*=y;  m22*=z;                 // 0 0 z 0
    }

    inline void Normalize() {
        xAxis().normalize();
        yAxis().normalize();
        zAxis().normalize();
        row.position4 = {0,0,0,1};
    }

    inline dmat4 Transpose() const {
        double d[16] = { m00,m01,m02,m03,  m10,m11,m12,m13,  m20,m21,m22,m23,  m30,m31,m32,m33 };
        return (dmat4)d;
    }

    inline void TransposeSelf() {
        double d[16] = { m00,m01,m02,m03,  m10,m11,m12,m13,  m20,m21,m22,m23,  m30,m31,m32,m33 };
        Set(d);
    }

    inline dmat4 Transpose3x3() const {  // Transpose 3x3 part of 4x4 matrix
        dmat4 R;
        R.xAxis().set(m00, m01, m02);
        R.yAxis().set(m10, m11, m12);
        R.zAxis().set(m20, m21, m22);
        return R;
    }

    inline void Rotate(const dmat4& M) {  // Matrix * Matrix_3x3  :Rotate this matrix by the rotation part of matrix M.
        const dmat4 t(*this);
        m00 = t.m00*M.m00 + t.m01*M.m10 + t.m02*M.m20;
        m01 = t.m00*M.m01 + t.m01*M.m11 + t.m02*M.m21;
        m02 = t.m00*M.m02 + t.m01*M.m12 + t.m02*M.m22;

        m10 = t.m10*M.m00 + t.m11*M.m10 + t.m12*M.m20;
        m11 = t.m10*M.m01 + t.m11*M.m11 + t.m12*M.m21;
        m12 = t.m10*M.m02 + t.m11*M.m12 + t.m12*M.m22;

        m20 = t.m20*M.m00 + t.m21*M.m10 + t.m22*M.m20;
        m21 = t.m20*M.m01 + t.m21*M.m11 + t.m22*M.m21;
        m22 = t.m20*M.m02 + t.m21*M.m12 + t.m22*M.m22;
    }

    dmat4 operator * (const dmat4& M) const {  // Matrix * Matrix
        double d[16] {
            m00 * M.m00 + m01 * M.m10 + m02 * M.m20 + m03 * M.m30,  // m00
            m10 * M.m00 + m11 * M.m10 + m12 * M.m20 + m13 * M.m30,  // m10
            m20 * M.m00 + m21 * M.m10 + m22 * M.m20 + m23 * M.m30,  // m20
            m30 * M.m00 + m31 * M.m10 + m32 * M.m20 + m33 * M.m30,  // m30

            m00 * M.m01 + m01 * M.m11 + m02 * M.m21 + m03 * M.m31,  // m01
            m10 * M.m01 + m11 * M.m11 + m12 * M.m21 + m13 * M.m31,  // m11
            m20 * M.m01 + m21 * M.m11 + m22 * M.m21 + m23 * M.m31,  // m21
            m30 * M.m01 + m31 * M.m11 + m32 * M.m21 + m33 * M.m31,  // m31

            m00 * M.m02 + m01 * M.m12 + m02 * M.m22 + m03 * M.m32,  // m02
            m10 * M.m02 + m11 * M.m12 + m12 * M.m22 + m13 * M.m32,  // m12
            m20 * M.m02 + m21 * M.m12 + m22 * M.m22 + m23 * M.m32,  // m22
            m30 * M.m02 + m31 * M.m12 + m32 * M.m22 + m33 * M.m32,  // m32

            m00 * M.m03 + m01 * M.m13 + m02 * M.m23 + m03 * M.m33,  // m03
            m10 * M.m03 + m11 * M.m13 + m12 * M.m23 + m13 * M.m33,  // m13
            m20 * M.m03 + m21 * M.m13 + m22 * M.m23 + m23 * M.m33,  // m23
            m30 * M.m03 + m31 * M.m13 + m32 * M.m23 + m33 * M.m33   // m33
        };
        return *((dmat4*) d);
    }

    inline void operator *= (const dmat4& b){ *this = *this * b; }  // dmat4 *= dmat4

    inline dvec3 operator * (const dvec3& v) const {  // dmat4 * dvec3
        return dvec3{ m00*v.x + m01*v.y + m02*v.z + m03,
                      m10*v.x + m11*v.y + m12*v.z + m13,
                      m20*v.x + m21*v.y + m22*v.z + m23 };
    }

    inline dvec4 operator * (const dvec4& v) const {  // dmat4 * dvec4
        return dvec4{ m00*v.x + m01*v.y + m02*v.z + m03,
                      m10*v.x + m11*v.y + m12*v.z + m13,
                      m20*v.x + m21*v.y + m22*v.z + m23,
                      m30*v.x + m31*v.y + m32*v.z + m33 };
    }

    void SetPerspective(double aspect, double fovy, double nearplane, double farplane) {
        double f = 1.0 / tan(fovy * toRAD / 2.0);
        double d = nearplane - farplane;
        m00 = -f/aspect;  m01 = 0;        m02 = 0;                        m03 = 0;
        m10 = 0;          m11 = f;        m12 = 0;                        m13 = 0;
        m20 = 0;          m21 = 0;        m22 = (farplane+nearplane)/d;   m23 = (2*farplane*nearplane)/d;
        m30 = 0;          m31 = 0;        m32 = -1;                       m33 = 0;
    }

    void SetOrtho(double left, double right, double bottom, double top, double nearplane, double farplane){
        double w  = right - left;
        double h  = top - bottom;
        double d  = farplane - nearplane;
        double tx = (right + left) / w;
        double ty = (top + bottom) / h;
        double tz = (farplane + nearplane) / d;
        m00=2/w;    m01=0;      m02=0;      m03=-tx;
        m10=0;      m11=2/h;    m12=0;      m13=-ty;
        m20=0;      m21=0;      m22=-2/d;   m23=-tz;
        m30=0;      m31=0;      m32=0;      m33=1;
    }
/*
    void SetCAHV(vec3 center, vec3 axis, vec3 horizontal, vec3 vertical, int width, int height) {
        mat4 m;
        m.SetCAHV(center, axis, horizontal, vertical, width, height);
        *this = m;
    }
*/
    void LookAt(dvec3 target, dvec3 up = { 0, 1, 0 }) {  // y-up
        //if(position()==target) return;        // prevent NAN error
        if(position().x==target.x && position().z==target.z) return;  // prevent NAN error
        zAxis() = position() - target;    zAxis().normalize();
        xAxis() = up.cross(zAxis());      xAxis().normalize();
        yAxis() = zAxis().cross(xAxis()); yAxis().normalize();
    }

    // Look at direction (direction must be normailzed)
    void Look(dvec3 direction, dvec3 up = { 0, 1, 0 }) {        // y-up
        zAxis() = - direction;        //zAxis.normalize();      // assume direction is normalized
        if(fabs(zAxis().y) < 0.99f)                             // prevent NAN if fwd==up/down
            {xAxis() = up.cross(zAxis()); xAxis().normalize();}
        yAxis() = zAxis().cross(xAxis()); yAxis().normalize();
    }

    void Print() {  // for debugging
        printf("%9f %9f %9f %9f\n", m00,m10,m20,m30);
        printf("%9f %9f %9f %9f\n", m01,m11,m21,m31);
        printf("%9f %9f %9f %9f\n", m02,m12,m22,m32);
        printf("%9f %9f %9f %9f\n", m03,m13,m23,m33);
        printf(" \n");
    }

    inline dmat4 Inverse_fast() const {  // Assumes matrix is orthogonal
        dmat4 R = Transpose3x3();
        dvec3 pos = R * row.position4;
        R.position() = -pos;
        return R;
    }

    inline dmat4 WorldToView() const {  // Inverse and normalize
        dmat4 R = Transpose3x3();
        R.xAxis().normalize();
        R.yAxis().normalize();
        R.zAxis().normalize();
        dvec3 pos = R * row.position4;
        R.row.position4 = -pos;
        return R;
    }

    void from_quat(quat q) {  // convert quaternion to matrix (untested.. may need transpose)
        double w = q.w;
        double x = q.x;
        double y = q.y;
        double z = q.z;
        double w2 = w * w;
        double x2 = x * x;
        double y2 = y * y;
        double z2 = z * z;

        m00 = w2 + x2 - y2 - z2;
        m01 = 2.f * (x*y + w*z);
        m02 = 2.f * (x*z - w*y);
        m03 = 0.f;

        m10 = 2.f * (x*y - w*z);
        m11 = w2 - x2 + y2 - z2;
        m12 = 2.f * (y*z + w*x);
        m13 = 0.f;

        m20 = 2.f * (x*z + w*y);
        m21 = 2.f * (y*z - w*x);
        m22 = w2 - x2 - y2 + z2;
        m23 = 0.f;

        m30 = m31 = m32 = 0.f;
        m33 = 1.f;
    }

};

//---------------------------------------------------------

#endif
