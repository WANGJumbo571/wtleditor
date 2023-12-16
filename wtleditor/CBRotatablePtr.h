#pragma once

#include "stdafx.h"
#include "atlmisc.h"
#include "ImageOperation.h"

//--------------------------------------------------------------------------------------------------------------------------
// Layer 和 Text 共有的图层翻转变换的数据
//--------------------------------------------------------------------------------------------------------------------------
class CBRotatablePtr
{
public:
	CBRotatablePtr() {}
	~CBRotatablePtr() {}

	D2D1_POINT_2F										m_rotatableRelativePt;
	FLOAT														m_rotatableBorderWidth;
	FLOAT														m_rotatableBorderHeight;
	std::vector<ComPtr<IImageOperation>>	m_rotatableOperations;

	//D2D1_POINT_2F										m_rotatableTextMidPoint;
};

//--------------------------------------------------------------------------------------------------------------------------
// Layer 和 Text 的共有的整个图像的前i-1次变换的正反变换矩阵
//--------------------------------------------------------------------------------------------------------------------------
class CBPastingPtr
{
public:
	CBPastingPtr() {}
	~CBPastingPtr() {}

	D2D1::Matrix3x2F m_pastingMatrix;
	D2D1::Matrix3x2F m_pastingReverseMatrix;
};