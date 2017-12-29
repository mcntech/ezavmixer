#define LOG_TAG "AudDecApi"
#include <android/log.h>
#include <stdio.h>
#include "jni.h"

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&); \
void operator=(const TypeName&)

// An older, deprecated, politically incorrect name for the above.
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName) DISALLOW_COPY_AND_ASSIGN(TypeName)

typedef int status_t;

#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

// A smart pointer that deletes a JNI local reference when it goes out of scope.
template<typename T>
class ScopedLocalRef {
public:
    ScopedLocalRef(JNIEnv* env, T localRef) : mEnv(env), mLocalRef(localRef) {
    }
    ~ScopedLocalRef() {
        reset();
    }
    void reset(T ptr = NULL) {
        if (ptr != mLocalRef) {
            if (mLocalRef != NULL) {
                mEnv->DeleteLocalRef(mLocalRef);
            }
            mLocalRef = ptr;
        }
    }
    T release() __attribute__((warn_unused_result)) {
        T localRef = mLocalRef;
        mLocalRef = NULL;
        return localRef;
    }
    T get() const {
        return mLocalRef;
    }
private:
    JNIEnv* mEnv;
    T mLocalRef;
    // Disallow copy and assignment.
    ScopedLocalRef(const ScopedLocalRef&);
    void operator=(const ScopedLocalRef&);
};

static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

