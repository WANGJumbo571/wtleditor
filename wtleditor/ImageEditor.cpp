//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================
#pragma once

#include "StdAfx.h"
#include <atlapp.h>
#include <atlbase.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlribbon.h>

#include "ImageEditor.h"

#include <atldlgs.h>
#include <commdlg.h>
#include <minwindef.h>

#include <gdiplus.h> 
#include <gdiplusbitmap.h>
#include <gdipluscachedbitmap.h>
#include <gdiplusheaders.h>
#include <gdiplusimaging.h>

#include "DrawGeometryOperation.h"
#include "ImageClippingOperation.h"
#include "ImageLayerOperation.h"
#include "ImagePicsOperation.h"
#include "ImageSVGOperation.h"
#include "ImageTextOperation.h"
#include "ImageTransformationOperation.h"

#include "aboutdlg.h"
#include "AnimationUtility2.h"
#include "combdlg.h"
#include "Direct2DUtility.h"
#include "external.h"
#include "LayerAnimation.h"
#include "mainfrm.h"
#include "resizedlg.h"
#include "Resource.h"
#include "Ribbon.h"
#include "ShellFileDialog.h"
#include "UIRibbon.h"

#include <fstream>
#include <iostream>

#include "tinyxml2.h"

using namespace tinyxml2;
using namespace std;

extern HINSTANCE		g_hInstance;
extern text_style			g_textStyle;
extern CMainFrame *	g_frameWork;
extern bool					g_bFullScreen;

HCURSOR g_cursorCenter;
HCURSOR g_cursorEdit;
HCURSOR g_cursorRight;
HCURSOR g_cursorLeft;
HCURSOR g_cursorBottom;
HCURSOR g_cursorBottomRight;
HCURSOR g_cursorNormal;

const int ImageEditorHandler::BackgroundColor = 0x222222;
const int ImageEditorHandler::PreviousNextImageRangeCount = 1; // 2;

const float ImageEditorHandler::ImageMargin = 30;
const float ImageEditorHandler::KeyboardPanDistance = 25;
const float ImageEditorHandler::PreviousNextImageMargin = 60;
const float ImageEditorHandler::SlideAnimationDuration = 1.25f; //0.25f;
static float TransformationAnimationDuration = 1.0f; // 0.25f;
const float ImageEditorHandler::ZoomMinimum = 1.0;
const float ImageEditorHandler::ZoomMaximum = 4.0;
const float ImageEditorHandler::ZoomStep = 0.25f;

const float ImageEditorHandler::StrokeSizes[] = { 2.0f, 4.0f, 8.0f, 15.0f };

WCHAR		g_textFormatFamilyName[32];
FLOAT			g_textFormatFontSize = 30;
COLORREF	g_fontColor		= 0xffffff;
COLORREF	g_bkColor			= 0x66380f;
int				g_charSpace		= 0;
int				g_lineSpace		= 2;
bool				g_bAboutDialogShow = false;
bool				g_bCombDialogShow = false;
FLOAT			g_animDuration = 75;
bool				g_fullScreen = false;
CAboutDlg*	g_aboutDlg = NULL;
CombDlg*		g_combDlg = NULL;

using namespace Hilo::AnimationHelpers;
using namespace Hilo::Direct2DHelpers;

#define FSTRING L"小园可有绿牡丹\r江山待识春风面\r寄问闺中真国色\r何时花开住人间"
//#define FSTRING L"《沁园春·雪》\r毛泽东\r\r北国风光，千里冰封，万里雪飘。\r望长城内外，惟余莽莽；大河上下，顿失滔滔。\r山舞银蛇，原驰蜡象，欲与天公试比高。\r须晴日，看红装素裹，分外妖娆。\r\r江山如此多娇，引无数英雄竞折腰。\r惜秦皇汉武，略输文采；唐宗宋祖，稍逊风骚。\r一代天骄，成吉思汗，只识弯弓射大雕。\r俱往矣，数风流人物，还看今朝。"

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")
#pragma once

//--------------------------------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------------------------------
ImageEditorHandler::ImageEditorHandler() :
	m_animationEnabled(false),
	m_switchingImages(false),
	m_currentIndex(0),
	m_currentRangeStart(-1),
	m_currentRangeEnd(-1),
	m_currentZoom(1),
	m_maxSlideDistance(0),
	m_currentPanPoint(D2D1::Point2F(0, 0)),
	m_currentPanBoundary(D2D1::RectF(0, 0, 0, 0)),
	m_currentDrawingOperationType(ImageOperationTypeNone),
	m_isDrawing(false),
	m_isClipping(false),
	m_isPasting(false),
	m_isTexting(false),
	m_isLayering(false),
	m_isRotation(false),
	m_isFlip(false),
	m_isCopying(false),
	m_charSpace((FLOAT)g_charSpace),
	m_lineSpace((FLOAT)g_lineSpace),
	m_charSize((FLOAT)g_textFormatFontSize),
	m_penSize(2),
	m_textColor(D2D1::ColorF(static_cast<float>(GetRValue(g_fontColor)) / 255.0f,
											static_cast<float>(GetGValue(g_fontColor)) / 255.0f,
											static_cast<float>(GetBValue(g_fontColor)) / 255.0f)),
	m_penColor(D2D1::ColorF(D2D1::ColorF::Black))
{
	GdiplusStartupInput in;
	GdiplusStartup(&token, &in, NULL);

	g_cursorCenter				= ::LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_CENTER));
	g_cursorEdit					= ::LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_EDIT));
	g_cursorRight				= ::LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_RIGHT));
	g_cursorLeft					= ::LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_LEFT));
	g_cursorBottom			= ::LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_BOTTOM));
	g_cursorBottomRight	= ::LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_BOTTOMRIGHT));
	g_cursorNormal			= ::LoadCursor(NULL, IDC_ARROW);

}

//--------------------------------------------------------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------------------------------------------------------
ImageEditorHandler::~ImageEditorHandler()
{
	GdiplusShutdown(token);
}

