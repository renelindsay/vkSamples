#ifndef JCLASS_H
#define JCLASS_H

#include <jni.h>
#include <string>
#include <vector>
//#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include "native.h"  // TODO: Move Android_App to here.

class JClass {
    bool attached = false;
public:
    JavaVM* jvm=0;
    JNIEnv* env=0;
    jobject activity_obj=0;
    jclass  cls=0;
    jobject obj=0;

    jobject object() const { return obj; }
    JClass() { Init(Android_App->activity); }

    void Init(ANativeActivity* activity) {
        if (env) return;
        jvm = activity->vm;
        env = activity->env;
        activity_obj = activity->clazz;
        if((jvm->GetEnv((void**)&env,JNI_VERSION_1_6)!=JNI_OK)) {  // if not attached:
            jvm->AttachCurrentThread(&env, nullptr);               // attach
            attached = true;                                       // and flag for detach
        }
    }

    virtual ~JClass() {
        if(obj) env->DeleteLocalRef(obj);
        if(cls) env->DeleteLocalRef(cls);
        if(attached) jvm->DetachCurrentThread();  // if this obj attached, then detach
        attached = false;
    }

    jclass FindClass(const char* class_name) {  // using local ref
        cls = env->FindClass(class_name);
        return cls;
    }

    //jmethodID MethodID(const char* name, const char* sig) {
    //    return env->GetMethodID(cls, name, sig);
    //}


    // ----------------- jstring to std::string -----------------
    std::string asString(jstring jStr) const {
        if (!jStr) return "";
        const char* chars = env->GetStringUTFChars(jStr, nullptr);
        std::string result(chars);
        env->ReleaseStringUTFChars(jStr, chars);
        return result;
    }
    std::string asString(jobject jStr) const {return asString((jstring)jStr);}
    //-----------------------------------------------------------
    //-------------- jintArray to std::vector<int> --------------
    std::vector<int> asIntVector(jintArray intArray) const { // Convert Java int[] to std::vector<int>
        jsize length = env->GetArrayLength(intArray);
        std::vector<int> array(length);
        jint *elements = env->GetIntArrayElements(intArray, nullptr);
        for (jsize i=0; i<length; ++i) array[i] = elements[i];
        env->ReleaseIntArrayElements(intArray, elements, JNI_ABORT);
        return array;
    }
    std::vector<int> asIntVector(jobject intArray) const {return asIntVector((jintArray)intArray);}
    //-----------------------------------------------------------
    /*
    //-------------- std::vector<int> to jintArray --------------  // not used
    jintArray asJintArray(std::vector<int> intvec) const {
        jintArray array = env->NewIntArray(intvec.size());
        if(array) env->SetIntArrayRegion(array, 0, intvec.size(), intvec.data());
        return array;
    }
    //-----------------------------------------------------------
     */
    //------------------- int to jintArray[1] -------------------
    jintArray asJintArray(int i) const {
        jintArray array = env->NewIntArray(1);
        env->SetIntArrayRegion(array, 0, 1, &i);
        return array;
    }
    //-----------------------------------------------------------
    /*
    //----------- jbooleanArray to std::vector<bool> ------------  // not used
    std::vector<bool> asBoolVector(jbooleanArray array) {
        jsize length = env->GetArrayLength(array);
        std::vector<bool> result(length);
        jboolean* elements = env->GetBooleanArrayElements(array, nullptr);
        for (jsize i = 0; i < length; ++i) {
            result[i] = elements[i] == JNI_TRUE;
        }
        env->ReleaseBooleanArrayElements(array, elements, JNI_ABORT);
        return result;
    }
    std::vector<bool> asBoolVector(jobject array) {return asBoolVector((jbooleanArray)array);}
    //-----------------------------------------------------------
    */
    //---------------- jbooleanArray[1] to bool -----------------
    bool asBool(jbooleanArray array) {
        jboolean result = JNI_FALSE;
        env->GetBooleanArrayRegion(array, 0, 1, &result);
        return result == JNI_TRUE;
    }
    bool asBool(jobject boolArray) { return asBool((jbooleanArray)boolArray); }
    //-----------------------------------------------------------

};
//------------------------------------------------------------------------------

class JInputManager : public JClass {
    jclass cls = FindClass("android/hardware/input/InputManager");
    jmethodID m_getInputDeviceIds = env->GetMethodID(cls, "getInputDeviceIds", "()[I");
public:
    JInputManager(jobject inputManagerObj) { obj = inputManagerObj; }
    std::vector<int>  getInputDeviceIds() const { return asIntVector(env->CallObjectMethod(obj, m_getInputDeviceIds)); }
};

