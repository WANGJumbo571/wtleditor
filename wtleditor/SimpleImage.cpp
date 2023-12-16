//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================

#include "StdAfx.h"

#include <gdiplus.h> 
#include <gdiplusheaders.h>
#include <gdiplusbitmap.h>
#include <gdipluscachedbitmap.h>
#include <gdiplusimaging.h>

#include "SimpleImage.h"
#include "ImageTransformationOperation.h"
#include "ImageClippingOperation.h"
#include "ImageTextOperation.h"
#include "ImageLayerOperation.h"
#include "ImagePicsOperation.h"
#include "ImageSVGOperation.h"

#include "Direct2DUtility.h"
#include "ImageEditor.h"

#include "AnimationUtility2.h"
#include "LayerAnimation.h"

extern bool g_ifShowCursor;

using namespace Hilo::AnimationHelpers;
using namespace Hilo::Direct2DHelpers;

const float SimpleImage::ShadowDepth = 8;

D2D1::Matrix3x2F GetMatrix1(D2D1_RECT_F imageRect, D2D1_RECT_F drawingRect, float scale)
{
	return	D2D1::Matrix3x2F::Translation(-imageRect.left, -imageRect.top) *
				D2D1::Matrix3x2F::Scale(1 / scale, 1 / scale) *
				D2D1::Matrix3x2F::Translation(drawingRect.left, drawingRect.top);
}

D2D1::Matrix3x2F GetMatrix2(D2D1_RECT_F imageRect, D2D1_RECT_F drawingRect, float scale)
{
	return	D2D1::Matrix3x2F::Translation(-drawingRect.left, -drawingRect.top) *
				D2D1::Matrix3x2F::Scale(scale, scale) *
				D2D1::Matrix3x2F::Translation(imageRect.left, imageRect.top);
}

//
// Constructor
//
SimpleImage::SimpleImage(ImageInfo info) :
    m_imageInfo(info),
    m_isHorizontal(true) // by default images are loaded as horizontal
{
}

//
// Destructor
//
SimpleImage::~SimpleImage()
{
}

//
// Retrieves the current rectangle used for drawing this image
//
HRESULT SimpleImage::GetDrawingRect(D2D1_RECT_F* rect)
{
    if (rect == nullptr)
    {
        return E_POINTER;
    }

    (*rect) = m_drawingRect;

    return S_OK;
}


//
// Updates the current rectangle used for drawing this image
//
HRESULT SimpleImage::SetDrawingRect(const D2D1_RECT_F& rect)
{
    m_drawingRect = rect;
    m_originalDrawingRect = rect;
    return S_OK;
}

//
// Retrieves the current crop rectangle of this image
//
HRESULT SimpleImage::GetClipRect(D2D1_RECT_F* rect)
{
    if (rect == nullptr)
    {
        return E_POINTER;
    }

    (*rect) = m_clipRect;

    return S_OK;
}

//
// Retrieves the current ImageInfo struct for this image
HRESULT SimpleImage::GetImageInfo(ImageInfo* info)
{
    if (nullptr == info)
    {
        return E_POINTER;
    }

    (*info) = m_imageInfo;

    return S_OK;
}

//
// Specifies the bounding rectangle used to calculate the drawing rectangle for this image
//
HRESULT SimpleImage::SetBoundingRect(const D2D1_RECT_F& rect)
{
    m_boundingRect = rect;
    CalculateDrawingRect();
    return S_OK;
}

//
// Specifies the current rendering parameters for this image
//
HRESULT SimpleImage::SetRenderingParameters(const RenderingParameters& renderingParameters)
{
    m_renderingParameters = renderingParameters;

    return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// Draws the image using the current drawing rectangle
//
// m_clipRect is the image's drawing content's range(within the actual size of the original image).
//     which is also called source rectangle.
// m_drawingRect is the range of the area upon which the image's m_clipRect area is drawn.
//     which is also called destination rectangle.
//--------------------------------------------------------------------------------------------------------------------------
HRESULT SimpleImage::Draw(bool ifCenterImage)
{
    HRESULT hr = S_OK;

    if (nullptr == m_bkgBitmap)
    {
        // Load the image if needed
		hr = LoadBitmapFromImageInfo();
    }

    if (SUCCEEDED(hr))
    {
        hr = DrawImage(ifCenterImage, m_drawingRect, m_clipRect, false);
    }

    return hr;
}

//
// Loads the current image if necessary
//
HRESULT SimpleImage::Load()
{
	return LoadBitmapFromImageInfo();
}

//--------------------------------------------------------------------------------------------------------------------------
// Discards Direct2D resources associated with this image
//--------------------------------------------------------------------------------------------------------------------------
HRESULT SimpleImage::DiscardResources()
{
    for (auto operation = m_imageOperations.begin(); operation != m_imageOperations.end(); operation++)
    {
        ComPtr<IDrawGeometryOperation> drawOperation;
        if (SUCCEEDED((*operation).QueryInterface(&drawOperation)))
        {
            drawOperation->DiscardResources();
        }
    }

    m_bkgBitmap = nullptr;

    return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// Loads the specified image using the specified shell item
// 重新加载基层图片（包括三种类型：从图片文件而来，从New创建而来，从剪贴板而来）
//--------------------------------------------------------------------------------------------------------------------------
HRESULT SimpleImage::LoadBitmapFromImageInfo()
{
    HRESULT hr = S_OK;

	if (m_imageInfo.source_type == sourceFromFile)
	{
		m_wicSourceBitmap = g_render->LoadWicBitmapFromFile(m_imageInfo.fileName.c_str(), 0, 0);
	}
	
	if (SUCCEEDED(hr))
	{
		if (m_bkgBitmap == nullptr)
		{
			hr = g_pD2DDeviceContext->CreateBitmapFromWicBitmap(m_wicSourceBitmap, &m_bkgBitmap);
		}
	}

	if (SUCCEEDED(hr))
	{
		m_clipRect = D2D1::RectF(0, 0, m_bkgBitmap->GetSize().width, m_bkgBitmap->GetSize().height);
	}

	return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
// Calculates the current drawing rectangle based on the orignial image size and the specified boundary rectangle
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::CalculateDrawingRect()
{
	// Load our bitmap if necessary
	if (nullptr == m_bkgBitmap)
	{
		if (FAILED(LoadBitmapFromImageInfo()))
		{
			return;
		}
	}

	// Calculate bitmap rectangle
	float boundingWidth = m_boundingRect.right - m_boundingRect.left;
	float boundingHeight = m_boundingRect.bottom - m_boundingRect.top;

	float width = Direct2DUtility::GetRectWidth(m_clipRect);
	float height = Direct2DUtility::GetRectHeight(m_clipRect);

	if (!m_isHorizontal)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Swap width and height to calculate boundaries
		//--------------------------------------------------------------------------------------------------------------------------
		float widthTemp = width;
		width = height;
		height = widthTemp;
	}

	if (width > boundingWidth)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Width is larger than bounding box. Scale width to fit
		//--------------------------------------------------------------------------------------------------------------------------
		float scale = boundingWidth / width;
		width *= scale;
		height *= scale;
	}

	if (height > boundingHeight)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Height is larger than bounding box. Scale height to fit
		//--------------------------------------------------------------------------------------------------------------------------
		float scale = boundingHeight / height;
		width *= scale;
		height *= scale;
	}
	
	if (!m_isHorizontal)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Swap width and height to calculate boundaries
		//--------------------------------------------------------------------------------------------------------------------------
		float widthTemp = width;
		width = height;
		height = widthTemp;
	}
	
	// Update drawing rect
	m_drawingRect.left = m_boundingRect.left + boundingWidth / 2 - width / 2;
	m_drawingRect.top = m_boundingRect.top + boundingHeight / 2 - height / 2;
	m_drawingRect.right = m_drawingRect.left + width;
	m_drawingRect.bottom = m_drawingRect.top + height;
}

//--------------------------------------------------------------------------------------------------------------------------
// Draws a drop shadow around the specified rectangle
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::DrawShadow(const D2D1_RECT_F& bitmapRect)
{
    float savedOpacity = g_solidBrush->GetOpacity();
    D2D1_COLOR_F savedColor = g_solidBrush->GetColor();

    float opacity = 0.25f;
    float opacityStep = 0.25f / ShadowDepth;

	g_solidBrush->SetColor(D2D1::ColorF(0xeeeeee));
	g_pD2DDeviceContext->FillRectangle(bitmapRect, g_solidBrush);

	g_solidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    g_pD2DDeviceContext->DrawRectangle(bitmapRect, g_solidBrush);

    for (int i = 0; i < static_cast<int>(ShadowDepth); i++)
    {
        g_solidBrush->SetOpacity(opacity);
        // Draw right shadow
        g_pD2DDeviceContext->DrawLine(
            D2D1::Point2F(bitmapRect.right + i, bitmapRect.top + ShadowDepth),
            D2D1::Point2F(bitmapRect.right + i, bitmapRect.bottom + i),
            g_solidBrush);

        // Draw bottom shadow
        g_pD2DDeviceContext->DrawLine(
            D2D1::Point2F(bitmapRect.left + ShadowDepth , bitmapRect.bottom + i),
            D2D1::Point2F(bitmapRect.right + i, bitmapRect.bottom + i),
            g_solidBrush);

		opacity -= opacityStep;
    }

	g_solidBrush->SetOpacity(savedOpacity);
    g_solidBrush->SetColor(savedColor);
}

HRESULT SimpleImage::PushImageOperation(IImageOperation* imageOperation)
{
    if (nullptr == imageOperation)
    {
        return E_INVALIDARG; // a null operation is not allowed
    }

    m_imageOperations.push_back(imageOperation);

    ComPtr<IImageTransformationOperation> transformation;
    ComPtr<IImageClippingOperation> clip;

    if (SUCCEEDED(imageOperation->QueryInterface(IID_PPV_ARGS(&transformation))))
    {
        ImageOperationType transformationType;
        if (SUCCEEDED(transformation->GetTransformationType(&transformationType)) && IsRotation(transformationType))
        {
            m_isHorizontal = !m_isHorizontal;
        }
	}
    else if (SUCCEEDED(imageOperation->QueryInterface(IID_PPV_ARGS(&clip))))
    {
        // Save the current scale
        float scale = Direct2DUtility::GetRectWidth(m_clipRect) / Direct2DUtility::GetRectWidth(m_drawingRect);

        clip->GetClippingRect(&m_clipRect);
        m_clipRect = Direct2DUtility::FixRect(m_clipRect);

        // Save current clip rect
        clip->SetClippingRect(m_clipRect);

        m_drawingRect.left = m_drawingRect.left + 
										0.5f * (Direct2DUtility::GetRectWidth(m_drawingRect) - (Direct2DUtility::GetRectWidth(m_clipRect) / scale));
        m_drawingRect.right = m_drawingRect.left + Direct2DUtility::GetRectWidth(m_clipRect) / scale;
        
        m_drawingRect.top = m_drawingRect.top + 
										0.5f * (Direct2DUtility::GetRectHeight(m_drawingRect) - (Direct2DUtility::GetRectHeight(m_clipRect) / scale));
        m_drawingRect.bottom = m_drawingRect.top + Direct2DUtility::GetRectHeight(m_clipRect) / scale;

        // Save new drawing rect
        clip->SetDrawingRect(m_drawingRect);
    }

    // Since we can't redo anymore, empty redo stack
    while (!m_redoStack.empty())
    {
        m_redoStack.pop();
    }

    return S_OK;
}

