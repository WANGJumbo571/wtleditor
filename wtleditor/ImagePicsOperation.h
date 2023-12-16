#pragma once

#include "stdafx.h"
#include "atlmisc.h"
#include "ImageOperation.h"
#include "ImageLayerOperation.h"
#include <vector>

class CBPicsPtr
{
public:
	CBPicsPtr() {}
	~CBPicsPtr() {}

	std::vector<CString>										m_picsFileNames;
	std::vector<ComPtr<IWICBitmapSource> >	m_picsWicBitmaps;
	std::vector < ComPtr<ID2D1Bitmap> >		m_picsBitmaps;
};

__interface DX_DECLARE_INTERFACE("42D792A6-A59B-404D-9BBD-1C27591A3F60") IImagePicsOperation : public IUnknown
{
	HRESULT _stdcall GetPicsEquipment(__out CBPicsPtr* te);
	HRESULT _stdcall SetPicsEquipment(__in CBPicsPtr te);
};

class ImagePicsOperation : public IImageOperation, public IImageLayerOperation, public IImagePicsOperation
{
public:
	HRESULT _stdcall GetPicsEquipment(__out CBPicsPtr* te);
	HRESULT _stdcall SetPicsEquipment(__in CBPicsPtr te);

	HRESULT _stdcall GetLayerEquipment(__out CBLayerPtr* te);
	HRESULT _stdcall SetLayerEquipment(__in CBLayerPtr te);

	void		SetActive(bool isActive);
	bool		GetActive() { return layerInfo.m_layerIsActive; }
	void		SetOpacity(FLOAT opacity);
	FLOAT	GetOpacity();
	void		SetDuration(FLOAT duration) { m_duration = duration; }
	FLOAT	GetDuration() { return m_duration; }

protected:
	ImagePicsOperation();
	virtual ~ImagePicsOperation();

	// Interface helper
	bool QueryInterfaceHelper(const IID& iid, void** object)
	{
		return CastHelper<IImageOperation>::CastTo(iid, this, object) ||
			CastHelper<IImageLayerOperation>::CastTo(iid, this, object) ||
			CastHelper<IImagePicsOperation>::CastTo(iid, this, object);
	}

private:
	CBPicsPtr      picsInfo;
	CBLayerPtr	 layerInfo;
	FLOAT			m_duration = 0.0f;
};