class JContext : public JClass {
    jclass cls = FindClass("android/content/Context");
    jfieldID  m_INPUT_SERVICE    = env->GetStaticFieldID(cls, "INPUT_SERVICE", "Ljava/lang/String;");
    jmethodID m_getSystemService = env->GetMethodID(cls, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
public:
    jstring INPUT_SERVICE() const { return (jstring)env->GetStaticObjectField(cls, m_INPUT_SERVICE); }
    JInputManager getSystemService(jstring jstr) const { return {env->CallObjectMethod(activity_obj, m_getSystemService, jstr)}; }
};

//------------------------------------------------------------------------------

class JMotionRange : public JClass {
    jclass cls = FindClass("android/view/InputDevice$MotionRange");
    jmethodID m_getAxis      = env->GetMethodID(cls, "getAxis", "()I");
    jmethodID m_getSource    = env->GetMethodID(cls, "getSource", "()I");
    jmethodID m_getMin       = env->GetMethodID(cls, "getMin", "()F");
    jmethodID m_getMax       = env->GetMethodID(cls, "getMax", "()F");
    jmethodID m_getRange     = env->GetMethodID(cls, "getRange", "()F");
    jmethodID m_getFlat      = env->GetMethodID(cls, "getFlat", "()F");
    jmethodID m_getFuzz      = env->GetMethodID(cls, "getFuzz", "()F");
    jmethodID m_getResolution= env->GetMethodID(cls, "getResolution", "()F");
public:
    JMotionRange(jobject motionRangeObj) { obj = motionRangeObj; }
    int getAxis()        const { return env->CallIntMethod(obj, m_getAxis); }
    int getSource()      const { return env->CallIntMethod(obj, m_getSource); }
    float getMin()       const { return env->CallFloatMethod(obj, m_getMin); }
    float getMax()       const { return env->CallFloatMethod(obj, m_getMax); }
    float getRange()     const { return env->CallFloatMethod(obj, m_getRange); }
    float getFlat()      const { return env->CallFloatMethod(obj, m_getFlat); }
    float getFuzz()      const { return env->CallFloatMethod(obj, m_getFuzz); }
    float getResolution()const { return env->CallFloatMethod(obj, m_getResolution); }
};

template<typename T>
class JList : public JClass {
    jclass  cls = FindClass("java/util/List");
    jmethodID m_size = env->GetMethodID(cls, "size", "()I");
    jmethodID m_get  = env->GetMethodID(cls, "get", "(I)Ljava/lang/Object;");
public:
    JList(jobject listObj) { obj = listObj; }
    jint size() { return env->CallIntMethod(obj, m_size); }
    T get(int i)  { {return env->CallObjectMethod(obj, m_get, i);} }

    //std::vector<T> asVector() {  // convert to std::vector<>
    //    std::vector<T> items;
    //    for(int i=0; i<size(); ++i) {items.emplace_back(get(i));}
    //    return items;
    //}
};

class JInputDevice : public JClass {
    jclass  cls = FindClass("android/view/InputDevice");
    jmethodID m_getDevice      = env->GetStaticMethodID(cls, "getDevice", "(I)Landroid/view/InputDevice;");
    jmethodID m_getName        = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
    jmethodID m_getVendorId    = env->GetMethodID(cls, "getVendorId", "()I");
    jmethodID m_getProductId   = env->GetMethodID(cls, "getProductId", "()I");
  //jmethodID m_getSources     = env->GetMethodID(cls, "getSources", "()I");
  //jmethodID m_getKeyboardType= env->GetMethodID(cls, "getKeyboardType", "()I");
    jmethodID m_getMotionRange = env->GetMethodID(cls, "getMotionRange", "(I)Landroid/view/InputDevice$MotionRange;");
    jmethodID m_getMotionRanges= env->GetMethodID(cls, "getMotionRanges", "()Ljava/util/List;");
    jmethodID m_hasKeys        = env->GetMethodID(cls, "hasKeys", "([I)[Z");
public:
    JInputDevice(int deviceId) { obj = env->CallStaticObjectMethod(cls, m_getDevice, deviceId); }

    std::string getName()                     { return asString(env->CallObjectMethod(obj, m_getName)); }
    int getVendorId()                         { return env->CallIntMethod(obj, m_getVendorId); }
    int getProductId()                        { return env->CallIntMethod(obj, m_getProductId); }
  //int getSources()                          { return env->CallIntMethod(obj, m_getSources); }
  //int getKeyboardType()                     { return env->CallIntMethod(obj, m_getKeyboardType); }
    JMotionRange getMotionRange(int axis)     { return {env->CallObjectMethod(obj, m_getMotionRange, axis)}; }
    JList<JMotionRange> getMotionRanges()     { return {env->CallObjectMethod(obj, m_getMotionRanges)}; }
    //jobject hasKeys(jintArray keyCodes)       { return env->CallObjectMethod(obj, m_hasKeys, keyCodes);}
    //jbooleanArray hasKeys(std::vector<int> keyCodes){ return (jbooleanArray)env->CallObjectMethod(obj, m_hasKeys, asJintArray(keyCodes));}
    //std::vector<bool> hasKeys(std::vector<int> keyCodes){ return asBoolVector(env->CallObjectMethod(obj, m_hasKeys, asJintArray(keyCodes)));}
    bool hasKey(int keyCode) {return asBool(env->CallObjectMethod(obj, m_hasKeys, asJintArray(keyCode)));}
};

#endif