HRESULT SimpleImage::ContainsPoint(D2D1_POINT_2F point, bool *doesImageContainPoint)
{    
    if (nullptr == doesImageContainPoint)
    {
        return E_POINTER;
    }

    *doesImageContainPoint = Direct2DUtility::HitTest(GetTransformedRect(Direct2DUtility::GetMidPoint(m_drawingRect), m_drawingRect), point);

    return S_OK;
}

HRESULT SimpleImage::CanUndo(__out bool* canUndo)
{
    if (nullptr == canUndo)
    {
        return E_POINTER;
    }
    
    (*canUndo) = !m_imageOperations.empty();

    return S_OK;
}

HRESULT SimpleImage::CanRedo(__out bool* canRedo)
{
    if (nullptr == canRedo)
    {
        return E_POINTER;
    }
    
    (*canRedo) = !m_redoStack.empty();

    return S_OK;
}

HRESULT SimpleImage::UndoImageOperation()
{
    if (m_imageOperations.empty())
    {
        return E_FAIL;
    }
    
    ComPtr<IImageOperation> operation = m_imageOperations.back();
    m_imageOperations.pop_back();
    m_redoStack.push(operation);

    if (IsRotationOperation(operation))
    {
        m_isHorizontal = !m_isHorizontal;
    }

    RecalculateClipRect();

    return S_OK;
}

HRESULT SimpleImage::RedoImageOperation()
{
    if (m_redoStack.empty())
    {
        return E_FAIL;
    }
    
    ComPtr<IImageOperation> operation = m_redoStack.top();
    m_redoStack.pop();
    m_imageOperations.push_back(operation);

    if (IsRotationOperation(operation))
    {
        m_isHorizontal = !m_isHorizontal;
    }

    RecalculateClipRect();

    return S_OK;
}

bool SimpleImage::IsRotationOperation(IImageOperation* operation)
{
    bool isRotation = false;
    ComPtr<IImageTransformationOperation> transformation;

    if (SUCCEEDED(operation->QueryInterface(IID_PPV_ARGS(&transformation))))
    {
        ImageOperationType transformationType;
        if (SUCCEEDED(transformation->GetTransformationType(&transformationType)) && IsRotation(transformationType))
        {
            isRotation = true;
        }
    }

    return isRotation;
}

HRESULT SimpleImage::GetTransformedRect(D2D1_POINT_2F midPoint, __out D2D1_RECT_F* rect)
{
    if (nullptr == rect)
    {
        return E_POINTER;
    }

    *rect = GetTransformedRect(midPoint, m_drawingRect);

    return S_OK;
}

D2D1_RECT_F SimpleImage::GetTransformedRect(D2D1_POINT_2F midPoint, const D2D1_RECT_F& rect)
{
    D2D1::Matrix3x2F transform = GetNormalTransformations(midPoint);

    D2D1_POINT_2F upperLeft = transform.TransformPoint(D2D1::Point2F(rect.left, rect.top));
    D2D1_POINT_2F lowerRight = transform.TransformPoint(D2D1::Point2F(rect.right, rect.bottom));

    return Direct2DUtility::FixRect(D2D1::RectF(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y));
}

void SimpleImage::RecalculateClipRect()
{
    m_clipRect = D2D1::RectF(0, 0, m_bkgBitmap->GetSize().width, m_bkgBitmap->GetSize().height); 
    m_drawingRect = m_originalDrawingRect;

    for (int i = static_cast<int>(m_imageOperations.size()) - 1; i >= 0 ; i--)
    {
        ComPtr<IImageClippingOperation> clip;
        if (SUCCEEDED(m_imageOperations[i].QueryInterface(&clip)))
        {
            clip->GetClippingRect(&m_clipRect);
            clip->GetDrawingRect(&m_drawingRect);
            // We need only the last clip
            break;
        }
    }
}

HRESULT SimpleImage::GetScale(float* scale)
{
    if (scale == nullptr)
    {
        return E_POINTER;
    }

    *scale = GetCurrentImageScale();

    return S_OK;
}

FLOAT SimpleImage::GetWidth(D2D1_RECT_F rect)
{
	return rect.right - rect.left;
}

FLOAT SimpleImage::GetHeight(D2D1_RECT_F rect)
{
	return rect.bottom - rect.top;
}

//--------------------------------------------------------------------------------------------------------------------------
// 整个图片的翻转变换的例程（InverseTransformation.）
//--------------------------------------------------------------------------------------------------------------------------
D2D1::Matrix3x2F SimpleImage::GetInverseTransformations(D2D1_POINT_2F midPoint)
{
	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Identity();

	for (int i = 0; i < static_cast<int>(m_imageOperations.size()); i++)
	{
		ComPtr<IImageTransformationOperation> transformOperation;
		if (SUCCEEDED(m_imageOperations[i].QueryInterface(&transformOperation)))
		{
			D2D1::Matrix3x2F operationTransform;
			transformOperation->GetInverseTransformationMatrix(midPoint, &operationTransform);
			transform = operationTransform * transform;
		}
	}

	return transform;
}

D2D1::Matrix3x2F SimpleImage::GetInverseTransformationsToFrom(D2D1_POINT_2F midPoint, int from, int to)
{
	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Identity();

	for (int i = from; i <= to; i++)
	{
		ComPtr<IImageTransformationOperation> transformOperation;
		if (SUCCEEDED(m_imageOperations[i].QueryInterface(&transformOperation)))
		{
			D2D1::Matrix3x2F operationTransform;
			transformOperation->GetInverseTransformationMatrix(midPoint, &operationTransform);
			transform = operationTransform * transform;
		}
	}

	return transform;
}

//--------------------------------------------------------------------------------------------------------------------------
// 整个图片的翻转变换的例程（NormalTransformation.）
//--------------------------------------------------------------------------------------------------------------------------
D2D1::Matrix3x2F SimpleImage::GetNormalTransformations(D2D1_POINT_2F midPoint)
{
	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Identity();

	for (int i = static_cast<int>(m_imageOperations.size()) - 1; i >= 0; i--)
	{
		ComPtr<IImageTransformationOperation> transformOperation;
		if (SUCCEEDED(m_imageOperations[i].QueryInterface(&transformOperation)))
		{
			D2D1::Matrix3x2F operationTransform;
			transformOperation->GetTransformationMatrix(midPoint, &operationTransform);
			transform = operationTransform * transform;
		}
	}

	return transform;
}

D2D1::Matrix3x2F SimpleImage::GetNormalTransformationsFromTo(D2D1_POINT_2F midPoint, int from, int upto)
{
	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Identity();

	for (int i = upto; i >= from; i--)
	{
		ComPtr<IImageTransformationOperation> transformOperation;
		if (SUCCEEDED(m_imageOperations[i].QueryInterface(&transformOperation)))
		{
			D2D1::Matrix3x2F operationTransform;
			transformOperation->GetTransformationMatrix(midPoint, &operationTransform);
			transform = operationTransform * transform;
		}
	}

	return transform;
}

//------------------------------------------------------------------------------------------------------------------------------
/*
D2D1_POINT_2F ImageEditorHandler::GetAbsolutePosition(D2D1_POINT_2F mousePosition)
{
// First transform back the point (disregarding current translation and scale)
mousePosition = RemoveRenderingTransformations(mousePosition);

// Translate to an absolute point within the image current drawing rect
D2D1_POINT_2F absPoint;
m_images[m_currentIndex].Image->TranslateToAbsolutePoint(mousePosition, &absPoint);

D2D1_RECT_F drawingRect;
m_images[m_currentIndex].Image->GetDrawingRect(&drawingRect);
float scale;
m_images[m_currentIndex].Image->GetScale(&scale);

// D2D1::Matrix3x2F matrix2 =
// D2D1::Matrix3x2F::Translation(-drawingRect.left, -drawingRect.top) *
// D2D1::Matrix3x2F::Scale(scale, scale) *
// D2D1::Matrix3x2F::Translation(m_clipRect.left, m_clipRect.top);

return AdjustToClipRect(D2D1_POINT_2F(Translation(-drawingRect.left, -drawingRect.top)*Scale(scale, scale)))
}
*/
//------------------------------------------------------------------------------------------------------------------------------
// 绝对坐标变成mouse坐标后转换经过了 0 - n 次的Normal Transformation，
// mouse 坐标 point 要变为绝对坐标，也就得反过来，先从 n 到 0 的 Inverse Transformation (都是在mouse坐标系里完成)。
// 最后，用matrix2加壳。invertedMatrix = Inverse_N_n * Inverse_N_i-1 * Inverse_N_i-2 ... * Inverse_N_1 * Inverse_N_0
//------------------------------------------------------------------------------------------------------------------------------
HRESULT SimpleImage::TranslateToAbsolutePoint(D2D1_POINT_2F point, D2D1_POINT_2F *translatedPoint)
{
	if (nullptr == translatedPoint)
	{
		return E_POINTER;
	}

	D2D1::Matrix3x2F invertedMatrix = GetInverseTransformationsToFrom(Direct2DUtility::GetMidPoint(m_drawingRect), 
																												0, 
																												m_imageOperations.size() - 1);
	*translatedPoint = invertedMatrix.TransformPoint(point);

	(*translatedPoint).x += m_clipRect.left / GetCurrentImageScale();
	(*translatedPoint).y += m_clipRect.top / GetCurrentImageScale();

	return S_OK;
}

#define DEFINE_END 	int end;\
									if (i < (int)bp.m_textRowStart.size() - 1)\
									{\
										end = bp.m_textRowStart.at(i + 1);\
									}\
									else\
									{\
										end = bp.m_textString.GetLength();\
									}

#define WRITE_LAYOUTRECT 	D2D1_RECT_F charRect = { dest.left + destX,\
									dest.top + destY,\
									dest.left + destX + bp.m_textLayout.at(k)->m_charLayoutWidth,\
									dest.top + destY + bp.m_textSingleCharHeight };\
									bp.m_textLayout.at(k)->m_charLayoutRect = charRect;\
									bp.m_textDest = dest;

#define WRITE_CURSORXY 	if (k == bp.m_textPoint)\
									{\
										cursorX0 = destX;\
										cursorY0 = destY;\
									}

#define WRITE_MARKEDBG 	if (bp.m_textIsActive == true &&\
									bp.m_textIfMarked == true &&\
									k >= bp.m_textMarkLeft &&\
									k < bp.m_textMarkRight)\
									{\
										D2D1_COLOR_F penOld = bp.m_textBrush->GetColor();\
										bp.m_textBrush->SetColor(D2D1::ColorF(0xffaaaaaa));\
									\
										renderTarget->FillRectangle(charRect, bp.m_textBrush);\
										bp.m_textBrush->SetColor(penOld);\
									}

#define WRITE_ANIMATION switch (wStyle)\
									{\
									case normal:\
										TEXT_WriteNormal(renderTarget, bp, dest, currentWrite, intCurrentWrite, destX, destY, k);\
										break;\
									case opacity:\
										TEXT_WriteOpacity(renderTarget, bp, dest, currentWrite, intCurrentWrite, destX, destY, k);\
										break;\
									case punch:\
										TEXT_WritePunch(renderTarget, bp, dest, currentWrite, intCurrentWrite, destX, destY, k);\
										break;\
									case wave:\
										TEXT_WriteWave(renderTarget, bp, dest, currentWrite, intCurrentWrite, destX, destY, k);\
										break;\
									default:\
										break;\
									}

