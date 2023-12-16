//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================

#pragma once

enum ImageOperationType
{
    ImageOperationTypeNone = 0,
	ImageOperationTypeText,
	ImageOperationTypeLayer,
	ImageOperationTypeCopy,
	ImageOperationTypePen,
    ImageOperationTypeCrop,
	ImageOperationTypeDummy,
    ImageOperationTypeRotateClockwise,
    ImageOperationTypeRotate360,
	ImageOperationTypeRotate30,
	ImageOperationTypeRotateCounterClockwise,
    ImageOperationTypeFlipHorizontal,
    ImageOperationTypeFlipVertical,
	ImageOperationTypeSizeBigger,
	ImageOperationTypeSizeSmaller
};

__interface DX_DECLARE_INTERFACE("8D4D6824-63E7-4BC9-A8E8-15ED1FD63F0E") IImageOperation : public IUnknown
{
	void		SetActive(bool isActive);
	bool		GetActive();

	void		SetOpacity(FLOAT opacity);
	FLOAT	GetOpacity();

	void		SetDuration(FLOAT duration);
	FLOAT	GetDuration();
};