// Keep these in sync with their equivalents in MediaCodec.java !!!
    enum {
        DEQUEUE_INFO_TRY_AGAIN_LATER = -1,
        DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED = -2,
        DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED = -3,
    };

    static struct CodecActionCodes {
        jint codecActionTransient;
        jint codecActionRecoverable;
    } gCodecActionCodes;
    static struct CodecErrorCodes {
        jint errorInsufficientResource;
        jint errorReclaimed;
    } gCodecErrorCodes;
    static struct {
        jclass clazz;
        jfieldID mLock;
        jfieldID mPersistentObject;
        jmethodID ctor;
        jmethodID setNativeObjectLocked;
    } gPersistentSurfaceClassInfo;


    struct fields_t {
        jfieldID context;
/*
    jmethodID postEventFromNativeID;
    jfieldID cryptoInfoNumSubSamplesID;
    jfieldID cryptoInfoNumBytesOfClearDataID;
    jfieldID cryptoInfoNumBytesOfEncryptedDataID;
    jfieldID cryptoInfoKeyID;
    jfieldID cryptoInfoIVID;
    jfieldID cryptoInfoModeID;
    jfieldID cryptoInfoPatternID;
    jfieldID patternEncryptBlocksID;
    jfieldID patternSkipBlocksID;
*/
    };
    static fields_t gFields;




    struct JAudDecApi {
        JAudDecApi(
                JNIEnv *env, jobject thiz,
                const char *name);
        status_t initCheck() const;
        void registerSelf();
        void release();
        status_t start();
        status_t stop();
        status_t reset();
        status_t queueInputBuffer(
                size_t index,
                size_t offset, size_t size, int64_t timeUs, uint32_t flags);
        status_t dequeueInputBuffer(size_t *index, int64_t timeoutUs);
        status_t dequeueOutputBuffer(
                JNIEnv *env, jobject bufferInfo, size_t *index, int64_t timeoutUs);
        status_t releaseOutputBuffer(
                size_t index, bool render, bool updatePTS, int64_t timestampNs);
        status_t getBuffer(
                JNIEnv *env, bool input, size_t index, jobject *buf) const;
    protected:
        virtual ~JAudDecApi(){return;}
    private:
        enum {
            kWhatCallbackNotify,
            kWhatFrameRendered,
        };
        jclass mClass;
        jweak mObject;

        // java objects cached
        jclass mByteBufferClass;
        jobject mNativeByteOrderObj;
        jmethodID mByteBufferOrderMethodID;
        jmethodID mByteBufferPositionMethodID;
        jmethodID mByteBufferLimitMethodID;
        jmethodID mByteBufferAsReadOnlyBufferMethodID;
        // TODO replace with component
        //sp<MediaCodec> mCodec;
        status_t mInitStatus;

/*
        template <typename T>
        status_t createByteBufferFromABuffer(
                JNIEnv *env, bool readOnly, bool clearBuffer, const sp<T> &buffer,
                jobject *buf) const;
*/

        void cacheJavaObjects(JNIEnv *env);
        void deleteJavaObjects(JNIEnv *env){return ;}
        DISALLOW_EVIL_CONSTRUCTORS(JAudDecApi);
    };












    JAudDecApi::JAudDecApi(
            JNIEnv *env, jobject thiz,
            const char *name)
            : mClass(NULL),
              mObject(NULL) {
        jclass clazz = env->GetObjectClass(thiz);
        //CHECK(clazz != NULL);
        mClass = (jclass)env->NewGlobalRef(clazz);
        mObject = env->NewWeakGlobalRef(thiz);
        cacheJavaObjects(env);
        //mLooper = new ALooper;
        //mLooper->setName("MediaCodec_looper");
        //mLooper->start(
        //        false,      // runOnCallingThread
        //        true,       // canCallJava
         //       ANDROID_PRIORITY_VIDEO);


        // Create Component
        //    mCodec = MediaCodec::CreateByComponentName(mLooper, name, &mInitStatus);

        //CHECK((mCodec != NULL) != (mInitStatus != OK));
    }

    void JAudDecApi::cacheJavaObjects(JNIEnv *env) {
        jclass clazz = (jclass)env->FindClass("java/nio/ByteBuffer");
        mByteBufferClass = (jclass)env->NewGlobalRef(clazz);
        //CHECK(mByteBufferClass != NULL);
        ScopedLocalRef<jclass> byteOrderClass(
                env, env->FindClass("java/nio/ByteOrder"));
        //CHECK(byteOrderClass.get() != NULL);
        jmethodID nativeOrderID = env->GetStaticMethodID(
                byteOrderClass.get(), "nativeOrder", "()Ljava/nio/ByteOrder;");
        //CHECK(nativeOrderID != NULL);
        jobject nativeByteOrderObj =
                env->CallStaticObjectMethod(byteOrderClass.get(), nativeOrderID);
        mNativeByteOrderObj = env->NewGlobalRef(nativeByteOrderObj);
        //CHECK(mNativeByteOrderObj != NULL);
        env->DeleteLocalRef(nativeByteOrderObj);
        nativeByteOrderObj = NULL;
        mByteBufferOrderMethodID = env->GetMethodID(
                mByteBufferClass,
                "order",
                "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;");
        //CHECK(mByteBufferOrderMethodID != NULL);
        mByteBufferAsReadOnlyBufferMethodID = env->GetMethodID(
                mByteBufferClass, "asReadOnlyBuffer", "()Ljava/nio/ByteBuffer;");
        //CHECK(mByteBufferAsReadOnlyBufferMethodID != NULL);
        mByteBufferPositionMethodID = env->GetMethodID(
                mByteBufferClass, "position", "(I)Ljava/nio/Buffer;");
        //CHECK(mByteBufferPositionMethodID != NULL);
        mByteBufferLimitMethodID = env->GetMethodID(
                mByteBufferClass, "limit", "(I)Ljava/nio/Buffer;");
        //CHECK(mByteBufferLimitMethodID != NULL);
    }


    status_t JAudDecApi::initCheck() const {
        return mInitStatus;
    }

    status_t JAudDecApi::start() {
        return 0;//mCodec->start();
    }
    status_t JAudDecApi::stop() {
        return 0;//mCodec->stop();
    }

    status_t JAudDecApi::reset() {
        return 0;//mCodec->reset();
    }
    status_t JAudDecApi::queueInputBuffer(
            size_t index,
            size_t offset, size_t size, int64_t timeUs, uint32_t flags) {
        return 0;//mCodec->queueInputBuffer(index, offset, size, timeUs, flags, errorDetailMsg);
    }

    status_t JAudDecApi::dequeueInputBuffer(size_t *index, int64_t timeoutUs) {
        return 0;//mCodec->dequeueInputBuffer(index, timeoutUs);
    }
    status_t JAudDecApi::dequeueOutputBuffer(
            JNIEnv *env, jobject bufferInfo, size_t *index, int64_t timeoutUs) {
        size_t size, offset;
        int64_t timeUs;
        uint32_t flags;
/*
        status_t err = mCodec->dequeueOutputBuffer(
                index, &offset, &size, &timeUs, &flags, timeoutUs);
        if (err != OK) {
            return err;
        }
        ScopedLocalRef<jclass> clazz(
                env, env->FindClass("android/media/MediaCodec$BufferInfo"));
        jmethodID method = env->GetMethodID(clazz.get(), "set", "(IIJI)V");
        env->CallVoidMethod(bufferInfo, method, (jint)offset, (jint)size, timeUs, flags);
*/
        return 0;//OK;
    }
    status_t JAudDecApi::releaseOutputBuffer(
            size_t index, bool render, bool updatePTS, int64_t timestampNs) {
               return 0; //: mCodec->releaseOutputBuffer(index);
    }

    status_t JAudDecApi::getBuffer(
            JNIEnv *env, bool input, size_t index, jobject *buf) const {

/*
        sp<ABuffer> buffer;
        status_t err =
                input
                ? mCodec->getInputBuffer(index, &buffer)
                : mCodec->getOutputBuffer(index, &buffer);
        if (err != OK) {
            return err;
        }
        return createByteBufferFromABuffer(
                env, !input , input , buffer, buf);
*/

        return 0;
    }
