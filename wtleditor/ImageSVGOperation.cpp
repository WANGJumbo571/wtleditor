//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================
#include "StdAfx.h"
#include "ImageSVGOperation.h"

ImageSVGOperation::ImageSVGOperation()
{
	layerInfo.m_layerIsActive = false;
	layerInfo.m_layerOpacity = 1.0f;
}

ImageSVGOperation::~ImageSVGOperation()
{
}

HRESULT ImageSVGOperation::GetSVGEquipment(CBSVGPtr* bp)
{
	assert(bp);
	*bp = SVGInfo;
	return S_OK;
}

HRESULT ImageSVGOperation::SetSVGEquipment(CBSVGPtr bp)
{
	SVGInfo = bp;
	return S_OK;
}

HRESULT ImageSVGOperation::GetLayerEquipment(CBLayerPtr* bp)
{
	assert(bp);
	*bp = layerInfo;
	return S_OK;
}

HRESULT ImageSVGOperation::SetLayerEquipment(CBLayerPtr bp)
{
	layerInfo = bp;
	return S_OK;
}

void ImageSVGOperation::SetActive(bool isActive)
{
	layerInfo.m_layerIsActive = isActive;
}

void ImageSVGOperation::SetOpacity(FLOAT opacity)
{
	layerInfo.m_layerOpacity = opacity;
}

FLOAT ImageSVGOperation::GetOpacity()
{
	return layerInfo.m_layerOpacity;
}
