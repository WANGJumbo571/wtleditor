#pragma once

#include "stdafx.h"
#include "atlmisc.h"
#include "ImageOperation.h"
#include "ImageLayerOperation.h"

class CBSVGPtr
{
public:
	CBSVGPtr() {}
	~CBSVGPtr() {}

	ComPtr<ID2D1SvgDocument>	m_svgDocument;
	float											m_scaleValue;
	int											m_svgOffSet = 0;
};

__interface DX_DECLARE_INTERFACE("42D792A6-A59B-414D-9BBD-1C27581A3F40") IImageSVGOperation : public IUnknown
{
	HRESULT _stdcall GetSVGEquipment(__out CBSVGPtr * te);
	HRESULT _stdcall SetSVGEquipment(__in CBSVGPtr te);
	int _stdcall GetSVGOffSet();
	void _stdcall SetSVGOffSet(int offset);
};

class ImageSVGOperation : public IImageOperation, public IImageLayerOperation, public IImageSVGOperation
{
public:
	HRESULT _stdcall GetSVGEquipment(__out CBSVGPtr* te);
	HRESULT _stdcall SetSVGEquipment(__in CBSVGPtr te);
	int _stdcall GetSVGOffSet() { return SVGInfo.m_svgOffSet; }
	void _stdcall SetSVGOffSet(int offset) { SVGInfo.m_svgOffSet = offset; }

	HRESULT _stdcall GetLayerEquipment(__out CBLayerPtr* te);
	HRESULT _stdcall SetLayerEquipment(__in CBLayerPtr te);

	void		SetActive(bool isActive);
	bool		GetActive() { return layerInfo.m_layerIsActive; }
	void		SetOpacity(FLOAT opacity);
	FLOAT	GetOpacity();
	void		SetDuration(FLOAT duration) { m_duration = duration; }
	FLOAT	GetDuration() { return m_duration; }

protected:
	ImageSVGOperation();
	virtual ~ImageSVGOperation();

	// Interface helper
	bool QueryInterfaceHelper(const IID& iid, void** object)
	{
		return CastHelper<IImageOperation>::CastTo(iid, this, object) ||
			CastHelper<IImageLayerOperation>::CastTo(iid, this, object) ||
			CastHelper<IImageSVGOperation>::CastTo(iid, this, object);
	}

private:
	CBSVGPtr      SVGInfo;
	CBLayerPtr	 layerInfo;
	FLOAT			m_duration = 0.0f;
};

