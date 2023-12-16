#pragma once

#include "stdafx.h"
#include "atlmisc.h"
#include "ImageOperation.h"

//--------------------------------------------------------------------------------------------------------------------------
// Layer �� Text ���е�ͼ�㷭ת�任������
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
// Layer �� Text �Ĺ��е�����ͼ���ǰi-1�α任�������任����
//--------------------------------------------------------------------------------------------------------------------------
class CBPastingPtr
{
public:
	CBPastingPtr() {}
	~CBPastingPtr() {}

	D2D1::Matrix3x2F m_pastingMatrix;
	D2D1::Matrix3x2F m_pastingReverseMatrix;
};