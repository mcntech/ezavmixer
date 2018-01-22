#define LOG_TAG "AudDecApi"
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include "jni.h"
#include "decmp2.h"
#include "strmconn.h"
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
        int getOutputData(char *pData, int numBytes);
        int isInputFull();
        status_t sendInputData(char *pData, size_t size, int64_t timeUs, uint32_t flags);
        int isOutputEmpty();
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

        StrmCompIf *mCodec;
        ConnCtxT *mConnIn;
        ConnCtxT *mConnOut;
        status_t mInitStatus;
        long long mllOutPts;
        unsigned long mulOutFlags;
        DISALLOW_EVIL_CONSTRUCTORS(JAudDecApi);
    };












    JAudDecApi::JAudDecApi(
            JNIEnv *env, jobject thiz,
            const char *name)
            : mClass(NULL),
              mObject(NULL) {
        jclass clazz = env->GetObjectClass(thiz);
        //CHECK(clazz != NULL);
        mClass = (jclass) env->NewGlobalRef(clazz);
        mObject = env->NewWeakGlobalRef(thiz);
        mCodec = NULL;
        mInitStatus = -1;
        //mLooper = new ALooper;
        //mLooper->setName("MediaCodec_looper");
        //mLooper->start(
        //        false,      // runOnCallingThread
        //        true,       // canCallJava
        //       ANDROID_PRIORITY_VIDEO);


        // Create Component
        //    mCodec = MediaCodec::CreateByComponentName(mLooper, name, &mInitStatus);
        if (strcmp(name, "audio/mpeg") == 0 || strcmp(name, "audio/mpeg") == 0) {
            mCodec = decmp2Create();
        }

        if(mCodec != NULL) {
            mInitStatus = 0;
            mConnIn = CreateStrmConn(16 * 1024, 8);
            mConnOut = CreateStrmConn(16 * 1024, 8);
            mCodec->SetInputConn(mCodec, 0, mConnIn);
            mCodec->SetOutputConn(mCodec, 0, mConnOut);
        }
        //CHECK((mCodec != NULL) != (mInitStatus != OK));
    }

    status_t JAudDecApi::initCheck() const {
        return mInitStatus;
    }

    status_t JAudDecApi::start() {
        return mCodec->Start(mCodec);
    }
    status_t JAudDecApi::stop() {
        return mCodec->Stop(mCodec);
    }

    status_t JAudDecApi::reset() {
        return 0;//mCodec->reset();
    }
    status_t JAudDecApi::sendInputData(
            char *pData, size_t len, int64_t timeUs, uint32_t flags) {
        //mCodec->queueInputBuffer(index, offset, size, timeUs, flags, errorDetailMsg);
        return mConnOut->Write(mConnOut, pData, len, flags, timeUs);
    }

    int JAudDecApi::getOutputData(char *pData, int numBytes) {
        int res = 0;
        res =  mConnOut->Read(mConnOut, pData, numBytes, &mulOutFlags, &mllOutPts);
        return res;//OK;
    }

    int JAudDecApi::isInputFull() {
        return mConnIn->IsFull(mConnIn);
    }
    int JAudDecApi::isOutputEmpty() {
        return mConnOut->IsEmpty(mConnOut);
    }

    static JAudDecApi *setMediaCodec(
            JNIEnv *env, jobject thiz, JAudDecApi *codec) {
        JAudDecApi * old = (JAudDecApi *)env->GetLongField(thiz, gFields.context);
#if 0
        if (codec != NULL) {
            codec->incStrong(thiz);
        }
        if (old != NULL) {
            /* release MediaCodec and stop the looper now before decStrong.
             * otherwise JMediaCodec::~JMediaCodec() could be called from within
             * its message handler, doing release() from there will deadlock
             * (as MediaCodec::release() post synchronous message to the same looper)
             */
            old->release();
            old->decStrong(thiz);
        }
#endif
        env->SetLongField(thiz, gFields.context, (jlong)codec);
        return old;
    }

    static JAudDecApi *getMediaCodec(JNIEnv *env, jobject thiz) {
        return (JAudDecApi *)env->GetLongField(thiz, gFields.context);
    }

    static void Java_com_mcntech_udpplayer_AudDecApi_release(JNIEnv *env, jobject thiz) {
        setMediaCodec(env, thiz, NULL);
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

    static void Java_com_mcntech_udpplayer_AudDecApi_start(JNIEnv *env, jobject thiz) {

        //ALOGV("android_media_MediaCodec_start");
        JAudDecApi *codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            //throwExceptionAsNecessary(env, INVALID_OPERATION);
            return;
        }
        status_t err = codec->start();
        //throwExceptionAsNecessary(env, err, ACTION_CODE_FATAL, "start failed");

    }
    static void Java_com_mcntech_udpplayer_AudDecApi_stop(JNIEnv *env, jobject thiz) {
        JAudDecApi *codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            return;
        }
        status_t err = codec->stop();
    }

    static int Java_com_mcntech_udpplayer_AudDecApi_sendInputData(
            JNIEnv *env, jobject thiz, jobject buf, jint nBytes, jlong timestampUs, jint flags) {

        JAudDecApi *codec = getMediaCodec(env, thiz);
        if (codec == NULL) {
            return -1;
        }
        uint8_t* rawjBytes = static_cast<uint8_t*>(env->GetDirectBufferAddress(buf));
        status_t err = codec->sendInputData((char *)rawjBytes, nBytes, timestampUs, flags);
        return nBytes;
    }

    static int Java_com_mcntech_udpplayer_AudDecApi_isInputFull(
            JNIEnv *env, jobject thiz) {

        JAudDecApi *codec = getMediaCodec(env, thiz);
        return codec->isInputFull();
    }
    static int Java_com_mcntech_udpplayer_AudDecApi_getOutputData(
            JNIEnv *env, jobject thiz, jobject buf, jint numBytes) {
        JAudDecApi *codec= getMediaCodec(env, thiz);
        if (codec == NULL) {
           return 0;
        }
        uint8_t* rawjBytes = static_cast<uint8_t*>(env->GetDirectBufferAddress(buf));
        int len = codec->getOutputData((char *)rawjBytes, numBytes);
        return (jint) len;
    }

    static int Java_com_mcntech_udpplayer_AudDecApi_isOutputEmpty(
            JNIEnv *env, jobject thiz) {

        JAudDecApi *codec = getMediaCodec(env, thiz);
        return codec->isOutputEmpty();
    }

    static void Java_com_mcntech_udpplayer_AudDecApi_native_init(JNIEnv *env, jobject thiz) {
       ScopedLocalRef<jclass> clazz(env, env->FindClass("com/mcntech/udpplayer/AudDecApi"));
        //CHECK(clazz.get() != NULL);
        gFields.context = env->GetFieldID(clazz.get(), "mNativeContext", "J");
    }

    static void Java_com_mcntech_udpplayer_AudDecApi_native_setup(
            JNIEnv *env, jobject thiz, jstring name) {
        const char *tmp = env->GetStringUTFChars(name, NULL);
        if (tmp == NULL) {
            return;
        }
        JAudDecApi *codec = new JAudDecApi(env, thiz, tmp);
        const status_t err = codec->initCheck();
        if (err != 0) {
            env->ReleaseStringUTFChars(name, tmp);
            return;
        }
        env->ReleaseStringUTFChars(name, tmp);
        //codec->registerSelf();
        setMediaCodec(env,thiz, codec);
    }

    static const JNINativeMethod gMethods[] = {
            {"native_release",                "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_release},
            {"native_reset",                  "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_reset},

            {"native_start",                  "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_start},
            {"native_stop",                   "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_stop},
            {"native_sendInputData",       "(Ljava/nio/ByteBuffer;IJI)I",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_sendInputData},
            {"native_isInputFull",       "()I",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_isInputFull},
            {"native_getOutputData",    "(Ljava/nio/ByteBuffer;I)I",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_getOutputData},
            {"native_isOutputEmpty",    "()I",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_isOutputEmpty},

            {"native_init",                   "()V", (void *) Java_com_mcntech_udpplayer_AudDecApi_native_init},
            {"native_setup",                  "(Ljava/lang/String;)V",
                                                     (void *) Java_com_mcntech_udpplayer_AudDecApi_native_setup},
    };

int register_Java_com_mcntech_udpplayer_AudDecApi(JNIEnv *env) {
    return registerNativeMethods(env,
                                 "com/mcntech/udpplayer/AudDecApi", (JNINativeMethod*)gMethods, NELEM(gMethods));
}