//=============
    static void Java_com_mcntech_udpplayer_AudDecApi_release(JNIEnv *env, jobject thiz) {
        //setMediaCodec(env, thiz, NULL);
    }

    static void Java_com_mcntech_udpplayer_AudDecApi_reset(JNIEnv *env, jobject thiz) {
        //ALOGV("android_media_MediaCodec_reset");
        //sp <JMediaCodec> codec = getMediaCodec(env, thiz);
        //if (codec == NULL) {
        //    throwExceptionAsNecessary(env, INVALID_OPERATION);
        //    return;
        //}
        //status_t err = codec->reset();
    }

    static void Java_com_mcntech_udpplayer_AudDecApi_configure(
            JNIEnv *env,
            jobject thiz,
            jobjectArray keys, jobjectArray values) {
/*
        sp<JMediaCodec> codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            throwExceptionAsNecessary(env, INVALID_OPERATION);
            return;
        }
        sp<AMessage> format;
        status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);
        if (err != OK) {
            jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
            return;
        }
        sp<IGraphicBufferProducer> bufferProducer;
        if (jsurface != NULL) {
            sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
            if (surface != NULL) {
                bufferProducer = surface->getIGraphicBufferProducer();
            } else {
                jniThrowException(
                        env,
                        "java/lang/IllegalArgumentException",
                        "The surface has been released");
                return;
            }
        }
        sp<ICrypto> crypto;
        if (jcrypto != NULL) {
            crypto = JCrypto::GetCrypto(env, jcrypto);
        }
        err = codec->configure(format, bufferProducer, crypto, flags);
        throwExceptionAsNecessary(env, err);
*/
    }

    static void Java_com_mcntech_udpplayer_AudDecApi_start(JNIEnv *env, jobject thiz) {
/*
        ALOGV("android_media_MediaCodec_start");
        sp<JMediaCodec> codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            throwExceptionAsNecessary(env, INVALID_OPERATION);
            return;
        }
        status_t err = codec->start();
        throwExceptionAsNecessary(env, err, ACTION_CODE_FATAL, "start failed");
*/
    }
    static void Java_com_mcntech_udpplayer_AudDecApi_stop(JNIEnv *env, jobject thiz) {
 /*       ALOGV("android_media_MediaCodec_stop");
        sp<JMediaCodec> codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            throwExceptionAsNecessary(env, INVALID_OPERATION);
            return;
        }
        status_t err = codec->stop();
        throwExceptionAsNecessary(env, err);*/
    }

    static void Java_com_mcntech_udpplayer_AudDecApi_queueInputBuffer(
            JNIEnv *env,
            jobject thiz,
            jint index,
            jint offset,
            jint size,
            jlong timestampUs,
            jint flags) {
/*
        ALOGV("android_media_MediaCodec_queueInputBuffer");
        sp<JMediaCodec> codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            throwExceptionAsNecessary(env, INVALID_OPERATION);
            return;
        }
        AString errorDetailMsg;
        status_t err = codec->queueInputBuffer(
                index, offset, size, timestampUs, flags, &errorDetailMsg);
        throwExceptionAsNecessary(
                env, err, ACTION_CODE_FATAL, errorDetailMsg.empty() ? NULL : errorDetailMsg.c_str());
*/
    }


    static jint Java_com_mcntech_udpplayer_AudDecApi_dequeueInputBuffer(
            JNIEnv *env, jobject thiz, jlong timeoutUs) {
  /*      ALOGV("android_media_MediaCodec_dequeueInputBuffer");
        sp<JMediaCodec> codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            throwExceptionAsNecessary(env, INVALID_OPERATION);
            return -1;
        }
        size_t index;
        status_t err = codec->dequeueInputBuffer(&index, timeoutUs);
        if (err == OK) {
            return (jint) index;
        }
        return throwExceptionAsNecessary(env, err);*/
        return 0;
    }
    static jint Java_com_mcntech_udpplayer_AudDecApi_dequeueOutputBuffer(
            JNIEnv *env, jobject thiz, jobject bufferInfo, jlong timeoutUs) {
        /*       ALOGV("android_media_MediaCodec_dequeueOutputBuffer");
               sp<JMediaCodec> codec = getMediaCodec(env, thiz);
               if (codec == NULL) {
                   throwExceptionAsNecessary(env, INVALID_OPERATION);
                   return 0;
               }
               size_t index;
               status_t err = codec->dequeueOutputBuffer(
                       env, bufferInfo, &index, timeoutUs);
               if (err == OK) {
                   return (jint) index;
               }
               return throwExceptionAsNecessary(env, err);*/
        return 0;
    }
    static void Java_com_mcntech_udpplayer_AudDecApi_releaseOutputBuffer(
            JNIEnv *env, jobject thiz,
            jint index, jboolean render, jboolean updatePTS, jlong timestampNs) {
        /*       ALOGV("android_media_MediaCodec_renderOutputBufferAndRelease");
               sp<JMediaCodec> codec = getMediaCodec(env, thiz);
               if (codec == NULL) {
                   throwExceptionAsNecessary(env, INVALID_OPERATION);
                   return;
               }
               status_t err = codec->releaseOutputBuffer(index, render, updatePTS, timestampNs);
               throwExceptionAsNecessary(env, err);*/
    }

    static jobject Java_com_mcntech_udpplayer_AudDecApi_getBuffer(
            JNIEnv *env, jobject thiz, jboolean input, jint index) {
/*
        ALOGV("android_media_MediaCodec_getBuffer");
        sp<JMediaCodec> codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            throwExceptionAsNecessary(env, INVALID_OPERATION);
            return NULL;
        }
        jobject buffer;
        status_t err = codec->getBuffer(env, input, index, &buffer);
        if (err == OK) {
            return buffer;
        }
        // if we're out of memory, an exception was already thrown
        if (err != NO_MEMORY) {
            throwExceptionAsNecessary(env, err);
        }
*/
        return NULL;
    }

    static void Java_com_mcntech_udpplayer_AudDecApi_native_init(JNIEnv *env) {
 /*       ScopedLocalRef<jclass> clazz(
                env, env->FindClass("android/media/MediaCodec"));
        CHECK(clazz.get() != NULL);
        gFields.context = env->GetFieldID(clazz.get(), "mNativeContext", "J");
        CHECK(gFields.context != NULL);
        gFields.postEventFromNativeID =
                env->GetMethodID(
                        clazz.get(), "postEventFromNative", "(IIILjava/lang/Object;)V");
        CHECK(gFields.postEventFromNativeID != NULL);
        clazz.reset(env->FindClass("android/media/MediaCodec$CryptoInfo"));
        CHECK(clazz.get() != NULL);
        gFields.cryptoInfoNumSubSamplesID =
                env->GetFieldID(clazz.get(), "numSubSamples", "I");
        CHECK(gFields.cryptoInfoNumSubSamplesID != NULL);
        gFields.cryptoInfoNumBytesOfClearDataID =
                env->GetFieldID(clazz.get(), "numBytesOfClearData", "[I");
        CHECK(gFields.cryptoInfoNumBytesOfClearDataID != NULL);
        gFields.cryptoInfoNumBytesOfEncryptedDataID =
                env->GetFieldID(clazz.get(), "numBytesOfEncryptedData", "[I");
        CHECK(gFields.cryptoInfoNumBytesOfEncryptedDataID != NULL);
        gFields.cryptoInfoKeyID = env->GetFieldID(clazz.get(), "key", "[B");
        CHECK(gFields.cryptoInfoKeyID != NULL);
        gFields.cryptoInfoIVID = env->GetFieldID(clazz.get(), "iv", "[B");
        CHECK(gFields.cryptoInfoIVID != NULL);
        gFields.cryptoInfoModeID = env->GetFieldID(clazz.get(), "mode", "I");
        CHECK(gFields.cryptoInfoModeID != NULL);
        clazz.reset(env->FindClass("android/media/MediaCodec$CryptoException"));
        CHECK(clazz.get() != NULL);
        jfieldID field;
        field = env->GetStaticFieldID(clazz.get(), "ERROR_NO_KEY", "I");
        CHECK(field != NULL);
        gCryptoErrorCodes.cryptoErrorNoKey =
                env->GetStaticIntField(clazz.get(), field);
        field = env->GetStaticFieldID(clazz.get(), "ERROR_KEY_EXPIRED", "I");
        CHECK(field != NULL);
        gCryptoErrorCodes.cryptoErrorKeyExpired =
                env->GetStaticIntField(clazz.get(), field);
        field = env->GetStaticFieldID(clazz.get(), "ERROR_RESOURCE_BUSY", "I");
        CHECK(field != NULL);
        gCryptoErrorCodes.cryptoErrorResourceBusy =
                env->GetStaticIntField(clazz.get(), field);
        field = env->GetStaticFieldID(clazz.get(), "ERROR_INSUFFICIENT_OUTPUT_PROTECTION", "I");
        CHECK(field != NULL);
        gCryptoErrorCodes.cryptoErrorInsufficientOutputProtection =
                env->GetStaticIntField(clazz.get(), field);
        clazz.reset(env->FindClass("android/media/MediaCodec$CodecException"));
        CHECK(clazz.get() != NULL);
        field = env->GetStaticFieldID(clazz.get(), "ACTION_TRANSIENT", "I");
        CHECK(field != NULL);
        gCodecActionCodes.codecActionTransient =
                env->GetStaticIntField(clazz.get(), field);
        field = env->GetStaticFieldID(clazz.get(), "ACTION_RECOVERABLE", "I");
        CHECK(field != NULL);
        gCodecActionCodes.codecActionRecoverable =
                env->GetStaticIntField(clazz.get(), field);*/
    }
    static void Java_com_mcntech_udpplayer_AudDecApi_native_setup(
            JNIEnv *env, jobject thiz,
            jstring name, jboolean nameIsType, jboolean encoder) {
/*
        if (name == NULL) {
            jniThrowException(env, "java/lang/NullPointerException", NULL);
            return;
        }
        const char *tmp = env->GetStringUTFChars(name, NULL);
        if (tmp == NULL) {
            return;
        }
        sp<JMediaCodec> codec = new JMediaCodec(env, thiz, tmp, nameIsType, encoder);
        const status_t err = codec->initCheck();
        if (err == NAME_NOT_FOUND) {
            // fail and do not try again.
            jniThrowException(env, "java/lang/IllegalArgumentException",
                              String8::format("Failed to initialize %s, error %#x", tmp, err));
            env->ReleaseStringUTFChars(name, tmp);
            return;
        } else if (err != OK) {
            // believed possible to try again
            jniThrowException(env, "java/io/IOException",
                              String8::format("Failed to find matching codec %s, error %#x", tmp, err));
            env->ReleaseStringUTFChars(name, tmp);
            return;
        }
        env->ReleaseStringUTFChars(name, tmp);
        codec->registerSelf();
        setMediaCodec(env,thiz, codec);
*/
    }

    static const JNINativeMethod gMethods[] = {
            {"native_release",                "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_release},
            {"native_reset",                  "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_reset},

            {"native_configure",
                                              "([Ljava/lang/String;[Ljava/lang/Object)V",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_configure},

            {"native_start",                  "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_start},
            {"native_stop",                   "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_stop},
            {"native_queueInputBuffer",       "(IIIJI)V",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_queueInputBuffer},
            {"native_dequeueInputBuffer",     "(J)I",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_dequeueInputBuffer},
            {"native_dequeueOutputBuffer",    "(Landroid/media/MediaCodec$BufferInfo;J)I",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_dequeueOutputBuffer},
            {"releaseOutputBuffer",           "(IZZJ)V",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_releaseOutputBuffer},
            {"getBuffer",                     "(ZI)Ljava/nio/ByteBuffer;",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_getBuffer},

            {"native_init",                   "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_native_init},
            {"native_setup",                  "(Ljava/lang/String;ZZ)V",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_native_setup},
    };

int register_Java_com_mcntech_udpplayer_AudDecApi(JNIEnv *env) {
    return registerNativeMethods(env,
                                 "Java/com/mcntech/udpplayer/AudDecApiMediaCodec", (JNINativeMethod*)gMethods, NELEM(gMethods));
}