#pragma once

#include <atlmisc.h>    // for RecentDocumentList classes
#include <atlframe.h>   // for Frame and UpdateUI classes
#include <atlctrls.h>   // required for atlctrlw.h
#include <atlctrlw.h>   // for CCommandBarCtrl
#include <atlribbon.h>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>
#pragma comment(lib, "propsys.lib")

template <UINT t_ID, size_t t_items, size_t t_categories = 0>
class ExtraCRibbonComboCtrl : public ExtraCollectionCtrlImpl<T, t_ID, ExtraComboCollectionImpl<ExtraCRibbonComboCtrl<t_ID, t_items, t_categories>, t_items, t_categories>>
{
public:
	CRibbonComboCtrl()
	{ }
};

template <class T, UINT t_ID, class TCollection>
class ExtraCollectionCtrlImpl : public ExtraCommandCtrlImpl<T, t_ID>, public TCollection
{
	typedef ExtraCollectionCtrlImpl<T, t_ID, TCollection> thisClass;
public:
	typedef ExtraCommandCtrlImpl<T, t_ID> CommandCtrl;
	typedef TCollection Collection;

	// Implementation
	virtual HRESULT DoUpdateProperty(UINT nCmdID, REFPROPERTYKEY key,
		const PROPVARIANT* ppropvarCurrentValue, PROPVARIANT* ppropvarNewValue)
	{
		ATLASSERT(nCmdID == GetID());
		ATLASSERT(ppropvarNewValue);

		HRESULT hr = Collection::DoUpdateProperty(nCmdID, key, ppropvarCurrentValue, ppropvarNewValue);
		if FAILED(hr)
			hr = CommandCtrl::DoUpdateProperty(nCmdID, key, ppropvarCurrentValue, ppropvarNewValue);

		return hr;
	}

	virtual HRESULT DoExecute(UINT nCmdID, UI_EXECUTIONVERB verb,
		const PROPERTYKEY* key, const PROPVARIANT* ppropvarValue,
		IUISimplePropertySet* /*pCommandExecutionProperties*/)
	{
		ATLASSERT(nCmdID == GetID());
		nCmdID; // avoid level4 warning

		if (key == NULL) // gallery button pressed
		{
			GetWndRibbon().OnRibbonItemSelected(GetID(), UI_EXECUTIONVERB_EXECUTE, UI_COLLECTION_INVALIDINDEX);
			return S_OK;
		}

		ATLASSERT(k_(*key) == k_SelectedItem);
		ATLASSERT(ppropvarValue);

		HRESULT hr = S_OK;
		UINT32 uSel = 0xffff;
		hr = UIPropertyToUInt32(*key, *ppropvarValue, &uSel);

		if (SUCCEEDED(hr))
		{
			if (GetWndRibbon().OnRibbonItemSelected(GetID(), verb, uSel))
				TCollection::Select(uSel);
		}

		return hr;
	}
};

template <class T, UINT t_ID>
class ExtraCommandCtrlImpl : public ExtraCtrlImpl<T, t_ID>
{
public:
	CBitmap m_hbm[4];

	HRESULT SetImage(REFPROPERTYKEY key, HBITMAP hbm, bool bUpdate = false)
	{
		ATLASSERT((k_(key) <= k_SmallHighContrastImage) && (k_(key) >= k_LargeImage));

		m_hbm[k_(key) - k_LargeImage].Attach(hbm);

		return bUpdate ?
			GetWndRibbon().InvalidateProperty(GetID(), key) :
			S_OK;
	}

	HRESULT OnGetImage(REFPROPERTYKEY key, PROPVARIANT* ppv)
	{
		ATLASSERT((k_(key) <= k_SmallHighContrastImage) && (k_(key) >= k_LargeImage));

		const INT iImage = k_(key) - k_LargeImage;

		if (m_hbm[iImage].IsNull())
			m_hbm[iImage] = GetWndRibbon().OnRibbonQueryImage(GetID(), key);

		return m_hbm[iImage].IsNull() ?
		E_NOTIMPL :
				  SetPropertyVal(key, GetImage(m_hbm[iImage], UI_OWNERSHIP_COPY), ppv);
	}