#define WRITE_TEXTENDCURSORXY 	if (bp.m_textPoint == bp.m_textString.GetLength())\
									{\
										cursorX0 = destX;\
										cursorY0 = destY;\
									}

void SimpleImage::TEXT_WriteNormal(ComPtr<ID2D1RenderTarget> renderTarget,
														CBTextPtr& bp,
														D2D1_RECT_F& dest,
														FLOAT& currentWrite,
														int& intCurrentWrite,
														FLOAT& destX,
														FLOAT& destY,
														int& k)
{
	renderTarget->DrawTextLayout(D2D1::Point2F(dest.left + destX, dest.top + destY),
		bp.m_textLayout.at(k)->m_charLayout,
		bp.m_textBrush);
}

void SimpleImage::TEXT_WriteWave(ComPtr<ID2D1RenderTarget> renderTarget,
														CBTextPtr& bp,
														D2D1_RECT_F& dest,
														FLOAT& currentWrite,
														int& intCurrentWrite,
														FLOAT& destX,
														FLOAT& destY,
														int& k)
{
	D2D1::Matrix3x2F original;
	renderTarget->GetTransform(&original);
	D2D1_POINT_2F center = D2D1::Point2F(
														dest.left + destX + bp.m_textLayout.at(k)->m_charLayoutWidth / 2,
														dest.top + destY + bp.m_textLayout.at(k)->m_charLayoutHeight);
	renderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(bp.m_waveAngle, center) * original);
	renderTarget->DrawTextLayout(D2D1::Point2F(dest.left + destX, dest.top + destY),
														bp.m_textLayout.at(k)->m_charLayout,
														bp.m_textBrush);
	renderTarget->SetTransform(original);
}

void SimpleImage::TEXT_WriteOpacity(ComPtr<ID2D1RenderTarget> renderTarget,
														CBTextPtr& bp,
														D2D1_RECT_F& dest,
														FLOAT& currentWrite,
														int& intCurrentWrite,
														FLOAT& destX,
														FLOAT& destY,
														int& k)
{
	float opacityOld = bp.m_textBrush->GetOpacity();
	if (currentWrite >= 0)
	{
		int thisWrite = currentWrite;
		if (intCurrentWrite == thisWrite)
		{
			bp.m_textBrush->SetOpacity(opacityOld * (currentWrite - thisWrite));
		}
		else if (intCurrentWrite > thisWrite)
		{
			bp.m_textBrush->SetOpacity(0);
		}
	}
	renderTarget->DrawTextLayout(D2D1::Point2F(dest.left + destX, dest.top + destY),
													bp.m_textLayout.at(k)->m_charLayout,
													bp.m_textBrush);
	bp.m_textBrush->SetOpacity(opacityOld);
}

void SimpleImage::TEXT_WritePunch(ComPtr<ID2D1RenderTarget> renderTarget,
													CBTextPtr& bp,
													D2D1_RECT_F& dest, 
													FLOAT& currentWrite, 
													int& intCurrentWrite, 
													FLOAT& destX, 
													FLOAT& destY,
													int& k)
{
	if (currentWrite < 0)
	{
		return;
	}

	int thisWrite = currentWrite;
	if (intCurrentWrite == thisWrite)
	{
		float opacityOld = bp.m_textBrush->GetOpacity();
		bp.m_textBrush->SetOpacity(opacityOld * (currentWrite - thisWrite));

		FLOAT ratio = currentWrite - thisWrite;

		ComPtr<IDWriteTextLayout> pTextLayout;
		ComPtr<IDWriteTextFormat> pTextFormat;

		CString singleString = bp.m_textString.GetAt(intCurrentWrite - 1);

		FLOAT fontSize = bp.m_textFormatFontSize + (1000 - bp.m_textFormatFontSize) * (1.0 - ratio);

		g_pDWriteFactory->CreateTextFormat(bp.m_textFormatFamilyName,
															nullptr,
															DWRITE_FONT_WEIGHT_REGULAR,
															DWRITE_FONT_STYLE_NORMAL,
															DWRITE_FONT_STRETCH_NORMAL,
															fontSize,
															L"en-us",
															&pTextFormat);

		// 创建文本布局 
		g_pDWriteFactory->CreateTextLayout(singleString,
															singleString.GetLength(),
															pTextFormat,
															(FLOAT)1200,
															(FLOAT)1200,
															&pTextLayout);

		// 获取文本尺寸  
		DWRITE_TEXT_METRICS textMetrics;
		pTextLayout->GetMetrics(&textMetrics);

		FLOAT charLayoutHeight = textMetrics.height;
		FLOAT charLayoutWidth = textMetrics.widthIncludingTrailingWhitespace;

		FLOAT thisX = dest.left + destX - (charLayoutWidth - bp.m_textLayout.at(k)->m_charLayoutWidth) / 2;
		FLOAT thisY = dest.top + destY - (charLayoutHeight - bp.m_textLayout.at(k)->m_charLayoutHeight) / 2;

		renderTarget->DrawTextLayout(D2D1::Point2F(thisX, thisY), pTextLayout, bp.m_textBrush);

		bp.m_textBrush->SetOpacity(opacityOld);
	}
	else if (intCurrentWrite < thisWrite)
	{
		renderTarget->DrawTextLayout(D2D1::Point2F(dest.left + destX, dest.top + destY),
														bp.m_textLayout.at(k)->m_charLayout,
														bp.m_textBrush);
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// text 层文字部分的绘制
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::TEXT_DrawText(ComPtr<ID2D1RenderTarget> renderTarget,
													CBTextPtr & bp, 
													D2D1_RECT_F & dest, 
													FLOAT& cursorX0, 
													FLOAT& cursorY0,
													FLOAT rollupFirst,
													FLOAT currentWrite,
													write_style wStyle)
{
	int intCurrentWrite = 0;
	
	D2D1_COLOR_F colorOld = bp.m_textBrush->GetColor();
	float opacityOld = bp.m_textBrush->GetOpacity();

	bp.m_textBrush->SetColor(bp.m_textColor);
	bp.m_textBrush->SetOpacity(bp.m_textOpacity);

	//--------------------------------------------------------------------------------------------------------------------------
	// 从上到下，从左到右绘制文字（现代汉语风格）
	//--------------------------------------------------------------------------------------------------------------------------
	switch (bp.m_textStyle)
	{
	case style_f:
	{
		FLOAT destX = DISTANCE;
		FLOAT destY = rollupFirst;
		cursorX0 = DISTANCE;
		cursorY0 = 0;

		for (int i = 0; i < (int)bp.m_textRowStart.size(); i++)
		{
			DEFINE_END;

			destX = DISTANCE;
			for (int k = bp.m_textRowStart.at(i); k < end; k++)
			{
				intCurrentWrite++;

				WRITE_CURSORXY;
				WRITE_LAYOUTRECT; 
				WRITE_MARKEDBG;
				WRITE_ANIMATION;

				destX += (bp.m_textLayout.at(k)->m_charLayoutWidth + bp.m_textCharSpace);
			}

			destY += (bp.m_textSingleCharHeight + bp.m_textLineSpace);
		}

		destY -= (bp.m_textSingleCharHeight + bp.m_textLineSpace);

		WRITE_TEXTENDCURSORXY;
		break;
	}
	case style_c:
	{
		FLOAT destX = DISTANCE;
		FLOAT destY = rollupFirst;
		cursorX0 = DISTANCE;
		cursorY0 = 0;

		for (int i = 0; i < (int)bp.m_textRowStart.size(); i++)
		{
			DEFINE_END;

			FLOAT thisLineLength = 0;
			for (int k = bp.m_textRowStart.at(i); k < end; k++)
			{
				thisLineLength += bp.m_textLayout.at(k)->m_charLayoutWidth;
			}

			destX = (bp.m_rotatableBorderWidth - thisLineLength) / 2;
			for (int k = bp.m_textRowStart.at(i); k < end; k++)
			{
				intCurrentWrite++;

				WRITE_CURSORXY;
				WRITE_LAYOUTRECT;
				WRITE_MARKEDBG;
				WRITE_ANIMATION;

				destX += (bp.m_textLayout.at(k)->m_charLayoutWidth + bp.m_textCharSpace);
			}

			destY += (bp.m_textSingleCharHeight + bp.m_textLineSpace);
		}

		destY -= (bp.m_textSingleCharHeight + bp.m_textLineSpace);

		WRITE_TEXTENDCURSORXY;
		break;
	}
	case style_r:
	{
		FLOAT destX = DISTANCE;
		FLOAT destY = rollupFirst;
		cursorX0 = DISTANCE;
		cursorY0 = 0;

		for (int i = 0; i < (int)bp.m_textRowStart.size(); i++)
		{
			DEFINE_END;

			FLOAT thisLineLength = 0;
			for (int k = bp.m_textRowStart.at(i); k < end; k++)
			{
				thisLineLength += bp.m_textLayout.at(k)->m_charLayoutWidth;
			}

			destX = bp.m_rotatableBorderWidth - thisLineLength - DISTANCE;
			for (int k = bp.m_textRowStart.at(i); k < end; k++)
			{
				intCurrentWrite++;

				WRITE_CURSORXY;
				WRITE_LAYOUTRECT;
				WRITE_MARKEDBG;
				WRITE_ANIMATION;

				destX += (bp.m_textLayout.at(k)->m_charLayoutWidth + bp.m_textCharSpace);
			}

			destY += (bp.m_textSingleCharHeight + bp.m_textLineSpace);
		}

		destY -= (bp.m_textSingleCharHeight + bp.m_textLineSpace);

		WRITE_TEXTENDCURSORXY;
		break;
	}
	//--------------------------------------------------------------------------------------------------------------------------
	// 从右到左，从上到下绘制文字 （中文古籍风格）
	//--------------------------------------------------------------------------------------------------------------------------
	case style_b:
	{
		FLOAT destX = bp.m_rotatableBorderWidth - bp.m_textSingleCharWidth - DISTANCE + rollupFirst;
		FLOAT destY = DISTANCE;
		cursorX0 = destX;
		cursorY0 = 0;

		for (int i = 0; i < (int)bp.m_textRowStart.size(); i++)
		{
			DEFINE_END;

			destY = DISTANCE;
			for (int k = bp.m_textRowStart.at(i); k < end; k++)
			{
				intCurrentWrite++;

				WRITE_CURSORXY;
				WRITE_LAYOUTRECT;
				WRITE_MARKEDBG;
				WRITE_ANIMATION;

				destY += (bp.m_textLayout.at(k)->m_charLayoutHeight + bp.m_textCharSpace);
			}

			destX -= (bp.m_textSingleCharWidth + bp.m_textLineSpace);
		}

		destX += (bp.m_textSingleCharWidth + bp.m_textLineSpace);

		WRITE_TEXTENDCURSORXY;
		break;
	}
	default:
		break;
	}

	bp.m_textBrush->SetColor(colorOld);
	bp.m_textBrush->SetOpacity(opacityOld);
}

#define LLENGTH 50.0f
#define SLENGTH 25.0f
#define SSLENGTH 5.0f
#define CORNER 10.0f
#define MARG (SSLENGTH+CORNER)

void SimpleImage::TEXT_PrintArrowLeftLong( CBTextPtr & bp, D2D1_POINT_2F source)
{
	g_pD2DDeviceContext->DrawLine(source, D2D1::Point2F(source.x - LLENGTH, source.y), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x - LLENGTH, source.y), D2D1::Point2F(source.x - LLENGTH + SSLENGTH, source.y - SSLENGTH), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x - LLENGTH, source.y), D2D1::Point2F(source.x - LLENGTH + SSLENGTH, source.y + SSLENGTH), bp.m_textBrush, 3);
}

