#pragma once

#include "stdafx.h"
#include "atlmisc.h"
#include "ImageOperation.h"
#include "CBRotatablePtr.h"

class CBLayerPtr : public CBRotatablePtr, public CBPastingPtr
{
public:
	CBLayerPtr() {}
	~CBLayerPtr() {}

	ComPtr<ID2D1SolidColorBrush>	m_layerFrameBrush;
	
	bool						m_layerIsActive;
	FLOAT					m_layerOpacity;

	bool						m_layerBorderIsActive;

	bool						m_layerIfPasting;
	CString					m_layerPicFileName;

	ComPtr<ID2D1Bitmap>			m_layerBitmap;
	ComPtr<IWICBitmapSource>	m_layerWicBitmap;
};

__interface DX_DECLARE_INTERFACE("42D782A6-A59B-404D-9BBD-2C27591A3F60") IImageLayerOperation : public IUnknown
{
	HRESULT _stdcall GetLayerEquipment(__out CBLayerPtr * te);
	HRESULT _stdcall SetLayerEquipment(__in CBLayerPtr te);
};

class ImageLayerOperation : public IImageOperation, public IImageLayerOperation
{
public:
	// IImageClippingOperation
	HRESULT _stdcall GetLayerEquipment(__out CBLayerPtr * te);
	HRESULT _stdcall SetLayerEquipment(__in CBLayerPtr te);

	void		SetActive(bool isActive);
	bool		GetActive() { return layerInfo.m_layerIsActive; }
	void		SetOpacity(FLOAT opacity);
	FLOAT	GetOpacity();
	void		SetDuration(FLOAT duration) { m_duration = duration; }
	FLOAT	GetDuration() { return m_duration; }

protected:
	ImageLayerOperation(CBLayerPtr te);
	virtual ~ImageLayerOperation();

	// Interface helper
	bool QueryInterfaceHelper(const IID &iid, void **object)
	{
		return CastHelper<IImageOperation>::CastTo(iid, this, object) ||
			CastHelper<IImageLayerOperation>::CastTo(iid, this, object);
	}

private:
	CBLayerPtr	 layerInfo;
	FLOAT			m_duration = 0.0f;
};

__interface DX_DECLARE_INTERFACE("43D702A6-A59B-404D-9CBD-2C27491A3F60") IImageDummyOperation : public IUnknown
{
};

class ImageDummyOperation : public IImageOperation, public IImageDummyOperation
{
public:
	void SetActive(bool isActive);
	bool GetActive() { return false; }

	void SetOpacity(FLOAT opacity);
	FLOAT GetOpacity();
	void		SetDuration(FLOAT duration) { }
	FLOAT	GetDuration() { return 0.0f; }

protected:
	ImageDummyOperation();
	virtual ~ImageDummyOperation();

	// Interface helper
	bool QueryInterfaceHelper(const IID &iid, void **object)
	{
		return CastHelper<IImageOperation>::CastTo(iid, this, object) ||
			CastHelper<IImageDummyOperation>::CastTo(iid, this, object);
	}
};
