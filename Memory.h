#include <cstdint>

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
    
    template<class U>
    SharedPtr(const SharedPtr<U>& other, StaticCastTag):
        _ptr(static_cast<T*>(other.get())), _refCountPtr(other.getRefCountPtr())
    {
        retain();
    }
    
    template<class U>
    SharedPtr(const SharedPtr<U>& other, ConstCastTag):
        _ptr(const_cast<T*>(other.get())), _refCountPtr(other.getRefCountPtr())
    {
        retain();
    }
    
    SharedPtr(const WeakPtr<T>& other):
        _ptr(other._ptr), _refCountPtr(other._refCountPtr)
    {
        
    }
    
    const SharedPtr& operator=(const SharedPtr& other)
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
    
    operator bool()
    {
        return _ptr && _refCountPtr;
    }
    
    T* operator->()
    {
        return _ptr;
    }
    
    inline T* get() const
    {
        return _ptr;
    }
    
    inline RefCount* getRefCountPtr() const
    {
        return _refCountPtr;
    }
    
protected:
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
    friend SharedPtr<T>;
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
    
    const WeakPtr& operator=(const WeakPtr& other)
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        retain();
        
        return *this;
    }
    
    const WeakPtr& operator=(const SharedPtr<T>& other)
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
    
protected:
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

template<class T>
class EnableSharedFromThis
{
protected:
    constexpr EnableSharedFromThis(): _weakPtr() {}
    EnableSharedFromThis(EnableSharedFromThis const &) {}
    EnableSharedFromThis& operator=(EnableSharedFromThis const &) { return *this; }
    ~EnableSharedFromThis() {}
public:
    SharedPtr<T> sharedFromThis() { return SharedPtr<T>(_weakPtr); }
    SharedPtr<T const> sharedFromThis() const { return SharedPtr<T const>(_weakPtr); }
    
private:
    WeakPtr<T> _weakPtr;
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
