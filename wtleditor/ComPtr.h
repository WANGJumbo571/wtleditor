//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================


#pragma once
#include <assert.h>

//
// This structs acts as a smart pointer for IUnknown pointers
// making sure to call AddRef() and Release() as needed.

/*////////////////////////////////////////////////////////////////////////////////
ComPtr<T> 定义了所有的操作符和运算符，使得其用法完全和(T *)的用法相
同，例如ComPtr<T>里定义的操作符&： 
---------------------------------------
template<typename T>
struct ComPtr
{
public:
    T** operator&()
	{
		assert(nullptr == m_ComPtr);
		return &m_ComPtr;
	}
};
-----------------------------------------
在实际运用中，加入我们定义了以ComPtr<It>为类型的数据成员d，和d的
生成方法SharedObject<D>::Create(&d)如下：
--------------------------------------------------------------------------------
class C {
	public:
		int k;
		ComPtr<It> d;
		B  b;

		C() {
			::MessageBox(0, L"C init", NULL, 0);
			SharedObject<D>::Create(&d);
			//d = (D *)&k;
		}
		~C() {
			::MessageBox(0, L"C del", NULL, 0);
		}
};
--------------------------------------------------------------------------------
里面的Create(&d)，这里操作符&表面上看起来像是为d取地址，其实重新定义
了&操作符后是取得ComPtr<It>里的m_ComPtr成员的地址，其类型为(It **)，
所以SharedObject<D>::Create(It **)的定义为：
--------------------------------------------------------------------------------
template <class T>
class SharedObject: public T
{
public:
  template <class Interface>
  static HRESULT Create(__out Interface** object)
  {
	static_assert(std::tr1::is_base_of<Interface, T>::value, 
						"Template arg T must derive from Interface");
	ComPtr<Interface> comPtr = new (std::nothrow) SharedObject<T>();
	if (!comPtr)
	{
		return E_FAIL;
	}
	return InitializeHelper(comPtr, object);
  }
};
--------------------------------------------------------------------------------
最后把comPtr的值赋给object。这里面(__out Interface ** object)里的参数类型
( Interface ** )和上面的例子代码里的接口类型( It ** )相同，而不是(ComPtr<It> *)
这种奇怪的类型。博大精深的C++模板类。但愿没有整错。
////////////////////////////////////////////////////////////////////////////////
------------------------------
template <class T>
class SharedObject : public T
------------------------------
和
------------------------------
template <typename Tt>
struct ComPtr
------------------------------
前者是SharedObject<T>继承类T，后者是ComPtr<Tt>和类Tt没有关系，只不
过使用了以Tt为类型的内部数据罢了。所以SharedObject<T>::Create(&d)可以
新申请一个SharedObject<T>为类型的对象（其父类型为T），然后把它赋值给
d(由于d的类型为ComPtr<Tt>，这个赋值真正是给了d的数据成员m_ComPtr）。

一般的用处是d为接口的智能指针对象(类型为ComPtr<Tt_Interface>), 而T为其
实现类。注意的是 Tt_Interface 只能以IUnknown为最初继承的接口，否则没有
定义AddRef和Release这样的接口函数。虽然在IUnknown中Release是virtual的
定义，但接口Tt_Interface只要被ComPtr包裹，MS的编译器就不会检查Tt_Interface
最终是否具体定义了Release这样的函数。反而在继承关系的类结构编译中编译器
必须要求Release被具体定义。

Create(__out Interface ** object)函数里的这行语句：
    ----------------------------------------------------------------------------
	static_assert(std::tr1::is_base_of<Interface, T>::value,
                       "Template arg T must derive from Interface");
    ComPtr<Interface> comPtr = new (std::nothrow) SharedObject<T>();
	-----------------------------------------------------------------------------
	由于保证了类T从Interface继承而来，而ShareObject<T>又从类T继承而来，
也就可以把新申请的SharedObject<T>对象以( Interface * )的名义用
ComPtr<Interface>的新operator = 赋给comPtr里的m_ComPtr成员，如下：
    -----------------------------------------------------------------------------
    Interface * operator=(Interface * lComPtr)  { . . . }
	-----------------------------------------------------------------------------
	Release和AddRef是在SharedObject<T>里定义的，在ComPtr<Interface>
里被m_ComPtr调用，所以这些调用最后都落实在SharedObject<T>上。

同上原因，由于SharedObject<T>以T为父类，且T为Interface的接口实现类，
Interface 调用的其他功能性函数，如d->method()，都被d重写的操作符( -> )
转向到m_ComPtr上去了，也就是说T包揽了Interface的所有实现。
///////////////////////////////////////////////////////////////////////////////*/
template<typename T>
struct ComPtr
{
public:

    ComPtr(T* lComPtr = nullptr) : m_ComPtr(lComPtr)
    {
        //static_assert(std::tr1::is_base_of<IUnknown, T>::value, "T needs to be IUnknown based");
        if (m_ComPtr)
        {
            m_ComPtr->AddRef();
        }
    }

    ComPtr(const ComPtr<T>& lComPtrObj)
    {
        //static_assert(std::tr1::is_base_of<IUnknown, T>::value, "T needs to be IUnknown based");
        m_ComPtr = lComPtrObj.m_ComPtr;

        if (m_ComPtr)
        {
            m_ComPtr->AddRef();
        }
    }

    ComPtr(ComPtr<T>&& lComPtrObj)
    {
        m_ComPtr = lComPtrObj.m_ComPtr;
        lComPtrObj.m_ComPtr = nullptr;
    }

    T* operator=(T* lComPtr)
    {
        if (m_ComPtr)
        {
            m_ComPtr->Release();
        }

        m_ComPtr = lComPtr;

        if (m_ComPtr)
        {
            m_ComPtr->AddRef();
        }

        return m_ComPtr;
    }

    T* operator=(const ComPtr<T>& lComPtrObj)
    {
        if (m_ComPtr)
        {
            m_ComPtr->Release();
        }

        m_ComPtr = lComPtrObj.m_ComPtr;

        if (m_ComPtr)
        {
            m_ComPtr->AddRef();
        }

        return m_ComPtr;
    }

    ~ComPtr()
    {
        if (m_ComPtr)
        {
            m_ComPtr->Release();
            m_ComPtr = nullptr;
        }
    }

    operator T*() const
    {
        return m_ComPtr;
    }

    T* GetInterface() const
    {
        return m_ComPtr;
    }

    T& operator*() const
    {
        return *m_ComPtr;
    }

    T** operator&()
    {
        //The assert on operator& usually indicates a bug. Could be a potential memory leak.
        // If this really what is needed, however, use GetInterface() explicitly.
        assert(nullptr == m_ComPtr);
		/*
		if (nullptr != m_ComPtr)
		{
			int i = 0;
			i++;
		}
		*/
        return &m_ComPtr;
    }

    T* operator->() const
    {
        return m_ComPtr;
    }

    bool operator!() const
    {    
        return (nullptr == m_ComPtr);
    }

    bool operator<(T* lComPtr) const
    {
        return m_ComPtr < lComPtr;
    }

    bool operator!=(T* lComPtr) const
    {
        return !operator==(lComPtr);
    }

    bool operator==(T* lComPtr) const
    {
        return m_ComPtr == lComPtr;
    }

    template <typename I>
    HRESULT QueryInterface(I **interfacePtr)
    {
        return m_ComPtr->QueryInterface(IID_PPV_ARGS(interfacePtr));
    }

protected:
    // The internal interface pointer
    T* m_ComPtr;
};
