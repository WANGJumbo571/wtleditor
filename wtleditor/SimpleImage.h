//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================

#pragma once

#include "Image.h"
#include "DrawGeometryOperation.h"
#include <stack>
#include "ImageTextOperation.h"
#include "ImageLayerOperation.h"
#include "ImagePicsOperation.h"
#include "ImageSVGOperation.h"

#include "LayerAnimation.h"
#include "ImageRenderer.h"

using namespace Hilo::Direct2DHelpers;
using namespace Hilo::AnimationHelpers;

class SimpleImage : public IImage
{
public:
	//--------------------------------------------------------------------------------------------------------------------------
	// Getters and setters
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall GetDrawingRect(__out D2D1_RECT_F* rect);
    HRESULT __stdcall SetDrawingRect(__in const D2D1_RECT_F& rect);
    HRESULT __stdcall GetTransformedRect(D2D1_POINT_2F midPoint, __out D2D1_RECT_F* rect);
    HRESULT __stdcall GetImageInfo(__out ImageInfo* info);
    HRESULT __stdcall SetBoundingRect(__in const D2D1_RECT_F& rect);
    HRESULT __stdcall SetRenderingParameters(__in const RenderingParameters& drawingObjects);
    HRESULT __stdcall ContainsPoint(__in D2D1_POINT_2F point, __out bool* doesImageContainPoint);
    HRESULT __stdcall TranslateToAbsolutePoint(__in D2D1_POINT_2F point, __out D2D1_POINT_2F *translatedPoint);
	HRESULT __stdcall GetScale(__out float* scale);
    HRESULT __stdcall GetClipRect(__out D2D1_RECT_F* rect);

	//--------------------------------------------------------------------------------------------------------------------------
	// Rendering method
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall Draw(bool ifcenter);
    HRESULT __stdcall Load();

	//--------------------------------------------------------------------------------------------------------------------------
	// Self added function
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall New();
	HRESULT __stdcall NewFromClipboard();

	//--------------------------------------------------------------------------------------------------------------------------
	// Resource management
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall DiscardResources();
	ComPtr<IImageOperation> __stdcall GetTopOperation();

	//--------------------------------------------------------------------------------------------------------------------------
	// Image operations
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall PushImageOperation(__in IImageOperation* imageOperation);
    HRESULT __stdcall CanUndo(__out bool* canUndo);
    HRESULT __stdcall CanRedo(__out bool* canRedo);
    HRESULT __stdcall UndoImageOperation();
    HRESULT __stdcall RedoImageOperation();

	//--------------------------------------------------------------------------------------------------------------------------
	// self added functions
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall GetPastingTransformation(__out D2D1::Matrix3x2F* matrix);
	HRESULT __stdcall GetPastingReverseTransformation(D2D1::Matrix3x2F* matrix);

	//--------------------------------------------------------------------------------------------------------------------------
	// self added function, for layer rotation transforms.
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1::Matrix3x2F GetRotatableTransformation(int i, CBRotatablePtr& bp, D2D1_POINT_2F midPoint);

	int GetIndexOfOperation(ComPtr<IImageOperation> operation);
	int GetIndexOfText(ComPtr<IImageTextOperation> textOperation);
	int GetIndexOfLayer(ComPtr<IImageLayerOperation> layerOperation);

	D2D1_RECT_F		__stdcall GetDrawingRect();
	D2D1_RECT_F		__stdcall GetClipRect();
	D2D1_POINT_2F	__stdcall CLBD_GetCopyPoint(__in D2D1_POINT_2F mousePosition);
	D2D1::Matrix3x2F __stdcall GetNormalTransformationsFromTo(D2D1_POINT_2F midPoint, int from, int to);

	ComPtr<IWICBitmapSource> __stdcall GetWICBitmap();
	ComPtr<IImageOperation>	__stdcall Get_ith_Operation(int i);
	int __stdcall GetOperationsSize();
	bool __stdcall GetIfHorizontal();
	void __stdcall DeleteOperation(ComPtr<IImageOperation> currentOperation);

protected:
	//--------------------------------------------------------------------------------------------------------------------------
	// Constructor/destructor
	//--------------------------------------------------------------------------------------------------------------------------
	SimpleImage(ImageInfo imageInfo);
    virtual ~SimpleImage();

