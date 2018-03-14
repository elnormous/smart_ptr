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
    typedef T ElementType;

    SharedPtr()
    {
    }
    
    SharedPtr(const SharedPtr& other):
        ptr(other.ptr), refCountPtr(other.refCountPtr)
    {
        addRef();
    }
    
    SharedPtr(SharedPtr<ElementType>&& other):
        ptr(other.ptr), refCountPtr(other.refCountPtr)
    {
        other.ptr = nullptr;
        other.refCountPtr = nullptr;
    }
    
    template<class U>
    SharedPtr(const SharedPtr<U>& other, StaticCastTag):
        ptr(static_cast<ElementType*>(other.ptr)), refCountPtr(other.refCountPtr)
    {
        addRef();
    }
    
    template<class U>
    SharedPtr(const SharedPtr<U>& other, ConstCastTag):
        ptr(const_cast<ElementType*>(other.ptr)), refCountPtr(other.refCountPtr)
    {
        addRef();
    }
    
    SharedPtr(const WeakPtr<ElementType>& other):
        ptr(other.ptr), refCountPtr(other.refCountPtr)
    {
        addRef();
    }
    
    const SharedPtr& operator=(const SharedPtr& other) noexcept
    {
        removeRef();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        addRef();
        
        return *this;
    }
    
    const SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        removeRef();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        other.ptr = nullptr;
        other.refCountPtr = nullptr;
        
        return *this;
    }
    
    explicit SharedPtr(ElementType* p):
        ptr(p)
    {
        if (ptr)
        {
            refCountPtr = new RefCount();
            addRef();
            enableWeakThis(ptr);
        }
    }
    
    ~SharedPtr()
    {
        removeRef();
    }
    
    void reset() noexcept
    {
        removeRef();
        ptr = nullptr;
        refCountPtr = nullptr;
    }

    void reset(ElementType* p) noexcept
    {
        removeRef();
        ptr = p;

        if (ptr)
        {
            refCountPtr = new RefCount();
            addRef();
            enableWeakThis(ptr);
        }
        else
            refCountPtr = nullptr;
    }

    inline operator bool() noexcept
    {
        return ptr && refCountPtr;
    }
    
    inline ElementType* operator->() noexcept
    {
        return ptr;
    }
    
    inline ElementType* get() const noexcept
    {
        return ptr;
    }
    
private:
    inline void addRef() noexcept
    {
        if (refCountPtr) ++refCountPtr->sharedCount;
    }
    
    inline void removeRef() noexcept
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
    
    ElementType* ptr = nullptr;
    RefCount* refCountPtr = nullptr;
};

template<class T>
class WeakPtr
{
    template <class U> friend class WeakPtr;
    template <class U> friend class SharedPtr;
public:
    typedef T ElementType;

    WeakPtr():
        ptr(nullptr), refCountPtr(nullptr)
    {
    }
    
    WeakPtr(const WeakPtr& other):
        ptr(other.ptr), refCountPtr(other.refCount)
    {
        addRef();
    }
    
    WeakPtr(WeakPtr<ElementType>&& other):
        ptr(other.ptr), refCountPtr(other.refCountPtr)
    {
        other.ptr = nullptr;
        other.refCountPtr = nullptr;
    }
    
    template<class U>
    WeakPtr(const SharedPtr<U>& other):
        ptr(other.ptr), refCountPtr(other.refCount)
    {
        addRef();
    }
    
    const WeakPtr& operator=(const WeakPtr& other) noexcept
    {
        removeRef();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        addRef();
        
        return *this;
    }
    
    template<class U>
    const WeakPtr& operator=(const SharedPtr<U>& other) noexcept
    {
        removeRef();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        addRef();
        
        return *this;
    }
    
    const WeakPtr& operator=(WeakPtr&& other) noexcept
    {
        removeRef();
        
        ptr = other.ptr;
        refCountPtr = other.refCountPtr;
        
        other.ptr = nullptr;
        other.refCountPtr = nullptr;
        
        return *this;
    }
    
    ~WeakPtr()
    {
        removeRef();
    }
    
    SharedPtr<ElementType> lock() noexcept
    {
        SharedPtr<ElementType> result;
        result.ptr = ptr;
        result.refCountPtr = refCountPtr;
        result.addRef();
        return result;
    }
    
private:
    inline void addRef() noexcept
    {
        if (refCountPtr) ++refCountPtr->weakCount;
    }
    
    inline void removeRef() noexcept
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
    
    ElementType* ptr;
    RefCount* refCountPtr;
};

template<class T>
class EnableSharedFromThis
{
    template <class U> friend class SharedPtr;
public:
    typedef T ElementType;

    SharedPtr<ElementType> sharedFromThis() { return SharedPtr<T>(weakPtr); }
    SharedPtr<ElementType const> sharedFromThis() const { return SharedPtr<T const>(weakPtr); }
    
protected:
    constexpr EnableSharedFromThis() {}
    EnableSharedFromThis(EnableSharedFromThis const &) {}
    EnableSharedFromThis& operator=(EnableSharedFromThis const &) { return *this; }
    ~EnableSharedFromThis() {}
    
private:
    mutable WeakPtr<ElementType> weakPtr;
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
