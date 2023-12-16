//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================
#include "StdAfx.h"
#include "ImagePicsOperation.h"

ImagePicsOperation::ImagePicsOperation()
{
	layerInfo.m_layerIsActive = false;
	layerInfo.m_layerOpacity = 1.0f;
}

ImagePicsOperation::~ImagePicsOperation()
{
}

HRESULT ImagePicsOperation::GetPicsEquipment(CBPicsPtr* bp)
{
	assert(bp);
	*bp = picsInfo;
	return S_OK;
}

HRESULT ImagePicsOperation::SetPicsEquipment(CBPicsPtr bp)
{
	picsInfo = bp;
	return S_OK;
}

HRESULT ImagePicsOperation::GetLayerEquipment(CBLayerPtr* bp)
{
	assert(bp);
	*bp = layerInfo;
	return S_OK;
}

HRESULT ImagePicsOperation::SetLayerEquipment(CBLayerPtr bp)
{
	layerInfo = bp;
	return S_OK;
}

void ImagePicsOperation::SetActive(bool isActive)
{
	layerInfo.m_layerIsActive = isActive;
}

void ImagePicsOperation::SetOpacity(FLOAT opacity)
{
	layerInfo.m_layerOpacity = opacity;
}

FLOAT ImagePicsOperation::GetOpacity()
{
	return layerInfo.m_layerOpacity;
}
