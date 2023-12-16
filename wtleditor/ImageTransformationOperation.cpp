//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================
#include "StdAfx.h"
#include "ImageTransformationOperation.h"

ImageTransformationOperation::ImageTransformationOperation(ImageOperationType transformationType) : m_transformationType(transformationType)
{
}

ImageTransformationOperation::~ImageTransformationOperation()
{
}

HRESULT ImageTransformationOperation::GetTransformationMatrix(D2D1_POINT_2F point, D2D1_MATRIX_3X2_F* transform)
{
    if (nullptr == transform)
    {
        return E_POINTER;
    }
    HRESULT hr = S_OK;
    switch (m_transformationType)
    {
    case ImageOperationTypeRotateClockwise:
        *transform = D2D1::Matrix3x2F::Rotation(90, point);
        break;
	case ImageOperationTypeRotate360:
		*transform = D2D1::Matrix3x2F::Rotation(360, point);
		break;
	case ImageOperationTypeRotate30:
		*transform = D2D1::Matrix3x2F::Rotation(30, point);
		break;
	case ImageOperationTypeRotateCounterClockwise:
        *transform = D2D1::Matrix3x2F::Rotation(-90, point);
        break;
    case ImageOperationTypeFlipHorizontal:
        *transform = D2D1::Matrix3x2F::Scale(-1, 1, point);
        break;
    case ImageOperationTypeFlipVertical:
        *transform = D2D1::Matrix3x2F::Scale(1, -1, point);
        break;
	case ImageOperationTypeSizeBigger:
		*transform = D2D1::Matrix3x2F::Scale(1/0.9f, 1/0.9f, point);
		break;
	case ImageOperationTypeSizeSmaller:
		*transform = D2D1::Matrix3x2F::Scale(0.9f, 0.9f, point);
		break;
	default:
        hr = E_UNEXPECTED;
    }
    return hr;
}

HRESULT ImageTransformationOperation::GetTransformationType(ImageOperationType* transformationType)
{
    if (nullptr == transformationType)
    {
        return E_POINTER;
    }
    *transformationType = m_transformationType;
    return S_OK;
}

HRESULT ImageTransformationOperation::GetInverseTransformationMatrix(D2D1_POINT_2F point, D2D1_MATRIX_3X2_F* transform)
{
    if (nullptr == transform)
    {
        return E_POINTER;
    }
    HRESULT hr = S_OK;
    switch (m_transformationType)
    {
    case ImageOperationTypeRotateClockwise:
        *transform = D2D1::Matrix3x2F::Rotation(-90, point);
        break;
	case ImageOperationTypeRotate360:
		*transform = D2D1::Matrix3x2F::Rotation(-360, point);
		break;
	case ImageOperationTypeRotate30:
		*transform = D2D1::Matrix3x2F::Rotation(-30, point);
		break;
	case ImageOperationTypeRotateCounterClockwise:
        *transform = D2D1::Matrix3x2F::Rotation(90, point);
        break;
    case ImageOperationTypeFlipHorizontal:
        *transform = D2D1::Matrix3x2F::Scale(-1, 1, point);
        break;
    case ImageOperationTypeFlipVertical:
        *transform = D2D1::Matrix3x2F::Scale(1, -1, point);
        break;
	case ImageOperationTypeSizeBigger:
		*transform = D2D1::Matrix3x2F::Scale(0.9f, 0.9f, point);
		break;
	case ImageOperationTypeSizeSmaller:
		*transform = D2D1::Matrix3x2F::Scale(1/0.9f, 1/0.9f, point);
		break;
	default:
        hr = E_UNEXPECTED;
    }
    return hr;
}

void ImageTransformationOperation::SetActive(bool isActive)
{
}

void ImageTransformationOperation::SetOpacity(FLOAT opacity)
{
}

FLOAT ImageTransformationOperation::GetOpacity()
{
	return 1.0f;
}