LRESULT ImageEditorHandler::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	SetTimer(CURSOR_TIMER, 1000);

	if (g_aboutDlg == NULL)
	{
		g_aboutDlg = new CAboutDlg();
		g_aboutDlg->Create(m_hWnd);
	}
	if (g_combDlg == NULL)
	{
		g_combDlg = new CombDlg();
		g_combDlg->Create(m_hWnd);

		CRect rect = { 100, 300, 490, 710 };
		g_combDlg->MoveWindow(&rect);
	}
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Reset the state of the image editor
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::Reset()
{
    m_images.clear();
    m_currentIndex = 0;
    m_currentRangeStart = -1;
    m_currentRangeEnd = -1;
    return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// Loads the images in the specifed list
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::LoadShellItems(const std::vector<ComPtr<IShellItem> >* shellItems, IShellItem* currentItem)
{
    HRESULT hr = Reset();
    if (SUCCEEDED(hr))
    {
        for (auto shellItem = shellItems->begin() ; shellItem != shellItems->end(); shellItem++)
        {
            ImageInfo info(sourceFromFile, *shellItem);
            ImageItem newItem;

            if (FAILED(SharedObject<SimpleImage>::Create(info, &newItem.Image)))
            {
                continue;
            }

            // For now do not create animation objects. These are managed when animation begins/ends
            m_images.push_back(newItem);

            if (nullptr != currentItem)
            {
                int compareResult;

                if (SUCCEEDED(currentItem->Compare(*shellItem, SICHINT_DISPLAY,  &compareResult)))
                {
                    if (0 == compareResult)
                    {
                        m_currentIndex = static_cast<int>(m_images.size()) - 1;
                    }
                }
            }
        }
    }

	reportError(170, (hr = ManageImageResources()));

	reportError(171, (hr = CalculateImagePositions()));

	m_currentOperation = nullptr;
    InvalidateWindow();

    return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
// Loads the images in the specifed folder
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::SetCurrentLocation(IShellItem* shellItem)
{
    // Clear current list of items
    HRESULT hr = Reset();

    // List of shell items from which to load our images
    std::vector<ComPtr<IShellItem> > shellItems;

    // Current item
    ComPtr<IShellItem> currentItem = shellItem;

    // Ignore the returned HRESULT because it's possible no elements are found in this folder
    ShellItemsLoader::EnumerateFolderItems(shellItem, FileTypeImage, false, shellItems);

    if (!shellItems.empty())
    {
		reportError(172, (hr = LoadShellItems(&shellItems, nullptr)));
    }
    else
    {
		reportError(173, (hr = OpenFile()));
    }

    return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::ImageEditor_CreateStrokAndTextFormat()
{
	HRESULT hr = S_OK;
	float dashes[] = {1.0f, 2.0f, 2.0f, 1.0f, 2.0f, 2.0f};
        // Stroke Style with custom Dash Style
	strokeStyleCustomOffsetZero = nullptr;
	reportError(40, (hr = g_pD2DFactory->CreateStrokeStyle(
									D2D1::StrokeStyleProperties(
																				D2D1_CAP_STYLE_FLAT,
																				D2D1_CAP_STYLE_FLAT,
																				D2D1_CAP_STYLE_ROUND,
																				D2D1_LINE_JOIN_MITER,
																				10.0f,
																				D2D1_DASH_STYLE_CUSTOM,
																				0.0f),
									dashes,
									ARRAYSIZE(dashes),
									&strokeStyleCustomOffsetZero)));

    if (SUCCEEDED(hr))
    {
        // Create text format
		m_textFormat = nullptr;
		reportError(41, (hr = g_pDWriteFactory->CreateTextFormat(
									g_textFormatFamilyName,
									nullptr,
									DWRITE_FONT_WEIGHT_REGULAR,
									DWRITE_FONT_STYLE_NORMAL,
									DWRITE_FONT_STRETCH_NORMAL,
									g_textFormatFontSize,
									L"en-us",
									&m_textFormat)));
    }

    // Create animation objects
    if (SUCCEEDED(hr))
    {
		m_animationManager = nullptr;
		reportError(42, (hr = AnimationUtility::GetAnimationManager(&m_animationManager)));
    }

    if (SUCCEEDED(hr))
    {
		m_transitionLibrary = nullptr;
		reportError(43, (hr = AnimationUtility::GetTransitionLibrary(&m_transitionLibrary)));
    }

	m_solidBrush = nullptr;
	reportError(47, (hr = g_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_solidBrush)));
	g_solidBrush = m_solidBrush;

    return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
// Discard any Direct2D resources which are no longer bound to a particular Direct2D render target
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::DiscardDeviceResources()
{
    // Discard image resources
    for (auto image = m_images.begin(); image != m_images.end(); ++image)
    {
        (*image).Image->DiscardResources();
    }

    return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// Manage the currently loaded images. This method calculates the effective range based on the current image index.
// Images that have just moved outside the valid range will be discards. Images that have just moved into the valid
// range will be loaded.
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::ManageImageResources()
{
    if (m_images.empty())
    {
        return S_OK;
    }

    // Calculate new range
    int rangeStart = m_currentIndex - PreviousNextImageRangeCount;
    int rangeEnd = m_currentIndex + PreviousNextImageRangeCount;

    // Update range based on valid values
    rangeStart = max(0, rangeStart);
    rangeEnd = min(static_cast<int>(m_images.size()) - 1, rangeEnd);

    // Discard resources for any images no longer in range
    for (int i = m_currentRangeStart; i < m_currentRangeEnd + 1; i++)
    {
        if (i >= 0 && (i < rangeStart || i > rangeEnd))
        {
            m_images[i].Image->DiscardResources();
        }
    }

    // Load resources as necessary
    for (int i = rangeStart; i < rangeEnd + 1; i++)
    {
        m_images[i].Image->SetRenderingParameters(m_renderingParameters);
        m_images[i].Image->Load();
        m_images[i].Image->SetBoundingRect(m_imageBoundaryRect);
    }

    // Update current range values
    m_currentRangeStart = rangeStart;
    m_currentRangeEnd = rangeEnd;

    return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// Calculate the pan boundary based on the current zoom and screen size
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::CalculatePanBoundary()
{
    if (m_images.empty())
    {
        return S_OK;
    }

    D2D1_RECT_F rect;
    HRESULT hr = m_images[m_currentIndex].Image->GetTransformedRect(GetCenter(), &rect);

    if (SUCCEEDED(hr))
    {
        float imageWidth = (rect.right - rect.left) * m_currentZoom;
        float imageHeight = (rect.bottom - rect.top) * m_currentZoom;

        float clientWidth = g_logicalSize.width;
        float clientHeight = g_logicalSize.height;

        if (clientWidth > imageWidth)
        {
            // Width is smaller than the client area. Don't allow panning in the x-axis
            m_currentPanBoundary.left = 0;
            m_currentPanBoundary.right = 0;
        }
        else
        {
            m_currentPanBoundary.left = (clientWidth / 2 ) - (imageWidth / 2);
            m_currentPanBoundary.right = m_currentPanBoundary.left * -1;
        }

        if (clientHeight > imageHeight)
        {
            // Height is smaller than the client area. Don't allow panning in the y-axis
            m_currentPanBoundary.top = 0;
            m_currentPanBoundary.bottom = 0;
        }
        else
        {
            m_currentPanBoundary.top = (clientHeight / 2) - (imageHeight / 2);
            m_currentPanBoundary.bottom = m_currentPanBoundary.top * -1;
        }
    }

    return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
// Calculate the visible image positions based on screen size
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::CalculateImagePositions()
{
    HRESULT hr = S_OK;

    if (m_images.empty())
    {
        return S_OK;
    }

    for (int i = m_currentRangeStart; i < m_currentRangeEnd + 1; i++)
    {
        D2D1_RECT_F currentRect;

        if (SUCCEEDED(m_images[i].Image->GetDrawingRect(&currentRect)))
        {
            float width = currentRect.right - currentRect.left;

            switch(m_currentIndex - i)
            {
            case 2:
                {
                    // Far left image
                    D2D1_RECT_F siblingRect;
                    hr = m_images[i + 1].Image->GetDrawingRect(&siblingRect);

                    if (SUCCEEDED(hr))
                    {
                        float siblingWidth = siblingRect.right - siblingRect.left;

                        currentRect.right = PreviousNextImageMargin - 
														siblingWidth / 2 - 
														g_logicalSize.width / 2;
                        currentRect.left = currentRect.right - width;
                    }

                    break;
                }
            case 1:
                {
                    // Left image
                    currentRect.right = PreviousNextImageMargin;
                    currentRect.left = currentRect.right - width;
                    break;
                }
            case 0:
                {
                    // Center image
                    currentRect.left = PreviousNextImageMargin + 
												ImageMargin + 
												(m_imageBoundaryRect.right - m_imageBoundaryRect.left) / 2 - 
												width / 2;
                    currentRect.right = currentRect.left + width;
                    break;
                }
            case -1:
                {
                    // Right image
                    currentRect.left = g_logicalSize.width - PreviousNextImageMargin;
                    currentRect.right = currentRect.left + width;
                    break;
                }
            case -2:
                {
                    // Far right image
                    D2D1_RECT_F siblingRect;
                    hr = m_images[i - 1].Image->GetDrawingRect(&siblingRect);

                    if (SUCCEEDED(hr))
                    {
                        float siblingWidth = siblingRect.right - siblingRect.left;

                        currentRect.left = g_logicalSize.width * 1.5f - PreviousNextImageMargin + siblingWidth / 2;
                        currentRect.right = currentRect.left + width;
                        break;
                    }
                }
            }

            m_images[i].Image->SetDrawingRect(currentRect);
        }
    }

    return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
// Navigate to the previous image
//--------------------------------------------------------------------------------------------------------------------------
bool ImageEditorHandler::PreviousImage()
{
    bool isPrevImage = false;

    // Ignore additional request to goto previous image while animation is active
    if (!m_animationEnabled)
    {
        if (m_currentIndex > 0)
        {
            --m_currentIndex;
            isPrevImage = true;
            m_switchingImages = true;
        }

        SetupAnimation();
    }

    return isPrevImage;
}

//--------------------------------------------------------------------------------------------------------------------------
// Navigate to the next image
//--------------------------------------------------------------------------------------------------------------------------
bool ImageEditorHandler::NextImage()
{
    bool isNextImage = false;

    // Ignore additional request to goto next image while animation is active
    if (!m_animationEnabled)
    {
        if (m_currentIndex < static_cast<int>(m_images.size()) - 1)
        {
            ++m_currentIndex;
            isNextImage = true;
            m_switchingImages = true;
        }

        SetupAnimation();
    }

    return isNextImage;
}

//--------------------------------------------------------------------------------------------------------------------------
// Zoom in on the current image
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::ZoomIn()
{
    if (m_currentZoom < ZoomMaximum)
    {
        m_currentZoom = min(ZoomMaximum, m_currentZoom + ZoomStep);
    }

	//--------------------------------------------------------------------------------------------------------------------------
	// Update pan boundaries based on current zoom
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT hr = CalculatePanBoundary();

	//--------------------------------------------------------------------------------------------------------------------------
	// Check for bounds
	//--------------------------------------------------------------------------------------------------------------------------
	PanImage(D2D1::Point2F(0, 0), true);

    return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
// Zoom out on the current image
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::ZoomOut()
{
    if (m_currentZoom > ZoomMinimum)
    {
        m_currentZoom = max(ZoomMinimum, m_currentZoom - ZoomStep);
    }

	//--------------------------------------------------------------------------------------------------------------------------
	// Update pan boundaries based on current zoom
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT hr = CalculatePanBoundary();

	//--------------------------------------------------------------------------------------------------------------------------
	// Check for bounds
	//--------------------------------------------------------------------------------------------------------------------------
	PanImage(D2D1::Point2F(0, 0), true);

    return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
// Zoom out to zoom minimum on the current image
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::ZoomFull()
{
    m_currentZoom = ZoomMinimum;

    return ZoomOut();
}

//--------------------------------------------------------------------------------------------------------------------------
// Pans the current image by the specified offset and snaps the image back to it's pan boundary
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::PanImage(D2D1_POINT_2F offset, bool snapToBounds)
{
	//--------------------------------------------------------------------------------------------------------------------------
	// Update pan values and then check for bounds
	//--------------------------------------------------------------------------------------------------------------------------
	m_currentPanPoint.x += offset.x;

    if (m_currentZoom > ZoomMinimum)
    {
        m_currentPanPoint.y += offset.y;
    }

    if (snapToBounds)
    {
		//--------------------------------------------------------------------------------------------------------------------------
		// Panning left
		//--------------------------------------------------------------------------------------------------------------------------
		if (m_currentPanPoint.x > m_currentPanBoundary.right)
        {
            m_currentPanPoint.x = m_currentPanBoundary.right;
        }

		//--------------------------------------------------------------------------------------------------------------------------
		// Panning right
		//--------------------------------------------------------------------------------------------------------------------------
		if (m_currentPanPoint.x < m_currentPanBoundary.left)
        {
            m_currentPanPoint.x = m_currentPanBoundary.left;
        }

		//--------------------------------------------------------------------------------------------------------------------------
		// Panning up
		//--------------------------------------------------------------------------------------------------------------------------
		if (m_currentPanPoint.y > m_currentPanBoundary.bottom)
        {
            m_currentPanPoint.y = m_currentPanBoundary.bottom;
        }

		//--------------------------------------------------------------------------------------------------------------------------
		// Panning down
		//--------------------------------------------------------------------------------------------------------------------------
		if (m_currentPanPoint.y < m_currentPanBoundary.top)
        {
            m_currentPanPoint.y = m_currentPanBoundary.top;
        }
    }

    InvalidateWindow();
}

void ImageEditorHandler::DrawAnimatedImages(int imageIndex)
{
	//--------------------------------------------------------------------------------------------------------------------------
	// Get drawing rectangle
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1_RECT_F rect;
    m_images[imageIndex].Image->GetDrawingRect(&rect);

	//--------------------------------------------------------------------------------------------------------------------------
	// Update transforms based on animation type
	//--------------------------------------------------------------------------------------------------------------------------
	if (m_isRotation || m_isFlip)
    {
		//--------------------------------------------------------------------------------------------------------------------------
		// Only apply rotation/flip to current image
		//--------------------------------------------------------------------------------------------------------------------------
		if (imageIndex == m_currentIndex)
        {
			//--------------------------------------------------------------------------------------------------------------------------
			// Store current transition
			//--------------------------------------------------------------------------------------------------------------------------
			D2D1_MATRIX_3X2_F originalTransform;
            g_pD2DDeviceContext->GetTransform(&originalTransform);

			//--------------------------------------------------------------------------------------------------------------------------
			// Calculate rotation point for animation
			//--------------------------------------------------------------------------------------------------------------------------
			D2D1_POINT_2F midPoint = D2D1::Point2F(
                m_currentPanPoint.x + rect.left + (rect.right - rect.left) / 2, 
                m_currentPanPoint.y + rect.top + (rect.bottom - rect.top) / 2);

            DOUBLE value = 0;
            m_transformationAnimationVariable->GetValue(&value);

            if (m_isRotation)
            {
                g_pD2DDeviceContext->SetTransform(
                    originalTransform * D2D1::Matrix3x2F::Rotation(static_cast<float>(value), midPoint));
            }
            else // m_isFlip is true
            {
                bool isHorizontalFlip = m_currentDrawingOperationType == ImageOperationTypeFlipHorizontal;
                g_pD2DDeviceContext->SetTransform(
																			originalTransform *
																			D2D1::Matrix3x2F::Skew(
																					isHorizontalFlip ? min(10, 10 * std::sin(static_cast<float>(PI * std::abs(value)))) : 0, 
																					isHorizontalFlip ? 0 : min(10, 10 * std::sin(static_cast<float>(PI * std::abs(value)))), 
																					midPoint) * 
																			D2D1::Matrix3x2F::Scale(
																					isHorizontalFlip ? -(static_cast<float>(value)) : 1.0f,
																					isHorizontalFlip ? 1.0f : -(static_cast<float>(value)),
																					midPoint));
            }

			//--------------------------------------------------------------------------------------------------------------------------
			// Draw image
			//--------------------------------------------------------------------------------------------------------------------------
			m_images[imageIndex].Image->Draw(imageIndex == m_currentIndex);

			//--------------------------------------------------------------------------------------------------------------------------
			// Restore previous tranform
			//--------------------------------------------------------------------------------------------------------------------------
			g_pD2DDeviceContext->SetTransform(originalTransform);
        } // if (i == m_currentIndex)
        else if (m_currentZoom <= ZoomMaximum)
        {
			//--------------------------------------------------------------------------------------------------------------------------
			// Simply draw the image
			//--------------------------------------------------------------------------------------------------------------------------
			m_images[imageIndex].Image->Draw(imageIndex == m_currentIndex);
        }
    }
	//--------------------------------------------------------------------------------------------------------------------------
	// not a rotation or flip animation
	//--------------------------------------------------------------------------------------------------------------------------
	else
    {
        D2D1_POINT_2F point;
        if (SUCCEEDED(m_images[imageIndex].Animation->GetCurrentPoint(&point)))
        {
			//--------------------------------------------------------------------------------------------------------------------------
			// Use translation matrix to draw this image at the specified point.
            // Scale should not play a factor here since the slide animation
            // should only be kicked off when zoomed all the way out
			//--------------------------------------------------------------------------------------------------------------------------
			g_pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(point.x - rect.left, point.y - rect.top));

			//--------------------------------------------------------------------------------------------------------------------------
			// Draw the image
			//--------------------------------------------------------------------------------------------------------------------------
			m_images[imageIndex].Image->Draw(imageIndex == m_currentIndex);
        }
    }
}

//
// Invalidates the current window which will cause a WM_PAINT to be generated
//
void ImageEditorHandler::InvalidateWindow()
{
	::InvalidateRect(m_hWnd, NULL, FALSE);
}

//--------------------------------------------------------------------------------------------------------------------------
// Setup all the necessary transitions for the slide animation
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::SetupAnimation()
{
	//--------------------------------------------------------------------------------------------------------------------------
	// Only allow one animation at a time
	//--------------------------------------------------------------------------------------------------------------------------
	if (m_animationEnabled)
    {
        return;
    }

	//--------------------------------------------------------------------------------------------------------------------------
	// Initialize Animation
	//--------------------------------------------------------------------------------------------------------------------------
	m_animationEnabled = true;

    if (m_isRotation || m_isFlip)
    {
		//--------------------------------------------------------------------------------------------------------------------------
		// Setup transformtion animations
		//--------------------------------------------------------------------------------------------------------------------------
		SetupTransformationAnimation();
    }
    else
    {
		//--------------------------------------------------------------------------------------------------------------------------
		// Setup slide animation
		//--------------------------------------------------------------------------------------------------------------------------
		for (int i = m_currentRangeStart; i < m_currentRangeEnd + 1; i++)
        {
            D2D1_RECT_F rect;

            if (SUCCEEDED(m_images[i].Image->GetDrawingRect(&rect)))
            {
                D2D1_POINT_2F initialPoint;
                initialPoint.x = rect.left + m_currentPanPoint.x;
                initialPoint.y = rect.top + m_currentPanPoint.y;

				//--------------------------------------------------------------------------------------------------------------------------
				// Initialize animation with initial point
				//--------------------------------------------------------------------------------------------------------------------------
				SharedObject<PointAnimation>::Create(initialPoint, &m_images[i].Animation);
            }
        }

		//--------------------------------------------------------------------------------------------------------------------------
		// Recaulate image positions
		//--------------------------------------------------------------------------------------------------------------------------
		CalculateImagePositions();

		//--------------------------------------------------------------------------------------------------------------------------
		// Animate to target location
		//--------------------------------------------------------------------------------------------------------------------------
		for (int i = m_currentRangeStart; i < m_currentRangeEnd + 1; i++)
        {
            D2D1_RECT_F rect;

            if (SUCCEEDED(m_images[i].Image->GetDrawingRect(&rect)))
            {
                if (m_images[i].Animation)
                {
					//--------------------------------------------------------------------------------------------------------------------------
					// Setup animation to animate to end location
					//--------------------------------------------------------------------------------------------------------------------------
					m_images[i].Animation->Setup(D2D1::Point2F(rect.left, rect.top), SlideAnimationDuration);
                }
            }
        }
    }

#ifdef _MEASURE_FPS
    // Setup animation time
    ::GetSystemTime(&m_startAnimationTime);
    m_totalFramesRendered = 0;
#endif
    InvalidateWindow();
}

//--------------------------------------------------------------------------------------------------------------------------
// Setup animation for rotation and flip transformations
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::SetupTransformationAnimation()
{
    HRESULT hr = S_OK;

    m_transformationAnimationVariable = nullptr;

    if (m_isRotation)
    {
        hr = m_animationManager->CreateAnimationVariable(0, &m_transformationAnimationVariable);
    }
    else if (m_isFlip)
    {
        hr = m_animationManager->CreateAnimationVariable(-1, &m_transformationAnimationVariable);
    }
    else
    {
        return /* Unexpected */;
    }

    ComPtr<IUIAnimationStoryboard> storyboard;
    ComPtr<IUIAnimationTransition> transition;

    if (SUCCEEDED(hr))
    {
        // Initialize storyboard
        hr = m_animationManager->CreateStoryboard(&storyboard);
    }

    if (SUCCEEDED(hr))
    {
        // Create rotation transition
        hr = m_transitionLibrary->CreateAccelerateDecelerateTransition(
									TransformationAnimationDuration,
									m_isFlip ? 1 : (m_currentDrawingOperationType == ImageOperationTypeRotateClockwise ? 90 /* degrees */: -90),
									0.5,
									0.5,
									&transition);
    }

    if (SUCCEEDED(hr))
    {
        hr = storyboard->AddTransition(m_transformationAnimationVariable, transition);
    }

    AnimationUtility::ScheduleStoryboard(storyboard);
}

//--------------------------------------------------------------------------------------------------------------------------
// Disposes of all the animation objects that are done animating
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::CleanupAnimation()
{
    // Disable animation
    m_animationEnabled = false;

    if (m_isRotation || m_isFlip)
    {
        ComPtr<IImageOperation> operation;
        if (SUCCEEDED(SharedObject<ImageTransformationOperation>::Create(m_currentDrawingOperationType, &operation)))
        {
            m_images[m_currentIndex].Image->PushImageOperation(operation);
        }

        m_currentDrawingOperationType = m_prevDrawingOperationType;

        m_isRotation = false;
        m_isFlip = false;
    }

    for (int i = m_currentRangeStart; i < m_currentRangeEnd + 1; i++)
    {
        m_images[i].Animation = nullptr;
    }

    m_images[m_currentIndex].Image->SetBoundingRect(m_imageBoundaryRect);

	//--------------------------------------------------------------------------------------------------------------------------
	// Redraw client area since animation uses a faster method of drawing bitmaps that is not
    // as crisp as a static image
	//--------------------------------------------------------------------------------------------------------------------------
	InvalidateWindow();
}

//--------------------------------------------------------------------------------------------------------------------------
// Event that is fired when the user clicks the File|Open menu item.
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::OpenFile()
{
    ShellFileDialog openDialog;
	HRESULT hr = S_OK;
	reportError(178, (hr = openDialog.SetDefaultFolder(FOLDERID_Pictures)));

    std::vector<ComPtr<IShellItem> > shellItems;
	reportError(176, (hr = openDialog.ShowOpenDialog(&shellItems)));
	reportError(177, (hr = LoadShellItems(&shellItems, nullptr)));
    return hr;
}

HRESULT ImageEditorHandler::SaveFiles()
{
    HCURSOR savedCursor = ::SetCursor(::LoadCursor(nullptr, IDC_WAIT));

    ManageImageResources();
    CalculateImagePositions();
    InvalidateWindow();

    ::SetCursor(savedCursor);
    return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// Event that is fired when the mouse enters the client area
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::OnMouseEnter(D2D1_POINT_2F /*mousePosition*/)
{
    ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
    return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// Event that is fired when the mousewheel is moved
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::OnMouseWheel(D2D1_POINT_2F /*mousePosition*/, short delta, int keys) 
{
    if ((keys & MK_CONTROL) == MK_CONTROL)
    {
        if (delta > 0)
        {
            ZoomIn();
        }
        else
        {
            ZoomOut();
        }
    }
    return S_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
// Event that is called when the corresponding window is resized
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::OnSize(unsigned int width, unsigned int height)
{
    HRESULT hr = S_OK;
	static int cnt = 0;

	D2D1_SIZE_F size = D2D1::SizeF(width, height);
	g_logicalSize = size;

	if (cnt >= 4 && size.width != 0 && size.height != 0)
	{
		g_render->CreateSwapAndSetD2DContextTarget();

		if (g_isDrawingRippleOn)
		{
			g_render->Ripple_UpdateEffectScale();
		}
		if (g_isDrawingScrollOn)
		{
			g_render->HScroll_UpdateEffectScale();
		}
		if (g_isDrawingVScrollOn)
		{
			g_render->VScroll_UpdateEffectScale();
		}
	}
	cnt++;

	if (g_logicalSize.width == 0 || g_logicalSize.height == 0)
	{
		g_logicalSize.width = PEDITOR_WIDTH;
		g_logicalSize.height = PEDITOR_HEIGHT;
	}

	// Update boundary rectangle for images
    m_imageBoundaryRect.left = PreviousNextImageMargin + ImageMargin;
    m_imageBoundaryRect.top = ImageMargin;
    m_imageBoundaryRect.right = size.width - PreviousNextImageMargin - ImageMargin;
    m_imageBoundaryRect.bottom = size.height - ImageMargin;

    // Set maximum slide distance
    m_maxSlideDistance = size.width;;

    hr = ManageImageResources();

    if (SUCCEEDED(hr))
    {
        hr = CalculateImagePositions();
    }
    return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::SetDrawingOperation(__in ImageOperationType imageDrawingOperation)
{
    // Save the previous drawing operation
    m_prevDrawingOperationType = m_currentDrawingOperationType;

    switch (imageDrawingOperation)
    {
    case ImageOperationTypeRotateClockwise:
    case ImageOperationTypeRotateCounterClockwise:
        {
            m_isRotation = true;
            m_currentDrawingOperationType = imageDrawingOperation;
            SetupAnimation();
            break;
        }
    case ImageOperationTypeFlipVertical:
    case ImageOperationTypeFlipHorizontal:
        {
            m_currentDrawingOperationType = imageDrawingOperation;
            m_isFlip = true;
            SetupAnimation();
            break;
        }
    case ImageOperationTypeCrop:
        {
            // flip pen drawing operation based on toggle button input
            if (m_currentDrawingOperationType == ImageOperationTypeCrop)
            {
                m_currentDrawingOperationType = ImageOperationTypeNone;
                m_startClipping = false;
                InvalidateWindow();
            }
            else
            {
                m_currentDrawingOperationType = ImageOperationTypeCrop;
                m_images[m_currentIndex].Image->GetTransformedRect(GetCenter(), &m_currentClipBoundary);
                m_startClipping = true;
                InvalidateWindow();
            }
            break;
        }
	case ImageOperationTypeCopy:
		{
			if (m_currentDrawingOperationType == ImageOperationTypeCopy)
			{
				m_currentDrawingOperationType = ImageOperationTypeNone;
			}
			else
			{
				m_currentDrawingOperationType = ImageOperationTypeCopy;
			}
			break;
		}
    case ImageOperationTypePen:
        {
            // flip pen drawing operation based on toggle button input
            if (m_currentDrawingOperationType == ImageOperationTypePen)
            {
                m_currentDrawingOperationType = ImageOperationTypeNone;
            }
            else
            {
                m_currentDrawingOperationType = ImageOperationTypePen;
            }
            break;
        }
	case ImageOperationTypeLayer:
		{
			if (m_currentDrawingOperationType == ImageOperationTypeLayer)
			{
				m_currentDrawingOperationType = ImageOperationTypeNone;
				PAST_Deactivate();
			}
			else
			{
				m_currentDrawingOperationType = ImageOperationTypeLayer;
			}
			break;
		}    
	default:
        {
            m_currentDrawingOperationType = imageDrawingOperation;
        }
    }

    return S_OK;
}

HRESULT ImageEditorHandler::SetPenColor(__in D2D1_COLOR_F penColor)
{
    m_penColor = penColor;
    return S_OK;
}

HRESULT ImageEditorHandler::SetPenSize(__in float penSize)
{
    m_penSize = penSize;
    return S_OK;
}

HRESULT ImageEditorHandler::CanUndo(__out bool* canUndo)
{
    if (nullptr == canUndo)
    {
        return E_POINTER;
    }

    if (m_images.empty())
    {
        *canUndo = false;
        return E_FAIL;
    }

    return m_images[m_currentIndex].Image->CanUndo(canUndo);
}

HRESULT ImageEditorHandler::CanRedo(__out bool* canRedo)
{
    if (nullptr == canRedo)
    {
        return E_POINTER;
    }

    if (m_images.empty())
    {
        *canRedo = false;
        return E_FAIL;
    }

    return m_images[m_currentIndex].Image->CanRedo(canRedo);
}

//--------------------------------------------------------------------------------------------------------------------------
// Displays a TaskDialog that shows the user which files failed to save
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::ShowSaveFailure(int imageIndex)
{
    // Get file name of file that failed to save
    ImageInfo info;
    if (SUCCEEDED(m_images[imageIndex].Image->GetImageInfo(&info)))
    {
        ::TaskDialog(
            (HWND)m_hWnd,
            nullptr, 
            MAKEINTRESOURCE(IDS_APP_TITLE),
            MAKEINTRESOURCE(IDS_SAVE_FAILED_TITLE),
            info.fileName.c_str(),
            TDCBF_OK_BUTTON,
            TD_WARNING_ICON, 
            nullptr);
    }

    m_images[imageIndex].Image->DiscardResources();
}

bool ImageEditorHandler::IsImageHit(D2D1_POINT_2F mousePosition)
{
	//--------------------------------------------------------------------------------------------------------------------------
	// First transform back the point (disregarding current translation and scale)
	//--------------------------------------------------------------------------------------------------------------------------
	mousePosition = RemoveRenderingTransformations(mousePosition);

    bool isHit = false;
    m_images[m_currentIndex].Image->ContainsPoint(mousePosition, &isHit);
    return isHit;
}

D2D1_POINT_2F ImageEditorHandler::RemoveRenderingTransformations(D2D1_POINT_2F mousePosition)
{
    mousePosition = D2D1::Matrix3x2F::Translation(-m_currentPanPoint.x, -m_currentPanPoint.y).TransformPoint(mousePosition);
    mousePosition = D2D1::Matrix3x2F::Scale(1/ m_currentZoom, 1 / m_currentZoom, GetCenter()).TransformPoint(mousePosition);
    return mousePosition;
}

D2D1_POINT_2F ImageEditorHandler::AdjustToClipRect(D2D1_POINT_2F absPoint)
{
    D2D1_RECT_F clipRect;
    m_images[m_currentIndex].Image->GetClipRect(&clipRect);

    return D2D1::Point2F(
									max(clipRect.left, min(clipRect.right, absPoint.x)),
									max(clipRect.top, min(clipRect.bottom, absPoint.y)));
}

void ImageEditorHandler::UpdateMouseCursor(D2D1_POINT_2F mousePosition)
{
    bool isHit = IsImageHit(mousePosition);

    if (m_currentDrawingOperationType == ImageOperationTypePen)
    {
        if (isHit || m_isDrawing)
        {
            ::SetCursor(::LoadCursor(nullptr, IDC_PEN));
        }
        else
        {
            ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
        }
    } 
    else if (m_currentDrawingOperationType == ImageOperationTypeCrop)
    {
        if (isHit || m_isClipping)
        {
            ::SetCursor(::LoadCursor(nullptr, IDC_CROSS));
        }
        else
        {
            ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
        }
    }
}

LRESULT ImageEditorHandler::AnnotCrop(WORD, WORD wID, HWND, BOOL&)
{
	PAST_Deactivate();
	SetDrawingOperation(ImageOperationTypeCrop);
	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotPencil(WORD, WORD wID, HWND, BOOL&)
{
	PAST_Deactivate();
	SetDrawingOperation(ImageOperationTypePen);
	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotOnMouseLeave(UINT message, WPARAM wParam, LPARAM lParam, BOOL &)
{
	m_isMouseCursorInWindow = false;
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotSaveFiles(WORD, WORD wID, HWND, BOOL&)
{
	ComPtr<IWICBitmapSource> wpic = m_images[m_currentIndex].Image->GetWICBitmap();
	Direct2DUtility::SaveBitmapToFile(wpic, L"K:\\output.png");
	return 0;
}

LRESULT ImageEditorHandler::AnnotText(WORD, WORD wID, HWND, BOOL&)
{
	m_currentDrawingOperationType = ImageOperationTypeText;
	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotLayer(WORD, WORD wID, HWND, BOOL&)
{
	m_currentDrawingOperationType = ImageOperationTypeLayer;
	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotStopText(WORD, WORD wID, HWND, BOOL&)
{
	PAST_Deactivate();
	m_currentDrawingOperationType = ImageOperationTypeNone;
	InvalidateWindow();
	return 0;
}

static D2D1_POINT_2F previousTextingPoint;
static D2D1_POINT_2F startTextingPoint;
static D2D1_POINT_2F previousLayeringPoint;

//--------------------------------------------------------------------------------------------------------------------------
// Event that is called when the left mouse button is pressed
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotOnLeftMouseButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL &)
{
	if (g_isDrawingRippleOn)
	{
		g_render->Ripple_UpdatePointer(D2D1::Point2F(LOWORD(lParam), HIWORD(lParam)));
	}

	D2D1_POINT_2F mousePosition = Direct2DUtility::GetMousePositionForCurrentDpi(lParam);

	HRESULT hr = S_OK;
	bool isHit = IsImageHit(mousePosition);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	if (m_currentDrawingOperationType == ImageOperationTypeText)
	{
		if (TEXT_IsTextHit(mousePosition))
		{
			m_isTexting = true;
		}

		if (TEXT_IsActive(m_currentOperation))
		{
			TEXT_StartTracking(mousePosition);
		}
		else
		{
			TEXT_CreateNewText(mousePosition);
		}

		InvalidateWindow();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_currentDrawingOperationType == ImageOperationTypeLayer)
	{
		if (PLYR_IsLayerHit(mousePosition))
		{
			m_isLayering = true;
		}

		if (PLYR_IsActive(m_currentOperation))
		{
			PLYR_StartTracking(mousePosition);
		}

		InvalidateWindow();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (!m_isDrawing && m_currentDrawingOperationType == ImageOperationTypePen)
	{
		m_currentOperation = nullptr;
		hr = SharedObject<DrawGeometryOperation>::Create(&m_currentOperation);

		ComPtr<IDrawGeometryOperation> drawGeometry;
		if (SUCCEEDED(m_currentOperation.QueryInterface(&drawGeometry)))
		{
			drawGeometry->SetBrushColor(m_penColor);
			drawGeometry->SetStrokeSize(m_penSize);

			drawGeometry->AppendPoint(g_pD2DDeviceContext, GetAbsolutePosition(mousePosition)); 
			drawGeometry->AppendPoint(g_pD2DDeviceContext, GetAbsolutePosition(mousePosition));
			m_images[m_currentIndex].Image->PushImageOperation(m_currentOperation);
		}

		m_isDrawing = true;
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_currentDrawingOperationType == ImageOperationTypeCrop && isHit)
	{
		m_currentClipBoundary.left = m_currentClipBoundary.right = mousePosition.x;
		m_currentClipBoundary.top = m_currentClipBoundary.bottom = mousePosition.y;

		//--------------------------------------------------------------------------------------------------------------------------
		// First transform back the point (disregarding current translation and scale)
		//--------------------------------------------------------------------------------------------------------------------------
		mousePosition = RemoveRenderingTransformations(mousePosition);

		//--------------------------------------------------------------------------------------------------------------------------
		// Adjust for drawing
		//--------------------------------------------------------------------------------------------------------------------------
		m_currentClipDrawBox.left = m_currentClipDrawBox.right = mousePosition.x;
		m_currentClipDrawBox.top = m_currentClipDrawBox.bottom = mousePosition.y;

		m_isClipping = true;
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_currentDrawingOperationType == ImageOperationTypeCopy && isHit)
	{
		D2D1_POINT_2F absPoint = GetAbsolutePosition(mousePosition);

		m_images[m_currentIndex].Image->GetPastingTransformation(&m_copyMatrix);
		m_copyFramePoint = m_copyMatrix.TransformPoint(absPoint);

		m_copyStartPoint = CLBD_GetCopyPoint(mousePosition);
		m_copyFinalPoint = m_copyStartPoint;
		m_copyBorderWidth = 0;
		m_copyBorderHeight = 0;

		m_isCopying = true;
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else
	{
		// Store mouse point for panning
		m_previousMousePosition = mousePosition;

		// Store mouse point for mouse up
		m_mouseDownPosition = mousePosition;
	}

	::SetCapture(m_hWnd);

	if (SUCCEEDED(hr))
	{
		// We use S_FALSE to signal that the parent of 
		// this window should be notified as well
		if (hr == S_FALSE)
		{
			::SendNotifyMessage(::GetParent(m_hWnd), message, wParam, lParam);
		}
	}
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Event that is fired when the mouse is moved
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotOnMouseMove(UINT message, WPARAM wParam, LPARAM lParam, BOOL &)
{
	D2D1_POINT_2F mousePosition = Direct2DUtility::GetMousePositionForCurrentDpi(lParam);
	if (!m_isMouseCursorInWindow)
	{
		m_isMouseCursorInWindow = true;

		TRACKMOUSEEVENT trackMouseEvent = { sizeof(trackMouseEvent) };
		trackMouseEvent.dwFlags = TME_LEAVE;
		trackMouseEvent.hwndTrack = m_hWnd;
		TrackMouseEvent(&trackMouseEvent);

		//--------------------------------------------------------------------------------------------------------------------------
		// Ignore the mouse enter return value because it's not used by Windows
		//--------------------------------------------------------------------------------------------------------------------------
		OnMouseEnter(mousePosition);
	}
	
	HRESULT hr = S_OK;

	UpdateMouseCursor(mousePosition);

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	if (m_isDrawing)
	{
		ComPtr<IDrawGeometryOperation> drawGeometry;
		if (SUCCEEDED(m_currentOperation.QueryInterface(&drawGeometry)))
		{
			drawGeometry->AppendPoint(g_pD2DDeviceContext, GetAbsolutePosition(mousePosition));
			InvalidateWindow();
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_isClipping)
	{
		m_currentClipBoundary.right = mousePosition.x;
		m_currentClipBoundary.bottom = mousePosition.y;

		mousePosition = RemoveRenderingTransformations(mousePosition);

		D2D1_RECT_F rect;
		m_images[m_currentIndex].Image->GetTransformedRect(GetCenter(), &rect);

		m_currentClipDrawBox.right = max(rect.left, min(rect.right, mousePosition.x));
		m_currentClipDrawBox.bottom = max(rect.top, min(rect.bottom, mousePosition.y));
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	// m_copyFinalPoint 为 copy area 最终的左上角位置
	// m_copyBorderWidth 和 m_copyBorderHeight 的绝对值为 copy area 的宽度和高度
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_isCopying)
	{
		D2D1_POINT_2F mPoint = GetAbsolutePosition(mousePosition);
		D2D1_POINT_2F mpCopying = m_copyMatrix.TransformPoint(mPoint);

		m_copyBorderWidth = mpCopying.x - m_copyFramePoint.x;
		m_copyBorderHeight = mpCopying.y - m_copyFramePoint.y;

		D2D1_POINT_2F nowPoint = CLBD_GetCopyPoint(mousePosition);
		FLOAT x = max(0, min(nowPoint.x, m_copyStartPoint.x));
		FLOAT y = max(0, min(nowPoint.y, m_copyStartPoint.y));
		m_copyFinalPoint = D2D1::Point2F(x, y);

		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	// 鼠标左键在Text操作中按下后，如果拖动的是边框，则AdjustingCenterOrBorder，如果鼠标在编辑窗口内按下，
	// 则高亮 MarkLeft 到 MarkRight 标记的字符串
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_isTexting)
	{
		TEXT_AdjustingCenterOrBorder(mousePosition);

		ComPtr<IImageTextOperation> textOperation;
		if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
		{
			CBTextPtr bp;
			textOperation->GetTextEquipment(&bp);

			D2D1_POINT_2F ptBase, mpTexting;
			if (ROTA_GetRelativeAndMousePt(textOperation, bp, mousePosition, &ptBase, &mpTexting))
			{
				//D2D1_POINT_2F mPoint = GetAbsolutePosition(mousePosition);
				//D2D1_POINT_2F mpTexting = bp.m_pastingMatrix.TransformPoint(mPoint);
				if (bp.m_tbIsActiveEdit)
				{
					for (int i = 0; i < (int)bp.m_textLayout.size(); i++)
					{
						//--------------------------------------------------------------------------------------------------------------------------
						// if 为 true, 则鼠标扫过当前字符的空间范围。
						//--------------------------------------------------------------------------------------------------------------------------
						if (mpTexting.x >= bp.m_textLayout.at(i)->m_charLayoutRect.left &&
							mpTexting.x <= bp.m_textLayout.at(i)->m_charLayoutRect.right + bp.m_textCharSpace &&
							mpTexting.y >= bp.m_textLayout.at(i)->m_charLayoutRect.top &&
							mpTexting.y <= bp.m_textLayout.at(i)->m_charLayoutRect.bottom + bp.m_textLineSpace)
							{
								TEXT_TrackFollowingWord(bp, i);
								break;
							}

						if (bp.m_textStyle != style_b)
						{
							if (i != bp.m_textLayout.size() - 1)
							{
								if (mpTexting.x >= bp.m_textLayout.at(i)->m_charLayoutRect.left &&
									mpTexting.x <= bp.m_rotatableRelativePt.x + bp.m_rotatableBorderWidth &&
									mpTexting.y >= bp.m_textLayout.at(i)->m_charLayoutRect.top &&
									mpTexting.y < bp.m_textLayout.at(i + 1)->m_charLayoutRect.top)
								{
									TEXT_TrackFollowingWord(bp, i);
									break;
								}
							}
							else
							{
								if (mpTexting.x >= bp.m_textLayout.at(i)->m_charLayoutRect.left &&
									mpTexting.x <= bp.m_rotatableRelativePt.x + bp.m_rotatableBorderWidth &&
									mpTexting.y >= bp.m_textLayout.at(i)->m_charLayoutRect.top &&
									mpTexting.y <= bp.m_textLayout.at(i)->m_charLayoutRect.bottom + bp.m_textLineSpace)
								{
									TEXT_TrackFollowingWord(bp, i+1);
									break;
								}
							}
						}
					}
				}
			}
		
			textOperation->SetTextEquipment(bp);
		}
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	// 拖着Layer到处跑
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_isLayering)
	{
		ComPtr<IImageLayerOperation> layerOperation;
		if (m_currentOperation != nullptr && SUCCEEDED(m_currentOperation.QueryInterface(&layerOperation)))
		{
			D2D1_POINT_2F mPoint = GetAbsolutePosition(mousePosition);
			CBLayerPtr bp;
			layerOperation->GetLayerEquipment(&bp);

			D2D1_POINT_2F mpLayering = bp.m_pastingMatrix.TransformPoint(mPoint);

			bp.m_rotatableRelativePt.x += mpLayering.x - previousLayeringPoint.x;
			bp.m_rotatableRelativePt.y += mpLayering.y - previousLayeringPoint.y;
			
			ROTA_ShowPosition(bp);

			layerOperation->SetLayerEquipment(bp);

			PLYR_SetCursor(bp.m_layerBorderIsActive);
			previousLayeringPoint = mpLayering;
		}
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// 光标在Text上划过
		//--------------------------------------------------------------------------------------------------------------------------
		if (m_currentDrawingOperationType == ImageOperationTypeText)
		{
			if (m_currentOperation != nullptr && TEXT_IsActive(m_currentOperation))
			{
				ComPtr<IImageTextOperation> textOperation;
				if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
				{
					bool center, edit, right, left, bottom;
					TEXT_HitTextBorderTest(textOperation, mousePosition, center, edit, right, left, bottom);
					TEXT_SetCursor(center, edit, right, left, bottom);
				}
			}
		}
		//--------------------------------------------------------------------------------------------------------------------------
		// 光标在Layer上划过
		//--------------------------------------------------------------------------------------------------------------------------
		else if (m_currentDrawingOperationType == ImageOperationTypeLayer)
		{
			if (m_currentOperation != nullptr && PLYR_IsActive(m_currentOperation))
			{
				ComPtr<IImageLayerOperation> layerOperation;
				if (SUCCEEDED(m_currentOperation.QueryInterface(&layerOperation)))
				{
					bool center;
					center = PLYR_HitLayerBorderTest(layerOperation, mousePosition);
					PLYR_SetCursor(center);
				}
			}
		}
		else
		{
			if (SUCCEEDED(hr))
			{
				bool isMouseCaptured = (::GetCapture() == m_hWnd);
				//--------------------------------------------------------------------------------------------------------------------------
				// 在 text、Layer 或 draw 都 off 的状态下，如果按鼠标左键再拖动鼠标，则相应移动 Pan 的位移
				//--------------------------------------------------------------------------------------------------------------------------
				if (isMouseCaptured)
				{
					// Update pan based on delta
					D2D1_POINT_2F delta;
					delta.x = (mousePosition.x - m_previousMousePosition.x);
					delta.y = (mousePosition.y - m_previousMousePosition.y);

					if (!(delta.x == 0 && delta.y == 0))
					{
						PanImage(delta, false);

						// Save current mouse position for next mouse message
						m_previousMousePosition = mousePosition;

						// Keep pan within the boundaries on the maximum slide distance
						if (std::abs(m_currentPanPoint.x) > m_maxSlideDistance)
						{
							if (m_currentPanPoint.x > 0)
							{
								m_currentPanPoint.x = m_maxSlideDistance;
							}
							else
							{
								m_currentPanPoint.x = -m_maxSlideDistance;
							}
						}
					}
				}
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// A return value of S_FALSE signlas the need to notify parent window
		//--------------------------------------------------------------------------------------------------------------------------
		if (hr == S_FALSE)
		{
			::SendNotifyMessage(::GetParent(m_hWnd), message, wParam, lParam);
		}
	}
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Event that is fired when the left mouse button is released
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotOnLeftMouseButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL &)
{
	D2D1_POINT_2F mousePosition = Direct2DUtility::GetMousePositionForCurrentDpi(lParam);

	HRESULT hr = S_OK;

	// Release mouse capture
	::ReleaseCapture();

	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	if (m_isDrawing)
	{
		// Reset the current image operation
		m_currentOperation = nullptr;
		m_isDrawing = false;

		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_isClipping)
	{
		// Crop only if the crop area is large enough
		if (Direct2DUtility::GetRectWidth(m_currentClipBoundary) > 0 && 
			Direct2DUtility::GetRectHeight(m_currentClipBoundary) > 0)
		{
			D2D1_POINT_2F clipStart = GetAbsolutePosition(D2D1::Point2F(	m_currentClipBoundary.left, 
																												m_currentClipBoundary.top));
			D2D1_POINT_2F clipEnd = GetAbsolutePosition(mousePosition);

			ComPtr<IImageOperation> clip;
			hr = SharedObject<ImageClippingOperation>::Create(
																							D2D1::RectF(clipStart.x, clipStart.y, clipEnd.x, clipEnd.y),
																							&clip);

			m_images[m_currentIndex].Image->PushImageOperation(clip);

			// Reset the pan points
			m_currentPanPoint.x = 0;
			m_currentPanPoint.y = 0;
		}
		m_isClipping = false;
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_isTexting)
	{
		ComPtr<IImageTextOperation> textOperation;
		if (m_currentOperation != nullptr && 
			SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
		{
			CBTextPtr bp;
			textOperation->GetTextEquipment(&bp);
			bp.m_tbIsActiveLeftTop = false;
			bp.m_tbIsActiveEdit = false;
			bp.m_tbIsActiveRight = false;
			bp.m_tblIsActiveLeft = false;
			bp.m_tbIsActiveBottom = false;

			D2D1_POINT_2F ptBase, mpTexting;
			if (ROTA_GetRelativeAndMousePt(textOperation, bp, mousePosition, &ptBase, &mpTexting))
			{
				//--------------------------------------------------------------------------------------------------------------------------
				// startTextingPoint 为鼠标左键按下去时，在经翻转变换后编辑窗口的位置(由ROTA_GetRelativeAndMousePt计算得来
				//--------------------------------------------------------------------------------------------------------------------------
				if (mpTexting.x == startTextingPoint.x && mpTexting.y == startTextingPoint.y)
				{
					bp.m_textIfMarked = false;
				}
			}

			textOperation->SetTextEquipment(bp);
		}

		m_isTexting = false;
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_isLayering)
	{
		m_isLayering = false;
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_isCopying)
	{
		if (m_copyBorderWidth != 0 && m_copyBorderHeight != 0)
		{
			CLBD_CopyToClipboard(m_copyFinalPoint, m_copyBorderWidth, m_copyBorderHeight);
		}

		m_isCopying = false;
		InvalidateWindow();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	// 如果在整图显示的状态下，鼠标拖动图片，则划到左右图片或开启动画效果恢复原始图片位置
	//--------------------------------------------------------------------------------------------------------------------------
	else if (m_currentZoom <= ZoomMinimum)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Determine if user has clicked on left or right image
		//--------------------------------------------------------------------------------------------------------------------------
		if (mousePosition.x < PreviousNextImageMargin && m_mouseDownPosition.x < PreviousNextImageMargin)
		{
			PreviousImage();
		}
		else if (mousePosition.x > g_logicalSize.width - PreviousNextImageMargin &&
			m_mouseDownPosition.x > g_logicalSize.width - PreviousNextImageMargin)
		{
			NextImage();
		}
		else
		{
			//--------------------------------------------------------------------------------------------------------------------------
			// Determine if user has panned far enough to go to the next/previous image. The trigger
			// distance is 1/4 of the main image area
			//--------------------------------------------------------------------------------------------------------------------------
			float triggerDistance = (g_logicalSize.width - PreviousNextImageMargin * 2) / 4;

			if (std::fabs(m_currentPanPoint.x) > triggerDistance)
			{
				if (m_currentPanPoint.x > 0)
				{
					PreviousImage();
				}
				else
				{
					NextImage();
				}
			}
			else
			{
				SetupAnimation();
			}
		}
	}

	PanImage(D2D1::Point2F(0, 0), true);

	if (SUCCEEDED(hr))
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// We use S_FALSE to signal that the parent of 
		// this window should be notified as well
		//--------------------------------------------------------------------------------------------------------------------------
		if (hr == S_FALSE)
		{
			::SendNotifyMessage(::GetParent(m_hWnd), message, wParam, lParam);
		}
	}
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotCopyCurrentImageDataToClipboard(WORD, WORD wID, HWND, BOOL&)
{
	PAST_Deactivate();
	SetDrawingOperation(ImageOperationTypeCopy);
	return 0;
}

static bool isCtrldown = false;
static bool isVdown = false;

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotChar(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	unsigned int keyp = wParam;

	if (m_currentDrawingOperationType == ImageOperationTypeText && m_currentOperation != nullptr)
	{
		ComPtr<IImageTextOperation> textOperation;
		if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
		{
			CBTextPtr bp;
			textOperation->GetTextEquipment(&bp);

			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			if (keyp == VK_BACK)
			{
				ImageTextOperation::TEXT_KeyDown_Back(bp);
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (keyp == ('C' & 0x1f))		// Ctrl - C
			{
				if (bp.m_textIfMarked == true && bp.m_textMarkLeft != bp.m_textMarkRight)
				{
					CString right = bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textMarkLeft);
					CString left = right.Left(bp.m_textMarkRight - bp.m_textMarkLeft);
					CLBD_CopyToClipboard(left);
				}
				bp.m_textIfMarked = false;
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (keyp == ('X' & 0x1f))		// Ctrl - X
			{
				if (bp.m_textIfMarked == true && bp.m_textMarkLeft != bp.m_textMarkRight)
				{
					CString right = bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textMarkLeft);
					CString left = right.Left(bp.m_textMarkRight - bp.m_textMarkLeft);
					CLBD_CopyToClipboard(left);

					bp.m_textString = bp.m_textString.Left(bp.m_textMarkLeft) +
						bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textMarkRight);
					bp.m_textPoint = bp.m_textMarkLeft;
					bp.m_textIfMarked = false;
					ImageTextOperation::TEXT_FrameOutText(bp);
				}
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (keyp == ('V' & 0x1f))		// Ctrl - V
			{
				WCHAR * strClip = CLBD_GetTextFromClipboard();
				if (strClip == nullptr)
					return 0;

				int left = bp.m_textPoint - 1;
				if (left < 0)
					left = 0;

				CString sadded(strClip);
				sadded.Replace(L"\n", L"");

				if (bp.m_textIfMarked == true)
				{
					CString left = bp.m_textString.Left(bp.m_textMarkLeft);
					CString right = bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textMarkRight);
					bp.m_textString = left + right;
					bp.m_textPoint = bp.m_textMarkLeft;
				}
				CString s1 = bp.m_textString.Left(bp.m_textPoint);
				CString s2 = bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textPoint);
				bp.m_textString = s1 + sadded + s2;
				bp.m_textPoint += sadded.GetLength();
				bp.m_textIfMarked = false;
				ImageTextOperation::TEXT_FrameOutText(bp);
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else
			{
				if (bp.m_textIfMarked == true)
				{
					CString left = bp.m_textString.Left(bp.m_textMarkLeft);
					CString right = bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textMarkRight);
					bp.m_textString = left + right;
					bp.m_textPoint = bp.m_textMarkLeft;
				}
				bp.m_textString = bp.m_textString.Left(bp.m_textPoint) +
					(TCHAR)keyp +
					bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textPoint);
				bp.m_textPoint++;
				bp.m_textIfMarked = false;
				ImageTextOperation::TEXT_FrameOutText(bp);
			}
			textOperation->SetTextEquipment(bp);

			CombDlg::COMB_UpdateTextLayerString(m_currentOperation);
			
			InvalidateWindow();
		}
	}
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Event that is fired when a key on the keyboard is pressed
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotKeyDown(UINT, WPARAM wParam, LPARAM lParam, BOOL &)
{
	unsigned int vKey = wParam;
	unsigned int key = lParam;

	if (vKey == VK_ESCAPE && (g_isLayerAnimationOn || g_isDrawingRippleOn || g_isDrawingScrollOn || g_isDrawingVScrollOn || g_fullScreen))
	{
		g_isLayerAnimationOn = false;
		g_isDrawingRippleOn = false;
		g_isDrawingScrollOn = false;
		g_isDrawingVScrollOn = false;

		g_fullScreen = false;
		ZoomFull();
		OffMaximize();
		TransformationAnimationDuration = 1.0f;
		return 0;
	}

	if (m_currentDrawingOperationType == ImageOperationTypeText && m_currentOperation != nullptr)
	{
		ComPtr<IImageTextOperation> textOperation;
		if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
		{
			CBTextPtr bp;
			textOperation->GetTextEquipment(&bp);
			bool ifDelete = false;
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			if (vKey == VK_RIGHT && GetAsyncKeyState(VK_SHIFT) & 0x8000)
			{
				ImageTextOperation::TEXT_KeyDown_Shift_Right(bp);
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (vKey == VK_LEFT && GetAsyncKeyState(VK_SHIFT) & 0x8000)
			{
				ImageTextOperation::TEXT_KeyDown_Shift_Left(bp);
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (vKey == VK_UP && GetAsyncKeyState(VK_SHIFT) & 0x8000)
			{
				ImageTextOperation::TEXT_KeyDown_Shift_Up(bp);
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (vKey == VK_DOWN && GetAsyncKeyState(VK_SHIFT) & GetAsyncKeyState(VK_DOWN) & 0x8000)
			{
				ImageTextOperation::TEXT_KeyDown_Shift_Down(bp);
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (vKey == VK_LEFT)
			{
				ImageTextOperation::TEXT_KeyDown_Left(bp);
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (vKey == VK_RIGHT)
			{
				ImageTextOperation::TEXT_KeyDown_Right(bp);
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (vKey == VK_UP)
			{
				ImageTextOperation::TEXT_KeyDown_Up(bp, bp.m_textPoint);
				bp.m_textIfMarked = false;
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (vKey == VK_DOWN && GetAsyncKeyState(VK_DOWN) & 0x8000)
			{
				ImageTextOperation::TEXT_KeyDown_Down(bp, bp.m_textPoint);
				bp.m_textIfMarked = false;
			}
			//--------------------------------------------------------------------------------------------------------------------------
			//--------------------------------------------------------------------------------------------------------------------------
			else if (vKey == VK_DELETE)
			{
				ImageTextOperation::TEXT_KeyDown_Delete(bp);
				ifDelete = true;
			}
			else if (vKey == VK_ESCAPE)
			{
				ImageTextOperation::TEXT_KeyDown_Escape(bp);
			}
			else
			{
				//bp.m_textIfMarked = false;
			}

			textOperation->SetTextEquipment(bp);
			if (ifDelete == true)
			{
				CombDlg::COMB_UpdateTextLayerString(m_currentOperation);
			}
			InvalidateWindow();
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (vKey == VK_LEFT)
	{
		if (m_currentZoom <= ZoomMinimum)
		{
			PreviousImage();
		}
		else
		{
			PanImage(D2D1::Point2F(KeyboardPanDistance, 0), true);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (vKey == VK_RIGHT)
	{
		if (m_currentZoom <= ZoomMinimum)
		{
			NextImage();
		}
		else
		{
			PanImage(D2D1::Point2F(-KeyboardPanDistance, 0), true);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (vKey == VK_UP)
	{
		if (m_currentZoom > ZoomMinimum)
		{
			PanImage(D2D1::Point2F(0, KeyboardPanDistance), true);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (vKey == VK_DOWN)
	{
		if (m_currentZoom > ZoomMinimum)
		{
			PanImage(D2D1::Point2F(0, -KeyboardPanDistance), true);
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (vKey == VK_ADD || vKey == VK_OEM_PLUS)
	{
		ZoomIn();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (vKey == VK_SUBTRACT || vKey == VK_OEM_MINUS)
	{
		ZoomOut();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	else if (vKey == VK_ESCAPE)
	{
		m_currentZoom = ZoomMinimum;
		ZoomOut();
	}
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::TEXT_UpdateFont(CHARFORMAT2& cf)
{
	if (cf.dwMask & CFM_COLOR)
	{
		COLORREF color = cf.crTextColor;
		m_textColor = D2D1::ColorF(static_cast<float>(GetRValue(color)) / 255.0f,
			static_cast<float>(GetGValue(color)) / 255.0f,
			static_cast<float>(GetBValue(color)) / 255.0f);
	}

	if (cf.dwMask & CFM_FACE)
	{
		SecureHelper::strcpy_x(g_textFormatFamilyName, LF_FACESIZE, cf.szFaceName);
	}

	if (cf.dwMask & CFM_SIZE)
	{
		m_charSize = cf.yHeight / 20.0f;
	}

	if ((cf.dwMask & CFM_FACE) || (cf.dwMask & CFM_SIZE))
	{
		m_textFormat = nullptr;

		WCHAR * wsFaceName;
		DWRITE_FONT_WEIGHT weight;
		CString faceName = cf.szFaceName;
		if (faceName.Compare(L"Microsoft JhengHei UI") == 0)
		{ 
			wsFaceName = L"Microsoft JhengHei UI";
			weight = DWRITE_FONT_WEIGHT_LIGHT;
		}
		else
		{
			wsFaceName = cf.szFaceName;
			weight = DWRITE_FONT_WEIGHT_REGULAR;
		}

		g_pDWriteFactory->CreateTextFormat(
																	wsFaceName,
																	nullptr,
																	weight,
																	DWRITE_FONT_STYLE_NORMAL,
																	DWRITE_FONT_STRETCH_NORMAL,
																	cf.yHeight / 20.0f, 
																	L"en-us",
																	&m_textFormat);

		SecureHelper::strcpy_x(g_textFormatFamilyName, LF_FACESIZE, wsFaceName);
		g_textFormatFontSize = cf.yHeight / 20.0f;
	}

	if (!((cf.dwMask & CFM_COLOR) || (cf.dwMask & CFM_FACE) || (cf.dwMask & CFM_SIZE)))
		return;

	if (m_currentOperation == nullptr)
		return;

	ComPtr<IImageTextOperation> textOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
	{
		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);
		bp.m_textFormat = m_textFormat;
		bp.m_textColor = m_textColor;
		
		SecureHelper::strcpy_x(bp.m_textFormatFamilyName, LF_FACESIZE, g_textFormatFamilyName);
		bp.m_textFormatFontSize = g_textFormatFontSize;

		ImageTextOperation::TEXT_FrameOutText(bp);
		textOperation->SetTextEquipment(bp);

		InvalidateWindow();
	}
}

LRESULT ImageEditorHandler::OnRibbonFont(WORD hwParam, WORD lwParam, HWND lParam, BOOL&)
{
	UI_EXECUTIONVERB verb = (UI_EXECUTIONVERB)HIWORD(hwParam);
	CHARFORMAT2 * pcf = (CHARFORMAT2 *)lParam;

	//--------------------------------------------------------------------------------------------------------------------------
	//if (verb == UI_EXECUTIONVERB_CANCELPREVIEW)
	//	SetFont(m_font);
	//else
	//	SetDefaultCharFormat(*pcf);
	//--------------------------------------------------------------------------------------------------------------------------

	if (verb == UI_EXECUTIONVERB_EXECUTE)
		TEXT_UpdateFont(*pcf);
	return 0;
}

HRESULT ImageEditorHandler::Undo()
{
	HRESULT hr = m_images[m_currentIndex].Image->UndoImageOperation();
	
	PAST_Deactivate();

	m_currentOperation = nullptr;
	m_currentDrawingOperationType = ImageOperationTypeNone;
	g_frameWork->SetRibbonModes(UI_MAKEAPPMODE(0));
	InvalidateWindow();
	return hr;
}

HRESULT ImageEditorHandler::Redo()
{
	HRESULT hr = m_images[m_currentIndex].Image->RedoImageOperation();
	
	PAST_Deactivate();

	m_currentOperation = nullptr;
	m_currentDrawingOperationType = ImageOperationTypeNone;
	g_frameWork->SetRibbonModes(UI_MAKEAPPMODE(0));
	InvalidateWindow();
	return hr;
}

LRESULT ImageEditorHandler::AnnotColorPicker(WORD, WORD wID, HWND, BOOL&)
{
	COLORREF color = g_frameWork->m_colorForeground.m_color;
	SetPenColor(D2D1::ColorF(static_cast<float>(GetRValue(color)) / 255.0f,
											static_cast<float>(GetGValue(color)) / 255.0f,
											static_cast<float>(GetBValue(color)) / 255.0f));
	return 0;
}

static FLOAT copyFrameDataFontSize = 0.0f;

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::DrawImages(int imageIndex)
{
	if (imageIndex == m_currentIndex)
	{
		D2D1_RECT_F rect = m_images[imageIndex].Image->GetClipRect();
		CString title;
		int width = (int)(rect.right - rect.left);
		int height = (int)(rect.bottom - rect.top);
		
		if (m_images[imageIndex].Image->GetIfHorizontal() == false)
		{
			int tmp = width;
			width = height;
			height = tmp;
		}

		FLOAT scale;
		m_images[imageIndex].Image->GetScale(&scale);

		title.Format(L"image size: %dx%d scaling: %d%%", width, height, (int)(100 * m_currentZoom/scale));
		::SetWindowText(::GetParent(m_hWnd), title.GetBuffer(100));
	}

	//--------------------------------------------------------------------------------------------------------------------------
	// Simply draw image
	//--------------------------------------------------------------------------------------------------------------------------
	if (imageIndex == m_currentIndex || m_currentZoom <= ZoomMinimum)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// 画底层图片和其上的所有layer，text和pen drawing。
		//--------------------------------------------------------------------------------------------------------------------------
		m_images[imageIndex].Image->Draw(imageIndex == m_currentIndex);

		//--------------------------------------------------------------------------------------------------------------------------
		// 如果是copy操作，画copy 操作的框和说明文字。
		//--------------------------------------------------------------------------------------------------------------------------
		if (imageIndex == m_currentIndex && m_currentDrawingOperationType == ImageOperationTypeCopy)
		{
			if (m_isCopying)
			{
				D2D1_MATRIX_3X2_F originalTransform;
				g_pD2DDeviceContext->GetTransform(&originalTransform);

				D2D1_RECT_F imageRect = m_images[imageIndex].Image->GetClipRect();
				D2D1_RECT_F drawingRect = m_images[imageIndex].Image->GetDrawingRect();
				float scale = Direct2DUtility::GetRectWidth(imageRect) / Direct2DUtility::GetRectWidth(drawingRect);

				D2D1::Matrix3x2F matrix1 =
					D2D1::Matrix3x2F::Translation(-imageRect.left, -imageRect.top) *
					D2D1::Matrix3x2F::Scale(1 / scale, 1 / scale) *
					D2D1::Matrix3x2F::Translation(drawingRect.left, drawingRect.top);

				D2D1_POINT_2F pt = m_copyFramePoint;
				D2D1_RECT_F dest = D2D1::RectF(pt.x, pt.y, pt.x + m_copyBorderWidth, pt.y + m_copyBorderHeight);

				//--------------------------------------------------------------------------------------------------------------------------
				// 因为在当前图片的当前位置画框，所以只需pt这个当前相对位置和大小值即可，没有额外翻转
				// 所以 matrix1 * originalTransform 这样简单。
				//--------------------------------------------------------------------------------------------------------------------------
				D2D1_MATRIX_3X2_F transform = matrix1 * originalTransform;

				g_pD2DDeviceContext->SetTransform(transform);

				D2D1_COLOR_F savedColor = m_solidBrush->GetColor();
				float savedOpacity = m_solidBrush->GetOpacity();

				m_solidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
				m_solidBrush->SetOpacity(1.0f);

				g_pD2DDeviceContext->DrawRectangle(dest, m_solidBrush, 3, strokeStyleCustomOffsetZero);

				CString recString;
				recString.Format(L"%dx%d", abs((int)m_copyBorderWidth), abs((int)m_copyBorderHeight));

				//--------------------------------------------------------------------------------------------------------------------------
				// fontSize 按 scale 比例对应放大。
				//--------------------------------------------------------------------------------------------------------------------------
				FLOAT fontSize = 12 * scale;
				if (fontSize != copyFrameDataFontSize)
				{
					m_copyFrameDataFormat = nullptr;
					g_pDWriteFactory->CreateTextFormat(
																			L"arial",
																			nullptr,
																			DWRITE_FONT_WEIGHT_REGULAR,
																			DWRITE_FONT_STYLE_NORMAL,
																			DWRITE_FONT_STRETCH_NORMAL,
																			fontSize,
																			L"en-us",
																			&m_copyFrameDataFormat);
					copyFrameDataFontSize = fontSize;
				}

				m_copyFrameDataLayout = nullptr;
				g_pDWriteFactory->CreateTextLayout(
																			recString.GetBuffer(20),
																			static_cast<unsigned int>(recString.GetLength()),
																			m_copyFrameDataFormat,
																			fontSize * 20,
																			fontSize + 4,
																			&m_copyFrameDataLayout);

				D2D1_RECT_F new_dest = Direct2DUtility::FixRect(dest);
				FLOAT destY = 0;
				if (m_copyFinalPoint.y >= (fontSize + 6 * scale))
					destY = -fontSize - 4 * scale;
				else
					destY = 4 * scale;
				g_pD2DDeviceContext->DrawTextLayout(D2D1::Point2F(new_dest.left, new_dest.top + destY), m_copyFrameDataLayout, m_solidBrush);

				m_solidBrush->SetColor(savedColor);
				m_solidBrush->SetOpacity(savedOpacity);

				g_pD2DDeviceContext->SetTransform(originalTransform);
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// 如果是 crop 操作，再在整个图片上蒙画crop窗口。
		//--------------------------------------------------------------------------------------------------------------------------
		if (imageIndex == m_currentIndex && m_currentDrawingOperationType == ImageOperationTypeCrop)
		{
			D2D1_RECT_F imageRect;
			m_images[imageIndex].Image->GetTransformedRect(GetCenter(), &imageRect);

			D2D1_COLOR_F savedColor = m_solidBrush->GetColor();
			float savedOpacity = m_solidBrush->GetOpacity();

			m_solidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
			m_solidBrush->SetOpacity(0.75f);

			if (m_isClipping)
			{
				D2D1_RECT_F selectionRect = Direct2DUtility::FixRect(m_currentClipDrawBox);

				//--------------------------------------------------------------------------------------------------------------------------
				// Wash out unneeded areas
				//--------------------------------------------------------------------------------------------------------------------------
				g_pD2DDeviceContext->FillRectangle(D2D1::RectF(imageRect.left, imageRect.top, imageRect.right, selectionRect.top), m_solidBrush);
				g_pD2DDeviceContext->FillRectangle(D2D1::RectF(imageRect.left, selectionRect.top, selectionRect.left, selectionRect.bottom), m_solidBrush);
				g_pD2DDeviceContext->FillRectangle(D2D1::RectF(selectionRect.right, selectionRect.top, imageRect.right, selectionRect.bottom), m_solidBrush);
				g_pD2DDeviceContext->FillRectangle(D2D1::RectF(imageRect.left, selectionRect.bottom, imageRect.right, imageRect.bottom), m_solidBrush);
			}
			else if (m_startClipping) // Not yet clipping
			{
				//--------------------------------------------------------------------------------------------------------------------------
				// Wash out the whole image
				//--------------------------------------------------------------------------------------------------------------------------
				g_pD2DDeviceContext->FillRectangle(imageRect, m_solidBrush);
				m_currentClipDrawBox = imageRect;
			}

			if (m_isClipping || m_startClipping)
			{
				m_solidBrush->SetOpacity(1);
				m_solidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));

				//--------------------------------------------------------------------------------------------------------------------------
				// The boundary box of the clipping rectangle
				//--------------------------------------------------------------------------------------------------------------------------
				g_pD2DDeviceContext->DrawRectangle(m_currentClipDrawBox, m_solidBrush, 1, strokeStyleCustomOffsetZero);

				m_solidBrush->SetColor(savedColor);
				m_solidBrush->SetOpacity(savedOpacity);
				m_startClipping = false;
			}
		}
	}
}

D2D1_POINT_2F ImageEditorHandler::CLBD_GetCopyPoint(D2D1_POINT_2F mousePosition)
{
	//--------------------------------------------------------------------------------------------------------------------------
	// First transform back the point (disregarding current translation and scale)
	//--------------------------------------------------------------------------------------------------------------------------
	mousePosition = RemoveRenderingTransformations(mousePosition);
	return m_images[m_currentIndex].Image->CLBD_GetCopyPoint(mousePosition);
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::TEXT_StartTracking(D2D1_POINT_2F mousePosition)
{
	ComPtr<IImageTextOperation> textOperation;
	if (m_currentOperation != nullptr && SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
	{
		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);

		TEXT_HitTextBorderTest(textOperation,
											mousePosition,
											bp.m_tbIsActiveLeftTop,
											bp.m_tbIsActiveEdit,
											bp.m_tbIsActiveRight,
											bp.m_tblIsActiveLeft,
											bp.m_tbIsActiveBottom);
		if (bp.m_tbIsActiveLeftTop || bp.m_tbIsActiveEdit || bp.m_tbIsActiveRight || bp.m_tblIsActiveLeft || bp.m_tbIsActiveBottom)
		{
			D2D1_POINT_2F ptBase, mpTexting;
			//--------------------------------------------------------------------------------------------------------------------------
			// mpTexting 是经过翻转变换后的编辑窗口内的鼠标位置
			//--------------------------------------------------------------------------------------------------------------------------
			if (ROTA_GetRelativeAndMousePt(textOperation, bp, mousePosition, &ptBase, &mpTexting))
			{
				D2D1_POINT_2F mPoint = GetAbsolutePosition(mousePosition);
				D2D1_POINT_2F mpTextingLeftTop = bp.m_pastingMatrix.TransformPoint(mPoint);
				previousTextingPoint = mpTextingLeftTop;

				//--------------------------------------------------------------------------------------------------------------------------
				// 如果鼠标按到了编辑区域，则初始化 m_textPoint 编辑光标位置和 m_textMarkLeft 和 m_textMarkRight 高亮变量
				//--------------------------------------------------------------------------------------------------------------------------
				if (bp.m_tbIsActiveEdit)
				{
					startTextingPoint = mpTexting;
					for (int i = 0; i < (int)bp.m_textLayout.size(); i++)
					{
						//--------------------------------------------------------------------------------------------------------------------------
						// 如果鼠标点在某个字符上
						//--------------------------------------------------------------------------------------------------------------------------
						if (mpTexting.x >= bp.m_textLayout.at(i)->m_charLayoutRect.left &&
							mpTexting.x <= bp.m_textLayout.at(i)->m_charLayoutRect.right + bp.m_textCharSpace &&
							mpTexting.y >= bp.m_textLayout.at(i)->m_charLayoutRect.top &&
							mpTexting.y <= bp.m_textLayout.at(i)->m_charLayoutRect.bottom + bp.m_textLineSpace)
						{
							TEXT_TrackFirstWord(bp, i);
							break;
						}

						if (bp.m_textStyle != style_b)
						{
							if (i != bp.m_textLayout.size() - 1)
							{
								if (mpTexting.x >= bp.m_textLayout.at(i)->m_charLayoutRect.left &&
									mpTexting.x <= bp.m_rotatableRelativePt.x + bp.m_rotatableBorderWidth &&
									mpTexting.y >= bp.m_textLayout.at(i)->m_charLayoutRect.top &&
									mpTexting.y < bp.m_textLayout.at(i + 1)->m_charLayoutRect.top)
								{
									TEXT_TrackFirstWord(bp, i);
									break;
								}
							}
							else
							{
								if (mpTexting.x >= bp.m_textLayout.at(i)->m_charLayoutRect.left &&
									mpTexting.x <= bp.m_rotatableRelativePt.x + bp.m_rotatableBorderWidth &&
									mpTexting.y >= bp.m_textLayout.at(i)->m_charLayoutRect.top &&
									mpTexting.y <= bp.m_textLayout.at(i)->m_charLayoutRect.bottom + bp.m_textLineSpace)
								{
									TEXT_TrackFirstWord(bp, i+1);
									break;
								}
							}
						}
					}
				}
			}
			//--------------------------------------------------------------------------------------------------------------------------
			// bp.m_tbIsActiveLeftTop || bp.m_tbIsActiveEdit || bp.m_tbIsActiveRight || bp.m_tblIsActiveLeft || bp.m_tbIsActiveBottom
			// 任何一个标志被置为 On，则触发 m_isTexting
			//--------------------------------------------------------------------------------------------------------------------------
			m_isTexting = true;
		}

		textOperation->SetTextEquipment(bp);
	}
}

void ImageEditorHandler::TEXT_CreateNewText(D2D1_POINT_2F mousePosition)
{
	TEXT_CreateTextItem(mousePosition);
	TEXT_SetCursor(true, false, false, false, false);
	m_isTexting = true;

	CombDlg::COMB_SetItemActive(m_currentOperation, true);
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
bool ImageEditorHandler::TEXT_IsTextHit(D2D1_POINT_2F mousePosition)
{
	ComPtr<IImage> image = m_images[m_currentIndex].Image;
	for (int i = image->GetOperationsSize() - 1; i >= 0; i--)
	{
		ComPtr<IImageOperation> thisOperation = image->Get_ith_Operation(i);

		ComPtr<IImageTextOperation> textOperation;
		if (SUCCEEDED(thisOperation.QueryInterface(&textOperation)))
		{
			CBTextPtr bp;
			textOperation->GetTextEquipment(&bp);

			D2D1_POINT_2F ptBase, mpTexting;
			//--------------------------------------------------------------------------------------------------------------------------
			// mpTexting 非翻转变换不可
			//--------------------------------------------------------------------------------------------------------------------------
			if (ROTA_GetRelativeAndMousePt(textOperation, bp, mousePosition, &ptBase, &mpTexting))
			{
				if ((mpTexting.x >= ptBase.x && mpTexting.x <= ptBase.x + bp.m_rotatableBorderWidth &&
					mpTexting.y >= ptBase.y && mpTexting.y <= ptBase.y + bp.m_rotatableBorderHeight) ||
					(mpTexting.x >= ptBase.x - DISTANCE && mpTexting.x <= ptBase.x &&
					mpTexting.y >= ptBase.y - DISTANCE && mpTexting.y <= ptBase.y))
				{
					m_currentOperation = thisOperation;
					CombDlg::COMB_SetItemActive(m_currentOperation, true);
					
					ROTA_ShowPosition(bp);
					return true;
				}
			}
		}
	}

	return false;
}

bool ImageEditorHandler::TEXT_IsActive(ComPtr<IImageOperation> operation)
{
	if (operation == nullptr)
		return false;

	return operation->GetActive();
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::TEXT_HitTextBorderTest(ComPtr<IImageTextOperation> textOperation,
																			D2D1_POINT_2F mousePosition, 
																			bool&lefttop, 
																			bool&edit, 
																			bool&right, 
																			bool&left, 
																			bool&bottom)
{
	lefttop = edit = right = left = bottom = false;

	if (textOperation == nullptr)
		return;

	CBTextPtr bp;
	textOperation->GetTextEquipment(&bp);

	D2D1_POINT_2F ptBase, mpTexting;
	if (ROTA_GetRelativeAndMousePt(textOperation, bp, mousePosition, &ptBase, &mpTexting) == false)
		return;

	lefttop = (mpTexting.x >= ptBase.x - DISTANCE)
			&& (mpTexting.x < ptBase.x)
			&& (mpTexting.y >= ptBase.y - DISTANCE)
			&& (mpTexting.y < ptBase.y);

	right = (mpTexting.x <= (ptBase.x + bp.m_rotatableBorderWidth))
		&& (abs(mpTexting.x - (ptBase.x + bp.m_rotatableBorderWidth)) <= DISTANCE)
		&& (mpTexting.y >= ptBase.y)
		&& (mpTexting.y <= ptBase.y + bp.m_rotatableBorderHeight);

	bottom = (mpTexting.y <= (ptBase.y + bp.m_rotatableBorderHeight))
		&& (abs(mpTexting.y - (ptBase.y + bp.m_rotatableBorderHeight)) <= DISTANCE)
		&& (mpTexting.x >= ptBase.x)
		&& (mpTexting.x <= ptBase.x + bp.m_rotatableBorderWidth);

	left = (mpTexting.x >= ptBase.x)
		&& (abs(mpTexting.x - ptBase.x) <= DISTANCE)
		&& (mpTexting.y >= ptBase.y)
		&& (mpTexting.y <= ptBase.y + bp.m_rotatableBorderHeight);

	if (right == false && bottom == false && left == false)
	{
		if (mpTexting.x >= ptBase.x && mpTexting.x <= ptBase.x + bp.m_rotatableBorderWidth
			&& mpTexting.y >= ptBase.y && mpTexting.y <= ptBase.y + bp.m_rotatableBorderHeight)
		{
			edit = true;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::TEXT_AdjustingCenterOrBorder(D2D1_POINT_2F mousePosition)
{
	ComPtr<IImageTextOperation> textOperation;
	if (m_currentOperation != nullptr && SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
	{
		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);

		D2D1_POINT_2F mPoint = GetAbsolutePosition(mousePosition);
		D2D1_POINT_2F mpTexting = bp.m_pastingMatrix.TransformPoint(mPoint);

		if (bp.m_tbIsActiveRight)
		{
			bp.m_rotatableBorderWidth += mpTexting.x - previousTextingPoint.x;
			if (bp.m_rotatableBorderWidth < 100)
				bp.m_rotatableBorderWidth = 100;
		}
		
		if (bp.m_tblIsActiveLeft)
		{
			D2D1_POINT_2F pt = D2D1::Point2F(mpTexting.x - previousTextingPoint.x, 0);
			D2D1::Matrix3x2F matrix = m_images[m_currentIndex].Image->ROTA_GetNormalTransformations(bp, D2D1::Point2F(0, 0));

			D2D1_POINT_2F rel = matrix.TransformPoint(pt);
			bp.m_rotatableRelativePt.x += rel.x;
			bp.m_rotatableBorderWidth += -pt.x;
			bp.m_rotatableRelativePt.y += rel.y;

			if (bp.m_rotatableBorderWidth < 100)
				bp.m_rotatableBorderWidth = 100;
		}
		
		if (bp.m_tbIsActiveBottom)
		{
			bp.m_rotatableBorderHeight += mpTexting.y - previousTextingPoint.y;
			if (bp.m_rotatableBorderHeight < 100)
				bp.m_rotatableBorderHeight = 100;
		}

		if (bp.m_tbIsActiveRight || bp.m_tblIsActiveLeft || bp.m_tbIsActiveBottom)
		{
			ImageTextOperation::TEXT_FrameOutText(bp);
		}
		else if (bp.m_tbIsActiveLeftTop)
		{
			bp.m_rotatableRelativePt.x += mpTexting.x - previousTextingPoint.x;
			bp.m_rotatableRelativePt.y += mpTexting.y - previousTextingPoint.y;
		}

		previousTextingPoint = mpTexting;

		ROTA_ShowPosition(bp);

		textOperation->SetTextEquipment(bp);
		TEXT_SetCursor(bp.m_tbIsActiveLeftTop, 
								bp.m_tbIsActiveEdit, 
								bp.m_tbIsActiveRight, 
								bp.m_tblIsActiveLeft, 
								bp.m_tbIsActiveBottom);
	}
}

void ImageEditorHandler::TEXT_CreateTextItem(D2D1_POINT_2F mousePosition)
{
	CBTextPtr bp;
	bp.m_textStyle = g_textStyle;
	bp.m_textString = FSTRING;
	bp.m_textFormat = m_textFormat;
	
	SecureHelper::strcpy_x(bp.m_textFormatFamilyName, LF_FACESIZE, g_textFormatFamilyName);
	bp.m_textFormatFontSize = g_textFormatFontSize;

	bp.m_textBrush = m_solidBrush;
	bp.m_rotatableBorderWidth =	(m_charSize + m_charSpace) * 5;
	bp.m_rotatableBorderHeight = (m_charSize + m_lineSpace) * 8;
	bp.m_textCursorBottom = D2D1::Point2F(0, 0);
	bp.m_textCursorTop = D2D1::Point2F(0, 0);
	bp.m_textIsActive = true;
	bp.m_textPoint = 0;
	bp.m_textIfMarked = false;
	bp.m_textMarkLeft = bp.m_textMarkRight = 0;
	bp.m_tbIsActiveLeftTop = true;
	bp.m_tbIsActiveEdit = false;
	bp.m_tbIsActiveRight = false;
	bp.m_tblIsActiveLeft = false;
	bp.m_tbIsActiveBottom = false;
	bp.m_textCharSpace = m_charSpace;
	bp.m_textLineSpace = m_lineSpace;
	bp.m_textColor = m_textColor;

	ImageTextOperation::TEXT_FrameOutText(bp);

	ComPtr<IImageOperation> text;
	SharedObject<ImageTextOperation>::Create(bp, &text);

	CombDlg::COMB_SetItemActive(m_currentOperation, false);
	
	m_currentOperation = text;

	m_images[m_currentIndex].Image->PushImageOperation(text);

	ComPtr<IImageTextOperation> textOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
	{
		D2D1_POINT_2F absPoint = GetAbsolutePosition(mousePosition);

		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);

		PAST_CalculatePastingMatrix(bp, m_images[m_currentIndex].Image);
		bp.m_rotatableRelativePt = bp.m_pastingMatrix.TransformPoint(absPoint);
		bp.m_rotatableRelativePt.x += DISTANCE;
		bp.m_rotatableRelativePt.y += DISTANCE;

		textOperation->SetTextEquipment(bp);

		previousTextingPoint = bp.m_pastingMatrix.TransformPoint(absPoint);

		ROTA_ShowPosition(bp);
	}
	m_currentOperation->SetDuration(g_animDuration);
	CombDlg::COMB_InitList();
}

void ImageEditorHandler::TEXT_SetCursor(bool center, bool edit, bool right, bool left, bool bottom)
{
	if (right && bottom)
	{
		SetCursor(g_cursorBottomRight);
	}
	else if (edit)
	{
		SetCursor(g_cursorEdit);
	}
	else if (right)
	{
		SetCursor(g_cursorRight);
	}
	else if (left)
	{
		SetCursor(g_cursorLeft);
	}
	else if (bottom)
	{
		SetCursor(g_cursorBottom);
	}
	else if (center)
	{
		SetCursor(g_cursorCenter);
	}
	else
	{
		SetCursor(g_cursorNormal);
	}
}

void ImageEditorHandler::TEXT_SetCharSpace(LONG val)
{
	m_charSpace = (FLOAT)val;
	
	if (m_currentOperation == nullptr)
		return;

	ComPtr<IImageTextOperation> textOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
	{
		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);
		bp.m_textCharSpace = m_charSpace;
		ImageTextOperation::TEXT_FrameOutText(bp);
		textOperation->SetTextEquipment(bp);
	}
}

void ImageEditorHandler::TEXT_SetLineSpace(LONG val)
{
	m_lineSpace = (FLOAT)val;

	if (m_currentOperation == nullptr)
		return;

	ComPtr<IImageTextOperation> textOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
	{
		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);
		bp.m_textLineSpace = m_lineSpace;
		ImageTextOperation::TEXT_FrameOutText(bp);
		textOperation->SetTextEquipment(bp);
	}
}

void ImageEditorHandler::TEXT_SetCharSize(LONG val)
{
	m_charSize = (FLOAT)val;

	m_textFormat = nullptr;
	WCHAR * wsFaceName;
	DWRITE_FONT_WEIGHT weight;
	CString faceName = g_textFormatFamilyName;
	if (faceName.Compare(L"Microsoft JhengHei UI") == 0)
	{
		wsFaceName = L"Microsoft JhengHei UI";
		weight = DWRITE_FONT_WEIGHT_LIGHT;
	}
	else
	{
		wsFaceName = g_textFormatFamilyName;
		weight = DWRITE_FONT_WEIGHT_REGULAR;
	}
	g_pDWriteFactory->CreateTextFormat(	wsFaceName,
																nullptr,
																weight,
																DWRITE_FONT_STYLE_NORMAL,
																DWRITE_FONT_STRETCH_NORMAL,
																m_charSize,
																L"en-us",
																&m_textFormat);

	SecureHelper::strcpy_x(g_textFormatFamilyName, LF_FACESIZE, wsFaceName);
	g_textFormatFontSize = (FLOAT)m_charSize;

	if (m_currentOperation == nullptr)
		return;

	ComPtr<IImageTextOperation> textOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
	{
		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);
		bp.m_textFormat = m_textFormat;

		SecureHelper::strcpy_x(bp.m_textFormatFamilyName, LF_FACESIZE, g_textFormatFamilyName);
		bp.m_textFormatFontSize = g_textFormatFontSize;

		ImageTextOperation::TEXT_FrameOutText(bp);
		textOperation->SetTextEquipment(bp);
	}
}

void ImageEditorHandler::PLYR_StartTracking(D2D1_POINT_2F mousePosition)
{
	ComPtr<IImageLayerOperation> layerOperation;
	if (m_currentOperation != nullptr && SUCCEEDED(m_currentOperation.QueryInterface(&layerOperation)))
	{
		CBLayerPtr bp;
		layerOperation->GetLayerEquipment(&bp);

		bp.m_layerBorderIsActive = PLYR_HitLayerBorderTest(layerOperation, mousePosition);
		if (bp.m_layerBorderIsActive)
		{
			D2D1_POINT_2F mPoint = GetAbsolutePosition(mousePosition);
			D2D1_POINT_2F mpLayering = bp.m_pastingMatrix.TransformPoint(mPoint);
			previousLayeringPoint = mpLayering;

			m_isLayering = true;

			ROTA_ShowPosition(bp);
		}
		layerOperation->SetLayerEquipment(bp);
	}
}

bool ImageEditorHandler::PLYR_IsLayerHit(D2D1_POINT_2F mousePosition)
{
	ComPtr<IImage> image = m_images[m_currentIndex].Image;
	for (int i = image->GetOperationsSize() - 1; i >= 0; i--)
	{
		ComPtr<IImageOperation> thisOperation = image->Get_ith_Operation(i);

		ComPtr<IImageLayerOperation> layerOperation;
		if (SUCCEEDED(thisOperation.QueryInterface(&layerOperation)))
		{
			if (PLYR_HitLayerBorderTest(layerOperation, mousePosition))
			{
				CBLayerPtr bp;
				layerOperation->GetLayerEquipment(&bp);
				ROTA_ShowPosition(bp);

				m_currentOperation = thisOperation;

				//CombDlg::COMB_InitList();
				CombDlg::COMB_ActivateItem(thisOperation);
				return true;
			}
		}
	}

	return false;
}

bool ImageEditorHandler::PLYR_IsActive(ComPtr<IImageOperation> operation)
{
	if (operation == nullptr)
		return false;

	ComPtr<IImageLayerOperation> layerOperation;
	if (SUCCEEDED(operation.QueryInterface(&layerOperation)))
	{
		CBLayerPtr bp;
		layerOperation->GetLayerEquipment(&bp);
		return bp.m_layerIsActive;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------------
// ROTA_GetRelativeAndMousePt 下两个函数定义类似，不同之处在于
// ComPtr<IImageTextOperation> 和 ComPtr<IImageLayerOperation> 的参数
//--------------------------------------------------------------------------------------------------------------------------
bool ImageEditorHandler::ROTA_GetRelativeAndMousePt(ComPtr<IImageTextOperation> textOperation,
																						CBRotatablePtr& bp, 
																						D2D1_POINT_2F mousePosition, 
																						D2D1_POINT_2F * relative, 
																						D2D1_POINT_2F * mouse)
{
	ComPtr<IImage> image = m_images[m_currentIndex].Image;
	
	int success = image->GetIndexOfText(textOperation);
	if (success == -1)
		return false;

	D2D1_POINT_2F relMidPoint = bp.m_rotatableRelativePt;
	//D2D1_POINT_2F relMidPoint = bp.m_rotatableTextMidPoint;

	D2D1::Matrix3x2F localMatrix = image->GetRotatableTransformation(success, bp, relMidPoint);
	D2D1_POINT_2F mPoint = GetAbsolutePosition(mousePosition);
	*mouse = localMatrix.TransformPoint(mPoint);

	*relative = bp.m_rotatableRelativePt;
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
bool ImageEditorHandler::ROTA_GetRelativeAndMousePt(ComPtr<IImageLayerOperation> layerOperation,
																						CBRotatablePtr& bp, 
																						D2D1_POINT_2F mousePosition, 
																						D2D1_POINT_2F * relative, 
																						D2D1_POINT_2F * mouse)
{
	ComPtr<IImage> image = m_images[m_currentIndex].Image;

	int success = image->GetIndexOfLayer(layerOperation);
	if (success == -1)
		return false;

	D2D1_POINT_2F relMidPoint = D2D1::Point2F(bp.m_rotatableRelativePt.x + bp.m_rotatableBorderWidth / 2,
																		bp.m_rotatableRelativePt.y + bp.m_rotatableBorderHeight / 2);

	D2D1::Matrix3x2F localMatrix = image->GetRotatableTransformation(success, bp, relMidPoint);
	D2D1_POINT_2F mPoint = GetAbsolutePosition(mousePosition);
	*mouse = localMatrix.TransformPoint(mPoint);

	*relative = bp.m_rotatableRelativePt;
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------
// 测试鼠标是否在经过翻转变换后的 Layer 图片上空
//--------------------------------------------------------------------------------------------------------------------------
bool ImageEditorHandler::PLYR_HitLayerBorderTest(ComPtr<IImageLayerOperation> layerOperation, D2D1_POINT_2F mousePosition)
{
	if (layerOperation == nullptr)
		return false;

	CBLayerPtr bp;
	layerOperation->GetLayerEquipment(&bp);

	D2D1_POINT_2F ptBase, mpLayering;
	if (ROTA_GetRelativeAndMousePt(layerOperation, bp, mousePosition, &ptBase, &mpLayering) == false)
		return false;

	return ((mpLayering.x >= ptBase.x)
		&& (mpLayering.x <= ptBase.x + bp.m_rotatableBorderWidth)
		&& (mpLayering.y >= ptBase.y)
		&& (mpLayering.y <= ptBase.y + bp.m_rotatableBorderHeight));
}

//--------------------------------------------------------------------------------------------------------------------------
// m_pastingMatrix 用来做从 absolute point 到（当前 text 或 layer 生成时的）相对值的转换。
// m_pastingReverseMatrix 用来做从相对值到 absolute point 的转换。
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::PAST_CalculatePastingMatrix(CBPastingPtr& bp, ComPtr<IImage> image)
{
	image->GetPastingTransformation(&(bp.m_pastingMatrix));
	image->GetPastingReverseTransformation(&(bp.m_pastingReverseMatrix));
}

void ImageEditorHandler::PLYR_SetCursor(bool center)
{
	if (center)
	{
		SetCursor(g_cursorCenter);
	}
	else
	{
		SetCursor(g_cursorNormal);
	}
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
ComPtr<IWICBitmapSource> ImageEditorHandler::makeAWindow(ComPtr<IWICBitmapSource> source)
{
	ComPtr<IWICImagingFactory> factory;
	ComPtr<IWICBitmap> pBitmap;
	ComPtr<IWICBitmapLock> pILock;
	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		hr = g_pWICFactory->CreateBitmapFromSource(source, WICBitmapCacheOnDemand, &pBitmap);
	}

	WICRect rcLock = { 50, 50, 50, 50 };  // X, Y, Width, Height
	UINT cbBufferSize = 0;
	BYTE * pv = NULL;
	UINT stride = 0;
	UINT width, height;

	source->GetSize(&width, &height);
	rcLock = { 0, 0, (INT)width, (INT)height };
	if (SUCCEEDED(hr))
	{
		hr = pBitmap->Lock(&rcLock, WICBitmapLockRead | WICBitmapLockWrite, &pILock);
	}

	if (SUCCEEDED(hr))
	{
		hr = pILock->GetDataPointer(&cbBufferSize, &pv);
	}

	if (SUCCEEDED(hr))
	{
		hr = pILock->GetStride(&stride);
	}

	if (SUCCEEDED(hr))
	{
		for (UINT i = 0; i < height; i++)		// height
		{
			for (UINT j = 0; j < width; j++)	// width
			{
				{
					pv[j * 4] = 0;				// blue
					pv[j * 4 + 1] = 0;		// green
					pv[j * 4 + 2] = 0;		// red
					pv[j * 4 + 3] = 0;		// alpha
				}
			}
			pv += stride;
		}
	}

	pILock = nullptr;

	return static_cast<ComPtr<IWICBitmapSource>>(pBitmap);
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
void	ImageEditorHandler::ROTA_ShowPosition(CBRotatablePtr&bp)
{
	m_currentLeft = bp.m_rotatableRelativePt.x;
	m_currentTop = bp.m_rotatableRelativePt.y;
	m_currentWidth = bp.m_rotatableBorderWidth;
	m_currentHeight = bp.m_rotatableBorderHeight;
	::PostMessage(GetParent().m_hWnd, WM_UPDATEROWCOL, 1, 0);
}

LRESULT ImageEditorHandler::AnnotStopLayer(WORD, WORD wID, HWND, BOOL&)
{
	PAST_Deactivate();
	m_currentDrawingOperationType = ImageOperationTypeNone;
	m_isLayering = false;
	
	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotHome(WORD, WORD wID, HWND, BOOL&)
{
	//--------------------------------------------------------------------------------------------------------------------------
	g_bAboutDialogShow = !g_bAboutDialogShow;

	if (g_bAboutDialogShow)
		g_aboutDlg->ShowWindow(SW_SHOWNORMAL);
	else
		g_aboutDlg->ShowWindow(SW_HIDE);
	return 0;
}

LRESULT ImageEditorHandler::AnnotCombDlg(WORD, WORD wID, HWND, BOOL&)
{
	//--------------------------------------------------------------------------------------------------------------------------
	g_bCombDialogShow = !g_bCombDialogShow;

	if (g_bCombDialogShow)
		g_combDlg->ShowWindow(SW_SHOWNORMAL);
	else
		g_combDlg->ShowWindow(SW_HIDE);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::CLBD_CopyToClipboard(D2D1_POINT_2F copyFinalPoint, FLOAT copyBorderWidth, FLOAT copyBorderHeight)
{
	FLOAT width = abs(copyBorderWidth);
	FLOAT height = abs(copyBorderHeight);

	ComPtr<IWICBitmapSource> wpic = m_images[m_currentIndex].Image->GetWICBitmap();

	ComPtr<IWICImagingFactory> wicFactory;
	ComPtr<IWICBitmapClipper> wicBitmapClipper;
	ComPtr<IWICBitmapFlipRotator> wicBitmapFRer;

	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		hr = g_pWICFactory->CreateBitmapClipper(&wicBitmapClipper);
	}

	if (SUCCEEDED(hr))
	{
		hr = g_pWICFactory->CreateBitmapFlipRotator(&wicBitmapFRer);
	}

	WICRect rect = { (int)copyFinalPoint.x, (int)copyFinalPoint.y, (int)width, (int)height };
	if (SUCCEEDED(hr))
	{
		hr = wicBitmapClipper->Initialize(wpic, &rect);
	}

	if (SUCCEEDED(hr))
	{
		hr = wicBitmapFRer->Initialize(wicBitmapClipper, WICBitmapTransformFlipVertical);
	}

	HBITMAP hbitmap = Direct2DUtility::CreateHBITMAP(wicBitmapFRer);

	if (::OpenClipboard(m_hWnd))
	{
		DIBSECTION ds;
		::GetObject(hbitmap, sizeof(DIBSECTION), &ds);

		//--------------------------------------------------------------------------------------------------------------------------
		//make sure compression is BI_RGB
		//--------------------------------------------------------------------------------------------------------------------------
		ds.dsBmih.biCompression = BI_RGB;

		//--------------------------------------------------------------------------------------------------------------------------
		//Convert DIB to DDB
		//--------------------------------------------------------------------------------------------------------------------------
		HDC hdc = ::GetDC(m_hWnd);
		HBITMAP hbitmap_ddb = ::CreateDIBitmap(
			hdc, &ds.dsBmih, CBM_INIT, ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS);
		::ReleaseDC(NULL, hdc);

		::EmptyClipboard();
		::SetClipboardData(CF_BITMAP, hbitmap_ddb);
		::CloseClipboard();
	}

	wpic = nullptr;
	wicBitmapClipper = nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::CLBD_CopyToClipboard(CString m_strResult)
{
	int nLen = m_strResult.GetLength();
	HANDLE hText = ::GlobalAlloc(GMEM_MOVEABLE, (nLen + 1) * sizeof(WCHAR));

	if (hText == NULL)
		return;

	LPWSTR lpText = (LPWSTR)::GlobalLock(hText);
	if (NULL == lpText)
	{
		::CloseHandle(hText);
		return;
	}

	wcsncpy_s(lpText, nLen + 1, m_strResult.GetBuffer(nLen + 1), nLen + 1);
	lpText[nLen] = '\x00';
	::GlobalUnlock(hText);

	if (!OpenClipboard())
		return;

	if (EmptyClipboard())
	{
		::SetClipboardData(CF_UNICODETEXT, hText);
	}

	CloseClipboard();
}

bool	ImageEditorHandler::CLBD_GetWicInfoFromClipboard()
{
	m_pasteWicBitmapFromClipboard = CLBD_GetWicBitmapFromClipboard();
	return (m_pasteWicBitmapFromClipboard != nullptr);
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
ComPtr<IWICBitmapSource> ImageEditorHandler::CLBD_GetWicBitmapFromClipboard()
{
	if (!IsClipboardFormatAvailable(CF_BITMAP))
		return nullptr;

	if (!::OpenClipboard(m_hWnd))
		return nullptr;

	ComPtr<IWICBitmap> pWICBitmap;
	HRESULT hr = g_pWICFactory->CreateBitmapFromHBITMAP((HBITMAP)GetClipboardData(CF_BITMAP),
		NULL,
		WICBitmapIgnoreAlpha,
		&pWICBitmap
		);
	::CloseClipboard();
	if (!SUCCEEDED(hr))
		return nullptr;

	return static_cast<ComPtr<IWICBitmapSource>>(pWICBitmap);
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
WCHAR * ImageEditorHandler::CLBD_GetTextFromClipboard()
{
	WCHAR * strClip = nullptr;

	//判断剪贴板的数据格式是否可以处理。  
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
		return nullptr;

	//打开剪贴板。          
	if (!::OpenClipboard(m_hWnd))
		return nullptr;

	//获取数据  
	HANDLE hMem = GetClipboardData(CF_UNICODETEXT);
	if (hMem != NULL)
	{
		//获取字符串。  
		strClip = (LPWSTR)GlobalLock(hMem);
		if (strClip != nullptr)
		{
			//释放锁内存  
			GlobalUnlock(hMem);
		}
	}
	//关闭剪贴板        
	CloseClipboard();
	return strClip;
}

void ImageEditorHandler::PAST_SelectLayer(UINT index)
{
	PAST_UnselectLayer();

	ComPtr<IImage> image = m_images[m_currentIndex].Image;
	ComPtr<IImageOperation> operation = image->Get_ith_Operation(index);
	m_currentOperation = operation;

	operation->SetActive(true);
}

void ImageEditorHandler::PAST_SelectLayer(ComPtr<IImageOperation> thisOper)
{
	ComPtr<IImage> image = m_images[m_currentIndex].Image;
	for (int i = 0; i < image->GetOperationsSize(); i++)
	{
		ComPtr<IImageOperation> operation = image->Get_ith_Operation(i);
		if (operation == thisOper)
		{
			PAST_SelectLayer(i);
			return;
		}
	}
}

void ImageEditorHandler::PAST_UnselectLayer()
{
	ComPtr<IImage> image = m_images[m_currentIndex].Image;
	for (int i = 0; i < image->GetOperationsSize(); i++)
	{
		ComPtr<IImageOperation> operation = image->Get_ith_Operation(i);
		operation->SetActive(false);
	}
}

void ImageEditorHandler::PAST_Deactivate()
{
	m_isLayering = false;
	m_isTexting = false;
	PAST_UnselectLayer();
	m_currentOperation = nullptr;
}

void ImageEditorHandler::PAST_SetPastOpacity(LONG lVal)
{
	if (m_currentOperation != nullptr)
	{
		m_currentOperation->SetOpacity(lVal / 100.0f);
		InvalidateWindow();
	}
}

void ImageEditorHandler::ANIM_SetDurationTime(LONG IVal)
{
	g_animDuration = (FLOAT)IVal;
	if (m_currentOperation != nullptr)
	{
		m_currentOperation->SetDuration(g_animDuration);
	}
}

extern int g_resize_width;
extern int g_resize_height;

void ImageEditorHandler::AnnotInnerNew(COLORREF bgColor)
{
	Reset();
	
	ImageInfo info(sourceFromNew);
	info.bkgColor = D2D1::ColorF(static_cast<float>(GetRValue(bgColor)) / 255.0f,
												static_cast<float>(GetGValue(bgColor)) / 255.0f,
												static_cast<float>(GetBValue(bgColor)) / 255.0f);
	info.bkgSize = D2D1::SizeF(g_resize_width, g_resize_height);

	ImageItem newItem;
	SharedObject<SimpleImage>::Create(info, &newItem.Image);

	m_images.push_back(newItem);

	int rangeStart = m_currentIndex - PreviousNextImageRangeCount;
	int rangeEnd = m_currentIndex + PreviousNextImageRangeCount;

	rangeStart = max(0, rangeStart);
	rangeEnd = min(static_cast<int>(m_images.size()) - 1, rangeEnd);

	for (int i = m_currentRangeStart; i < m_currentRangeEnd + 1; i++)
	{
		if (i >= 0 && (i < rangeStart || i > rangeEnd))
		{
			m_images[i].Image->DiscardResources();
		}
	}

	for (int i = rangeStart; i < rangeEnd + 1; i++)
	{
		m_images[i].Image->SetRenderingParameters(m_renderingParameters);
		m_images[i].Image->New();
		m_images[i].Image->SetBoundingRect(m_imageBoundaryRect);
	}

	m_currentRangeStart = rangeStart;
	m_currentRangeEnd = rangeEnd;
	
	CalculateImagePositions();

	m_currentOperation = nullptr;
	InvalidateWindow();
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotNew(WORD, WORD wID, HWND, BOOL&)
{
	CResizeDlg resizeDlg;
	resizeDlg.setInitialSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	resizeDlg.bp.dialogName			= L"新建图像";
	resizeDlg.bp.titleName				= L"新建图像尺寸";
	resizeDlg.bp.title1PicFileName	= IDR_MYPNG_NEW_TITLE;
	resizeDlg.bp.ifHasTitle2Pic			= true;
	resizeDlg.bp.ifTitle2PicFromFile = true;
	resizeDlg.bp.title2PicFileName	= IDR_MYPNG_RESIZE_PIC;

	if (resizeDlg.DoModal() != IDOK) return 0;

	static DWORD custColors[16] = { 0x105e06, 0x66380f, 0x5c5c5c, 0x66c10f, 0xd77800 };
	CHOOSECOLOR stChooseColor;

	COLORREF rgbLineColor(0x777777);
	stChooseColor.lStructSize = sizeof(CHOOSECOLOR);
	stChooseColor.hwndOwner = m_hWnd;
	stChooseColor.rgbResult = rgbLineColor;
	stChooseColor.lpCustColors = custColors;;
	stChooseColor.Flags = CC_RGBINIT;
	stChooseColor.lCustData = 0;
	stChooseColor.lpfnHook = NULL;
	stChooseColor.lpTemplateName = NULL;

	if (::ChooseColor(&stChooseColor) == FALSE)
	{
		AnnotInnerNew(g_bkColor);
	}
	else
	{
		AnnotInnerNew(stChooseColor.rgbResult);
	}

	CombDlg::COMB_InitList();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotNewFromClipboard(WORD, WORD wID, HWND, BOOL&)
{
	ComPtr<IWICBitmapSource> clip = CLBD_GetWicBitmapFromClipboard();
	if (clip == nullptr)		return 0;

	HRESULT hr = Reset();
	if (SUCCEEDED(hr))
	{
		ImageInfo info(sourceFromClipboard);

		UINT width, height;
		clip->GetSize(&width, &height);
		info.bkgSize = D2D1::SizeF(width, height);

		info.wicSourceClipBitmap = clip;

		ImageItem newItem;
		SharedObject<SimpleImage>::Create(info, &newItem.Image);

		//--------------------------------------------------------------------------------------------------------------------------
		// For now do not create animation objects. These are managed when animation begins/ends
		//--------------------------------------------------------------------------------------------------------------------------
		m_images.push_back(newItem);
	}

	if (SUCCEEDED(hr))
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Calculate new range
		//--------------------------------------------------------------------------------------------------------------------------
		int rangeStart = m_currentIndex - PreviousNextImageRangeCount;
		int rangeEnd = m_currentIndex + PreviousNextImageRangeCount;

		//--------------------------------------------------------------------------------------------------------------------------
		// Update range based on valid values
		//--------------------------------------------------------------------------------------------------------------------------
		rangeStart = max(0, rangeStart);
		rangeEnd = min(static_cast<int>(m_images.size()) - 1, rangeEnd);

		//--------------------------------------------------------------------------------------------------------------------------
		// Discard resources for any images no longer in range
		//--------------------------------------------------------------------------------------------------------------------------
		for (int i = m_currentRangeStart; i < m_currentRangeEnd + 1; i++)
		{
			if (i >= 0 && (i < rangeStart || i > rangeEnd))
			{
				m_images[i].Image->DiscardResources();
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// Load resources as necessary
		//--------------------------------------------------------------------------------------------------------------------------
		for (int i = rangeStart; i < rangeEnd + 1; i++)
		{
			m_images[i].Image->SetRenderingParameters(m_renderingParameters);
			m_images[i].Image->NewFromClipboard();
			m_images[i].Image->SetBoundingRect(m_imageBoundaryRect);
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// Update current range values
		//--------------------------------------------------------------------------------------------------------------------------
		m_currentRangeStart = rangeStart;
		m_currentRangeEnd = rangeEnd;
	}

	if (SUCCEEDED(hr))
	{
		hr = CalculateImagePositions();
	}

	m_currentOperation = nullptr;
	InvalidateWindow();

	return hr;
}

void ImageEditorHandler::PLYR_CreateLayerItem(D2D1_POINT_2F mousePosition, bool ifLocateToCenter)
{
	CBLayerPtr bp;
	ComPtr<IImageOperation> layer;
	SharedObject<ImageLayerOperation>::Create(bp, &layer);
	m_currentOperation = layer;

	m_images[m_currentIndex].Image->PushImageOperation(layer);

	ComPtr<IImageLayerOperation> layerOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&layerOperation)))
	{
		CBLayerPtr bp;
		layerOperation->GetLayerEquipment(&bp);

		bp.m_layerFrameBrush = m_solidBrush;

		bp.m_layerIfPasting = m_ifLayerFromPasting;
		if (m_ifLayerFromPasting == true)
		{
			bp.m_layerWicBitmap = m_pasteWicBitmapFromClipboard;
		}
		else
		{
			//--------------------------------------------------------------------------------------------------------------------------
			// bp.m_layerPicFileName 仅仅是为了在 LayerCombo 中显示其文件名称
			//--------------------------------------------------------------------------------------------------------------------------
			bp.m_layerPicFileName = m_layerPicFileName;
			bp.m_layerWicBitmap = g_render->LoadWicBitmapFromFile(m_layerPicFileName, 0, 0);
		}

		//bp.m_layerWicBitmap = makeAWindow(bp.m_layerWicBitmap);
		
		g_pD2DDeviceContext->CreateBitmapFromWicBitmap(bp.m_layerWicBitmap, &bp.m_layerBitmap);

		bp.m_rotatableBorderWidth = bp.m_layerBitmap->GetSize().width;
		bp.m_rotatableBorderHeight = bp.m_layerBitmap->GetSize().height;

		PAST_CalculatePastingMatrix(bp, m_images[m_currentIndex].Image);

		D2D1_POINT_2F absPoint;
		if (ifLocateToCenter == false)
		{
			absPoint = GetAbsolutePosition(mousePosition);
		}
		else
		{
			CRect rect;
			GetClientRect(&rect);
			float scale;
			m_images[m_currentIndex].Image->GetScale(&scale);

			//--------------------------------------------------------------------------------------------------------------------------
			// mousePosition 是模拟鼠标在 ClientRect 中的中心位置
			//--------------------------------------------------------------------------------------------------------------------------
			mousePosition.x = rect.right / 2.0 - bp.m_rotatableBorderWidth / scale / 2.0;
			mousePosition.y = rect.bottom / 2.0 - bp.m_rotatableBorderHeight / scale / 2.0;
			absPoint = GetAbsolutePosition(mousePosition, false);
		}

		bp.m_rotatableRelativePt = bp.m_pastingMatrix.TransformPoint(absPoint);
		previousLayeringPoint = bp.m_rotatableRelativePt;

		layerOperation->SetLayerEquipment(bp);
	}
	m_currentOperation->SetDuration(g_animDuration);
	CombDlg::COMB_InitList();
}

//--------------------------------------------------------------------------------------------------------------------------
// Translate any given point (mostly mouse clicks) to an absolute position within
// the currently active image
// absolute pos --> translation(-m_clipRect.left) --> scale(1/scale） --> translation(drawingRect.left) --> normal Oper(0) --> ... --> normal Oper(n) --> mousePosition
// mousePosition --> inverse Oper(n) --> ... --> inverse Oper(0) --> translation(-drawingRect.left) --> scale(scale) --> absolute pos in picture.
//--------------------------------------------------------------------------------------------------------------------------
D2D1_POINT_2F ImageEditorHandler::GetAbsolutePosition(D2D1_POINT_2F mousePosition, bool withBorder)
{
	//--------------------------------------------------------------------------------------------------------------------------
	// First transform back the point (disregarding current translation and scale)
	//--------------------------------------------------------------------------------------------------------------------------
	mousePosition = RemoveRenderingTransformations(mousePosition);

	// Translate to an absolute point within the image current drawing rect
	D2D1_POINT_2F absPoint;
	m_images[m_currentIndex].Image->TranslateToAbsolutePoint(mousePosition, &absPoint);

	D2D1_RECT_F rect;
	m_images[m_currentIndex].Image->GetDrawingRect(&rect);

	//--------------------------------------------------------------------------------------------------------------------------
	// Scale to actual point relative to the original bitmap
	//--------------------------------------------------------------------------------------------------------------------------
	float scale;
	m_images[m_currentIndex].Image->GetScale(&scale);

	if (withBorder == true)
	{
		return AdjustToClipRect(
			D2D1::Point2F(
			scale * (absPoint.x - rect.left),
			scale * (absPoint.y - rect.top)));
	}
	else
	{
		return D2D1::Point2F(
			scale * (absPoint.x - rect.left),
			scale * (absPoint.y - rect.top));
	}
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotResizeImage(WORD, WORD wID, HWND, BOOL&)
{
	ShellFileDialog openDialog;
	HRESULT hr = openDialog.SetDefaultFolder(FOLDERID_Pictures);

	std::vector<ComPtr<IShellItem> > shellItems;
	openDialog.ShowOpenDialog(&shellItems);

	if (shellItems.size() != 1)
		return 0;

	ImageInfo info(sourceFromFile, shellItems.at(0));

	ComPtr<IWICBitmapDecoder> decoder;
	ComPtr<IWICBitmapFrameDecode> bitmapSource;
	ComPtr<IWICFormatConverter> converter;
	ComPtr<IWICBitmapScaler> scaler;
	ComPtr<IWICImagingFactory> wicFactory;

	if (SUCCEEDED(hr))
	{
		hr = g_pWICFactory->CreateDecoderFromFilename(
			info.fileName.c_str(),
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&decoder);
	}

	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = decoder->GetFrame(0, &bitmapSource);
	}

	if (SUCCEEDED(hr))
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		//--------------------------------------------------------------------------------------------------------------------------
		hr = g_pWICFactory->CreateFormatConverter(&converter);
	}

	if (SUCCEEDED(hr))
	{
		hr = converter->Initialize(
			bitmapSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.f,
			WICBitmapPaletteTypeMedianCut);
	}

	UINT width, height;
	converter->GetSize(&width, &height);

	CResizeDlg resizeDlg;
	resizeDlg.setInitialSize(width, height);

	resizeDlg.bp.dialogName = L"新建图像";
	resizeDlg.bp.titleName = L"新建图像尺寸";
	resizeDlg.bp.title1PicFileName = IDR_MYPNG_NEW_TITLE;
	resizeDlg.bp.ifHasTitle2Pic = true;
	resizeDlg.bp.ifTitle2PicFromFile = false;
	//resizeDlg.bp.title2PicFileName = IDR_MYPNG_RESIZE_PIC;
	resizeDlg.bp.title2PicWicBitmap = converter;

	//--------------------------------------------------------------------------------------------------------------------------
	//DoModal()能够创建对话框并且发InitDialog消息后显示它。
	//Create也是靠DialogBoxParam创建对话框并初始化后显示。
	//resizeDlg->Create(NULL, m_hWnd);
	//--------------------------------------------------------------------------------------------------------------------------
	if (resizeDlg.DoModal() != IDOK) return 0;

	g_pWICFactory->CreateBitmapScaler(&scaler);
	scaler->Initialize(converter, g_resize_width, g_resize_height, WICBitmapInterpolationModeLinear);

	ShellFileDialog saveDialog;
	hr = saveDialog.SetDefaultFolder(FOLDERID_Pictures);

	ComPtr<IShellItem> outputItem;
	if (SUCCEEDED(hr))
	{
		ShellFileDialog saveDialog;
		hr = saveDialog.ShowSaveDialog(info.shellItem, &outputItem);
	}

	wchar_t * saveAsFileName;
	hr = outputItem->GetDisplayName(SIGDN_FILESYSPATH, &saveAsFileName);

	Direct2DUtility::SaveBitmapToFile(scaler, saveAsFileName);

	return 0;
}

LRESULT ImageEditorHandler::OnMenuPasteText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::SendMessage(m_hWnd, WM_CHAR, ('V' & 0x1f), 0);
	return 0;
}

LRESULT ImageEditorHandler::OnMenuCopyText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::SendMessage(m_hWnd, WM_CHAR, ('C' & 0x1f), 0);
	return 0;
}

LRESULT ImageEditorHandler::OnMenuCutText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::SendMessage(m_hWnd, WM_CHAR, ('X' & 0x1f), 0);
	return 0;
}

bool ImageEditorHandler::CLBD_IfAvailablePasteText()
{
	WCHAR * strClip = CLBD_GetTextFromClipboard();
	if (strClip == nullptr)
		return false;
	if (m_currentDrawingOperationType != ImageOperationTypeText || m_currentOperation == nullptr)
		return false;
	return true;
}

bool ImageEditorHandler::CLBD_IfAvailableCopyText()
{
	if (m_currentDrawingOperationType != ImageOperationTypeText || m_currentOperation == nullptr)
		return false;

	ComPtr<IImageTextOperation> textOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&textOperation)))
	{
		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);
		if (bp.m_textIfMarked == true && bp.m_textMarkLeft != bp.m_textMarkRight)
			return true;
	}
	return false;
}

void ImageEditorHandler::TEXT_TrackFirstWord(CBTextPtr& bp, int i)
{
	bp.m_textMarkStart = i;
	bp.m_textMarkShift = 0;

	bp.m_textMarkLeft = bp.m_textMarkStart;
	bp.m_textMarkRight = bp.m_textMarkStart;
	bp.m_textPoint = bp.m_textMarkStart;

	bp.m_textIfMarked = true;
}

void ImageEditorHandler::TEXT_TrackFollowingWord(CBTextPtr& bp, int i)
{
	bp.m_textMarkShift = i - bp.m_textMarkStart;
	if (bp.m_textMarkShift == 0)
	{
		bp.m_textMarkLeft = bp.m_textMarkStart;
		bp.m_textMarkRight = bp.m_textMarkStart + 1;
	}
	else if (bp.m_textMarkShift > 0)
	{
		bp.m_textMarkShift++;

		bp.m_textMarkLeft = bp.m_textMarkStart;
		bp.m_textMarkRight = bp.m_textMarkStart + bp.m_textMarkShift;
		if (bp.m_textMarkRight > bp.m_textString.GetLength())
			bp.m_textMarkRight = bp.m_textString.GetLength();
	}
	else
	{
		bp.m_textMarkLeft = bp.m_textMarkStart + bp.m_textMarkShift;
		if (bp.m_textMarkLeft < 0)
			bp.m_textMarkLeft = 0;
		bp.m_textMarkRight = bp.m_textMarkStart + 1;
	}

	if (bp.m_textMarkShift > 0)
		bp.m_textPoint = bp.m_textMarkRight;
	else
		bp.m_textPoint = bp.m_textMarkLeft;
	bp.m_textIfMarked = true;
}

//--------------------------------------------------------------------------------------------------------------------------
// Event for rendering to the client area. During animation, checks the animation manager
// to see if the client are needs to be invalidated to continue rendering the next frame
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::OnRender()
{
	HRESULT hr = S_OK; //CreateGradientBrush();

	//--------------------------------------------------------------------------------------------------------------------------
	// Update the animation manager with the current time
	//--------------------------------------------------------------------------------------------------------------------------
	hr = AnimationUtility::UpdateAnimationManagerTime();
	hr = AnimationUtility2::UpdateAnimationManagerTime();

	//--------------------------------------------------------------------------------------------------------------------------
	// Draw the client area
	//--------------------------------------------------------------------------------------------------------------------------
	DrawClientArea();

	//--------------------------------------------------------------------------------------------------------------------------
	// Continue drawing as long as there are animations scheduled
	//--------------------------------------------------------------------------------------------------------------------------
	bool isBusy;
	bool p2Busy;

	if (SUCCEEDED(AnimationUtility::IsAnimationManagerBusy(&isBusy)) &&
		SUCCEEDED(AnimationUtility2::IsAnimationManagerBusy(&p2Busy)))
	{
		if (isBusy || p2Busy || g_isDrawingRippleOn || g_isDrawingScrollOn || g_isDrawingVScrollOn)
		{
			InvalidateWindow();
		}
		else
		{
			//--------------------------------------------------------------------------------------------------------------------------
			// Save the previous/next images if the user has recently switched images
			//--------------------------------------------------------------------------------------------------------------------------
			if (m_switchingImages)
			{
				SaveFiles();
				m_switchingImages = false;
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// Cleanup animation objects
		//--------------------------------------------------------------------------------------------------------------------------
		if (!isBusy && m_animationEnabled)
		{
			CleanupAnimation();
		}

		void * key;
		ComPtr<AnimationPackage2> unBusyPack;
		while ((unBusyPack = AnimationUtility2::GetAnUnbusyAnimationPackage(&key)) != nullptr)
		{
			AnimationUtility2::DeleteAnimationPackage(key);

			ComPtr<ILayerAnimation> layerAnimation = unBusyPack->m_animation;

			//--------------------------------------------------------------------------------------------------------------------------
			// Rota 动画的停止或接续操作。
			//--------------------------------------------------------------------------------------------------------------------------
			ComPtr<ILayerAnimationRota> rotaAnim;
			if (SUCCEEDED(layerAnimation->QueryInterface(&rotaAnim)))
			{
				rotaAnim->Cleanup();

				ComPtr<IImageOperation> opera = rotaAnim->m_rotaOperation;
				CombDlg::COMB_SetItemActive(opera, true);

				if (g_isLayerAnimationOn == true && rotaAnim->m_rotaOperationType == ImageOperationTypeRotate360)
				{
					CombDlg::COMB_SetItemActive(opera, false);
					rotaAnim->Setup(opera, ImageOperationTypeRotate360, layerAnimation);

					InvalidateWindow();
				}
			}

			//--------------------------------------------------------------------------------------------------------------------------
			// Text layer 的 Rollup 动画的停止或接续操作。
			//--------------------------------------------------------------------------------------------------------------------------
			ComPtr<ILayerAnimationRollup> rollupAnim;
			if (SUCCEEDED(layerAnimation->QueryInterface(&rollupAnim)))
			{
				ComPtr<IImageOperation> opera = rollupAnim->m_rollupOperation;
				CombDlg::COMB_SetItemActive(opera, true);

				if (g_isLayerAnimationOn == true)
				{
					CombDlg::COMB_SetItemActive(opera, false);
					rollupAnim->Setup(opera, layerAnimation, start_from_low, opera->GetDuration());
				}

				InvalidateWindow();
			}

			//--------------------------------------------------------------------------------------------------------------------------
			// Text layer 的两种 写 动画的停止或接续操作。
			//--------------------------------------------------------------------------------------------------------------------------
			ComPtr<ILayerAnimationWrite> writeAnim;
			if (SUCCEEDED(layerAnimation->QueryInterface(&writeAnim)))
			{
				ComPtr<IImageOperation> opera = writeAnim->m_writeOperation;
				CombDlg::COMB_SetItemActive(opera, true);

				if (g_isLayerAnimationOn == true)
				{
					CombDlg::COMB_SetItemActive(opera, false);
					writeAnim->Setup(opera, layerAnimation, opera->GetDuration(), writeAnim->m_writeStyle);
				}

				InvalidateWindow();
			}

			//--------------------------------------------------------------------------------------------------------------------------
			// Text layer 的两种 写 动画的停止或接续操作。
			//--------------------------------------------------------------------------------------------------------------------------
			ComPtr<ILayerAnimationPics> picsAnim;
			if (SUCCEEDED(layerAnimation->QueryInterface(&picsAnim)))
			{
				ComPtr<IImageOperation> opera = picsAnim->m_picsOperation;
				CombDlg::COMB_SetItemActive(opera, true);
				
				if (g_isLayerAnimationOn == true)
				{
					CombDlg::COMB_SetItemActive(opera, false);
					picsAnim->Setup(opera, layerAnimation, opera->GetDuration());
				}
				
				InvalidateWindow();
			}

			//--------------------------------------------------------------------------------------------------------------------------
			// Pic layer的 平移 动画的停止或接续操作。
			//--------------------------------------------------------------------------------------------------------------------------
			ComPtr<ILayerAnimationPast> pastAnim;
			if (SUCCEEDED(layerAnimation->QueryInterface(&pastAnim)))
			{
				ComPtr<IImageOperation> opera = pastAnim->m_pastOperation;
				CombDlg::COMB_SetItemActive(opera, true);
				
				if (g_isLayerAnimationOn == true)
				{
					CombDlg::COMB_SetItemActive(opera, false);
					pastAnim->Setup(D2D1::Point2F(0.0f, 0.0f), D2D1::Point2F(250.0f, 0.0f), 25.0f, layerAnimation);

					InvalidateWindow();
				}
			}

			//--------------------------------------------------------------------------------------------------------------------------
			// Write Wave Text的 左右摇摆 动画的停止或接续操作。
			//--------------------------------------------------------------------------------------------------------------------------
			ComPtr<ILayerAnimationWWave> wwaveAnim;
			if (SUCCEEDED(layerAnimation->QueryInterface(&wwaveAnim)))
			{
				ComPtr<IImageOperation> opera = wwaveAnim->m_textOperation;
				CombDlg::COMB_SetItemActive(opera, true);

				if (g_isLayerAnimationOn == true)
				{
					CombDlg::COMB_SetItemActive(opera, false);
					wwaveAnim->Setup(opera, layerAnimation);

					InvalidateWindow();
				}
			}
		}
	}

	return hr;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotLayerR360(WORD, WORD wID, HWND, BOOL&)
{
	g_isLayerAnimationOn = true;
	CombDlg::COMB_SetItemActive(m_currentOperation, false);

	ROTA_SetRotationOperation(ImageOperationTypeRotate360);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotLayerR30(WORD, WORD wID, HWND, BOOL&)
{
	ROTA_SetRotationOperation(ImageOperationTypeRotate30);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Delete 可显示layer的操作，删除后把 Operation队列里的另一个可显示 item 设置为 m_currentOperation。
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotDeleteLayer(WORD, WORD wID, HWND, BOOL&)
{
	if (m_currentOperation == nullptr)
		return 0;

	ComPtr<IImage> image = m_images[m_currentIndex].Image;
	image->DeleteOperation(m_currentOperation);
	//ComPtr<IImageOperation> tmpOper = m_currentOperation;
	CombDlg::COMB_InitList();
	CombDlg::COMB_DeactivateItemWithFollower(m_currentOperation);

	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotFullscreen(WORD, WORD wID, HWND, BOOL&)
{
	g_fullScreen = true;
	//g_frameWork->OnMaximize();
	OnMaximize();
	BOOL dumbo;
	AnnotZoom100(0, 0, 0, dumbo);
	if (m_currentOperation != nullptr)
	{
		CombDlg::COMB_SetItemActive(m_currentOperation, false);
	}
	InvalidateWindow();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// 按照 m_currentOperation 的类型设置 Ribbon 按钮的显示和激活状态。
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::UpdateUIFramework()
{
	g_frameWork->UIEnable(ID_LAYERS, CombDlg::COMB_HasListMember(), TRUE);
	g_frameWork->UIEnable(ID_PICOPACITY, CombDlg::COMB_HasListMember(), TRUE);
	g_frameWork->UIEnable(ID_DURATION, CombDlg::COMB_HasListMember(), TRUE);

	g_frameWork->UIEnable(ID_WTL_MENU_NEW_CLIPBOARD, 
		IsClipboardFormatAvailable(CF_BITMAP), 
		TRUE);

	g_frameWork->UIEnable(ID_WTL_MENU_PASTE, 
		IsClipboardFormatAvailable(CF_BITMAP), 
		TRUE);

	ComPtr<IImageTextOperation> textOperation;
	ComPtr<IImageLayerOperation> layerOperation;
	ComPtr<IDrawGeometryOperation> geoOperation;
	ComPtr<IImagePicsOperation> picsOperation;
	ComPtr<IImageSVGOperation> svgOperation;

	g_frameWork->UIEnable(layer_rotateButton,
		m_currentOperation != nullptr &&
		!SUCCEEDED(m_currentOperation->QueryInterface(&picsOperation)) &&
		(SUCCEEDED(m_currentOperation->QueryInterface(&layerOperation)) ||
		SUCCEEDED(m_currentOperation->QueryInterface(&textOperation))),
		TRUE);

	layerOperation = nullptr;
	picsOperation = nullptr;
	svgOperation = nullptr;
	g_frameWork->UIEnable(IDC_WTL_PIC_SIZE_BIGGER,
		m_currentOperation != nullptr &&
		!SUCCEEDED(m_currentOperation->QueryInterface(&picsOperation)) &&
		SUCCEEDED(m_currentOperation->QueryInterface(&layerOperation)),
		TRUE);

	textOperation = nullptr;
	layerOperation = nullptr;
	geoOperation = nullptr;
	g_frameWork->UIEnable(ID_WTL_DELETE_LAYER,
		m_currentOperation != nullptr &&
		(SUCCEEDED(m_currentOperation->QueryInterface(&layerOperation)) ||
		(SUCCEEDED(m_currentOperation->QueryInterface(&textOperation))) ||
		(SUCCEEDED(m_currentOperation->QueryInterface(&geoOperation)))),
		TRUE);

	layerOperation = nullptr;
	picsOperation = nullptr;
	svgOperation = nullptr;
	g_frameWork->UIEnable(ID_WTL_TRANSLATION_LAYER,
		m_currentOperation != nullptr &&
		!SUCCEEDED(m_currentOperation->QueryInterface(&svgOperation)) &&
		!SUCCEEDED(m_currentOperation->QueryInterface(&picsOperation)) &&
		(SUCCEEDED(m_currentOperation->QueryInterface(&layerOperation))),
		TRUE);

	textOperation = nullptr;
	g_frameWork->UIEnable(ID_WTL_ROLLUP,
		m_currentOperation != nullptr &&
		(SUCCEEDED(m_currentOperation->QueryInterface(&textOperation))),
		TRUE);

	textOperation = nullptr;
	g_frameWork->UIEnable(ID_WTL_WRITE_OPACITY,
		m_currentOperation != nullptr &&
		(SUCCEEDED(m_currentOperation->QueryInterface(&textOperation))),
		TRUE);

	textOperation = nullptr;
	g_frameWork->UIEnable(ID_WTL_WRITE_PUNCH,
		m_currentOperation != nullptr &&
		(SUCCEEDED(m_currentOperation->QueryInterface(&textOperation))),
		TRUE);

	textOperation = nullptr;
	g_frameWork->UIEnable(ID_WTL_WRITE_WAVE,
		m_currentOperation != nullptr &&
		(SUCCEEDED(m_currentOperation->QueryInterface(&textOperation))),
		TRUE);
	
	picsOperation = nullptr;
	g_frameWork->UIEnable(ID_WTL_SHOW_PICS,
		m_currentOperation != nullptr &&
		(SUCCEEDED(m_currentOperation->QueryInterface(&picsOperation))),
		TRUE);

	g_frameWork->UIEnable(ID_LOCATE_PICS,
		m_picsFirstPicFileName.GetLength() != 0,
		TRUE);

	g_frameWork->UIEnable(ID_LOCATE, 
		m_ifLayerFromPasting == true || m_layerBitmap != nullptr, 
		TRUE);

	g_frameWork->UISetCheck(cropButton, false, TRUE);
	g_frameWork->UISetCheck(ID_WTL_MENU_PEN, false, TRUE);
	g_frameWork->UISetCheck(ID_WTL_MENU_COPY, false, TRUE);
	g_frameWork->UISetCheck(ID_WTL_MENU_TEXT, false, TRUE);
	g_frameWork->UISetCheck(ID_WTL_MENU_PASTE, false, TRUE);

	switch (m_currentDrawingOperationType)
	{
	case ImageOperationTypeCrop:
		g_frameWork->UISetCheck(cropButton, true, TRUE);
		break;
	case ImageOperationTypePen:
		g_frameWork->UISetCheck(ID_WTL_MENU_PEN, true, TRUE);
		break;
	case ImageOperationTypeText:
		g_frameWork->UISetCheck(ID_WTL_MENU_TEXT, true, TRUE);
		break;
	case ImageOperationTypeCopy:
		g_frameWork->UISetCheck(ID_WTL_MENU_COPY, true, TRUE);
		break;
	default:
		break;
	}

	g_frameWork->UISetCheck(ID_WTL_F, false, TRUE);
	g_frameWork->UISetCheck(ID_WTL_R, false, TRUE);
	g_frameWork->UISetCheck(ID_WTL_T, false, TRUE);
	g_frameWork->UISetCheck(ID_WTL_B, false, TRUE);

	text_style tstyle = g_textStyle;
	if (m_currentOperation != nullptr)
	{
		ComPtr<IImageTextOperation> operation;
		if (SUCCEEDED(m_currentOperation->QueryInterface(&operation)))
		{
			CBTextPtr bp;
			operation->GetTextEquipment(&bp);
			tstyle = bp.m_textStyle;
		}
	}

	switch (tstyle)
	{
	case style_f:
		g_frameWork->UISetCheck(ID_WTL_F, true, TRUE);
		break;
	case style_r:
		g_frameWork->UISetCheck(ID_WTL_R, true, TRUE);
		break;
	case style_c:
		g_frameWork->UISetCheck(ID_WTL_T, true, TRUE);
		break;
	case style_b:
		g_frameWork->UISetCheck(ID_WTL_B, true, TRUE);
		break;
	default:
		break;
	}

	g_frameWork->UIEnable(IDC_WTL_MENU_COPY_TEXT, 
		CLBD_IfAvailableCopyText(), 
		TRUE);
	g_frameWork->UIEnable(IDC_WTL_MENU_CUT_TEXT, 
		CLBD_IfAvailableCopyText(), 
		TRUE);
	g_frameWork->UIEnable(IDC_WTL_MENU_PASTE_TEXT, 
		CLBD_IfAvailablePasteText(), 
		TRUE);

	InvalidateWindow();
	return S_OK;
}


LRESULT ImageEditorHandler::AnnotPause(WORD, WORD wID, HWND, BOOL&)
{
	::PostMessage(m_hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Publish 读取用户指定的XML文件，并按照格式显示其内容。
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotPublish(WORD, WORD wID, HWND, BOOL&)
{
	WCHAR strFileFilters[1024] = L"Xml Files(*.xml)\0*.xml\0\0";
	CFileDialog xmlDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFileFilters, m_hWnd);

	WCHAR temp[1024];
	ZeroMemory(temp, sizeof(WCHAR) * 1024);

	xmlDialog.m_ofn.lpstrFile = temp;
	xmlDialog.m_ofn.nMaxFile = 1024;

	if (IDCANCEL == xmlDialog.DoModal())
		return 0;

	tinyxml2::XMLDocument doc;
	doc.LoadwFile(temp);

	XMLElement* element = doc.FirstChildElement("document")->FirstChildElement("Framework");
	u_int bgColor = element->Unsigned64Attribute("bgcolor", 0xd77800);
	g_resize_width = 1920;
	g_resize_height = 1080;
	AnnotInnerNew((COLORREF)bgColor);

	CString tfstring = utf8ToUnicode(element->GetText());

	//--------------------------------------------------------------------------------------------------------------------------
	// Publish 里的XML Picture 项的读取和设置。
	//--------------------------------------------------------------------------------------------------------------------------
	element = doc.FirstChildElement("document")->FirstChildElement("Picture");
	for (; element != NULL; element = element->NextSiblingElement("Picture"))
	{
		CString pos;
		const char* charpos = element->Attribute("location");
		if (charpos == nullptr)
		{
			pos = L"specify";
		}
		else 
		{
			pos = utf8ToUnicode(charpos);
		}

		FLOAT x = (FLOAT)element->IntAttribute("x", 100);
		FLOAT y = (FLOAT)element->IntAttribute("y", 100);
		g_animDuration = (FLOAT)element->IntAttribute("duration", 30);

		WCHAR* ctitle = utf8ToUnicode(element->GetText());

		CBLayerPtr bp;
		ComPtr<IImageOperation> layer;
		SharedObject<ImageLayerOperation>::Create(bp, &layer);
		m_currentOperation = layer;

		m_images[m_currentIndex].Image->PushImageOperation(layer);

		ComPtr<IImageLayerOperation> layerOperation;
		if (SUCCEEDED(m_currentOperation.QueryInterface(&layerOperation)))
		{
			CBLayerPtr bp;
			layerOperation->GetLayerEquipment(&bp);

			bp.m_layerFrameBrush = m_solidBrush;

			bp.m_layerIfPasting = false;
			bp.m_layerPicFileName = ctitle;
			bp.m_layerWicBitmap = g_render->LoadWicBitmapFromFile(ctitle, 0, 0);
			g_pD2DDeviceContext->CreateBitmapFromWicBitmap(bp.m_layerWicBitmap, &bp.m_layerBitmap);

			bp.m_rotatableBorderWidth = bp.m_layerBitmap->GetSize().width;
			bp.m_rotatableBorderHeight = bp.m_layerBitmap->GetSize().height;

			PAST_CalculatePastingMatrix(bp, m_images[m_currentIndex].Image);

			if (pos.Compare(L"center") == 0)
			{
				bp.m_rotatableRelativePt.x = (1920 - bp.m_rotatableBorderWidth) / 2;
				bp.m_rotatableRelativePt.y = (1080 - bp.m_rotatableBorderHeight) / 2;
			}
			else if (pos.Compare(L"left") == 0)
			{
				bp.m_rotatableRelativePt.x = 0;
				bp.m_rotatableRelativePt.y = (1080 - bp.m_rotatableBorderHeight) / 2;
			}
			else if (pos.Compare(L"right") == 0)
			{
				bp.m_rotatableRelativePt.x = (1920 - bp.m_rotatableBorderWidth);
				bp.m_rotatableRelativePt.y = (1080 - bp.m_rotatableBorderHeight) / 2;
			}
			else if (pos.Compare(L"top") == 0)
			{
				bp.m_rotatableRelativePt.x = (1920 - bp.m_rotatableBorderWidth) / 2;
				bp.m_rotatableRelativePt.y = 0;
			}
			else if (pos.Compare(L"bottom") == 0)
			{
				bp.m_rotatableRelativePt.x = (1920 - bp.m_rotatableBorderWidth) / 2;
				bp.m_rotatableRelativePt.y = (1080 - bp.m_rotatableBorderHeight);
			}
			else if (pos.Compare(L"topleft") == 0)
			{
				bp.m_rotatableRelativePt.x = 0;
				bp.m_rotatableRelativePt.y = 0;
			}
			else if (pos.Compare(L"bottomleft") == 0)
			{
				bp.m_rotatableRelativePt.x = 0;
				bp.m_rotatableRelativePt.y = (1080 - bp.m_rotatableBorderHeight);
			}
			else if (pos.Compare(L"topright") == 0)
			{
				bp.m_rotatableRelativePt.x = (1920 - bp.m_rotatableBorderWidth);
				bp.m_rotatableRelativePt.y = 0;
			}
			else if (pos.Compare(L"bottomright") == 0)
			{
				bp.m_rotatableRelativePt.x = (1920 - bp.m_rotatableBorderWidth);
				bp.m_rotatableRelativePt.y = (1080 - bp.m_rotatableBorderHeight);
			}
			else
			{
				bp.m_rotatableRelativePt = D2D1::Point2F(x, y);
			}

			previousLayeringPoint = bp.m_rotatableRelativePt;

			layerOperation->SetLayerEquipment(bp);
		}
		m_currentOperation->SetDuration(g_animDuration);
		//--------------------------------------------------------------------------------------------------------------------------
		// 调用 BomboBox 的初始化，同步数据。
		//--------------------------------------------------------------------------------------------------------------------------
		CombDlg::COMB_InitList();
	}

	element = doc.FirstChildElement("document")->FirstChildElement("PictureBox");
	for (; element != NULL; element = element->NextSiblingElement("PictureBox"))
	{
		FLOAT x = (FLOAT)element->IntAttribute("x", 100);
		FLOAT y = (FLOAT)element->IntAttribute("y", 100);
		g_animDuration = (FLOAT)element->IntAttribute("duration", 30);

		WCHAR* ctitle = utf8ToUnicode(element->GetText());
		m_picsFirstPicFileName = ctitle;

		PICS_CreatePicsItem(x, y);

		m_currentOperation->SetDuration(g_animDuration);
		//--------------------------------------------------------------------------------------------------------------------------
		// 调用 BomboBox 的初始化，同步数据。
		//--------------------------------------------------------------------------------------------------------------------------
		CombDlg::COMB_InitList();
	}

	if (tfstring.Compare(L"text") == 0)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// Publish 里的XML Text 项的读取和设置。
		//--------------------------------------------------------------------------------------------------------------------------
		XMLElement* element = doc.FirstChildElement("document")->FirstChildElement("Text");
		for (; element != NULL; element = element->NextSiblingElement("Text"))
		{
			FLOAT x = (FLOAT)element->IntAttribute("x", 100);
			FLOAT y = (FLOAT)element->IntAttribute("y", 100);
			FLOAT width = (FLOAT)element->IntAttribute("width", 100);
			FLOAT height = (FLOAT)element->IntAttribute("height", 100);
			CString sps = utf8ToUnicode(element->Attribute("printstyle"));

			g_animDuration = (FLOAT)element->IntAttribute("duration", 20);
			g_frameWork->COMB_SetSpinnerDuration((LONG)g_animDuration);

			m_textFormat = nullptr;
			SecureHelper::strcpy_x(g_textFormatFamilyName, LF_FACESIZE, utf8ToUnicode(element->Attribute("fontfamily")));
			g_textFormatFontSize = (FLOAT)element->IntAttribute("fontsize", 30);
			g_pDWriteFactory->CreateTextFormat(
																		g_textFormatFamilyName,
																		nullptr,
																		DWRITE_FONT_WEIGHT_REGULAR,
																		DWRITE_FONT_STYLE_NORMAL,
																		DWRITE_FONT_STRETCH_NORMAL,
																		g_textFormatFontSize,
																		L"en-us",
																		&m_textFormat);

			u_int fgColor = element->Unsigned64Attribute("fgcolor", 0xffffff);

			if (sps.Compare(L"book") == 0)
			{
				g_textStyle = style_b;
			}
			else if (sps.Compare(L"center") == 0)
			{
				g_textStyle = style_c;
			}
			else
			{
				g_textStyle = style_f;
			}
			WCHAR* ctitle = utf8ToUnicode(element->GetText());

			D2D1_POINT_2F aliasMousePos = D2D1::Point2F(70, 70);
			TEXT_CreateTextItem(aliasMousePos);
			ComPtr<IImageTextOperation> text;
			if (SUCCEEDED(m_currentOperation->QueryInterface(&text)))
			{
				CBTextPtr bp;
				text->GetTextEquipment(&bp);
				bp.m_rotatableBorderWidth = width;
				bp.m_rotatableBorderHeight = height;
				bp.m_textString = ctitle;
				bp.m_rotatableRelativePt.x = x;
				bp.m_rotatableRelativePt.y = y;

				D2D1_COLOR_F color = D2D1::ColorF(static_cast<float>(GetRValue(fgColor)) / 255.0f,
																			static_cast<float>(GetGValue(fgColor)) / 255.0f,
																			static_cast<float>(GetBValue(fgColor)) / 255.0f);
				bp.m_textColor = color;
				bp.m_textString.Replace(L"\n", L"\r");
				if (bp.m_textString.GetLength() > 0 && bp.m_textString.GetAt(0) == L'\r')
				{
					bp.m_textString = bp.m_textString.Right(bp.m_textString.GetLength() - 1);
				}
				ImageTextOperation::TEXT_FrameOutText(bp);
				text->SetTextEquipment(bp);
			}
			m_currentOperation->SetDuration(g_animDuration);
			//--------------------------------------------------------------------------------------------------------------------------
			// 调用 BomboBox 的初始化，同步数据。
			//--------------------------------------------------------------------------------------------------------------------------
			CombDlg::COMB_InitList();
		}
	}
	else if (tfstring.Compare(L"poem") == 0)
	{
		FLOAT relX = 500;
		//--------------------------------------------------------------------------------------------------------------------------
		// Publish 里的XML Poem 项的读取和设置。
		//--------------------------------------------------------------------------------------------------------------------------
		XMLElement* element = doc.FirstChildElement("document")->FirstChildElement("Poem");
		for (; element != NULL; element = element->NextSiblingElement("Poem"))
		{
			g_animDuration = (FLOAT)element->IntAttribute("duration", 20);
			g_frameWork->COMB_SetSpinnerDuration((LONG)g_animDuration);

			m_textFormat = nullptr;
			
			SecureHelper::strcpy_x(g_textFormatFamilyName, LF_FACESIZE, utf8ToUnicode(element->Attribute("fontfamily")));
			g_textFormatFontSize = (FLOAT)element->IntAttribute("fontsize", 30);
			g_pDWriteFactory->CreateTextFormat(
																		g_textFormatFamilyName,
																		nullptr,
																		DWRITE_FONT_WEIGHT_REGULAR,
																		DWRITE_FONT_STYLE_NORMAL,
																		DWRITE_FONT_STRETCH_NORMAL,
																		g_textFormatFontSize,
																		L"en-us",
																		&m_textFormat);

			u_int fgColor = element->Unsigned64Attribute("fgcolor", 0xffffff);

			WCHAR* ctitle = utf8ToUnicode(element->GetText());
			g_textStyle = style_f;

			D2D1_POINT_2F aliasMousePos = D2D1::Point2F(70, 70);
			TEXT_CreateTextItem(aliasMousePos);
			ComPtr<IImageTextOperation> text;
			if (SUCCEEDED(m_currentOperation->QueryInterface(&text)))
			{
				CBTextPtr bp;
				text->GetTextEquipment(&bp);
				bp.m_rotatableBorderWidth = 270;
				bp.m_rotatableBorderHeight = 750;
				bp.m_textString = ctitle;
				bp.m_rotatableRelativePt.x = relX;
				bp.m_rotatableRelativePt.y = 150;
				relX += 300;

				D2D1_COLOR_F color = D2D1::ColorF(static_cast<float>(GetRValue(fgColor)) / 255.0f,
																			static_cast<float>(GetGValue(fgColor)) / 255.0f,
																			static_cast<float>(GetBValue(fgColor)) / 255.0f);
				bp.m_textColor = color;
				bp.m_textString.Replace(L"\n", L"\r");
				if (bp.m_textString.GetLength() > 0 && bp.m_textString.GetAt(0) == L'\r')
				{
					bp.m_textString = bp.m_textString.Right(bp.m_textString.GetLength() - 1);
				}
				ImageTextOperation::TEXT_FrameOutText(bp);
				text->SetTextEquipment(bp);
			}
			m_currentOperation->SetDuration(g_animDuration);
			//--------------------------------------------------------------------------------------------------------------------------
			// 调用 BomboBox 的初始化，同步数据。
			//--------------------------------------------------------------------------------------------------------------------------
			CombDlg::COMB_InitList();
		}
	}

	m_currentOperation = nullptr;
	InvalidateWindow();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Text 文本框的 上翻动画的 Setup。
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotRollup(WORD, WORD wID, HWND, BOOL&)
{
	g_isLayerAnimationOn = true;
	CombDlg::COMB_SetItemActive(m_currentOperation, false);

	ComPtr<ILayerAnimation> layerAnim;
	ComPtr<ILayerAnimationRollup> rollupAnim;
	SharedObject<ILayerAnimationRollup>::Create(&layerAnim);
	if (SUCCEEDED(layerAnim->QueryInterface(&rollupAnim)))
	{
		rollupAnim->Setup(m_currentOperation, layerAnim, start_from_above, m_currentOperation->GetDuration());
	}
	
	InvalidateWindow();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// 动画类型的 写 Appear 风格。
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotWriteOpacity(WORD, WORD wID, HWND, BOOL&)
{
	ANIM_Write(opacity);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// 动画类型 写 Punch 风格。
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotWritePunch(WORD, WORD wID, HWND, BOOL&)
{
	ANIM_Write(punch);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Text 文本框的 写 动画的 Setup。
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::ANIM_Write(write_style wStyle)
{
	g_isLayerAnimationOn = true;
	CombDlg::COMB_SetItemActive(m_currentOperation, false);

	ComPtr<ILayerAnimation> layerAnim;
	ComPtr<ILayerAnimationWrite> writeAnim;
	SharedObject<ILayerAnimationWrite>::Create(&layerAnim);
	if (SUCCEEDED(layerAnim->QueryInterface(&writeAnim)))
	{
		writeAnim->Setup(m_currentOperation, layerAnim, m_currentOperation->GetDuration(), wStyle);
	}

	InvalidateWindow();
}

//--------------------------------------------------------------------------------------------------------------------------
// 动画类型 写 wave 风格。
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotWriteWave(WORD, WORD wID, HWND, BOOL&)
{
	g_isLayerAnimationOn = true;
	CombDlg::COMB_SetItemActive(m_currentOperation, false);

	ComPtr<ILayerAnimation> layerAnim;
	ComPtr<ILayerAnimationWWave> wwaveAnim;
	SharedObject<ILayerAnimationWWave>::Create(&layerAnim);
	if (SUCCEEDED(layerAnim->QueryInterface(&wwaveAnim)))
	{
		wwaveAnim->Setup(m_currentOperation, layerAnim);
	}

	InvalidateWindow();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Pic layer 的平移动画的 Setup。
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotPast(WORD, WORD wID, HWND, BOOL&)
{
	CombDlg::COMB_SetItemActive(m_currentOperation, false);
	g_isLayerAnimationOn = true;

	ComPtr<ILayerAnimation> layerAnim;
	ComPtr<ILayerAnimationPast> pastAnim;
	SharedObject<ILayerAnimationPast>::Create(m_currentOperation, &layerAnim);
	if (SUCCEEDED(layerAnim->QueryInterface(&pastAnim)))
	{
		pastAnim->Setup(D2D1::Point2F(0.0f, 0.0f), D2D1::Point2F(250.0f, 0.0f), 25.0f, layerAnim);
	}

	InvalidateWindow();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Rota 类型（Text和Pic）的layer的翻转动画的 Setup。
//--------------------------------------------------------------------------------------------------------------------------
HRESULT ImageEditorHandler::ROTA_SetRotationOperation(__in ImageOperationType imageDrawingOperation)
{
	if (m_currentOperation == nullptr)
		return S_OK;

	ComPtr<ILayerAnimation> layerAnim;
	ComPtr<ILayerAnimationRota> rotaAnim;
	SharedObject<ILayerAnimationRota>::Create(&layerAnim);
	if (SUCCEEDED(layerAnim->QueryInterface(&rotaAnim)))
	{
		rotaAnim->Setup(m_currentOperation, imageDrawingOperation, layerAnim);
	}

	InvalidateWindow();
	return S_OK;
}

LRESULT ImageEditorHandler::AnnotLayerLocate(WORD, WORD wID, HWND, BOOL&)
{
	if (m_ifLayerFromPasting == false && m_layerBitmap == nullptr)
		return 0;

	D2D1_POINT_2F mousePosition;
	mousePosition.x = 0;
	mousePosition.y = 0;
	PLYR_CreateLayerItem(mousePosition, true);

	PAST_UnselectLayer();

	CombDlg::COMB_InitList();
	InvalidateWindow();

	SetTimer(DELAY_TIMER, 1000);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotOpenLayerFile(WORD, WORD wID, HWND, BOOL&)
{
	ShellFileDialog openDialog;
	HRESULT hr = openDialog.SetDefaultFolder(FOLDERID_Pictures);

	std::vector<ComPtr<IShellItem> > shellItems;
	openDialog.ShowOpenDialog(&shellItems);

	if (shellItems.size() != 1)
		return 0;

	ImageInfo info(sourceFromFile, shellItems.at(0));

	m_layerBitmap = g_render->LoadBitmapFromFile(info.fileName.c_str(),	0,	0);

	m_layerPicFileName = info.fileName.c_str();

	InvalidateWindow();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotOpenPicsFile(WORD, WORD wID, HWND, BOOL&)
{
	ShellFileDialog openDialog;
	HRESULT hr = openDialog.SetDefaultFolder(FOLDERID_Pictures);

	std::vector<ComPtr<IShellItem> > shellItems;
	openDialog.ShowOpenDialog(&shellItems);

	if (shellItems.size() != 1)
		return 0;

	ImageInfo info(sourceFromFile, shellItems.at(0));

	m_picsFirstPicFileName = info.fileName.c_str();

	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotPicsLocate(WORD, WORD wID, HWND, BOOL&)
{
	if (m_picsFirstPicFileName.GetLength() == 0)
		return 0;

	PICS_CreatePicsItem(-1, -1);
	PAST_UnselectLayer();

	CombDlg::COMB_InitList();
	InvalidateWindow();

	SetTimer(DELAY_TIMER, 1000);
	return 0;
}

void ImageEditorHandler::PICS_CreatePicsItem(FLOAT picX, FLOAT picY)
{
	CBLayerPtr bp;
	ComPtr<IImageOperation> pics;
	SharedObject<ImagePicsOperation>::Create(&pics);
	m_currentOperation = pics;

	m_images[m_currentIndex].Image->PushImageOperation(pics);

	ComPtr<IImageLayerOperation> layerOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&layerOperation)))
	{
		CBLayerPtr bp;
		layerOperation->GetLayerEquipment(&bp);

		bp.m_layerFrameBrush = m_solidBrush;

		bp.m_layerIfPasting = false;
		//--------------------------------------------------------------------------------------------------------------------------
		// bp.m_layerPicFileName 仅仅是为了在 LayerCombo 中显示其文件名称
		//--------------------------------------------------------------------------------------------------------------------------
		bp.m_layerPicFileName = m_picsFirstPicFileName;
		bp.m_layerWicBitmap = g_render->LoadWicBitmapFromFile(m_picsFirstPicFileName, 0, 0);

		g_pD2DDeviceContext->CreateBitmapFromWicBitmap(bp.m_layerWicBitmap, &bp.m_layerBitmap);

		bp.m_rotatableBorderWidth = bp.m_layerBitmap->GetSize().width;
		bp.m_rotatableBorderHeight = bp.m_layerBitmap->GetSize().height;

		PAST_CalculatePastingMatrix(bp, m_images[m_currentIndex].Image);

		if (picX == -1)
		{
			D2D1_POINT_2F mousePosition;
			D2D1_POINT_2F absPoint;
			CRect rect;
			GetClientRect(&rect);
			float scale;

			m_images[m_currentIndex].Image->GetScale(&scale);
			//--------------------------------------------------------------------------------------------------------------------------
			// mousePosition 是模拟鼠标在 ClientRect 中的中心位置
			//--------------------------------------------------------------------------------------------------------------------------
			mousePosition.x = rect.right / 2.0 - bp.m_rotatableBorderWidth / scale / 2.0;
			mousePosition.y = rect.bottom / 2.0 - bp.m_rotatableBorderHeight / scale / 2.0;
			absPoint = GetAbsolutePosition(mousePosition, false);

			bp.m_rotatableRelativePt = bp.m_pastingMatrix.TransformPoint(absPoint);
		}
		else 
		{
			bp.m_rotatableRelativePt = D2D1::Point2F(picX, picY);
		}
		previousLayeringPoint = bp.m_rotatableRelativePt;

		layerOperation->SetLayerEquipment(bp);
	}

	ComPtr<IImagePicsOperation> picsOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&picsOperation)))
	{
		CBPicsPtr bp;
		picsOperation->GetPicsEquipment(&bp);

		ComPtr<IWICBitmapSource> picWicBitmap = g_render->LoadWicBitmapFromFile(m_picsFirstPicFileName, 0, 0);
		ComPtr<ID2D1Bitmap> picBitmap;
		g_pD2DDeviceContext->CreateBitmapFromWicBitmap(picWicBitmap, &picBitmap);

		bp.m_picsFileNames.push_back(m_picsFirstPicFileName);
		bp.m_picsWicBitmaps.push_back(picWicBitmap);
		bp.m_picsBitmaps.push_back(picBitmap);

		CString currentPath = m_picsFirstPicFileName.Left(m_picsFirstPicFileName.ReverseFind(L'\\'));
		CString parentDirPath = currentPath;
		CString currentDirFile = parentDirPath;

		parentDirPath += CString(L"\\*");
		CFindFile ff;

		BOOL res = ff.FindFile(parentDirPath);
		while (res)
		{
			res = ff.FindNextFile();
			if (!ff.IsDirectory() && !ff.IsDots())
			{
				CString strFile = ff.GetFileName();
				CString strPath = CString(currentDirFile);
				strPath += CString(L"\\");
				strPath += strFile;

				if (strPath.Right(4).CompareNoCase(L".jpg") == 0 || strPath.Right(4).CompareNoCase(L".png") == 0 ||
					strPath.Right(5).CompareNoCase(L".jpeg") == 0)
				{
					if (strPath.Compare(m_picsFirstPicFileName) != 0)
					{
						ComPtr<IWICBitmapSource> picWicBitmap = g_render->LoadWicBitmapFromFile(strPath, 0, 0);
						ComPtr<ID2D1Bitmap> picBitmap;
						g_pD2DDeviceContext->CreateBitmapFromWicBitmap(picWicBitmap, &picBitmap);
						
						bp.m_picsFileNames.push_back(strPath);
						bp.m_picsWicBitmaps.push_back(picWicBitmap);
						bp.m_picsBitmaps.push_back(picBitmap);
					}
				}
			}
		}

		picsOperation->SetPicsEquipment(bp);
	}

	m_currentOperation->SetDuration(g_animDuration);
	CombDlg::COMB_InitList();
}

//--------------------------------------------------------------------------------------------------------------------------
// Text 文本框的 写 动画的 Setup。
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotShowPics(WORD, WORD wID, HWND, BOOL&)
{
	g_isLayerAnimationOn = true;
	CombDlg::COMB_SetItemActive(m_currentOperation, false);

	ComPtr<ILayerAnimation> layerAnim;
	ComPtr<ILayerAnimationPics> picsAnim;
	SharedObject<ILayerAnimationPics>::Create(&layerAnim);
	if (SUCCEEDED(layerAnim->QueryInterface(&picsAnim)))
	{
		picsAnim->Setup(m_currentOperation, layerAnim, m_currentOperation->GetDuration());
	}

	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotDrawHScroll(WORD, WORD wID, HWND, BOOL&)
{
	g_isDrawingRippleOn = false;
	g_isDrawingScrollOn = true;
	g_isDrawingVScrollOn = false;
	CString filename;
	if (m_layerPicFileName.GetLength() == 0)
	{
		filename = L"K:\\娱乐\\ico\\wtlpainter\\samples\\2560x1440.jpg";
	}
	else
	{
		filename = m_layerPicFileName;
	}
	g_render->HScroll_CreateEffect(filename);
	g_render->HScroll_UpdateEffectScale();
	g_render->HScroll_Update();
	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotDrawVScroll(WORD, WORD wID, HWND, BOOL&)
{
	g_isDrawingRippleOn = false;
	g_isDrawingScrollOn = false;
	g_isDrawingVScrollOn = true;
	CString filename;
	if (m_layerPicFileName.GetLength() == 0)
	{
		filename = L"K:\\娱乐\\ico\\wtlpainter\\samples\\2560x1440.jpg";
	}
	else
	{
		filename = m_layerPicFileName;
	}
	g_render->VScroll_CreateEffect(filename);
	g_render->VScroll_UpdateEffectScale();
	g_render->VScroll_Update();
	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotDrawRipple(WORD, WORD wID, HWND, BOOL&)
{
	g_isDrawingRippleOn = true;
	g_isDrawingScrollOn = false;
	g_isDrawingVScrollOn = false;
	CString filename;
	if (m_layerPicFileName.GetLength() == 0)
	{
		filename = L"K:\\娱乐\\ico\\wtlpainter\\samples\\2560x1440.jpg";
	}
	else
	{
		filename = m_layerPicFileName;
	}
	g_render->Ripple_CreateEffect(filename);
	g_render->Ripple_UpdateEffectScale();
	InvalidateWindow();
	return 0;
}

LRESULT ImageEditorHandler::AnnotOpenFile(WORD, WORD wID, HWND, BOOL&)
{
	OpenFile();
	return 0;
}

WINDOWPLACEMENT editorPlacement;
WINDOWPLACEMENT framePlacement;

void ImageEditorHandler::OnMaximize()
{
	CRect rect;

	::GetWindowPlacement(g_frameWork->m_hWnd, &framePlacement);
	//::GetWindowPlacement(m_hWnd, &editorPlacement);
	
	rect.left = -FRAME_PX;
	rect.top = -(RIBBON_PY+STATUS_PY);
	rect.right = 1920 + FRAME_PX;
	rect.bottom = 1080 + 2 * STATUS_PY + FRAME_PY;

	g_bFullScreen = true;
	::MoveWindow(g_frameWork->m_hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	//::MoveWindow(m_hWnd, 0, 0, 1920, 1080, TRUE);
}

void ImageEditorHandler::OffMaximize()
{
	g_bFullScreen = false;
	//::SetWindowPlacement(m_hWnd, &editorPlacement);
	::SetWindowPlacement(g_frameWork->m_hWnd, &framePlacement);
}

LRESULT ImageEditorHandler::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

	lpMMI->ptMaxTrackSize.x = 2 * GetSystemMetrics(SM_CXSCREEN);
	lpMMI->ptMaxTrackSize.y = 2 * GetSystemMetrics(SM_CYSCREEN);
	return 0;
}

ImageOperationType ImageEditorHandler::GetOperationType()
{
	return m_currentDrawingOperationType;
}

void ImageEditorHandler::SVG_CreateSVGItem()
{
	CBLayerPtr bp;
	ComPtr<IImageOperation> svg;
	SharedObject<ImageSVGOperation>::Create(&svg);
	m_currentOperation = svg;

	m_images[m_currentIndex].Image->PushImageOperation(svg);

	ComPtr<IImageLayerOperation> layerOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&layerOperation)))
	{
		CBLayerPtr bp;
		layerOperation->GetLayerEquipment(&bp);

		bp.m_layerFrameBrush = m_solidBrush;
		bp.m_layerIfPasting = false;
		//--------------------------------------------------------------------------------------------------------------------------
		// bp.m_layerPicFileName 仅仅是为了在 LayerCombo 中显示其文件名称
		//--------------------------------------------------------------------------------------------------------------------------
		bp.m_layerPicFileName = m_svgFileName;

		bp.m_rotatableBorderWidth = 339;
		bp.m_rotatableBorderHeight = 339;

		PAST_CalculatePastingMatrix(bp, m_images[m_currentIndex].Image);

		D2D1_POINT_2F mousePosition;
		D2D1_POINT_2F absPoint;
		CRect rect;
		GetClientRect(&rect);
		float scale;

		m_images[m_currentIndex].Image->GetScale(&scale);
		//--------------------------------------------------------------------------------------------------------------------------
		// mousePosition 是模拟鼠标在 ClientRect 中的中心位置
		//--------------------------------------------------------------------------------------------------------------------------
		mousePosition.x = rect.right / 2.0 - bp.m_rotatableBorderWidth / scale / 2.0;
		mousePosition.y = rect.bottom / 2.0 - bp.m_rotatableBorderHeight / scale / 2.0;
		absPoint = GetAbsolutePosition(mousePosition, false);
		bp.m_rotatableRelativePt = bp.m_pastingMatrix.TransformPoint(absPoint);

		previousLayeringPoint = bp.m_rotatableRelativePt;

		layerOperation->SetLayerEquipment(bp);
	}

	ComPtr<IImageSVGOperation> svgOperation;
	if (SUCCEEDED(m_currentOperation.QueryInterface(&svgOperation)))
	{
		CBSVGPtr sp;
		svgOperation->GetSVGEquipment(&sp);

		IStream* pStm;
		FILE* file = _wfopen(m_svgFileName.GetBuffer(100), L"rb");
		if (file == NULL)
		{
			CString message;
			message.Format(L"Error: Can't open file %s", m_svgFileName.GetBuffer(100));
			::MessageBox(NULL, message.GetBuffer(100), L"ERROR", MB_OK);
			return;
		}

		fseek(file, 0, SEEK_END);
		LONG length = ftell(file);
		fseek(file, 0, SEEK_SET);

		HGLOBAL	hGlobal = GlobalAlloc(GMEM_MOVEABLE, length);
		LPVOID		pvData = NULL;
		if (hGlobal != NULL)
		{
			if ((pvData = GlobalLock(hGlobal)) != NULL)
			{
				fread(pvData, 1, length, file);
				GlobalUnlock(hGlobal);
				CreateStreamOnHGlobal(hGlobal, TRUE, &pStm);

				g_pD2DDeviceContext->CreateSvgDocument(
					pStm,
					D2D1::SizeF(339, 339), // Create the document at a size of 500x500 DIPs.
					&sp.m_svgDocument
				);

				tinyxml2::XMLDocument doc;
				if (doc.LoadwFile(m_svgFileName.GetBuffer(100)) != XML_SUCCESS)
				{
					AtlMessageBox(NULL, L"Error in reading svg file in SVGCreate_item.", IDR_MAINFRAME);
					return;
				}

				XMLElement* elle = doc.FirstChildElement("svg");
				int height = (FLOAT)elle->IntAttribute("height", 339);

				sp.m_scaleValue = 339.0f / (float)height;
			}
		}

		fclose(file);
		svgOperation->SetSVGEquipment(sp);
	}

	m_currentOperation->SetDuration(g_animDuration);
}

LRESULT ImageEditorHandler::AnnotSVGLocate(WORD, WORD wID, HWND, BOOL&)
{
	SVG_CreateSVGItem();
	PAST_UnselectLayer();

	CombDlg::COMB_InitList();
	InvalidateWindow();

	SetTimer(DELAY_TIMER, 1000);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
LRESULT ImageEditorHandler::AnnotOpenSVGFile(WORD, WORD wID, HWND, BOOL&)
{
	ShellFileDialog openDialog;

	std::vector<ComPtr<IShellItem> > shellItems;
	openDialog.ShowOpenDialog(&shellItems);

	if (shellItems.size() != 1)
		return 0;

	ImageInfo info(sourceFromFile, shellItems.at(0));

	m_svgFileName = info.fileName.c_str();

	InvalidateWindow();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
// Draws the client area
//--------------------------------------------------------------------------------------------------------------------------
void ImageEditorHandler::DrawClientArea()
{
	g_pD2DDeviceContext->BeginDraw();

	D2D1_MATRIX_3X2_F scale = D2D1::Matrix3x2F::Scale(m_currentZoom, m_currentZoom, GetCenter());
	D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(m_currentPanPoint.x, m_currentPanPoint.y);
	g_pD2DDeviceContext->SetTransform(scale * translation);

	g_pD2DDeviceContext->Clear(D2D1::ColorF(0xffffff));

	if ((int)m_images.size() != 0)
	{
		for (int i = m_currentRangeStart; i < m_currentRangeEnd + 1; i++)
		{
			if (m_animationEnabled)
			{
				DrawAnimatedImages(i);
			}
			else
			{
				DrawImages(i);
			}
		}
	}

	reportError(79, g_pD2DDeviceContext->EndDraw());
	g_render->Present();
}
