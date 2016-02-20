#include <cstdint>

//TODO: custom deleter

struct RefCount
{
    int32_t sharedCount = 0;
    int32_t weakCount = 0;
};

template<class T>
class WeakPtr;

template<class T>
class SharedPtr
{
    friend WeakPtr<T>;
public:
    SharedPtr():
        _ptr(nullptr), _refCountPtr(nullptr)
    {
        
    }
    
    SharedPtr(const SharedPtr& other):
        _ptr(other._ptr), _refCountPtr(other._refCountPtr)
    {
        retain();
    }
    
    inline const SharedPtr& operator=(const SharedPtr& other)
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        retain();
        
        return *this;
    }
    
    // TODO: move constructor
    // TODO: move assignment
    
    SharedPtr(T* ptr):
        _ptr(ptr)
    {
        if (_ptr)
        {
            _refCountPtr = new RefCount();
            retain();
        }
    }
    
    ~SharedPtr()
    {
        release();
    }
    
    void reset()
    {
        release();
        _ptr = nullptr;
        _refCountPtr = nullptr;
    }
    
    inline operator bool()
    {
        return _ptr && _refCountPtr;
    }
    
    inline T* operator->()
    {
        return _ptr;
    }
    
    inline T* get()
    {
        return _ptr;
    }
    
private:
    void retain()
    {
        if (_refCountPtr)
        {
            ++_refCountPtr->sharedCount;
        }
    }
    
    void release()
    {
        if (_refCountPtr)
        {
            if (--_refCountPtr->sharedCount <= 0)
            {
                delete _ptr;
                
                if (_refCountPtr->weakCount <= 0)
                {
                    delete _refCountPtr;
                }
            }
        }
    }
    
    T* _ptr;
    RefCount* _refCountPtr;
};

template<class T>
class WeakPtr
{
public:
    WeakPtr():
        _ptr(nullptr), _refCountPtr(nullptr)
    {
        
    }
    
    WeakPtr(const WeakPtr& other):
        _ptr(other._ptr), _refCountPtr(other._refCount)
    {
        retain();
    }
    
    WeakPtr(const SharedPtr<T>& other):
        _ptr(other._ptr), _refCountPtr(other._refCount)
    {
        retain();
    }
    
    inline const WeakPtr& operator=(const WeakPtr& other)
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        retain();
        
        return *this;
    }
    
    inline const WeakPtr& operator=(const SharedPtr<T>& other)
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        retain();
        
        return *this;
    }
    
    // TODO: move constructor
    // TODO: move assignment
    
    ~WeakPtr()
    {
        release();
    }
    
    SharedPtr<T> lock()
    {
        SharedPtr<T> result;
        result._ptr = _ptr;
        result._refCountPtr = _refCountPtr;
        result.retain();
        return result;
    }
    
private:
    void retain()
    {
        if (_refCountPtr)
        {
            ++_refCountPtr->weakCount;
        }
    }
    
    void release()
    {
        if (_refCountPtr)
        {
            if (_refCountPtr->sharedCount == 0 && --_refCountPtr->weakCount == 0)
            {
                delete _refCountPtr;
            }
        }
    }
    
    T* _ptr;
    RefCount* _refCountPtr;
};

template<class T, class U>
SharedPtr<T> StaticPointerCast(SharedPtr<U> p)
{
    return SharedPtr<T>(p, static_cast<T*>(p.get()));
}

template<class T, class U>
SharedPtr<T> DynamicPointerCast(SharedPtr<U> p)
{
    return SharedPtr<T>(p, dynamic_cast<T*>(p.get()));
}
