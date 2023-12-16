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
ComPtr<T> ���������еĲ��������������ʹ�����÷���ȫ��(T *)���÷���
ͬ������ComPtr<T>�ﶨ��Ĳ�����&�� 
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
��ʵ�������У��������Ƕ�������ComPtr<It>Ϊ���͵����ݳ�Աd����d��
���ɷ���SharedObject<D>::Create(&d)���£�
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
�����Create(&d)�����������&�����Ͽ���������Ϊdȡ��ַ����ʵ���¶���
��&����������ȡ��ComPtr<It>���m_ComPtr��Ա�ĵ�ַ��������Ϊ(It **)��
����SharedObject<D>::Create(It **)�Ķ���Ϊ��
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
����comPtr��ֵ����object��������(__out Interface ** object)��Ĳ�������
( Interface ** )����������Ӵ�����Ľӿ�����( It ** )��ͬ��������(ComPtr<It> *)
������ֵ����͡��������C++ģ���ࡣ��Ըû������
////////////////////////////////////////////////////////////////////////////////
------------------------------
template <class T>
class SharedObject : public T
------------------------------
��
------------------------------
template <typename Tt>
struct ComPtr
------------------------------
ǰ����SharedObject<T>�̳���T��������ComPtr<Tt>����Ttû�й�ϵ��ֻ��
��ʹ������TtΪ���͵��ڲ����ݰ��ˡ�����SharedObject<T>::Create(&d)����
������һ��SharedObject<T>Ϊ���͵Ķ����丸����ΪT����Ȼ�������ֵ��
d(����d������ΪComPtr<Tt>�������ֵ�����Ǹ���d�����ݳ�Աm_ComPtr����

һ����ô���dΪ�ӿڵ�����ָ�����(����ΪComPtr<Tt_Interface>), ��TΪ��
ʵ���ࡣע����� Tt_Interface ֻ����IUnknownΪ����̳еĽӿڣ�����û��
����AddRef��Release�����Ľӿں�������Ȼ��IUnknown��Release��virtual��
���壬���ӿ�Tt_InterfaceֻҪ��ComPtr������MS�ı������Ͳ�����Tt_Interface
�����Ƿ���嶨����Release�����ĺ����������ڼ̳й�ϵ����ṹ�����б�����
����Ҫ��Release�����嶨�塣

Create(__out Interface ** object)�������������䣺
    ----------------------------------------------------------------------------
	static_assert(std::tr1::is_base_of<Interface, T>::value,
                       "Template arg T must derive from Interface");
    ComPtr<Interface> comPtr = new (std::nothrow) SharedObject<T>();
	-----------------------------------------------------------------------------
	���ڱ�֤����T��Interface�̳ж�������ShareObject<T>�ִ���T�̳ж�����
Ҳ�Ϳ��԰��������SharedObject<T>������( Interface * )��������
ComPtr<Interface>����operator = ����comPtr���m_ComPtr��Ա�����£�
    -----------------------------------------------------------------------------
    Interface * operator=(Interface * lComPtr)  { . . . }
	-----------------------------------------------------------------------------
	Release��AddRef����SharedObject<T>�ﶨ��ģ���ComPtr<Interface>
�ﱻm_ComPtr���ã�������Щ���������ʵ��SharedObject<T>�ϡ�

ͬ��ԭ������SharedObject<T>��TΪ���࣬��TΪInterface�Ľӿ�ʵ���࣬
Interface ���õ����������Ժ�������d->method()������d��д�Ĳ�����( -> )
ת��m_ComPtr��ȥ�ˣ�Ҳ����˵T������Interface������ʵ�֡�
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
