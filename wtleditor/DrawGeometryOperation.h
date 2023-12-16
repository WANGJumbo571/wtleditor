//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================

#pragma once
#include "ImageOperation.h"

__interface DX_DECLARE_INTERFACE("17A76C82-5618-451F-A3D3-CEEC1A503749") IDrawGeometryOperation : public IUnknown
{
    HRESULT __stdcall DrawToRenderTarget(__in ID2D1RenderTarget* g_pD2DDeviceContext, D2D1_RECT_F imageRect);
    HRESULT __stdcall DiscardResources();

    HRESULT __stdcall AppendPoint(__in ID2D1RenderTarget* g_pD2DDeviceContext, __in D2D1_POINT_2F point);
    HRESULT __stdcall SetBrushColor(__in D2D1_COLOR_F brushColor);
    HRESULT __stdcall SetStrokeSize(__in float strokeSize);
};

class DrawGeometryOperation : public IImageOperation, public IDrawGeometryOperation
{
public:
    // IDrawGeometryOperation methods
    HRESULT __stdcall DrawToRenderTarget(__in ID2D1RenderTarget* g_pD2DDeviceContext, D2D1_RECT_F imageRect);
    HRESULT __stdcall DiscardResources()
    {
        if (m_brush)
        {
            m_brush = nullptr;
        }

        return S_OK;
    }

	void SetActive(bool isActive);
	bool GetActive() { return false; }
	void SetOpacity(FLOAT opacity);
	FLOAT GetOpacity();
	void		SetDuration(FLOAT duration) {}
	FLOAT	GetDuration() { return 0.0f; }

protected:
    DrawGeometryOperation();
    virtual ~DrawGeometryOperation();

    // Interface helper
    bool QueryInterfaceHelper(const IID &iid, void **object)
    {
        return CastHelper<IImageOperation>::CastTo(iid, this, object) ||
            CastHelper<IDrawGeometryOperation>::CastTo(iid, this, object);
    }

    // IGeometryShape
    HRESULT __stdcall AppendPoint(__in ID2D1RenderTarget* g_pD2DDeviceContext, __in D2D1_POINT_2F point);
    HRESULT __stdcall SetBrushColor(__in D2D1_COLOR_F brushColor);
    HRESULT __stdcall SetStrokeSize(__in float strokeSize);

private:
    ComPtr<ID2D1StrokeStyle> m_strokeStyle;
    ComPtr<ID2D1Geometry> m_geometry;
    ComPtr<ID2D1SolidColorBrush> m_brush;

    D2D1_COLOR_F m_brushColor;
    float m_strokeSize;

	FLOAT m_opacity = 1.0f;
	
    std::vector<D2D1_POINT_2F> m_points;

    void GetSmoothingPoints(int i, __out D2D1_POINT_2F* point1, __out D2D1_POINT_2F* point2);
    HRESULT UpdateGeometry(ID2D1RenderTarget* g_pD2DDeviceContext);
};

