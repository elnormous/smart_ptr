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
class EnableSharedFromThis;

template<class T>
class SharedPtr
{
    template <class U> friend class WeakPtr;
    template <class U> friend class SharedPtr;
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
    
    SharedPtr(SharedPtr<T>&& other):
        _ptr(other._ptr), _refCountPtr(other._refCountPtr)
    {
        other._ptr = nullptr;
        other._refCountPtr = nullptr;
    }
    
    template<class U>
    SharedPtr(const SharedPtr<U>& other, StaticCastTag):
        _ptr(static_cast<T*>(other._ptr)), _refCountPtr(other._refCountPtr)
    {
        retain();
    }
    
    template<class U>
    SharedPtr(const SharedPtr<U>& other, ConstCastTag):
        _ptr(const_cast<T*>(other._ptr)), _refCountPtr(other._refCountPtr)
    {
        retain();
    }
    
    SharedPtr(const WeakPtr<T>& other):
        _ptr(other._ptr), _refCountPtr(other._refCountPtr)
    {
        retain();
    }
    
    const SharedPtr& operator=(const SharedPtr& other) noexcept
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        retain();
        
        return *this;
    }
    
    const SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        other._ptr = nullptr;
        other._refCountPtr = nullptr;
        
        return *this;
    }
    
    SharedPtr(T* ptr):
        _ptr(ptr)
    {
        if (_ptr)
        {
            _refCountPtr = new RefCount();
            retain();
            enableWeakThis(ptr);
        }
        else
        {
            _refCountPtr = nullptr;
        }
    }
    
    ~SharedPtr()
    {
        release();
    }
    
    void reset() noexcept
    {
        release();
        _ptr = nullptr;
        _refCountPtr = nullptr;
    }
    
    operator bool() noexcept
    {
        return _ptr && _refCountPtr;
    }
    
    T* operator->() noexcept
    {
        return _ptr;
    }
    
    inline T* get() const noexcept
    {
        return _ptr;
    }
    
private:
    inline void retain() noexcept
    {
        if (_refCountPtr)
        {
            _refCountPtr->sharedCount++;
        }
    }
    
    inline void release() noexcept
    {
        if (_refCountPtr)
        {
            if (_refCountPtr->sharedCount <= 1)
            {
                delete _ptr;
                _ptr = nullptr;
                _refCountPtr->sharedCount--;
                
                if (_refCountPtr->weakCount <= 0)
                {
                    delete _refCountPtr;
                    _refCountPtr = nullptr;
                }
            }
        }
    }
    
    template <class U>
    inline void enableWeakThis(const EnableSharedFromThis<U>* e) noexcept
    {
        if (e)
        {
            e->_weakPtr = *this;
        }
    }
    
    inline void enableWeakThis(const void*) noexcept {}
    
    T* _ptr;
    RefCount* _refCountPtr;
};

template<class T>
class WeakPtr
{
    template <class U> friend class WeakPtr;
    template <class U> friend class SharedPtr;
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
    
    WeakPtr(WeakPtr<T>&& other):
        _ptr(other._ptr), _refCountPtr(other._refCountPtr)
    {
        other._ptr = nullptr;
        other._refCountPtr = nullptr;
    }
    
    template<class U>
    WeakPtr(const SharedPtr<U>& other):
        _ptr(other._ptr), _refCountPtr(other._refCount)
    {
        retain();
    }
    
    const WeakPtr& operator=(const WeakPtr& other) noexcept
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        retain();
        
        return *this;
    }
    
    template<class U>
    const WeakPtr& operator=(const SharedPtr<U>& other) noexcept
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        retain();
        
        return *this;
    }
    
    const WeakPtr& operator=(WeakPtr&& other) noexcept
    {
        release();
        
        _ptr = other._ptr;
        _refCountPtr = other._refCountPtr;
        
        other._ptr = nullptr;
        other._refCountPtr = nullptr;
        
        return *this;
    }
    
    ~WeakPtr()
    {
        release();
    }
    
    SharedPtr<T> lock() noexcept
    {
        SharedPtr<T> result;
        result._ptr = _ptr;
        result._refCountPtr = _refCountPtr;
        result.retain();
        return result;
    }
    
private:
    inline void retain() noexcept
    {
        if (_refCountPtr)
        {
            ++_refCountPtr->weakCount;
        }
    }
    
    inline void release() noexcept
    {
        if (_refCountPtr)
        {
            if (--_refCountPtr->weakCount <= 0 && _refCountPtr->sharedCount <= 0)
            {
                delete _refCountPtr;
                _refCountPtr = nullptr;
                _ptr = nullptr;
            }
        }
    }
    
    T* _ptr;
    RefCount* _refCountPtr;
};

template<class T>
class EnableSharedFromThis
{
    template <class U> friend class SharedPtr;
public:
    SharedPtr<T> sharedFromThis() { return SharedPtr<T>(_weakPtr); }
    SharedPtr<T const> sharedFromThis() const { return SharedPtr<T const>(_weakPtr); }
    
protected:
    constexpr EnableSharedFromThis() {}
    EnableSharedFromThis(EnableSharedFromThis const &) {}
    EnableSharedFromThis& operator=(EnableSharedFromThis const &) { return *this; }
    ~EnableSharedFromThis() {}
    
private:
    mutable WeakPtr<T> _weakPtr;
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
