//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================
#include "StdAfx.h"
#include "ImageLayerOperation.h"

ImageLayerOperation::ImageLayerOperation(CBLayerPtr bp)
{
	layerInfo = bp;
	layerInfo.m_layerIsActive = false;
	layerInfo.m_layerOpacity = 1.0f;
}

ImageLayerOperation::~ImageLayerOperation()
{
}

HRESULT ImageLayerOperation::GetLayerEquipment(CBLayerPtr * bp)
{
	assert(bp);
	*bp = layerInfo;
	return S_OK;
}

HRESULT ImageLayerOperation::SetLayerEquipment(CBLayerPtr bp)
{
	layerInfo = bp;
	return S_OK;
}

void ImageLayerOperation::SetActive(bool isActive)
{
	layerInfo.m_layerIsActive = isActive;
}

void ImageLayerOperation::SetOpacity(FLOAT opacity)
{
	layerInfo.m_layerOpacity = opacity;
}

FLOAT ImageLayerOperation::GetOpacity()
{
	return layerInfo.m_layerOpacity;
}

ImageDummyOperation::ImageDummyOperation()
{
}

ImageDummyOperation::~ImageDummyOperation()
{
}

void ImageDummyOperation::SetActive(bool isActive)
{
}

void ImageDummyOperation::SetOpacity(FLOAT opacity)
{
}

FLOAT ImageDummyOperation::GetOpacity()
{
	return 1.0f;
}