void SimpleImage::TEXT_PrintArrowLeft( CBTextPtr & bp, D2D1_POINT_2F source)
{
	g_pD2DDeviceContext->DrawLine(source, D2D1::Point2F(source.x - SLENGTH, source.y), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x - SLENGTH, source.y), D2D1::Point2F(source.x - SLENGTH + SSLENGTH, source.y - SSLENGTH), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x - SLENGTH, source.y), D2D1::Point2F(source.x - SLENGTH + SSLENGTH, source.y + SSLENGTH), bp.m_textBrush, 3);
}

void SimpleImage::TEXT_PrintArrowRight( CBTextPtr & bp, D2D1_POINT_2F source)
{
	g_pD2DDeviceContext->DrawLine(source, D2D1::Point2F(source.x + SLENGTH, source.y), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x + SLENGTH, source.y), D2D1::Point2F(source.x + SLENGTH - SSLENGTH, source.y - SSLENGTH), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x + SLENGTH, source.y), D2D1::Point2F(source.x + SLENGTH - SSLENGTH, source.y + SSLENGTH), bp.m_textBrush, 3);
}

void SimpleImage::TEXT_PrintArrowDownLong( CBTextPtr & bp, D2D1_POINT_2F source)
{
	g_pD2DDeviceContext->DrawLine(source, D2D1::Point2F(source.x, source.y + LLENGTH), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x, source.y + LLENGTH), D2D1::Point2F(source.x - SSLENGTH, source.y + LLENGTH - SSLENGTH), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x, source.y + LLENGTH), D2D1::Point2F(source.x + SSLENGTH, source.y + LLENGTH - SSLENGTH), bp.m_textBrush, 3);
}

void SimpleImage::TEXT_PrintArrowDown( CBTextPtr & bp, D2D1_POINT_2F source)
{
	g_pD2DDeviceContext->DrawLine(source, D2D1::Point2F(source.x, source.y + SLENGTH), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x, source.y + SLENGTH), D2D1::Point2F(source.x - SSLENGTH, source.y + SLENGTH - SSLENGTH), bp.m_textBrush, 3);
	g_pD2DDeviceContext->DrawLine(D2D1::Point2F(source.x, source.y + SLENGTH), D2D1::Point2F(source.x + SSLENGTH, source.y + SLENGTH - SSLENGTH), bp.m_textBrush, 3);
}

void SimpleImage::TEXT_PrintOutTextWithoutBorder(ComPtr<ID2D1RenderTarget> renderTarget, CBTextPtr & bp, D2D1_RECT_F & dest)
{
	ComPtr<ID2D1Layer> layer;
	renderTarget->CreateLayer(D2D1::SizeF(Direct2DUtility::GetRectWidth(dest), Direct2DUtility::GetRectHeight(dest)), &layer);
	renderTarget->PushLayer(D2D1::LayerParameters(dest), layer);

	FLOAT cursorX0;
	FLOAT cursorY0;
	TEXT_DrawText(renderTarget, bp, dest, cursorX0, cursorY0, 0, -1, normal);

	renderTarget->PopLayer();
}

#define CURSOR_WIDTH 2
//--------------------------------------------------------------------------------------------------------------------------
// 绘制出Text层的全部内容，有必要的话画出Border和方向箭头以及编辑光标
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::TEXT_PrintOutText(ComPtr<ID2D1RenderTarget> renderTarget,
															CBTextPtr & bp, 
															D2D1_RECT_F & dest, 
															FLOAT rollupFirst, 
															FLOAT currentWrite,
															write_style wStyle)
{
	ComPtr<ID2D1Layer> layer;
	renderTarget->CreateLayer(D2D1::SizeF(Direct2DUtility::GetRectWidth(dest) + MARG, Direct2DUtility::GetRectHeight(dest) + MARG), &layer);
	D2D1_RECT_F new_dest;
	switch (bp.m_textStyle)
	{
	case style_f:
	case style_c:
	case style_r:
		new_dest = { dest.left - MARG, dest.top - MARG, dest.right, dest.bottom };
		break;
	case style_b:
		new_dest = { dest.left, dest.top - MARG, dest.right + MARG, dest.bottom };
		break;
	default:
		break;
	}

	if (currentWrite < 0)
	{
		renderTarget->PushLayer(D2D1::LayerParameters(new_dest), layer);
	}

	FLOAT cursorX0;
	FLOAT cursorY0;

	TEXT_DrawText(renderTarget, bp, dest, cursorX0, cursorY0, rollupFirst, currentWrite, wStyle);
	
	FLOAT x0 = dest.left;
	FLOAT y0 = dest.top;

	if (bp.m_textIsActive)
	{
		renderTarget->DrawRectangle(dest, bp.m_textBrush, 3);

		ComPtr<ID2D1Layer> handle_layer;
		renderTarget->CreateLayer(D2D1::SizeF(DISTANCE, DISTANCE), &handle_layer);
		D2D1_RECT_F handle_dest;
		handle_dest = { dest.left, dest.top, dest.left + DISTANCE, dest.top + DISTANCE };
		renderTarget->PushLayer(D2D1::LayerParameters(handle_dest), handle_layer);

		D2D1_ELLIPSE elli;
		elli.point = D2D1::Point2F(dest.left, dest.top);
		elli.radiusX = DISTANCE;
		elli.radiusY = DISTANCE;
		renderTarget->FillEllipse(elli, bp.m_textBrush);

		renderTarget->PopLayer();

		switch (bp.m_textStyle)
		{
		case style_b:
			TEXT_PrintArrowDown(bp, D2D1::Point2F(dest.right + CORNER, dest.top - CORNER));
			TEXT_PrintArrowLeftLong(bp, D2D1::Point2F(dest.right + CORNER, dest.top - CORNER));
			break;
		case style_f:
		case style_r:
			TEXT_PrintArrowRight(bp, D2D1::Point2F(dest.left - CORNER, dest.top - CORNER));
			TEXT_PrintArrowDownLong(bp, D2D1::Point2F(dest.left - CORNER, dest.top - CORNER));
			break;
		case style_c:
			TEXT_PrintArrowLeft(bp, D2D1::Point2F((dest.left + dest.right)/2, dest.top - CORNER));
			TEXT_PrintArrowRight(bp, D2D1::Point2F((dest.left + dest.right) / 2, dest.top - CORNER));
			break;
		default:
			break;
		}

		if (g_ifShowCursor)
		{
			D2D1_COLOR_F colorOld = bp.m_textBrush->GetColor();
			bp.m_textBrush->SetColor(D2D1::ColorF(0xffffff));

			switch (bp.m_textStyle)
			{
			case style_f:
			case style_c:
			case style_r:
			{
				FLOAT x1 = x0 + cursorX0;
				FLOAT y1 = y0 + cursorY0;
				FLOAT x2 = x0 + cursorX0;
				FLOAT y2 = y0 + cursorY0 + bp.m_textSingleCharHeight;
				renderTarget->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), bp.m_textBrush, CURSOR_WIDTH);
				FLOAT x3 = x1 - 10;
				FLOAT x4 = x1 - 2;
				renderTarget->DrawLine(D2D1::Point2F(x3, y1), D2D1::Point2F(x4, y1), bp.m_textBrush, CURSOR_WIDTH);
				FLOAT x5 = x1 + 10;
				FLOAT x6 = x1 + 2;
				renderTarget->DrawLine(D2D1::Point2F(x5, y1), D2D1::Point2F(x6, y1), bp.m_textBrush, CURSOR_WIDTH);
				renderTarget->DrawLine(D2D1::Point2F(x3, y2), D2D1::Point2F(x4, y2), bp.m_textBrush, CURSOR_WIDTH);
				renderTarget->DrawLine(D2D1::Point2F(x5, y2), D2D1::Point2F(x6, y2), bp.m_textBrush, CURSOR_WIDTH);
				break;
			}
			case style_b:
			{
				FLOAT x1 = x0 + cursorX0;
				FLOAT y1 = y0 + cursorY0;
				FLOAT x2 = x0 + cursorX0 + bp.m_textSingleCharWidth;
				FLOAT y2 = y0 + cursorY0;
				renderTarget->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), bp.m_textBrush, CURSOR_WIDTH);
				FLOAT y3 = y1 - 10;
				FLOAT y4 = y1 - 2;
				renderTarget->DrawLine(D2D1::Point2F(x1, y3), D2D1::Point2F(x1, y4), bp.m_textBrush, CURSOR_WIDTH);
				FLOAT y5 = y1 + 10;
				FLOAT y6 = y1 + 2;
				renderTarget->DrawLine(D2D1::Point2F(x1, y5), D2D1::Point2F(x1, y6), bp.m_textBrush, CURSOR_WIDTH);
				renderTarget->DrawLine(D2D1::Point2F(x2, y3), D2D1::Point2F(x2, y4), bp.m_textBrush, CURSOR_WIDTH);
				renderTarget->DrawLine(D2D1::Point2F(x2, y5), D2D1::Point2F(x2, y6), bp.m_textBrush, CURSOR_WIDTH);
				break;
			}
			default:
				break;
			}

			bp.m_textBrush->SetColor(colorOld);
		}
	}

	if (currentWrite < 0)
	{
		renderTarget->PopLayer();
	}
}

ComPtr<IImageOperation> SimpleImage::GetTopOperation()
{
	if (m_imageOperations.size() > 0)
		return m_imageOperations.at(m_imageOperations.size() - 1);
	return nullptr;
}

D2D1_RECT_F SimpleImage::GetDrawingRect()
{
	return m_drawingRect;
}

D2D1_RECT_F SimpleImage::GetClipRect()
{
	return m_clipRect;
}

D2D1_POINT_2F SimpleImage::CLBD_GetCopyPoint(D2D1_POINT_2F mousePosition)
{
	D2D1_RECT_F rect = GetTransformedRect(Direct2DUtility::GetMidPoint(m_drawingRect), m_drawingRect);
	float scale = GetCurrentImageScale();
	return D2D1::Point2F((mousePosition.x - rect.left) * scale, (mousePosition.y - rect.top) * scale);
}

