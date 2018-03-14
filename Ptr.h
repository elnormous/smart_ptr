#include <cstdint>

//TODO: make shared
//TODO: custom deleter

struct RefCount
{
    int32_t sharedCount = 0;
    int32_t weakCount = 0;
};

struct StaticCastTag {};
struct ConstCastTag {};

template<class T>
class WeakPtr;

template<class T>
class EnableSharedFromThis;

template<class T>
class SharedPtr
{
    template <class U> friend class WeakPtr;
    template <class U> friend class SharedPtr;
public:
    SharedPtr():
        ptr(nullptr), refCountPtr(nullptr)
    {
        
    }
    
    SharedPtr(const SharedPtr& other):
        ptr(other.ptr), refCountPtr(other.refCountPtr)
    {
        retain();
    }
    
    SharedPtr(SharedPtr<T>&& other):
        ptr(other.ptr), refCountPtr(other.refCountPtr)
    {
        other.ptr = nullptr;
        other.refCountPtr = nullptr;
    }
    
    template<class U>
    SharedPtr(const SharedPtr<U>& other, StaticCastTag):
        ptr(static_cast<T*>(other.ptr)), refCountPtr(other.refCountPtr)
    {
        retain();
    }
    
    template<class U>
    SharedPtr(const SharedPtr<U>& other, ConstCastTag):
        ptr(const_cast<T*>(other.ptr)), refCountPtr(other.refCountPtr)
    {
        retain();
    }
    
    SharedPtr(const WeakPtr<T>& other):
        ptr(other.ptr), refCountPtr(other.refCountPtr)
    {
        retain();
    }
    
    const SharedPtr& operator=(const SharedPtr& other) noexcept
    {
        release();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        retain();
        
        return *this;
    }
    
    const SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        release();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        other.ptr = nullptr;
        other.refCountPtr = nullptr;
        
        return *this;
    }
    
    SharedPtr(T* p):
        ptr(p)
    {
        if (ptr)
        {
            refCountPtr = new RefCount();
            retain();
            enableWeakThis(ptr);
        }
        else
        {
            refCountPtr = nullptr;
        }
    }
    
    ~SharedPtr()
    {
        release();
    }
    
    void reset() noexcept
    {
        release();
        ptr = nullptr;
        refCountPtr = nullptr;
    }
    
    operator bool() noexcept
    {
        return ptr && refCountPtr;
    }
    
    T* operator->() noexcept
    {
        return ptr;
    }
    
    inline T* get() const noexcept
    {
        return ptr;
    }
    
private:
    inline void retain() noexcept
    {
        if (refCountPtr) ++refCountPtr->sharedCount;
    }
    
    inline void release() noexcept
    {
        if (refCountPtr)
        {
            if (refCountPtr->sharedCount <= 1)
            {
                delete ptr;
                ptr = nullptr;
                --refCountPtr->sharedCount;
                
                if (refCountPtr->weakCount <= 0)
                {
                    delete refCountPtr;
                    refCountPtr = nullptr;
                }
            }
        }
    }
    
    template <class U>
    inline void enableWeakThis(const EnableSharedFromThis<U>* e) noexcept
    {
        if (e)
        {
            e->weakPtr = *this;
        }
    }
    
    inline void enableWeakThis(const void*) noexcept {}
    
    T* ptr;
    RefCount* refCountPtr;
};

template<class T>
class WeakPtr
{
    template <class U> friend class WeakPtr;
    template <class U> friend class SharedPtr;
public:
    WeakPtr():
        ptr(nullptr), refCountPtr(nullptr)
    {
        
    }
    
    WeakPtr(const WeakPtr& other):
        ptr(other.ptr), refCountPtr(other.refCount)
    {
        retain();
    }
    
    WeakPtr(WeakPtr<T>&& other):
        ptr(other.ptr), refCountPtr(other.refCountPtr)
    {
        other.ptr = nullptr;
        other.refCountPtr = nullptr;
    }
    
    template<class U>
    WeakPtr(const SharedPtr<U>& other):
        ptr(other.ptr), refCountPtr(other.refCount)
    {
        retain();
    }
    
    const WeakPtr& operator=(const WeakPtr& other) noexcept
    {
        release();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        retain();
        
        return *this;
    }
    
    template<class U>
    const WeakPtr& operator=(const SharedPtr<U>& other) noexcept
    {
        release();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        retain();
        
        return *this;
    }
    
    const WeakPtr& operator=(WeakPtr&& other) noexcept
    {
        release();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        other.ptr = nullptr;
        other.refCountPtr = nullptr;
        
        return *this;
    }
    
    ~WeakPtr()
    {
        release();
    }
    
    SharedPtr<T> lock() noexcept
    {
        SharedPtr<T> result;
        result.ptr = ptr;
        result.refCountPtr = refCountPtr;
        result.retain();
        return result;
    }
    
private:
    inline void retain() noexcept
    {
        if (refCountPtr) ++refCountPtr->weakCount;
    }
    
    inline void release() noexcept
    {
        if (refCountPtr)
        {
            if (--refCountPtr->weakCount <= 0 && refCountPtr->sharedCount <= 0)
            {
                delete refCountPtr;
                refCountPtr = nullptr;
                ptr = nullptr;
            }
        }
    }
    
    T* ptr;
    RefCount* refCountPtr;
};

template<class T>
class EnableSharedFromThis
{
    template <class U> friend class SharedPtr;
public:
    SharedPtr<T> sharedFromThis() { return SharedPtr<T>(weakPtr); }
    SharedPtr<T const> sharedFromThis() const { return SharedPtr<T const>(weakPtr); }
    
protected:
    constexpr EnableSharedFromThis() {}
    EnableSharedFromThis(EnableSharedFromThis const &) {}
    EnableSharedFromThis& operator=(EnableSharedFromThis const &) { return *this; }
    ~EnableSharedFromThis() {}
    
private:
    mutable WeakPtr<T> weakPtr;
};

template<class T, class U>
SharedPtr<T> StaticPointerCast(const SharedPtr<U>& p)
{
    return SharedPtr<T>(p, StaticCastTag());
}

template<class T, class U>
SharedPtr<T> ConstPointerCast(SharedPtr<U> p)
{
    return SharedPtr<T>(p, ConstCastTag());
}