	virtual HRESULT DoUpdateProperty(UINT nCmdID, REFPROPERTYKEY key,
		const PROPVARIANT* ppropvarCurrentValue, PROPVARIANT* ppropvarNewValue)
	{
		ATLASSERT(nCmdID == GetID());

		return (k_(key) <= k_SmallHighContrastImage) && (k_(key) >= k_LargeImage) ?
			OnGetImage(key, ppropvarNewValue) :
			CtrlImpl::DoUpdateProperty(nCmdID, key, ppropvarCurrentValue, ppropvarNewValue);
	}
};

template <class T, UINT t_ID>
class ATL_NO_VTABLE ExtraCtrlImpl : public ICtrl
{
protected:
	T* m_pWndRibbon;

public:
	typedef T WndRibbon;

	CtrlImpl() : m_pWndRibbon(T::pWndRibbon)
	{ }

	WndRibbon& GetWndRibbon()
	{
		return *m_pWndRibbon;
	}

	static WORD GetID()
	{
		return t_ID;
	}

	Text m_sTxt[5];

	// Operations
	HRESULT Invalidate()
	{
		return GetWndRibbon().InvalidateCtrl(GetID());
	}

	HRESULT Invalidate(REFPROPERTYKEY key, UI_INVALIDATIONS flags = UI_INVALIDATIONS_PROPERTY)
	{
		return GetWndRibbon().InvalidateProperty(GetID(), key, flags);
	}

	HRESULT SetText(REFPROPERTYKEY key, LPCWSTR sTxt, bool bUpdate = false)
	{
		ATLASSERT((k_(key) <= k_TooltipTitle) && (k_(key) >= k_LabelDescription));

		m_sTxt[k_(key) - k_LabelDescription] = sTxt;

		return bUpdate ?
			GetWndRibbon().InvalidateProperty(GetID(), key) :
			S_OK;
	}

	// Implementation
	template <typename V>
	HRESULT SetProperty(REFPROPERTYKEY key, V val)
	{
		return GetWndRibbon().SetProperty(GetID(), key, val);
	}

	HRESULT OnGetText(REFPROPERTYKEY key, PROPVARIANT* ppv)
	{
		ATLASSERT((k_(key) <= k_TooltipTitle) && (k_(key) >= k_LabelDescription));

		const INT iText = k_(key) - k_LabelDescription;
		if (m_sTxt[iText].IsEmpty())
		if (LPCWSTR sText = GetWndRibbon().OnRibbonQueryText(GetID(), key))
			m_sTxt[iText] = sText;

		return !m_sTxt[iText].IsEmpty() ?
			SetPropertyVal(key, (LPCWSTR)m_sTxt[iText], ppv) :
			S_OK;
	}

	virtual HRESULT DoExecute(UINT nCmdID, UI_EXECUTIONVERB verb,
		const PROPERTYKEY* key, const PROPVARIANT* ppropvarValue,
		IUISimplePropertySet* pCommandExecutionProperties)
	{
		ATLASSERT(nCmdID == t_ID);
		return GetWndRibbon().DoExecute(nCmdID, verb, key, ppropvarValue, pCommandExecutionProperties);
	}

	virtual HRESULT DoUpdateProperty(UINT nCmdID, REFPROPERTYKEY key,
		const PROPVARIANT* ppropvarCurrentValue, PROPVARIANT* ppropvarNewValue)
	{
		ATLASSERT(nCmdID == t_ID);

		const INT iMax = k_TooltipTitle - k_LabelDescription;
		const INT iVal = k_(key) - k_LabelDescription;

		return (iVal <= iMax) && (iVal >= 0) ?
			OnGetText(key, ppropvarNewValue) :
			GetWndRibbon().DoUpdateProperty(nCmdID, key, ppropvarCurrentValue, ppropvarNewValue);
	}
};

template <class TCtrl, size_t t_items, size_t t_categories = 0>
class ExtraComboCollectionImpl : public ExtraItemCollectionImpl<TCtrl, t_items, t_categories>
{
	typedef ExtraComboCollectionImpl<TCtrl, t_items, t_categories> thisClass;
public:
	typedef thisClass ComboCollection;