//--------------------------------------------------------------------------------------------------------------------------
// 得到整个编辑区域的最后生成的图像，以IWICBitmapSource形式返回
//--------------------------------------------------------------------------------------------------------------------------
ComPtr<IWICBitmapSource> SimpleImage::GetWICBitmap()
{
	ComPtr<IWICBitmap> wicBitmap;

	//--------------------------------------------------------------------------------------------------------------------------
	// Get the original bitmap rectangle in terms of the current crop
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1_RECT_F originalBitmapRect =
							D2D1::RectF(0, 0, Direct2DUtility::GetRectWidth(m_clipRect), Direct2DUtility::GetRectHeight(m_clipRect));

	//--------------------------------------------------------------------------------------------------------------------------
	// Adjust height and width based on current orientation and clipping rectangle
	//--------------------------------------------------------------------------------------------------------------------------
	float width = m_isHorizontal ? Direct2DUtility::GetRectWidth(m_clipRect) : Direct2DUtility::GetRectHeight(m_clipRect);
	float height = m_isHorizontal ? Direct2DUtility::GetRectHeight(m_clipRect) : Direct2DUtility::GetRectWidth(m_clipRect);

	HRESULT hr = S_OK;
	reportError(110, (hr = g_pWICFactory->CreateBitmap(
																					static_cast<unsigned int>(width),
																					static_cast<unsigned int>(height),
																					GUID_WICPixelFormat32bppPBGRA,
																					WICBitmapCacheOnLoad,
																					&wicBitmap)));

	g_wicRenderTarget = nullptr;
	D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();

	reportError(111, (hr = g_pD2DFactory->CreateWicBitmapRenderTarget(wicBitmap, rtProps, &g_wicRenderTarget)));

	//--------------------------------------------------------------------------------------------------------------------------
	// Replace current bitmap with one that's compatible with the WIC render target
	//--------------------------------------------------------------------------------------------------------------------------
	m_bkgBitmap = nullptr;

	reportError(112, (hr = g_wicRenderTarget->CreateBitmapFromWicBitmap(m_wicSourceBitmap, &m_bkgBitmap)));

	if (SUCCEEDED(hr))
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// When rotating images make sure that the point around which rotation occurs lines
		// up with the center of the rotated render target
		//--------------------------------------------------------------------------------------------------------------------------
		if (false == m_isHorizontal)
		{
			float offsetX;
			float offsetY;

			if (width > height)
			{
				offsetX = (width - height) / 2;
				offsetY = -offsetX;
			}
			else
			{
				offsetY = (height - width) / 2;
				offsetX = -offsetY;
			}

			D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(offsetX, offsetY);
			g_wicRenderTarget->SetTransform(translation); 
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// Draw updated image to WIC render target
		//--------------------------------------------------------------------------------------------------------------------------
		g_wicRenderTarget->BeginDraw();
		hr = DrawImage(true, originalBitmapRect, m_clipRect, true);
		g_wicRenderTarget->EndDraw();
	}
	
	m_bkgBitmap = nullptr;
	reportError(113, (hr = g_pD2DDeviceContext->CreateBitmapFromWicBitmap(m_wicSourceBitmap, &m_bkgBitmap)));
	
	return static_cast<ComPtr<IWICBitmapSource> >(wicBitmap);
}

int SimpleImage::GetOperationsSize()
{
	return m_imageOperations.size();
}

ComPtr<IImageOperation> SimpleImage::Get_ith_Operation(int i)
{
	return m_imageOperations.at(i);
}

//--------------------------------------------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------------------------------------------
HRESULT SimpleImage::New()
{
	ComPtr<ID2D1RenderTarget>	wicRenderTarget;

	float width = m_imageInfo.bkgSize.width;
	float height = m_imageInfo.bkgSize.height;

	D2D1_RECT_F originalBitmapRect = D2D1::RectF(0, 0, width, height);
	
	ComPtr<IWICBitmap> bkgBitmap;

	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Create WIC bitmap for rendering
		//--------------------------------------------------------------------------------------------------------------------------
		hr = g_pWICFactory->CreateBitmap(
														static_cast<unsigned int>(width),
														static_cast<unsigned int>(height),
														GUID_WICPixelFormat32bppPBGRA,
														WICBitmapCacheOnLoad,
														&bkgBitmap);
	}

	if (SUCCEEDED(hr))
	{
		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();

		hr = g_pD2DFactory->CreateWicBitmapRenderTarget(bkgBitmap, rtProps, &wicRenderTarget);
	}
	
	wicRenderTarget->BeginDraw();
	wicRenderTarget->Clear(m_imageInfo.bkgColor);
	wicRenderTarget->EndDraw();

	m_wicSourceBitmap = static_cast<ComPtr<IWICBitmapSource>>(bkgBitmap);
	//--------------------------------------------------------------------------------------------------------------------------
	// Create ComPtr<ID2D1Bitmap> and m_clipRect.
	//--------------------------------------------------------------------------------------------------------------------------
	g_pD2DDeviceContext->CreateBitmapFromWicBitmap(m_wicSourceBitmap, &m_bkgBitmap);

	m_clipRect = D2D1::RectF(0, 0, width, height);
	
	return S_OK;
}

D2D1::Matrix3x2F SimpleImage::ROTA_GetNormalTransformations(CBRotatablePtr& bp, D2D1_POINT_2F midPoint)
{
	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Identity();

	for (int i = 0; i < (int)bp.m_rotatableOperations.size(); i++)
	{
		ComPtr<IImageTransformationOperation> transformOperation;
		if (SUCCEEDED(bp.m_rotatableOperations[i].QueryInterface(&transformOperation)))
		{
			D2D1::Matrix3x2F operationTransform;
			transformOperation->GetTransformationMatrix(midPoint, &operationTransform);
			transform = transform * operationTransform;
		}
	}

	return transform;
}

D2D1::Matrix3x2F SimpleImage::ROTA_GetInverseTransformations(CBRotatablePtr& bp, D2D1_POINT_2F midPoint)
{
	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Identity();

	for (int i = 0; i < (int)bp.m_rotatableOperations.size(); i++)
	{
		ComPtr<IImageTransformationOperation> transformOperation;
		if (SUCCEEDED(bp.m_rotatableOperations[i].QueryInterface(&transformOperation)))
		{
			D2D1::Matrix3x2F operationTransform;
			transformOperation->GetInverseTransformationMatrix(midPoint, &operationTransform);
			transform = operationTransform * transform;
		}
	}

	return transform;
}

//--------------------------------------------------------------------------------------------------------------------------
// 函数参数 rotateMidPoint 为相对值，在函数内部转换为 临场值
//--------------------------------------------------------------------------------------------------------------------------
D2D1::Matrix3x2F SimpleImage::GetRotatableTransformation(int i, CBRotatablePtr& bp, D2D1_POINT_2F rotateMidPoint)
{
	float scale = GetCurrentImageScale();
	D2D1_RECT_F drawingRect = m_drawingRect;
	D2D1_POINT_2F midPoint = Direct2DUtility::GetMidPoint(drawingRect);

	D2D1::Matrix3x2F matrix1 = GetMatrix1(m_clipRect, drawingRect, scale);
	D2D1::Matrix3x2F matrix2 = GetMatrix2(m_clipRect, drawingRect, scale);
	//--------------------------------------------------------------------------------------------------------------------------
	// matrix3 负责整个图片 0 - i 阶段的旋转/翻转
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1::Matrix3x2F matrix3 = GetNormalTransformationsFromTo(midPoint, 0, i);

	rotateMidPoint = matrix1.TransformPoint(rotateMidPoint);
	
	return matrix1 * matrix3 * ROTA_GetInverseTransformations(bp, rotateMidPoint) * matrix2;
}

