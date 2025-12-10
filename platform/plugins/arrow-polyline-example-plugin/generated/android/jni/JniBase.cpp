/*

 *
 */

#include "JniBase.h"
#include "JniClassCache.h"

#include <pthread.h>

#ifdef __ANDROID__
/* Definition of PR_* constants */
#include <linux/prctl.h>
/* Declaration of prctl */
#include <sys/prctl.h>
#endif

namespace
{

static JavaVM* jvm;

static pthread_key_t s_thread_key;

char *get_thread_name()
{
#ifdef __ANDROID__
    thread_local char name[17] = {0};
    if (prctl(PR_GET_NAME, name) == 0)
    {
        return name;
    }
#endif
    return nullptr;
}

JNIEnv*
attach_current_thread( )
{
    JNIEnv* jniEnv;
    int envState = jvm->GetEnv( reinterpret_cast< void** >( &jniEnv ), JNI_VERSION_1_6 );
    if ( envState == JNI_EDETACHED )
    {
        JavaVMAttachArgs args;
        args.version = JNI_VERSION_1_6;
        args.group = NULL;
        args.name = get_thread_name();
#ifdef __ANDROID__
        jvm->AttachCurrentThread( &jniEnv, &args );
#else   // ifdef __ANDROID__
        jvm->AttachCurrentThread( reinterpret_cast< void** >( &jniEnv ), &args );
#endif  // ifdef __ANDROID__
    }

    return jniEnv;
}

void
detach_current_thread( void* )
{
    jvm->DetachCurrentThread( );
}

void
initialize_thread_data_key( )
{
    pthread_key_create( &s_thread_key, detach_current_thread );
}

}

// JNI_OnLoad() gets automatically called by java vm while loading the library.
// To make this work neither 'static' keyword (causes "static declaration of 'JNI_OnLoad' follows
// non-static declaration" - error) nor adding to anonymous namespace (prevents method from being
// called) is allowed.
JNIEXPORT jint
GLUECODIUM_JNI_ONLOAD( JavaVM* vm, void* )
{
    JNIEnv* env = nullptr;
    if ( vm->GetEnv( (void**)&env, JNI_VERSION_1_6 ) != JNI_OK )
    {
        return 0;
    }

    jvm = vm;
    ::glue_internal::jni::CachedJavaClassBase::init(env);
    return JNI_VERSION_1_6;
}

namespace glue_internal
{
namespace jni
{

JavaVM*
get_java_vm( ) noexcept
{
    return jvm;
}


JNIEnv* getJniEnvironmentForCurrentThread( ) noexcept
{
    // Add cleanup callback to current thread when called the first time
    static pthread_once_t s_key_once = PTHREAD_ONCE_INIT;
    pthread_once( &s_key_once, initialize_thread_data_key );

    JNIEnv* env;
    if ( ( env = static_cast< JNIEnv* >( pthread_getspecific( s_thread_key ) ) ) == nullptr )
    {
        env = attach_current_thread( );
        pthread_setspecific( s_thread_key, env );
    }

    return env;
}

}
}
