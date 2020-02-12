/****************************************************************************
Copyright (c) 2010 cocos2d-x.org
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "base/CCDirector.h"
#include "base/CCEventKeyboard.h"
#include "base/CCEventDispatcher.h"
#include "platform/android/CCGLViewImpl-android.h"

#include <android/log.h>
#include <jni.h>

using namespace cocos2d;

extern "C" {
    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeTouchesBegin(JNIEnv * env, jobject thiz, jint id, jfloat x, jfloat y) {
        intptr_t idlong = id;
        cocos2d::Director::getInstance()->getOpenGLView()->handleTouchesBegin(1, &idlong, &x, &y);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeTouchesEnd(JNIEnv * env, jobject thiz, jint id, jfloat x, jfloat y) {
        intptr_t idlong = id;
        cocos2d::Director::getInstance()->getOpenGLView()->handleTouchesEnd(1, &idlong, &x, &y);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeTouchesMove(JNIEnv * env, jobject thiz, jintArray ids, jfloatArray xs, jfloatArray ys) {
        int size = env->GetArrayLength(ids);
        jint id[size];
        jfloat x[size];
        jfloat y[size];

        env->GetIntArrayRegion(ids, 0, size, id);
        env->GetFloatArrayRegion(xs, 0, size, x);
        env->GetFloatArrayRegion(ys, 0, size, y);

        intptr_t idlong[size];
        for(int i = 0; i < size; i++)
            idlong[i] = id[i];

        cocos2d::Director::getInstance()->getOpenGLView()->handleTouchesMove(size, idlong, x, y);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeTouchesCancel(JNIEnv * env, jobject thiz, jintArray ids, jfloatArray xs, jfloatArray ys) {
        int size = env->GetArrayLength(ids);
        jint id[size];
        jfloat x[size];
        jfloat y[size];

        env->GetIntArrayRegion(ids, 0, size, id);
        env->GetFloatArrayRegion(xs, 0, size, x);
        env->GetFloatArrayRegion(ys, 0, size, y);

        intptr_t idlong[size];
        for(int i = 0; i < size; i++)
            idlong[i] = id[i];

        cocos2d::Director::getInstance()->getOpenGLView()->handleTouchesCancel(size, idlong, x, y);
    }

#define KEYCODE_BACK 0x04
#define KEYCODE_MENU 0x52
#define KEYCODE_DPAD_UP 0x13
#define KEYCODE_DPAD_DOWN 0x14
#define KEYCODE_DPAD_LEFT 0x15
#define KEYCODE_DPAD_RIGHT 0x16
#define KEYCODE_ENTER 0x42
#define KEYCODE_PLAY  0x7e
#define KEYCODE_DPAD_CENTER  0x17
    
    
    static std::unordered_map<int, cocos2d::EventKeyboard::KeyCode> g_keyCodeMap = {
        { KEYCODE_BACK , cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE},
        { KEYCODE_MENU , cocos2d::EventKeyboard::KeyCode::KEY_MENU},
        { KEYCODE_DPAD_UP  , cocos2d::EventKeyboard::KeyCode::KEY_DPAD_UP },
        { KEYCODE_DPAD_DOWN , cocos2d::EventKeyboard::KeyCode::KEY_DPAD_DOWN },
        { KEYCODE_DPAD_LEFT , cocos2d::EventKeyboard::KeyCode::KEY_DPAD_LEFT },
        { KEYCODE_DPAD_RIGHT , cocos2d::EventKeyboard::KeyCode::KEY_DPAD_RIGHT },
        { KEYCODE_ENTER  , cocos2d::EventKeyboard::KeyCode::KEY_ENTER},
        { KEYCODE_PLAY  , cocos2d::EventKeyboard::KeyCode::KEY_PLAY},
        { KEYCODE_DPAD_CENTER  , cocos2d::EventKeyboard::KeyCode::KEY_DPAD_CENTER},
        
        // 0-9
        { 7,  static_cast<EventKeyboard::KeyCode>(76)},
        { 8,  static_cast<EventKeyboard::KeyCode>(77)},
        { 9,  static_cast<EventKeyboard::KeyCode>(78)},
        { 10, static_cast<EventKeyboard::KeyCode>(79)},
        { 11, static_cast<EventKeyboard::KeyCode>(80)},
        { 12, static_cast<EventKeyboard::KeyCode>(81)},
        { 13, static_cast<EventKeyboard::KeyCode>(82)},
        { 14, static_cast<EventKeyboard::KeyCode>(83)},
        { 15, static_cast<EventKeyboard::KeyCode>(84)},
        { 16, static_cast<EventKeyboard::KeyCode>(85)},
        // a-z
        { 29, static_cast<EventKeyboard::KeyCode>(124)},
        { 30, static_cast<EventKeyboard::KeyCode>(125)},
        { 31, static_cast<EventKeyboard::KeyCode>(126)},
        { 32, static_cast<EventKeyboard::KeyCode>(127)},
        { 33, static_cast<EventKeyboard::KeyCode>(128)},
        { 34, static_cast<EventKeyboard::KeyCode>(129)},
        { 35, static_cast<EventKeyboard::KeyCode>(130)},
        { 36, static_cast<EventKeyboard::KeyCode>(131)},
        { 37, static_cast<EventKeyboard::KeyCode>(132)},
        { 38, static_cast<EventKeyboard::KeyCode>(133)},
        { 39, static_cast<EventKeyboard::KeyCode>(134)},
        { 40, static_cast<EventKeyboard::KeyCode>(135)},
        { 41, static_cast<EventKeyboard::KeyCode>(136)},
        { 42, static_cast<EventKeyboard::KeyCode>(137)},
        { 43, static_cast<EventKeyboard::KeyCode>(138)},
        { 44, static_cast<EventKeyboard::KeyCode>(139)},
        { 45, static_cast<EventKeyboard::KeyCode>(140)},
        { 46, static_cast<EventKeyboard::KeyCode>(141)},
        { 47, static_cast<EventKeyboard::KeyCode>(142)},
        { 48, static_cast<EventKeyboard::KeyCode>(143)},
        { 49, static_cast<EventKeyboard::KeyCode>(144)},
        { 50, static_cast<EventKeyboard::KeyCode>(145)},
        { 51, static_cast<EventKeyboard::KeyCode>(146)},
        { 52, static_cast<EventKeyboard::KeyCode>(147)},
        { 53, static_cast<EventKeyboard::KeyCode>(148)},
        { 54, static_cast<EventKeyboard::KeyCode>(149)},
        // special
        { 57, EventKeyboard::KeyCode::KEY_ALT},
        { 58, EventKeyboard::KeyCode::KEY_ALT},
        { 59, EventKeyboard::KeyCode::KEY_SHIFT},
        { 60, EventKeyboard::KeyCode::KEY_SHIFT},
        { 61, EventKeyboard::KeyCode::KEY_TAB},
        { 62, EventKeyboard::KeyCode::KEY_SPACE},
        //{ 68, EventKeyboard::KeyCode::KEY_ESCAPE},// use back for esc
        { 113, EventKeyboard::KeyCode::KEY_CTRL},
        { 114, EventKeyboard::KeyCode::KEY_CTRL},
    };
    
    JNIEXPORT jboolean JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeKeyEvent(JNIEnv * env, jobject thiz, jint keyCode, jboolean isPressed) {        
        auto iterKeyCode = g_keyCodeMap.find(keyCode);
        if (iterKeyCode == g_keyCodeMap.end()) {
            return JNI_FALSE;
        }
        
        cocos2d::EventKeyboard::KeyCode cocos2dKey = g_keyCodeMap.at(keyCode);
        cocos2d::EventKeyboard event(cocos2dKey, isPressed);
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
        return JNI_TRUE;
        
    }}