//--------------------------------------------------------------------------------------------------------------------------
// GetPastingTransformation 仅仅在 Text/Picture 层的创建之初调用，所以 m_imageOperations.size() 指向创建时的
// Operation 堆栈长度
//--------------------------------------------------------------------------------------------------------------------------
HRESULT SimpleImage::GetPastingTransformation(D2D1::Matrix3x2F* matrix)
{
	float scale = GetCurrentImageScale();
	D2D1_RECT_F drawingRect = m_drawingRect;
	D2D1_POINT_2F midPoint = Direct2DUtility::GetMidPoint(drawingRect);

	D2D1::Matrix3x2F matrix1 = GetMatrix1(m_clipRect, drawingRect, scale);
	D2D1::Matrix3x2F matrix2 = GetMatrix2(m_clipRect, drawingRect, scale);
	D2D1::Matrix3x2F matrix3 = GetNormalTransformationsFromTo(midPoint, 0, m_imageOperations.size() - 1);

	*matrix = matrix1 * matrix3 * matrix2;

	return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// 同上
//--------------------------------------------------------------------------------------------------------------------------
HRESULT SimpleImage::GetPastingReverseTransformation(D2D1::Matrix3x2F* matrix)
{
	float scale = GetCurrentImageScale();
	D2D1_RECT_F drawingRect = m_drawingRect;
	D2D1_POINT_2F midPoint = Direct2DUtility::GetMidPoint(drawingRect);

	D2D1::Matrix3x2F matrix1 = GetMatrix1(m_clipRect, drawingRect, scale);
	D2D1::Matrix3x2F matrix2 = GetMatrix2(m_clipRect, drawingRect, scale);
	D2D1::Matrix3x2F matrix3 = GetInverseTransformationsToFrom(midPoint, 0, m_imageOperations.size() - 1);

	*matrix = matrix1 * matrix3 * matrix2;

	return S_OK;
}

bool SimpleImage::GetIfHorizontal()
{
	return m_isHorizontal;
}

HRESULT SimpleImage::NewFromClipboard()
{
	float width = m_imageInfo.bkgSize.width;
	float height = m_imageInfo.bkgSize.height;

	m_wicSourceBitmap = m_imageInfo.wicSourceClipBitmap;
	//--------------------------------------------------------------------------------------------------------------------------
	// Create ComPtr<ID2D1Bitmap> and m_clipRect.
	//--------------------------------------------------------------------------------------------------------------------------
	g_pD2DDeviceContext->CreateBitmapFromWicBitmap(m_wicSourceBitmap, &m_bkgBitmap);

	m_clipRect = D2D1::RectF(0, 0, width, height);

	return S_OK;
}

//extern std::vector<ComPtr<ID2D1Bitmap1> > storedBitmaps;
//extern int QUEUESIZE;
//D2D1_RECT_F drawingRectFromSimpleImage;

//--------------------------------------------------------------------------------------------------------------------------
// Draws the image or portion of it using the given rectangle
//--------------------------------------------------------------------------------------------------------------------------
HRESULT SimpleImage::DrawImage(bool ifcenter, const D2D1_RECT_F& drawingRect, const D2D1_RECT_F& imageRect, bool isSaving)
{
	ComPtr<ID2D1RenderTarget> renderTarget;
	if (!isSaving)
	{
		renderTarget = g_pD2DDeviceContext;
	}
	else {
		renderTarget = g_wicRenderTarget;
	}

	D2D1_MATRIX_3X2_F originalTransform;
	renderTarget->GetTransform(&originalTransform);

	//--------------------------------------------------------------------------------------------------------------------------
	// midPoint 为旋转/翻转图片中心的 临场值，一般为窗口Client区域坐标内的中心点位置
	//                                                  ====
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1_POINT_2F midPoint = Direct2DUtility::GetMidPoint(drawingRect);
	D2D1_MATRIX_3X2_F mat = GetNormalTransformations(midPoint) * originalTransform;
	renderTarget->SetTransform(mat);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	
	if (ifcenter && g_isDrawingRippleOn && !isSaving)
	{
		g_render->Ripple_Render();
	}
	else if (ifcenter && g_isDrawingScrollOn && !isSaving)
	{
		g_render->HScroll_Render();
	}
	else if (ifcenter && g_isDrawingVScrollOn && !isSaving)
	{
		g_render->VScroll_Render();
	}
	else  if (ifcenter || (!g_isDrawingRippleOn && !g_isDrawingScrollOn && !g_isDrawingVScrollOn))
	{
		if (!isSaving) 
		{
			DrawShadow(drawingRect);
			g_pD2DDeviceContext->DrawBitmap(m_bkgBitmap, drawingRect, 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, imageRect);
		}
		else 
		{
			g_wicRenderTarget->DrawBitmap(m_bkgBitmap, drawingRect, 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, imageRect);
		}
	}
	
	ComPtr<ID2D1Layer> layer;
	renderTarget->CreateLayer(
		D2D1::SizeF(	Direct2DUtility::GetRectWidth(drawingRect), Direct2DUtility::GetRectHeight(drawingRect)), 
		&layer);
	renderTarget->PushLayer(D2D1::LayerParameters(drawingRect), layer);

	// The scale is relative to the full bitmap size
	float scale = GetCurrentImageScale();

	for (int i = 0; i <= static_cast<int>(m_imageOperations.size()) - 1; i++)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// text operation.
		//--------------------------------------------------------------------------------------------------------------------------
		ComPtr<IImageTextOperation> textOperation;
		if (SUCCEEDED((m_imageOperations[i]).QueryInterface(&textOperation)))
		{
			CBTextPtr bp;
			textOperation->GetTextEquipment(&bp);

			D2D1_POINT_2F x = bp.m_pastingReverseMatrix.TransformPoint(bp.m_rotatableRelativePt);

			TEXT_DrawTextOnRenderTarget(renderTarget, m_imageOperations[i],
															bp,
															x,
															scale,
															isSaving,
															drawingRect,
															imageRect,
															i,
															midPoint,
															originalTransform,
															bp.m_textOpacity);

			textOperation->SetTextEquipment(bp);
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// layer operation.
		//--------------------------------------------------------------------------------------------------------------------------
		ComPtr<IImageLayerOperation>		layerOperation;
		ComPtr<IImagePicsOperation>		picsOperation;
		ComPtr<IImageSVGOperation>		svgOperation;
		if (SUCCEEDED((m_imageOperations[i]).QueryInterface(&layerOperation)) &&
			!SUCCEEDED((m_imageOperations[i]).QueryInterface(&picsOperation)) &&
			!SUCCEEDED((m_imageOperations[i]).QueryInterface(&svgOperation)))
		{
			CBLayerPtr bp;
			layerOperation->GetLayerEquipment(&bp);

			D2D1_POINT_2F relativePt = bp.m_rotatableRelativePt;
			//--------------------------------------------------------------------------------------------------------------------------
			// x 为 relativePt 对应的 absolute Point 之值.
			//--------------------------------------------------------------------------------------------------------------------------
			D2D1_POINT_2F x = bp.m_pastingReverseMatrix.TransformPoint(relativePt);

			ComPtr<ID2D1Bitmap> bitmap;
			if (isSaving)
			{
				renderTarget->CreateBitmapFromWicBitmap(bp.m_layerWicBitmap, &bitmap);
			}
			else
			{
				bitmap = bp.m_layerBitmap;
			}

			PLYR_DrawLayerBitmapOnRenderTarget(renderTarget, m_imageOperations[i],
																bp, 
																bitmap, 
																x, 
																scale, 
																isSaving, 
																drawingRect, 
																imageRect, 
																i, 
																midPoint, 
																originalTransform, 
																bp.m_layerOpacity);
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// pics operation.
		//--------------------------------------------------------------------------------------------------------------------------
		picsOperation = nullptr;
		layerOperation = nullptr;
		if (SUCCEEDED((m_imageOperations[i]).QueryInterface(&layerOperation)) && 
			SUCCEEDED((m_imageOperations[i]).QueryInterface(&picsOperation)))
		{
			CBLayerPtr bp;
			layerOperation->GetLayerEquipment(&bp);
			CBPicsPtr pp;
			picsOperation->GetPicsEquipment(&pp);

			D2D1_POINT_2F relativePt = bp.m_rotatableRelativePt;
			//--------------------------------------------------------------------------------------------------------------------------
			// x 为 relativePt 对应的 absolute Point 之值.
			//--------------------------------------------------------------------------------------------------------------------------
			D2D1_POINT_2F x = bp.m_pastingReverseMatrix.TransformPoint(relativePt);

			ComPtr<ID2D1Bitmap> bitmap;
			if (isSaving)
			{
				renderTarget->CreateBitmapFromWicBitmap(pp.m_picsWicBitmaps[0], &bitmap);
			}
			else
			{
				bitmap = pp.m_picsBitmaps[0];
			}

			PICS_DrawPicsBitmapOnRenderTarget(renderTarget, m_imageOperations[i],
																		bp,
																		pp,
																		bitmap,
																		x,
																		scale,
																		isSaving,
																		drawingRect,
																		imageRect,
																		i,
																		midPoint,
																		originalTransform,
																		bp.m_layerOpacity);
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// svg operation.
		//--------------------------------------------------------------------------------------------------------------------------
		svgOperation = nullptr;
		layerOperation = nullptr;
		if (SUCCEEDED((m_imageOperations[i]).QueryInterface(&layerOperation)) &&
			SUCCEEDED((m_imageOperations[i]).QueryInterface(&svgOperation)))
		{
			CBLayerPtr bp;
			layerOperation->GetLayerEquipment(&bp);
			CBSVGPtr sp;
			svgOperation->GetSVGEquipment(&sp);

			D2D1_POINT_2F relativePt = bp.m_rotatableRelativePt;
			//--------------------------------------------------------------------------------------------------------------------------
			// x 为 relativePt 对应的 absolute Point 之值.
			//--------------------------------------------------------------------------------------------------------------------------
			D2D1_POINT_2F x = bp.m_pastingReverseMatrix.TransformPoint(relativePt);

			SVG_DrawSVGBitmapOnRenderTarget(renderTarget, m_imageOperations[i],
																		bp,
																		sp,
																		x,
																		scale,
																		isSaving,
																		drawingRect,
																		imageRect,
																		i,
																		midPoint,
																		originalTransform,
																		bp.m_layerOpacity);
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// geo operation.
		//--------------------------------------------------------------------------------------------------------------------------
		ComPtr<IDrawGeometryOperation> drawOperation;
		if (SUCCEEDED((m_imageOperations[i]).QueryInterface(&drawOperation)))
		{
			GEO_DrawGeoOnRenderTarget(renderTarget, drawOperation,
															scale,
															isSaving,
															drawingRect,
															imageRect,
															i,
															midPoint,
															originalTransform);
		}
	}

	renderTarget->PopLayer();
	renderTarget->SetTransform(originalTransform);
	return S_OK;
}

int SimpleImage::GetIndexOfLayer(ComPtr<IImageLayerOperation> layerOperation)
{
	ComPtr<IImageOperation> operation;
	if (SUCCEEDED(layerOperation.QueryInterface(&operation)))
	{
		return GetIndexOfOperation(operation);
	}
	return -1;
}

int SimpleImage::GetIndexOfText(ComPtr<IImageTextOperation> textOperation)
{
	ComPtr<IImageOperation> operation;
	if (SUCCEEDED(textOperation.QueryInterface(&operation)))
	{
		return GetIndexOfOperation(operation);
	}
	return -1;
}

int SimpleImage::GetIndexOfOperation(ComPtr<IImageOperation> operation)
{
	int success = -1;

	for (int i = 0; i < GetOperationsSize(); i++)
	{
		ComPtr<IImageOperation> ioperation = Get_ith_Operation(i);
		if (ioperation == operation)
		{
			success = i;
			break;
		}
	}

	return success;
}

D2D1::Matrix3x2F SimpleImage::ROTA_GetFlipMatrix(DOUBLE value, D2D1_POINT_2F midPoint, ImageOperationType type)
{
	bool isHorizontalFlip = type == ImageOperationTypeFlipHorizontal;
	return D2D1::Matrix3x2F::Skew(
													isHorizontalFlip ? min(10, 10 * sin(static_cast<float>(PI * std::abs(value)))) : 0,
													isHorizontalFlip ? 0 : min(10, 10 * sin(static_cast<float>(PI * std::abs(value)))),
													midPoint) *
				D2D1::Matrix3x2F::Scale(
													isHorizontalFlip ? -(static_cast<float>(value)) : 1.0f,
													isHorizontalFlip ? 1.0f : -(static_cast<float>(value)),
													midPoint);
}

void SimpleImage::DeleteOperation(ComPtr<IImageOperation> currentOperation)
{
	ComPtr<IImageTextOperation> text;
	ComPtr<IImageLayerOperation> layer;
	ComPtr<IDrawGeometryOperation> geo;

	if (currentOperation == nullptr ||
		(!SUCCEEDED(currentOperation->QueryInterface(&text)) &&
		!SUCCEEDED(currentOperation->QueryInterface(&layer)) &&
		!SUCCEEDED(currentOperation->QueryInterface(&geo))))
	{
		return;
	}
	std::vector<ComPtr<IImageOperation>> array;
	for (int i = 0; i < (int)m_imageOperations.size(); i++)
	{
		ComPtr<IImageOperation> oper = m_imageOperations[i];
		if (oper == currentOperation)
		{
			ComPtr<IImageOperation> dummy;
			SharedObject<ImageDummyOperation>::Create(&dummy);
			array.push_back(dummy);
		}
		else
		{
			array.push_back(oper);
		}
	}
	
	int k = array.size();

	m_imageOperations.clear();
	for (int i = 0; i < (int)array.size(); i++)
	{
		m_imageOperations.push_back(array[i]);
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// key func for the text drawing for textOperation.
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::TEXT_DrawTextOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
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
																			FLOAT opacity)
{
	D2D1_RECT_F dest;
	D2D1_MATRIX_3X2_F transform;

	D2D1::Matrix3x2F matrix1, matrix2;
	if (isSaving)
	{
		matrix1 = D2D1::Matrix3x2F::Translation(-imageRect.left, -imageRect.top);
		matrix2 = D2D1::Matrix3x2F::Translation(imageRect.left, imageRect.top);
	}
	else
	{
		matrix1 = GetMatrix1(imageRect, drawingRect, scale);
		matrix2 = GetMatrix2(imageRect, drawingRect, scale);
	}

	D2D1_POINT_2F pt = (matrix1 * GetNormalTransformationsFromTo(midPoint, 0, i) * matrix2).TransformPoint(x);

	//--------------------------------------------------------------------------------------------------------------------------
	// dest 为相对值
	//--------------------------------------------------------------------------------------------------------------------------
	dest = D2D1::RectF(pt.x, pt.y, pt.x + bp.m_rotatableBorderWidth, pt.y + bp.m_rotatableBorderHeight);

	//--------------------------------------------------------------------------------------------------------------------------
	// textMidPoint 为Text区域左上角的点的经过 0-i 图片翻转变换后的 相对值 对应的 临场值
	//                                                                                                ====           ====
	// 临场值 = Matrix1(相对值)
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1_POINT_2F textMidPoint = matrix1.TransformPoint(pt);
	FLOAT rollupFirst = 0;
	FLOAT currentWrite = -1;
	write_style wStyle = normal;

	D2D1::Matrix3x2F layerAnimationMatrix = D2D1::Matrix3x2F::Identity();

	std::vector<ComPtr<AnimationPackage2>> vec = AnimationUtility2::GetVectorOfPackages();
	for (int k = 0; k < (int)vec.size(); k++)
	{
		ComPtr<AnimationPackage2> pack = vec.at(k);
		ComPtr<ILayerAnimation> layerAnim = pack->m_animation;

		ComPtr<ILayerAnimationRota> rotaAnim;
		if (SUCCEEDED(layerAnim->QueryInterface(&rotaAnim)))
		{
			if (rotaAnim->m_rotaOperation == m_imageOperations[i])
			{
				DOUBLE value = 0;
				rotaAnim->m_rotaAnimationVariable->GetValue(&value);

				D2D1::Matrix3x2F textAnimationMatrix;
				if (rotaAnim->m_rotaOperationType == ImageOperationTypeFlipHorizontal ||
					rotaAnim->m_rotaOperationType == ImageOperationTypeFlipVertical)
				{
					textAnimationMatrix = ROTA_GetFlipMatrix(value, textMidPoint, rotaAnim->m_rotaOperationType);
				}
				else if (rotaAnim->m_rotaOperationType == ImageOperationTypeSizeBigger ||
					rotaAnim->m_rotaOperationType == ImageOperationTypeSizeSmaller)
				{
					textAnimationMatrix = D2D1::Matrix3x2F::Scale(static_cast<float>(value), static_cast<float>(value), textMidPoint);
				}
				else
				{
					textAnimationMatrix = D2D1::Matrix3x2F::Rotation(static_cast<float>(value), textMidPoint);
				}
				layerAnimationMatrix = layerAnimationMatrix * textAnimationMatrix;
			}
		}

		ComPtr<ILayerAnimationRollup> rollupAnim;
		if (SUCCEEDED(layerAnim->QueryInterface(&rollupAnim)))
		{
			if (rollupAnim->m_rollupOperation == m_imageOperations[i])
			{
				DOUBLE value = 0;
				rollupAnim->m_rollupAnimationVariable->GetValue(&value);
				rollupFirst = (FLOAT)value;
			}
		}

		ComPtr<ILayerAnimationWrite> writeAnim;
		if (SUCCEEDED(layerAnim->QueryInterface(&writeAnim)))
		{
			if (writeAnim->m_writeOperation == m_imageOperations[i])
			{
				DOUBLE value = 0;
				writeAnim->m_writeAnimationVariable->GetValue(&value);
				currentWrite = (FLOAT)value;
				wStyle = writeAnim->m_writeStyle;
			}
		}

		ComPtr<ILayerAnimationWWave> wwaveAnim;
		if (SUCCEEDED(layerAnim->QueryInterface(&wwaveAnim)))
		{
			if (wwaveAnim->m_textOperation == m_imageOperations[i])
			{
				DOUBLE value = 0;
				wwaveAnim->GetCurrentAngle(&value);
				bp.m_waveAngle = value;
				wStyle = wave;
			}
		}
	}

	transform = matrix1 *
		ROTA_GetNormalTransformations(bp, textMidPoint) *
		layerAnimationMatrix *
		GetNormalTransformationsFromTo(midPoint, i + 1, (int)m_imageOperations.size() - 1) *
		originalTransform;

	renderTarget->SetTransform(transform);

	if (isSaving)
	{
		ComPtr<ID2D1SolidColorBrush>	textBrush;
		D2D1_COLOR_F color = bp.m_textBrush->GetColor();
		renderTarget->CreateSolidColorBrush(color, &textBrush);

		ComPtr<ID2D1SolidColorBrush> oldBrush = bp.m_textBrush;
		bp.m_textBrush = textBrush;

		TEXT_PrintOutTextWithoutBorder(renderTarget, bp, dest);

		bp.m_textBrush = oldBrush;
	}
	else
	{
		TEXT_PrintOutText(renderTarget, bp, dest, rollupFirst, currentWrite, wStyle);
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// key func for the pic drawing for layer rotaOperation.
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::PLYR_DrawLayerBitmapOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
																							ComPtr<IImageOperation> rotaOperation,
																							CBLayerPtr& bp,
																							ComPtr<ID2D1Bitmap> bitmap,
																							D2D1_POINT_2F x,
																							float scale,
																							bool isSaving,
																							const D2D1_RECT_F& drawingRect,
																							const D2D1_RECT_F& imageRect,
																							int i,
																							D2D1_POINT_2F midPoint,
																							D2D1_MATRIX_3X2_F& originalTransform,
																							FLOAT opacity)
{
	FLOAT width = bitmap->GetSize().width;
	FLOAT height = bitmap->GetSize().height;
	D2D1_RECT_F src = D2D1::RectF(0, 0, width, height);

	D2D1_RECT_F				dest;
	D2D1_MATRIX_3X2_F	transform;
	D2D1_POINT_2F			layerMidPoint;
	D2D1::Matrix3x2F			matrix1, matrix2;

	if (isSaving)
	{
		matrix1 = D2D1::Matrix3x2F::Translation(-imageRect.left, -imageRect.top);
		matrix2 = D2D1::Matrix3x2F::Translation(imageRect.left, imageRect.top);
	}
	else
	{
		matrix1 = GetMatrix1(imageRect, drawingRect, scale);
		matrix2 = GetMatrix2(imageRect, drawingRect, scale);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	// pt 为 absolute point x 对应的从operation 0 执行到 i（生成layer时的operation index）时的相对值，
	//                                                                                                                                       ====
	// 此时 layer 的大小和真实大小是一样的。
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1_POINT_2F pt = (matrix1 * GetNormalTransformationsFromTo(midPoint, 0, i) * matrix2).TransformPoint(x);
	dest = D2D1::RectF(pt.x, pt.y, pt.x + width, pt.y + height);

	layerMidPoint = matrix1.TransformPoint(D2D1::Point2F(pt.x + width / 2, pt.y + height / 2));

	D2D1::Matrix3x2F layerAnimationMatrix = D2D1::Matrix3x2F::Identity();
	//--------------------------------------------------------------------------------------------------------------------------
	// 先将layer自己的翻转和平移transformation处理后，再做整个图片的i+1到size-1次的翻转，结果就是整个的效果transform。
	//--------------------------------------------------------------------------------------------------------------------------
	std::vector<ComPtr<AnimationPackage2>> vec = AnimationUtility2::GetVectorOfPackages();

	ComPtr<ILayerAnimationRota> rotaAnimOn;
	ComPtr<ILayerAnimationPast>  pastAnimOn;

	for (int k = 0; k < (int)vec.size(); k++)
	{
		ComPtr<AnimationPackage2> pack = vec.at(k);
		ComPtr<ILayerAnimation> layerAnim = pack->m_animation;

		ComPtr<ILayerAnimationRota> rotaAnim;
		if (SUCCEEDED(layerAnim->QueryInterface(&rotaAnim)))
		{
			if (rotaAnim->m_rotaOperation == rotaOperation)
			{
				rotaAnimOn = rotaAnim;
			}
		}

		ComPtr<ILayerAnimationPast> pastAnim;
		if (SUCCEEDED(layerAnim->QueryInterface(&pastAnim)))
		{
			if (pastAnim->m_pastOperation == rotaOperation)
			{
				pastAnimOn = pastAnim;
			}
		}
	}

	if (rotaAnimOn != nullptr)
	{
		DOUBLE value = 0;
		rotaAnimOn->m_rotaAnimationVariable->GetValue(&value);

		D2D1::Matrix3x2F rotaAnimationMatrix;
		if (rotaAnimOn->m_rotaOperationType == ImageOperationTypeFlipHorizontal ||
			rotaAnimOn->m_rotaOperationType == ImageOperationTypeFlipVertical)
		{
			rotaAnimationMatrix = ROTA_GetFlipMatrix(value, layerMidPoint, rotaAnimOn->m_rotaOperationType);
		}
		else if (rotaAnimOn->m_rotaOperationType == ImageOperationTypeSizeBigger ||
			rotaAnimOn->m_rotaOperationType == ImageOperationTypeSizeSmaller)
		{
			rotaAnimationMatrix = D2D1::Matrix3x2F::Scale(static_cast<float>(value), static_cast<float>(value), layerMidPoint);
		}
		else
		{
			rotaAnimationMatrix = D2D1::Matrix3x2F::Rotation(static_cast<float>(value), layerMidPoint);
		}
		layerAnimationMatrix = layerAnimationMatrix * rotaAnimationMatrix;
	}

	if (pastAnimOn != nullptr)
	{
		D2D1::Matrix3x2F translateAnimationMatrix = D2D1::Matrix3x2F::Identity();
		D2D1_POINT_2F point;
		if (SUCCEEDED(pastAnimOn->GetCurrentPoint(&point)))
		{
			translateAnimationMatrix = D2D1::Matrix3x2F::Translation(point.x, point.y);
		}
		layerAnimationMatrix = layerAnimationMatrix * translateAnimationMatrix;
	}

	transform = matrix1 *
		ROTA_GetNormalTransformations(bp, layerMidPoint) *
		layerAnimationMatrix *
		GetNormalTransformationsFromTo(midPoint, i + 1, (int)m_imageOperations.size() - 1) *
		originalTransform;

	renderTarget->SetTransform(transform);
	//--------------------------------------------------------------------------------------------------------------------------
	// 注意用的是 dest 和 src，就是把 src 的原图映射到 (pt.x, pt.y) 的位置上，同样大小，最后做layer自身翻转和整个图片的后续翻转。
	//--------------------------------------------------------------------------------------------------------------------------
	renderTarget->DrawBitmap(bitmap, dest, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src);

	//--------------------------------------------------------------------------------------------------------------------------
	// 利用前面的PLYR_DrawLayerBitmapOnRenderTarget所设置的transform画框
	//--------------------------------------------------------------------------------------------------------------------------
	if (bp.m_layerIsActive && !isSaving)
	{
		D2D1_RECT_F dest = D2D1::RectF(bp.m_rotatableRelativePt.x,
			bp.m_rotatableRelativePt.y,
			bp.m_rotatableRelativePt.x + bp.m_rotatableBorderWidth,
			bp.m_rotatableRelativePt.y + bp.m_rotatableBorderHeight);

		renderTarget->DrawRectangle(dest, bp.m_layerFrameBrush, 3);
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// key func for the pic drawing for pic box picsOperation.
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::PICS_DrawPicsBitmapOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
																							ComPtr<IImageOperation> picsOperation,
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
																							FLOAT opacity)
{
	FLOAT width = bitmap->GetSize().width;
	FLOAT height = bitmap->GetSize().height;
	D2D1_RECT_F src = D2D1::RectF(0, 0, width, height);

	D2D1_RECT_F				dest;
	D2D1_MATRIX_3X2_F	transform;
	//D2D1_POINT_2F			layerMidPoint;
	D2D1::Matrix3x2F			matrix1, matrix2;

	if (isSaving)
	{
		matrix1 = D2D1::Matrix3x2F::Translation(-imageRect.left, -imageRect.top);
		matrix2 = D2D1::Matrix3x2F::Translation(imageRect.left, imageRect.top);
	}
	else
	{
		matrix1 = GetMatrix1(imageRect, drawingRect, scale);
		matrix2 = GetMatrix2(imageRect, drawingRect, scale);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	// pt 为 absolute point x 对应的从operation 0 执行到 i（生成layer时的operation index）时的相对值，
	//                                                                                                                                       ====
	// 此时 layer 的大小和真实大小是一样的。
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1_POINT_2F pt = (matrix1 * GetNormalTransformationsFromTo(midPoint, 0, i) * matrix2).TransformPoint(x);
	dest = D2D1::RectF(pt.x, pt.y, pt.x + width, pt.y + height);
	//dest = D2D1::RectF(0, 0, width, height);

	//layerMidPoint = matrix1.TransformPoint(D2D1::Point2F(pt.x + width / 2, pt.y + height / 2));

	//--------------------------------------------------------------------------------------------------------------------------
	// 先将layer自己的翻转和平移transformation处理后，再做整个图片的i+1到size-1次的翻转，结果就是整个的效果transform。
	//--------------------------------------------------------------------------------------------------------------------------
	std::vector<ComPtr<AnimationPackage2>> vec = AnimationUtility2::GetVectorOfPackages();

	ComPtr<ILayerAnimationPics>		picsAnimOn;
	DOUBLE animValue = 0;

	for (int k = 0; k < (int)vec.size(); k++)
	{
		ComPtr<AnimationPackage2> pack = vec.at(k);
		ComPtr<ILayerAnimation> layerAnim = pack->m_animation;

		ComPtr<ILayerAnimationPics> picsAnim;
		if (SUCCEEDED(layerAnim->QueryInterface(&picsAnim)))
		{
			if (picsAnim->m_picsOperation == picsOperation)
			{
				picsAnimOn = picsAnim;
			}
		}
	}

	//transform = matrix1 *
	//	ROTA_GetNormalTransformations(bp, layerMidPoint) *
	//	GetNormalTransformationsFromTo(midPoint, i + 1, (int)m_imageOperations.size() - 1) *
	//	originalTransform;
	transform = //D2D1::Matrix3x2F::Translation(pt.x, pt.y) *
		matrix1 *
		//ROTA_GetNormalTransformations(bp, layerMidPoint) *
		GetNormalTransformationsFromTo(midPoint, i + 1, (int)m_imageOperations.size() - 1) *
		originalTransform;
	renderTarget->SetTransform(transform);

	if (picsAnimOn != nullptr)
	{
		picsAnimOn->m_picsAnimationVariable->GetValue(&animValue);
		int picsCnt = (int)pp.m_picsFileNames.size();
		FLOAT eachPieceDuration = picsAnimOn->m_picsAnimationDuration / picsCnt;
		if (eachPieceDuration == 0.0f)
			eachPieceDuration = 2.0f;
		int currentIndex = ((int)(animValue / eachPieceDuration));
		FLOAT ratio;
		FLOAT remain = eachPieceDuration - (animValue - currentIndex * eachPieceDuration);
		if (remain > 2.0f)
			ratio = 1.0f;
		else
		{
			ratio = remain / 2.0f;
		}

		currentIndex = ((int)(animValue / eachPieceDuration)) % picsCnt;
		int nextIndex = (currentIndex + 1) % picsCnt;
		renderTarget->DrawBitmap(pp.m_picsBitmaps[nextIndex], dest, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src);
		renderTarget->DrawBitmap(pp.m_picsBitmaps[currentIndex], dest, ratio, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src);
	}
	else 
	{
		renderTarget->DrawBitmap(bitmap, dest, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	// 利用前面的PLYR_DrawLayerBitmapOnRenderTarget所设置的transform画框
	//--------------------------------------------------------------------------------------------------------------------------
	if (bp.m_layerIsActive && !isSaving)
	{
		D2D1_RECT_F dest = D2D1::RectF(bp.m_rotatableRelativePt.x,
			bp.m_rotatableRelativePt.y,
			bp.m_rotatableRelativePt.x + bp.m_rotatableBorderWidth,
			bp.m_rotatableRelativePt.y + bp.m_rotatableBorderHeight);

		renderTarget->DrawRectangle(dest, bp.m_layerFrameBrush, 3);
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// key func for the pic drawing for pic box picsOperation.
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::SVG_DrawSVGBitmapOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
																						ComPtr<IImageOperation> svgOperation,
																						CBLayerPtr& bp,
																						CBSVGPtr& sp,
																						D2D1_POINT_2F x,
																						float scale,
																						bool isSaving,
																						const D2D1_RECT_F& drawingRect,
																						const D2D1_RECT_F& imageRect,
																						int i,
																						D2D1_POINT_2F midPoint,
																						D2D1_MATRIX_3X2_F& originalTransform,
																						FLOAT opacity)
{
	FLOAT width = 339;
	FLOAT height = 339;

	D2D1_MATRIX_3X2_F	transform;
	D2D1_POINT_2F			layerMidPoint;
	D2D1::Matrix3x2F			matrix1, matrix2;

	if (isSaving)
	{
		matrix1 = D2D1::Matrix3x2F::Translation(-imageRect.left, -imageRect.top);
		matrix2 = D2D1::Matrix3x2F::Translation(imageRect.left, imageRect.top);
	}
	else
	{
		matrix1 = GetMatrix1(imageRect, drawingRect, scale);
		matrix2 = GetMatrix2(imageRect, drawingRect, scale);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	// pt 为 absolute point x 对应的从operation 0 执行到 i（生成layer时的operation index）时的相对值，
	//                                                                                                                                       ====
	// 此时 layer 的大小和真实大小是一样的。
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1_POINT_2F pt = (matrix1 * GetNormalTransformationsFromTo(midPoint, 0, i) * matrix2).TransformPoint(x);

	//pt.x pt.y 可能不需要 matrix1 的转换
	//layerMidPoint = matrix1.TransformPoint(D2D1::Point2F(pt.x + width / 2, pt.y + height / 2));
	layerMidPoint = D2D1::Point2F(pt.x + width / 2, pt.y + height / 2);

	//--------------------------------------------------------------------------------------------------------------------------
	// 开始 Animation 处理
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1::Matrix3x2F layerAnimationMatrix = D2D1::Matrix3x2F::Identity();
	//--------------------------------------------------------------------------------------------------------------------------
	// 先将layer自己的翻转和平移transformation处理后，再做整个图片的i+1到size-1次的翻转，结果就是整个的效果transform。
	//--------------------------------------------------------------------------------------------------------------------------
	std::vector<ComPtr<AnimationPackage2>> vec = AnimationUtility2::GetVectorOfPackages();

	ComPtr<ILayerAnimationRota> rotaAnimOn;
	for (int k = 0; k < (int)vec.size(); k++)
	{
		ComPtr<AnimationPackage2> pack = vec.at(k);
		ComPtr<ILayerAnimation> layerAnim = pack->m_animation;

		ComPtr<ILayerAnimationRota> rotaAnim;
		if (SUCCEEDED(layerAnim->QueryInterface(&rotaAnim)))
		{
			if (rotaAnim->m_rotaOperation == svgOperation)
			{
				rotaAnimOn = rotaAnim;
			}
		}
	}

	if (rotaAnimOn != nullptr)
	{
		DOUBLE value = 0;
		rotaAnimOn->m_rotaAnimationVariable->GetValue(&value);

		D2D1::Matrix3x2F rotaAnimationMatrix;
		if (rotaAnimOn->m_rotaOperationType == ImageOperationTypeFlipHorizontal ||
			rotaAnimOn->m_rotaOperationType == ImageOperationTypeFlipVertical)
		{
			rotaAnimationMatrix = ROTA_GetFlipMatrix(value, layerMidPoint, rotaAnimOn->m_rotaOperationType);
		}
		else if (rotaAnimOn->m_rotaOperationType == ImageOperationTypeSizeBigger ||
			rotaAnimOn->m_rotaOperationType == ImageOperationTypeSizeSmaller)
		{
			rotaAnimationMatrix = D2D1::Matrix3x2F::Scale(static_cast<float>(value), static_cast<float>(value), layerMidPoint);
		}
		else
		{
			rotaAnimationMatrix = D2D1::Matrix3x2F::Rotation(static_cast<float>(value), layerMidPoint);
		}
		layerAnimationMatrix = layerAnimationMatrix * rotaAnimationMatrix;
	}

	transform = D2D1::Matrix3x2F::Translation(pt.x, pt.y) *
		D2D1::Matrix3x2F::Scale(static_cast<float>(sp.m_scaleValue), static_cast<float>(sp.m_scaleValue), layerMidPoint) *
		D2D1::Matrix3x2F::Translation(sp.m_svgOffSet, sp.m_svgOffSet) * 
		ROTA_GetNormalTransformations(bp, layerMidPoint) *
		layerAnimationMatrix *
		matrix1 *
		GetNormalTransformationsFromTo(midPoint, i + 1, (int)m_imageOperations.size() - 1) *
		originalTransform;

	//transform = D2D1::Matrix3x2F::Translation(pt.x, pt.y) *
	//	ROTA_GetNormalTransformations(bp, layerMidPoint) *
	//	matrix1 *
	//	GetNormalTransformationsFromTo(midPoint, i + 1, (int)m_imageOperations.size() - 1) *
	//	originalTransform;
	g_pD2DDeviceContext->SetTransform(transform);

	g_pD2DDeviceContext->DrawSvgDocument(sp.m_svgDocument);

	if (bp.m_layerIsActive && !isSaving)
	{
		transform = 
			ROTA_GetNormalTransformations(bp, layerMidPoint) *
			matrix1 *
			GetNormalTransformationsFromTo(midPoint, i + 1, (int)m_imageOperations.size() - 1) *
			originalTransform;
		
		g_pD2DDeviceContext->SetTransform(transform);

		D2D1_RECT_F dest = D2D1::RectF(bp.m_rotatableRelativePt.x,
															bp.m_rotatableRelativePt.y,
															bp.m_rotatableRelativePt.x + bp.m_rotatableBorderWidth,
															bp.m_rotatableRelativePt.y + bp.m_rotatableBorderHeight);

		g_pD2DDeviceContext->DrawRectangle(dest, bp.m_layerFrameBrush, 3);
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// key func for the geo drawing for pencil geoOperation.
//--------------------------------------------------------------------------------------------------------------------------
void SimpleImage::GEO_DrawGeoOnRenderTarget(ComPtr<ID2D1RenderTarget> renderTarget,
																			ComPtr<IDrawGeometryOperation> drawOperation,
																			float scale,
																			bool isSaving,
																			const D2D1_RECT_F& drawingRect,
																			const D2D1_RECT_F& imageRect,
																			int i,
																			D2D1_POINT_2F midPoint,
																			D2D1_MATRIX_3X2_F& originalTransform)
{
	D2D1_MATRIX_3X2_F transform;
	if (isSaving)
	{
		drawOperation->DiscardResources();

		transform =
			D2D1::Matrix3x2F::Translation(-imageRect.left, -imageRect.top) *
			GetNormalTransformations(midPoint) *
			originalTransform;
	}
	else
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Normal Transform from 0 - n, complete the whole transformation.
		//--------------------------------------------------------------------------------------------------------------------------
		transform =
			D2D1::Matrix3x2F::Translation(-imageRect.left, -imageRect.top) *
			D2D1::Matrix3x2F::Scale(1 / scale, 1 / scale) *
			D2D1::Matrix3x2F::Translation(drawingRect.left, drawingRect.top) *
			GetNormalTransformations(midPoint) *
			originalTransform;
	}
	renderTarget->SetTransform(transform);
	drawOperation->DrawToRenderTarget(renderTarget, drawingRect);
	if (isSaving)
	{
		drawOperation->DiscardResources();
	}
}