	//--------------------------------------------------------------------------------------------------------------------------
	// Interface support
	//--------------------------------------------------------------------------------------------------------------------------
	bool QueryInterfaceHelper(const IID &iid, void **object)
    {
        return CastHelper<IImage>::CastTo(iid, this, object);
    }

private:
	//--------------------------------------------------------------------------------------------------------------------------
	// SimpleImage 内部数据区
	//--------------------------------------------------------------------------------------------------------------------------
    static const float ShadowDepth;

	//--------------------------------------------------------------------------------------------------------------------------
	// Image information and Rendering parameters
	//--------------------------------------------------------------------------------------------------------------------------
	ImageInfo									m_imageInfo;
	ComPtr<ID2D1Bitmap>			m_bkgBitmap; 
	RenderingParameters				m_renderingParameters;

	//--------------------------------------------------------------------------------------------------------------------------
	// Background image's original references for three types: file, new and clipboard. 
	//--------------------------------------------------------------------------------------------------------------------------
	ComPtr<IWICBitmapSource>	m_wicSourceBitmap;

	//--------------------------------------------------------------------------------------------------------------------------
	// Image operations
	//--------------------------------------------------------------------------------------------------------------------------
	std::stack<ComPtr<IImageOperation>>	m_redoStack;
	std::vector<ComPtr<IImageOperation>>	m_imageOperations;

	D2D1_RECT_F		m_clipRect;
	D2D1_RECT_F		m_boundingRect;
	D2D1_POINT_2F	m_drawingPoint;
	D2D1_RECT_F		m_drawingRect;
	D2D1_RECT_F		m_originalDrawingRect;
	bool						m_isHorizontal;

	//--------------------------------------------------------------------------------------------------------------------------
	// SimpleImage 私有函数区
	//--------------------------------------------------------------------------------------------------------------------------
	// Bitmap methods
	HRESULT LoadBitmapFromImageInfo();

    // Rendering methods
    HRESULT DrawImage(bool ifcenter, const D2D1_RECT_F& drawingRect, const D2D1_RECT_F& imageRect, bool isSaving);
    void CalculateDrawingRect();

    void DrawShadow(const D2D1_RECT_F& bitmapRect);

    // Matrix helper functin
	D2D1::Matrix3x2F GetInverseTransformations(D2D1_POINT_2F midPoint);
	D2D1::Matrix3x2F GetNormalTransformations(D2D1_POINT_2F midPoint);
	D2D1::Matrix3x2F GetInverseTransformationsToFrom(D2D1_POINT_2F midPoint, int from, int to);

	D2D1_RECT_F GetTransformedRect(D2D1_POINT_2F midPoint, const D2D1_RECT_F& rect);

	FLOAT GetWidth(D2D1_RECT_F rect);
	FLOAT GetHeight(D2D1_RECT_F rect);
	
    // Determines if this image is rotated
    static bool IsRotationOperation(IImageOperation* operation);
	inline static bool IsRotation(ImageOperationType operationType)
	{
		return (operationType == ImageOperationTypeRotateClockwise ||
			 operationType == ImageOperationTypeRotate360 ||
			operationType == ImageOperationTypeRotateCounterClockwise);
	}
    
    inline float GetCurrentImageScale()
    {
		//--------------------------------------------------------------------------------------------------------------------------
		// image portion actual size / image portion display size
		// the m_clipRect and m_drawingRect is always in proportional shape.
		//--------------------------------------------------------------------------------------------------------------------------
        return Direct2DUtility::GetRectWidth(m_clipRect) / Direct2DUtility::GetRectWidth(m_drawingRect);
    }

