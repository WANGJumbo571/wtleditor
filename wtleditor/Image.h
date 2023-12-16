//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================

#pragma once

#include "stdafx.h"
#include "ImageOperation.h"
#include "ImageLayerOperation.h"
#include "ImageTextOperation.h"

typedef enum { sourceFromFile, sourceFromNew, sourceFromClipboard } new_source_type;

//--------------------------------------------------------------------------------------------------------------------------
// Provides basic information about a given image such as corresponding IShellItem, filename, and title
//--------------------------------------------------------------------------------------------------------------------------
struct ImageInfo
{
public:
	new_source_type source_type;
	
	//--------------------------------------------------------------------------------------------------------------------------
	// Self added data
	//--------------------------------------------------------------------------------------------------------------------------
	D2D1_SIZE_F			bkgSize;
	D2D1_COLOR_F	bkgColor;

    ComPtr<IShellItem> shellItem;
    std::wstring title;
    std::wstring fileName;
    std::wstring backupFileName;

	ComPtr<IWICBitmapSource> wicSourceClipBitmap;

	ImageInfo(new_source_type source_type = sourceFromFile, IShellItem* shellItemPtr = nullptr) : shellItem(shellItemPtr), source_type(source_type )
	{
		if (source_type == sourceFromFile)
		{
			if (shellItemPtr != nullptr)
			{
				wchar_t* name;

				HRESULT hr = shellItemPtr->GetDisplayName(SIGDN_FILESYSPATH, &name);
				if (SUCCEEDED(hr))
				{
					fileName = name;
					title = name;
					::CoTaskMemFree(name);
				}

				hr = shellItemPtr->GetDisplayName(SIGDN_NORMALDISPLAY, &name);
				if (SUCCEEDED(hr))
				{
					title = name;
					::CoTaskMemFree(name);
				}
			}
		}
	}
};

//--------------------------------------------------------------------------------------------------------------------------
// Collection of Direct2D resources used in drawing thumbnails
//--------------------------------------------------------------------------------------------------------------------------
struct RenderingParameters
{
    ID2D1SolidColorBrush * solidBrush;
};

//--------------------------------------------------------------------------------------------------------------------------
/// <summary>
/// The interface representing a thumbnail control
/// </summary>
//--------------------------------------------------------------------------------------------------------------------------

__interface DX_DECLARE_INTERFACE("B8DE25F6-70CF-4ECA-B9B8-24E9100DDF4A") IImage : IUnknown
{
    HRESULT __stdcall Draw(bool ifcenter);
    HRESULT __stdcall Load();
	
	//--------------------------------------------------------------------------------------------------------------------------
	// Self added functions
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall New();
	HRESULT __stdcall NewFromClipboard();

	//--------------------------------------------------------------------------------------------------------------------------
	// Getters and setters
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall GetDrawingRect(__out D2D1_RECT_F* rect);
    HRESULT __stdcall SetDrawingRect(__in const D2D1_RECT_F& rect);
    HRESULT __stdcall GetTransformedRect(D2D1_POINT_2F midPoint, __out D2D1_RECT_F* rect);
    HRESULT __stdcall GetImageInfo(__out ImageInfo* info);
    HRESULT __stdcall SetBoundingRect(__in const D2D1_RECT_F& rect);
    HRESULT __stdcall SetRenderingParameters(__in const RenderingParameters& drawingObjects);
    HRESULT __stdcall GetScale(__out float* scale);
    HRESULT __stdcall GetClipRect(__out D2D1_RECT_F* rect);
    
    HRESULT __stdcall ContainsPoint(__in D2D1_POINT_2F point, __out bool* doesImageContainPoint);
    HRESULT __stdcall TranslateToAbsolutePoint(__in D2D1_POINT_2F point, __out D2D1_POINT_2F *translatedPoint);

    HRESULT __stdcall CanUndo(__out bool* canUndo);
    HRESULT __stdcall CanRedo(__out bool* canRedo);
    HRESULT __stdcall UndoImageOperation();
    HRESULT __stdcall RedoImageOperation();
    HRESULT __stdcall PushImageOperation(__in IImageOperation* imageOperation);
    HRESULT __stdcall DiscardResources();

	//--------------------------------------------------------------------------------------------------------------------------
	// self added functions.
	//--------------------------------------------------------------------------------------------------------------------------
	HRESULT __stdcall GetPastingTransformation(__out D2D1::Matrix3x2F* matrix);
	HRESULT __stdcall GetPastingReverseTransformation(D2D1::Matrix3x2F* matrix);

	D2D1::Matrix3x2F GetRotatableTransformation(int i, CBRotatablePtr& bp, D2D1_POINT_2F midPoint);
	D2D1::Matrix3x2F ROTA_GetNormalTransformations(CBRotatablePtr& bp, D2D1_POINT_2F midPoint);
	D2D1::Matrix3x2F ROTA_GetInverseTransformations(CBRotatablePtr& bp, D2D1_POINT_2F midPoint);

	int GetIndexOfOperation(ComPtr<IImageOperation> operation);
	int GetIndexOfText(ComPtr<IImageTextOperation> textOperation);
	int GetIndexOfLayer(ComPtr<IImageLayerOperation> layerOperation);

	D2D1_RECT_F		__stdcall GetDrawingRect();
	D2D1_RECT_F		__stdcall GetClipRect();
	D2D1::Matrix3x2F	__stdcall GetNormalTransformationsFromTo(D2D1_POINT_2F midPoint, int from, int to);
	D2D1_POINT_2F	__stdcall CLBD_GetCopyPoint(__in D2D1_POINT_2F mousePosition);
	ComPtr<IWICBitmapSource> __stdcall GetWICBitmap();

	ComPtr<IImageOperation> __stdcall GetTopOperation();
	int		__stdcall GetOperationsSize();
	ComPtr<IImageOperation> __stdcall Get_ith_Operation(int i);
	bool		__stdcall GetIfHorizontal();
	void		__stdcall DeleteOperation(ComPtr<IImageOperation> oper);
};
