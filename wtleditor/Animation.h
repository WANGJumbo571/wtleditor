//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================

#pragma once

//#include "stdafx.h"

class DX_DECLARE_INTERFACE("F45A0BEF-398A-4F42-A3A0-653FED496EE7") ILayerAnimation : public IUnknown
{
public:
	ILayerAnimation() {}
	~ILayerAnimation() {}

	bool QueryInterfaceHelper(const IID &iid, void **object)
	{
		return CastHelper<ILayerAnimation>::CastTo(iid, this, object);
	}

};

__interface DX_DECLARE_INTERFACE("F45A0AEF-698A-4F42-A6A0-653FED496EE6") IPointAnimation : public IUnknown
{
public:
    HRESULT __stdcall GetCurrentPoint(__out D2D1_POINT_2F* point);
    HRESULT __stdcall Setup(D2D1_POINT_2F targetPoint, double duration);
};