	// Operations
	HRESULT SetComboText(LPCWSTR sText)
	{
		TCtrl::WndRibbon& ribbon = static_cast<TCtrl*>(this)->GetWndRibbon();
		return ribbon.IsRibbonUI() ?
			ribbon.SetProperty(TCtrl::GetID(), UI_PKEY_StringValue, sText) :
			S_OK;
	}

	LPCWSTR GetComboText()
	{
		static WCHAR sCombo[RIBBONUI_MAX_TEXT] = { 0 };
		TCtrl::WndRibbon& ribbon = static_cast<TCtrl*>(this)->GetWndRibbon();
		PROPVARIANT var;
		if (ribbon.IsRibbonUI())
		{
			HRESULT hr = ribbon.GetIUIFrameworkPtr()->GetUICommandProperty(TCtrl::GetID(), UI_PKEY_StringValue, &var);
			hr = PropVariantToString(var, sCombo, RIBBONUI_MAX_TEXT);
			return sCombo;
		}
		return NULL;
	}
};

template <class TCtrl, size_t t_items, size_t t_categories = 0>
class ExtraItemCollectionImpl : public ExtraTextCollectionImpl<TCtrl, t_items, t_categories>
{
	typedef ExtraItemCollectionImpl<TCtrl, t_items, t_categories> thisClass;
public:
	typedef thisClass ItemCollection;

	ItemCollectionImpl()
	{
		ZeroMemory(m_aBitmap, sizeof m_aBitmap);
	}

	CBitmap m_aBitmap[t_items];

	// Operations
	HRESULT SetItemImage(UINT uIndex, HBITMAP hbm, bool bUpdate = false)
	{
		ATLASSERT(uIndex < t_items);

		m_aBitmap[uIndex] = hbm;

		return bUpdate ? InvalidateItems() : S_OK;
	}

	// Implementation
	HRESULT DoGetItem(UINT uItem, REFPROPERTYKEY key, PROPVARIANT *value)
	{
		ATLASSERT(uItem < t_items);

		if (k_(key) == k_ItemImage)
		{
			if (m_aBitmap[uItem].IsNull())
			{
				TCtrl::WndRibbon& ribbon = static_cast<TCtrl*>(this)->GetWndRibbon();
				m_aBitmap[uItem] = ribbon.OnRibbonQueryItemImage(TCtrl::GetID(), uItem);
			}
			return m_aBitmap[uItem].IsNull() ?
			E_NOTIMPL :
					  SetPropertyVal(key, GetImage(m_aBitmap[uItem], UI_OWNERSHIP_COPY), value);
		}
		else
		{
			return TextCollection::DoGetItem(uItem, key, value);
		}
	}
};

template <class TCtrl, size_t t_items, size_t t_categories = 0>
class ExtraTextCollectionImpl : public ExtraCollectionImpl<TCtrl, t_items, t_categories>
{
	typedef ExtraTextCollectionImpl<TCtrl, t_items, t_categories> thisClass;
public:
	typedef thisClass TextCollection;

	ExtraTextCollectionImpl() : m_uSelected(UI_COLLECTION_INVALIDINDEX)
	{ }

	Text m_asText[t_items];
	UINT m_uSelected;

	// Operations
	HRESULT SetItemText(UINT uItem, LPCWSTR sText, bool bUpdate = false)
	{
		ATLASSERT(uItem < t_items);

		m_asText[uItem] = sText;

		return bUpdate ? InvalidateItems() : S_OK;
	}

	UINT GetSelected()
	{
		return m_uSelected;
	}

	HRESULT Select(UINT uItem, bool bUpdate = false)
	{
		ATLASSERT((uItem < t_items) || (uItem == UI_COLLECTION_INVALIDINDEX));

		m_uSelected = uItem;

		TCtrl::WndRibbon& ribbon = static_cast<TCtrl*>(this)->GetWndRibbon();
		return bUpdate ?
			ribbon.SetProperty(TCtrl::GetID(), UI_PKEY_SelectedItem, uItem) :
			S_OK;
	}