    void RecalculateClipRect();

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	void TEXT_PrintOutText(ComPtr<ID2D1RenderTarget> renderTarget,
										CBTextPtr & bp, 
										D2D1_RECT_F & dest,
										FLOAT rollupFirst,
										FLOAT currentWrite,
										write_style wStyle);
	void TEXT_PrintOutTextWithoutBorder(ComPtr<ID2D1RenderTarget> renderTarget,
																CBTextPtr & bp, 
																D2D1_RECT_F & dest);
	void TEXT_DrawText(ComPtr<ID2D1RenderTarget> renderTarget,
									CBTextPtr & bp, 
									D2D1_RECT_F & dest, 
									FLOAT& cursorX0, 
									FLOAT& cursorY0,
									FLOAT rollupFirst,
									FLOAT currentWrite,
									write_style wStyle);
	void TEXT_WriteNormal(ComPtr<ID2D1RenderTarget> renderTarget,
									CBTextPtr& bp,
									D2D1_RECT_F& dest,
									FLOAT& currentWrite,
									int& intCurrentWrite,
									FLOAT& destX,
									FLOAT& destY,
									int& k);
	void TEXT_WriteOpacity(ComPtr<ID2D1RenderTarget> renderTarget,
									CBTextPtr& bp,
									D2D1_RECT_F& dest,
									FLOAT& currentWrite,
									int& intCurrentWrite,
									FLOAT& destX,
									FLOAT& destY,
									int& k);
	void TEXT_WritePunch(ComPtr<ID2D1RenderTarget> renderTarget,
									CBTextPtr& bp,
									D2D1_RECT_F& dest,
									FLOAT& currentWrite,
									int& intCurrentWrite,
									FLOAT& destX,
									FLOAT& destY,
									int& k);
	void SimpleImage::TEXT_WriteWave(ComPtr<ID2D1RenderTarget> renderTarget,
									CBTextPtr& bp,
									D2D1_RECT_F& dest,
									FLOAT& currentWrite,
									int& intCurrentWrite,
									FLOAT& destX,
									FLOAT& destY,
									int& k);
	void TEXT_PrintArrowLeftLong( CBTextPtr & bp, D2D1_POINT_2F source);
	void TEXT_PrintArrowLeft( CBTextPtr & bp, D2D1_POINT_2F source);
	void TEXT_PrintArrowRight( CBTextPtr & bp, D2D1_POINT_2F source);
	void TEXT_PrintArrowDownLong( CBTextPtr & bp, D2D1_POINT_2F source);
	void TEXT_PrintArrowDown( CBTextPtr & bp, D2D1_POINT_2F source);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	void TEXT_DrawTextOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
														ComPtr<IImageOperation> textOperation,
														CBTextPtr& bp,
														D2D1_POINT_2F x,
														float scale,
														bool isSaving,
														const D2D1_RECT_F& drawingRect,
														const D2D1_RECT_F& imageRect,
														int i,
														D2D1_POINT_2F midPoint,
														D2D1_MATRIX_3X2_F& originalTransform,
														FLOAT opacity);
	void PLYR_DrawLayerBitmapOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
														ComPtr<IImageOperation> rotaOperation,
														CBLayerPtr & bp,
														ComPtr<ID2D1Bitmap> bitmap,
														D2D1_POINT_2F x,
														float scale,
														bool isSaving,
														const D2D1_RECT_F& drawingRect,
														const D2D1_RECT_F& imageRect,
														int i,
														D2D1_POINT_2F midPoint,
														D2D1_MATRIX_3X2_F& originalTransform,
														FLOAT opacity);
	void PICS_DrawPicsBitmapOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
														ComPtr<IImageOperation> rotaOperation,
														CBLayerPtr& bp,
														CBPicsPtr& pp,
														ComPtr<ID2D1Bitmap> bitmap,
														D2D1_POINT_2F x,
														float scale,
														bool isSaving,
														const D2D1_RECT_F& drawingRect,
														const D2D1_RECT_F& imageRect,
														int i,
														D2D1_POINT_2F midPoint,
														D2D1_MATRIX_3X2_F& originalTransform,
														FLOAT opacity);
	void SVG_DrawSVGBitmapOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
														ComPtr<IImageOperation> rotaOperation,
														CBLayerPtr& bp,
														CBSVGPtr& pp,
														D2D1_POINT_2F x,
														float scale,
														bool isSaving,
														const D2D1_RECT_F& drawingRect,
														const D2D1_RECT_F& imageRect,
														int i,
														D2D1_POINT_2F midPoint,
														D2D1_MATRIX_3X2_F& originalTransform,
														FLOAT opacity);
	void GEO_DrawGeoOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
														ComPtr<IDrawGeometryOperation> geoOperation,
														float scale,
														bool isSaving,
														const D2D1_RECT_F& drawingRect,
														const D2D1_RECT_F& imageRect,
														int i,
														D2D1_POINT_2F midPoint,
														D2D1_MATRIX_3X2_F& originalTransform);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
public:
	D2D1::Matrix3x2F ROTA_GetNormalTransformations(CBRotatablePtr& bp, D2D1_POINT_2F midPoint);
	D2D1::Matrix3x2F ROTA_GetInverseTransformations(CBRotatablePtr& bp, D2D1_POINT_2F midPoint);

private:
	D2D1::Matrix3x2F ROTA_GetFlipMatrix(DOUBLE value, D2D1_POINT_2F midPoint, ImageOperationType type);
};
