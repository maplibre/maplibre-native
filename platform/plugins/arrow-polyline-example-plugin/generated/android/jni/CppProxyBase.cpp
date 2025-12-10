/*

 *
 */

#include "CppProxyBase.h"
#include "JniBase.h"
#include "JniThrowNewException.h"

#include <mutex>
#include <unordered_map>
#include <utility>

namespace glue_internal
{
namespace jni
{

namespace
{
class GlobalJniLock final {
public:
    void
    lock( )
    {
        cacheMutex.lock( );
    }

    void
    unlock( )
    {
        jniEnvForCurrentThread = nullptr;
        cacheMutex.unlock( );
    }

    void
    setJniEnvForCurrentThread( JNIEnv* env ) noexcept
    {
        jniEnvForCurrentThread = env;
    }

    JNIEnv*
    getJniEnvForCurrentThread( ) noexcept
    {
        if ( jniEnvForCurrentThread == nullptr )
        {
            jniEnvForCurrentThread = ::glue_internal::jni::getJniEnvironmentForCurrentThread( );
        }
        return jniEnvForCurrentThread;
    }

private:
    ::std::mutex cacheMutex;
    JNIEnv* jniEnvForCurrentThread = nullptr;
};

static GlobalJniLock sGlobalJniLock;

struct ProxyCacheKey final
{
    jobject jObject;
    jint jHashCode;
    ::std::string type_key;

    bool operator==( const ProxyCacheKey& other ) const noexcept
    {
        return jHashCode == other.jHashCode &&
            sGlobalJniLock.getJniEnvForCurrentThread( )->IsSameObject( jObject, other.jObject );
    }
};

struct ProxyCacheKeyHash final
{
    inline size_t
    operator( )( const ProxyCacheKey& key ) const noexcept
    {
        size_t result = 7;
        result = 31 * result + key.jHashCode;
        result = 31 * result + ::std::hash<::std::string>{}(key.type_key);
        return result;
    }
};

using ReverseCacheKey = const void*;
using ProxyCache = ::std::unordered_map< ProxyCacheKey, ::std::weak_ptr< CppProxyBase >, ProxyCacheKeyHash >;
using ReverseProxyCache = ::std::unordered_map<ReverseCacheKey, jobject>;

static ProxyCache sProxyCache;
static ReverseProxyCache sReverseProxyCache;
} // namespace

JNIEnv*
CppProxyBase::getJniEnvironment( ) noexcept
{
    return ::glue_internal::jni::getJniEnvironmentForCurrentThread( );
}

CppProxyBase::CppProxyBase( ::glue_internal::jni::JniReference<jobject> globalRef,
                            jint jHashCode,
                            ::std::string type_key ) noexcept
    : mGlobalRef( std::move( globalRef ) ), jHashCode( jHashCode ), type_key( std::move( type_key ) )
{
}

CppProxyBase::~CppProxyBase( )
{
    JNIEnv* jniEnv = getJniEnvironment( );

    const ::std::lock_guard< GlobalJniLock > lock( sGlobalJniLock );
    sGlobalJniLock.setJniEnvForCurrentThread( jniEnv );
    sProxyCache.erase( ProxyCacheKey{mGlobalRef.get(), jHashCode, type_key} );
    removeSelfFromReverseCache( );
}

std::shared_ptr<CppProxyBase>
CppProxyBase::createProxyImpl( JNIEnv* const jenv,
                               const JniReference<jobject>& jobj,
                               const ::std::string& type_key,
                               const bool do_cache,
                               const ProxyFactoryFun factory)
{
    JniReference<jobject> globalRef = new_global_ref(jenv, jobj.get());
    const jint jHashCode = getHashCode(jenv, jobj.get());
    const ProxyCacheKey key{globalRef.get(), jHashCode, type_key};

    const ::std::lock_guard< GlobalJniLock > lock( sGlobalJniLock );
    sGlobalJniLock.setJniEnvForCurrentThread( jenv );

    if (do_cache) {
        if (const auto it = sProxyCache.find(key); it != sProxyCache.end())
        {
            if (auto cachedProxy = it->second.lock())
            {
                return cachedProxy;
            }
        }
    }

    auto [newProxy, reverseCacheKey] = factory(std::move(globalRef), jHashCode);

    if (newProxy == nullptr) {
        throw_new_runtime_exception(jenv, "Cannot allocate native memory.");
    } else if (do_cache) {
        sProxyCache[key] = ::std::weak_ptr<CppProxyBase>(newProxy);
        registerInReverseCache(newProxy.get(), reverseCacheKey, key.jObject);
    }
    return newProxy;
}

jint
CppProxyBase::getHashCode(JNIEnv* const jniEnv, const jobject jObj)
{
    const auto systemClass = find_class(jniEnv, "java/lang/System");
    const jmethodID jMethodId
        = jniEnv->GetStaticMethodID(systemClass.get(), "identityHashCode", "(Ljava/lang/Object;)I");

    return jniEnv->CallStaticIntMethod(systemClass.get(), jMethodId, jObj);
}

void CppProxyBase::registerInReverseCache( CppProxyBase* const proxyBase,
                                           ReverseCacheKey reverseCacheKey,
                                           const jobject jObj )
{
    proxyBase->mReverseCacheKey = reverseCacheKey;
    sReverseProxyCache[reverseCacheKey] = jObj;
}

void CppProxyBase::removeSelfFromReverseCache( )
{
    if ( mReverseCacheKey != nullptr )
    {
        sReverseProxyCache.erase( mReverseCacheKey );
        mReverseCacheKey = nullptr;
    }
}

JniReference<jobject>
CppProxyBase::getJavaObjectFromReverseCache(JNIEnv* jniEnv, ReverseCacheKey reverseCacheKey) {
    const ::std::lock_guard< GlobalJniLock > lock( sGlobalJniLock );
    if (const auto it = sReverseProxyCache.find(reverseCacheKey); it != sReverseProxyCache.end())
    {
        return make_local_ref(jniEnv, jniEnv->NewLocalRef(it->second));
    }
    return nullptr;
}

} // namespace jni
} // namespace glue_internal