	// Implementation
	HRESULT DoGetItem(UINT uItem, REFPROPERTYKEY key, PROPVARIANT *value)
	{
		ATLASSERT(uItem < t_items);

		if (k_(key) == k_Label)
		{
			if (m_asText[uItem].IsEmpty())
			{
				TCtrl::WndRibbon& ribbon = static_cast<TCtrl*>(this)->GetWndRibbon();
				m_asText[uItem] = ribbon.OnRibbonQueryItemText(TCtrl::GetID(), uItem);
			}
			return SetPropertyVal(key, (LPCWSTR)m_asText[uItem], value);
		}
		else
		{
			return Collection::DoGetItem(uItem, key, value);
		}
	}

	HRESULT DoUpdateProperty(UINT nCmdID, REFPROPERTYKEY key,
		const PROPVARIANT* ppropvarCurrentValue, PROPVARIANT* ppropvarNewValue)
	{
		ATLASSERT(nCmdID == TCtrl::GetID());

		if (k_(key) == k_SelectedItem)
		{
			TCtrl::WndRibbon& ribbon = static_cast<TCtrl*>(this)->GetWndRibbon();
			UINT uSel = UI_COLLECTION_INVALIDINDEX;
			if ((m_uSelected == UI_COLLECTION_INVALIDINDEX) &&
				ribbon.OnRibbonQuerySelectedItem(TCtrl::GetID(), uSel))
				m_uSelected = uSel;

			return SetPropertyVal(key, m_uSelected, ppropvarNewValue);
		}
		else
		{
			return Collection::DoUpdateProperty(nCmdID, key, ppropvarCurrentValue, ppropvarNewValue);
		}
	}
};

template <class TCtrl, size_t t_items, size_t t_categories>
class ExtraCollectionImpl : public ExtraCollectionImplBase<ExtraCollectionImpl<TCtrl, t_items, t_categories>, t_items + t_categories>
{
	typedef ExtraCollectionImpl<TCtrl, t_items, t_categories> thisClass;
public:
	typedef thisClass Collection;

	CollectionImpl() : m_size(t_items)
	{
		FillMemory(m_auItemCat, sizeof m_auItemCat, 0xff); // UI_COLLECTION_INVALIDINDEX
	}

	UINT32 * m_auItemCat;
	Text m_asCatName[__max(t_categories, 1)];
	size_t m_size;
	
	void DeclareItemsNumber(int number)
	{
		ExtraCollectionImplBase<ExtraCollectionImpl<TCtrl, t_items, t_categories>, t_items + t_categories>.DeclareItemsNumber(number);
		m_auItemCat = new UINT32[number];
		m_size = number;
	}

	// Operations
	HRESULT SetItemCategory(UINT uItem, UINT uCat, bool bUpdate = false)
	{
		//ATLASSERT((uItem < t_items) && (uCat < t_categories));
		ATLASSERT((uItem < itemSize) && (uCat < t_categories));

		m_auItemCat[uItem] = uCat;

		return bUpdate ? InvalidateItems() : S_OK;
	}

	HRESULT SetCategoryText(UINT uCat, LPCWSTR sText, bool bUpdate = false)
	{
		ATLASSERT(uCat < t_categories);

		m_asCatName[uCat] = sText;

		return bUpdate ? InvalidateCategories() : S_OK;
	}

	HRESULT Resize(size_t size, bool bUpdate = false)
	{
		//ATLASSERT(size <= t_items);
		ATLASSERT(size <= itemSize);

		m_size = size;

		return bUpdate ? InvalidateItems() : S_OK;
	}

	// Implementation
	HRESULT OnGetItem(UINT uIndex, REFPROPERTYKEY key, PROPVARIANT *value)
	{
		//ATLASSERT(uIndex < t_items + t_categories);
		ATLASSERT(uIndex < itemSize + t_categories);

		TCtrl* pCtrl = static_cast<TCtrl*>(this);

		//return uIndex < t_items ?
		return uIndex < itemSize ?
			pCtrl->DoGetItem(uIndex, key, value) :
			//pCtrl->DoGetCategory(uIndex - t_items, key, value);
			pCtrl->DoGetCategory(uIndex - itemSize, key, value);
	}

	HRESULT DoGetItem(UINT uItem, REFPROPERTYKEY key, PROPVARIANT *value)
	{
		ATLASSERT(k_(key) == k_CategoryId);
		UINT32 uCat = UI_COLLECTION_INVALIDINDEX;

		if (t_categories != 0)
		{
			if (m_auItemCat[uItem] == UI_COLLECTION_INVALIDINDEX)
			{
				TCtrl::WndRibbon& ribbon = static_cast<TCtrl*>(this)->GetWndRibbon();
				m_auItemCat[uItem] = ribbon.OnRibbonQueryItemCategory(TCtrl::GetID(), uItem);
			}
			uCat = m_auItemCat[uItem];
		}

		return SetPropertyVal(key, uCat, value);
	}

	HRESULT DoGetCategory(UINT uCat, REFPROPERTYKEY key, PROPVARIANT *value)
	{
		HRESULT hr = S_OK;

		switch (k_(key))
		{
		case k_Label:
			if (m_asCatName[uCat].IsEmpty())
			{
				TCtrl::WndRibbon& ribbon = static_cast<TCtrl*>(this)->GetWndRibbon();
				m_asCatName[uCat] = ribbon.OnRibbonQueryCategoryText(TCtrl::GetID(), uCat);
			}
			hr = SetPropertyVal(key, (LPCWSTR)m_asCatName[uCat], value);
			break;
		case k_CategoryId:
			hr = SetPropertyVal(key, uCat, value);
			break;
		default:
			ATLASSERT(FALSE);
			break;
		}

		return hr;
	}

	HRESULT InvalidateItems()
	{
		return static_cast<TCtrl*>(this)->Invalidate(UI_PKEY_ItemsSource);
	}

	HRESULT InvalidateCategories()
	{
		return static_cast<TCtrl*>(this)->Invalidate(UI_PKEY_Categories);
	}

	HRESULT DoUpdateProperty(UINT nCmdID, REFPROPERTYKEY key,
		const PROPVARIANT* ppropvarCurrentValue, PROPVARIANT* /*ppropvarNewValue*/)
	{
		ATLASSERT(nCmdID == TCtrl::GetID());
		nCmdID;   // avoid level 4 warning

		HRESULT hr = E_NOTIMPL;
		switch (k_(key))
		{
		case k_ItemsSource:
		{
							  ATL::CComQIPtr<IUICollection> pIUICollection(ppropvarCurrentValue->punkVal);
							  ATLASSERT(pIUICollection);
							  hr = pIUICollection->Clear();
							  //for (UINT i = 0; i < m_size; i++)
							  for (UINT i = 0; i < itemSize; i++)
							  {
								  if FAILED(hr = pIUICollection->Add(m_apItems[i]))
									  break;
							  }
							  ATLASSERT(SUCCEEDED(hr));
		}
			break;
		case k_Categories:
			if (t_categories != 0)
			{
				ATL::CComQIPtr<IUICollection> pIUICategory(ppropvarCurrentValue->punkVal);
				ATLASSERT(pIUICategory.p);
				hr = pIUICategory->Clear();
				//for (UINT i = t_items; i < (t_items + t_categories); i++)
				for (UINT i = itemSize; i < (itemSize + t_categories); i++)
				{
					if FAILED(hr = pIUICategory->Add(m_apItems[i]))
						break;
				}
				ATLASSERT(SUCCEEDED(hr));
			}
			break;
		}

		return hr;
	}
};

template <class TCollection, size_t t_size>
class ExtraCollectionImplBase
{
	typedef ExtraCollectionImplBase<TCollection, t_size> thisClass;
	typedef ItemProperty<TCollection> * thisItemClass;

public:
	ExtraCollectionImplBase()
	{
		m_apItems = new thisItemClass[t_size + 5];

		for (int i = 0; i < t_size; i++)
			m_apItems[i] = new ItemProperty<TCollection>(i, static_cast<TCollection*>(this));
	}

	~ExtraCollectionImplBase()
	{
		for (int i = 0; i < itemSize; i++)
			delete m_apItems[i];
	}

	void DeclareItemsNumber(int number)
	{
		m_apItems = new thisItemClass[number+5];
		for (int i = 0; i < number; i++)
		{
			m_apItems[i] = new ItemProperty<TCollection>(i, static_cast<TCollection*>(this));
		}

		itemSize = number;
	}
	// Data members
	//ItemProperty<TCollection>* m_apItems[t_size];
	ItemProperty<TCollection> ** m_apItems;
	int itemSize;
};