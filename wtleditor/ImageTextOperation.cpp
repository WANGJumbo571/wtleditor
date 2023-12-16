//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================
#include "StdAfx.h"
#include "ImageEditor.h"
#include "ImageTextOperation.h"

using namespace Hilo::Direct2DHelpers;

ImageTextOperation::ImageTextOperation(CBTextPtr bp) 
{
	textInfo = bp;
	textInfo.m_textIsActive = false;
	textInfo.m_textOpacity = 1.0f;
}

ImageTextOperation::~ImageTextOperation()
{
}

HRESULT ImageTextOperation::GetTextEquipment(CBTextPtr * bp)
{
	assert(bp);
	*bp = textInfo;
	return S_OK;
}

HRESULT ImageTextOperation::SetTextEquipment(CBTextPtr bp)
{
	textInfo = bp;
	return S_OK;
}

void ImageTextOperation::SetActive(bool isActive)
{
	textInfo.m_textIsActive = isActive;
}

void ImageTextOperation::SetOpacity(FLOAT opacity)
{
	textInfo.m_textOpacity = opacity;
}

FLOAT ImageTextOperation::GetOpacity()
{
	return textInfo.m_textOpacity;
}

void ImageTextOperation::TEXT_FrameOutText(CBTextPtr & bp)
{
	for (int i = 0; i < (int)bp.m_textLayout.size(); i++)
	{
		bp.m_textLayout.at(i)->m_charLayout = nullptr;
	}
	bp.m_textLayout.clear();

	bp.m_textString.Replace(L"\n", L"\r");

	//if (bp.m_textString.GetLength() > 0 && bp.m_textString.GetAt(0) == L'\r')
	//{
	//	bp.m_textString = bp.m_textString.Right(bp.m_textString.GetLength() - 1);
	//}

	//if (bp.m_textString.GetLength() > 0 && bp.m_textString.GetAt(bp.m_textString.GetLength() - 1) == L'\r')
	//{
	//	bp.m_textString = bp.m_textString.Left(bp.m_textString.GetLength() - 1);
	//}

	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		for (int i = 0; i < bp.m_textString.GetLength(); i++)
		{
			CString singleString = bp.m_textString.GetAt(i);

			ComPtr<IDWriteTextLayout> pTextLayout;
			ComPtr<CBSingleCharLayout> charLayout;
			SharedObject<CBSingleCharLayout>::Create(&charLayout);

			// 创建文本布局 
			hr = g_pDWriteFactory->CreateTextLayout(singleString,
				singleString.GetLength(),
				bp.m_textFormat,
				(FLOAT)200,
				(FLOAT)200,
				&pTextLayout);
			if (SUCCEEDED(hr))
			{
				// 获取文本尺寸  
				DWRITE_TEXT_METRICS textMetrics;
				hr = pTextLayout->GetMetrics(&textMetrics);
				charLayout->m_charActer = bp.m_textString.GetAt(i);
				charLayout->m_charLayout = pTextLayout;
				charLayout->m_charLayoutHeight = textMetrics.height;
				charLayout->m_charLayoutWidth = textMetrics.widthIncludingTrailingWhitespace;
				charLayout->m_charLayoutRect = { 0, 0, 0, 0 };
				bp.m_textLayout.push_back(charLayout);
			}
		}

		CString singleString = L'王';
		ComPtr<IDWriteTextLayout> pTextLayout;

		// 创建文本布局 
		hr = g_pDWriteFactory->CreateTextLayout(singleString,
			singleString.GetLength(),
			bp.m_textFormat,
			(FLOAT)200,
			(FLOAT)200,
			&pTextLayout);
		if (SUCCEEDED(hr))
		{
			// 获取文本尺寸  
			DWRITE_TEXT_METRICS textMetrics;
			hr = pTextLayout->GetMetrics(&textMetrics);
			bp.m_textSingleCharHeight = textMetrics.height;
			bp.m_textSingleCharWidth = textMetrics.width;
		}

		bp.m_textBoxWidth = 0;
		bp.m_textBoxHeight = 0;
		bp.m_textRowStart.clear();
		bp.m_textRowStart.push_back(0);

		switch (bp.m_textStyle)
		{
		case style_f:
		case style_c:
		case style_r:
		{
						FLOAT destX = DISTANCE;
						FLOAT destY = 0;
						for (int i = 0; i < (int)bp.m_textLayout.size(); i++)
						{
							if (bp.m_textLayout.at(i)->m_charActer == '\r')
							{
								destX = DISTANCE;
								destY += (bp.m_textSingleCharHeight + bp.m_textLineSpace);
								bp.m_textRowStart.push_back(i + 1);
								continue;
							}

							if (destX + bp.m_textLayout.at(i)->m_charLayoutWidth + bp.m_textCharSpace > bp.m_rotatableBorderWidth)
							{
								destX = DISTANCE;
								destY += (bp.m_textSingleCharHeight + bp.m_textLineSpace);
								bp.m_textRowStart.push_back(i);
							}
							destX += (bp.m_textLayout.at(i)->m_charLayoutWidth + bp.m_textCharSpace);
							bp.m_textBoxWidth = max(bp.m_textBoxWidth, destX);
						}

						bp.m_textBoxHeight = destY + (bp.m_textSingleCharHeight + bp.m_textLineSpace);
						break;
		}
		case style_b:
		{
						FLOAT destX = bp.m_rotatableBorderWidth - bp.m_textSingleCharWidth - DISTANCE;
						FLOAT destY = 0;
						for (int i = 0; i < (int)bp.m_textLayout.size(); i++)
						{
							if (bp.m_textLayout.at(i)->m_charActer == '\r')
							{
								destX -= (bp.m_textSingleCharWidth + bp.m_textLineSpace);
								destY = 0;
								bp.m_textRowStart.push_back(i + 1);
								continue;
							}

							if (destY + bp.m_textLayout.at(i)->m_charLayoutHeight + bp.m_textCharSpace > bp.m_rotatableBorderHeight)
							{
								destX -= (bp.m_textSingleCharWidth + bp.m_textLineSpace);
								destY = 0;
								bp.m_textRowStart.push_back(i);
							}
							destY += (bp.m_textLayout.at(i)->m_charLayoutHeight + bp.m_textCharSpace);
							bp.m_textBoxHeight = max(bp.m_textBoxHeight, destY);
						}

						bp.m_textBoxWidth = bp.m_rotatableBorderWidth - (destX - (bp.m_textSingleCharWidth + bp.m_textLineSpace));
						break;
		}
		default:
			break;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// 宝贝例程
//--------------------------------------------------------------------------------------------------------------------------
void ImageTextOperation::TEXT_KeyDown_Up(CBTextPtr &bp, int &textPoint)
{
	int rowIndex = (int)bp.m_textRowStart.size() - 1;
	for (int i = 0; i < (int)bp.m_textRowStart.size(); i++)
	{
		if (textPoint < bp.m_textRowStart.at(i))
		{
			rowIndex = i - 1;
			break;
		}
	}
	int lineIndex = textPoint - bp.m_textRowStart.at(rowIndex);
	if (rowIndex > 0)
	{
		int prevLineLength = bp.m_textRowStart.at(rowIndex) - bp.m_textRowStart.at(rowIndex - 1);
		int prevRowIndex = rowIndex - 1;

		int tPoint = bp.m_textRowStart.at(prevRowIndex) + min(lineIndex, prevLineLength);
		if (tPoint > 0 && tPoint != bp.m_textRowStart.at(prevRowIndex) && bp.m_textString.GetAt(tPoint - 1) == '\r')
			tPoint--;
		textPoint = tPoint;
	}
	else
	{
		textPoint = 0;
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// 宝贝例程
//--------------------------------------------------------------------------------------------------------------------------
void ImageTextOperation::TEXT_KeyDown_Down(CBTextPtr &bp, int &textPoint)
{
	int rowIndex = (int)bp.m_textRowStart.size() - 1;
	for (int i = 0; i < (int)bp.m_textRowStart.size(); i++)
	{
		if (textPoint < bp.m_textRowStart.at(i))
		{
			rowIndex = i - 1;
			break;
		}
	}
	int lineIndex = textPoint - bp.m_textRowStart.at(rowIndex);
	int nextLineLength;
	int nextRowIndex;
	if (rowIndex < (int)bp.m_textRowStart.size() - 2)
	{
		nextLineLength = bp.m_textRowStart.at(rowIndex + 2) - bp.m_textRowStart.at(rowIndex + 1);
		nextRowIndex = rowIndex + 1;

		int tPoint = bp.m_textRowStart.at(nextRowIndex) + min(lineIndex, nextLineLength);
		if (tPoint > 0 && tPoint != bp.m_textRowStart.at(nextRowIndex) && bp.m_textString.GetAt(tPoint - 1) == '\r')
			tPoint--;
		textPoint = tPoint;
	}
	else if (rowIndex < (int)bp.m_textRowStart.size() - 1)
	{
		nextLineLength = bp.m_textString.GetLength() - bp.m_textRowStart.at(rowIndex + 1);
		nextRowIndex = rowIndex + 1;

		int tPoint = bp.m_textRowStart.at(nextRowIndex) + min(lineIndex, nextLineLength);
		if (tPoint > 0 && tPoint != bp.m_textRowStart.at(nextRowIndex) && bp.m_textString.GetAt(tPoint - 1) == '\r')
			tPoint--;
		textPoint = tPoint;
	}
	else
	{
		textPoint = bp.m_textString.GetLength();
	}
}

void ImageTextOperation::TEXT_KeyDown_Shift_Right(CBTextPtr &bp)
{
	if (bp.m_textIfMarked == false)
	{
		bp.m_textMarkStart = bp.m_textPoint;
		bp.m_textMarkShift = 0;
	}
	bp.m_textMarkShift++;
	if (bp.m_textMarkStart + bp.m_textMarkShift > bp.m_textString.GetLength())
		bp.m_textMarkShift--;

	bp.m_textMarkLeft = min(bp.m_textMarkStart, bp.m_textMarkStart + bp.m_textMarkShift);
	bp.m_textMarkRight = max(bp.m_textMarkStart, bp.m_textMarkStart + bp.m_textMarkShift);

	if (bp.m_textMarkShift > 0)
		bp.m_textPoint = bp.m_textMarkRight;
	else
		bp.m_textPoint = bp.m_textMarkLeft;
	bp.m_textIfMarked = true;
}

void ImageTextOperation::TEXT_KeyDown_Shift_Left(CBTextPtr &bp)
{
	if (bp.m_textIfMarked == false)
	{
		bp.m_textMarkStart = bp.m_textPoint;
		bp.m_textMarkShift = 0;
	}

	bp.m_textMarkShift--;
	if (bp.m_textMarkStart + bp.m_textMarkShift < 0)
		bp.m_textMarkShift++;

	bp.m_textMarkLeft = min(bp.m_textMarkStart, bp.m_textMarkStart + bp.m_textMarkShift);
	bp.m_textMarkRight = max(bp.m_textMarkStart, bp.m_textMarkStart + bp.m_textMarkShift);

	if (bp.m_textMarkShift > 0)
		bp.m_textPoint = bp.m_textMarkRight;
	else
		bp.m_textPoint = bp.m_textMarkLeft;
	bp.m_textIfMarked = true;
}

void ImageTextOperation::TEXT_KeyDown_Shift_Up(CBTextPtr &bp)
{
	if (bp.m_textIfMarked == false)
	{
		bp.m_textMarkStart = bp.m_textPoint;
		bp.m_textMarkShift = 0;
	}

	int textPoint = bp.m_textMarkStart + bp.m_textMarkShift;
	TEXT_KeyDown_Up(bp, textPoint);
	bp.m_textMarkShift = textPoint - bp.m_textMarkStart;

	bp.m_textMarkLeft = min(bp.m_textMarkStart, bp.m_textMarkStart + bp.m_textMarkShift);
	bp.m_textMarkRight = max(bp.m_textMarkStart, bp.m_textMarkStart + bp.m_textMarkShift);

	if (bp.m_textMarkShift > 0)
		bp.m_textPoint = bp.m_textMarkRight;
	else
		bp.m_textPoint = bp.m_textMarkLeft;
	bp.m_textIfMarked = true;
}

void ImageTextOperation::TEXT_KeyDown_Shift_Down(CBTextPtr &bp)
{
	if (bp.m_textIfMarked == false)
	{
		bp.m_textMarkStart = bp.m_textPoint;
		bp.m_textMarkShift = 0;
	}

	int textPoint = bp.m_textMarkStart + bp.m_textMarkShift;
	TEXT_KeyDown_Down(bp, textPoint);
	bp.m_textMarkShift = textPoint - bp.m_textMarkStart;

	bp.m_textMarkLeft = min(bp.m_textMarkStart, bp.m_textMarkStart + bp.m_textMarkShift);
	bp.m_textMarkRight = max(bp.m_textMarkStart, bp.m_textMarkStart + bp.m_textMarkShift);

	if (bp.m_textMarkShift > 0)
		bp.m_textPoint = bp.m_textMarkRight;
	else
		bp.m_textPoint = bp.m_textMarkLeft;
	bp.m_textIfMarked = true;
}

void ImageTextOperation::TEXT_KeyDown_Left(CBTextPtr &bp)
{
	if (bp.m_textIfMarked == true)
	{
		bp.m_textPoint = bp.m_textMarkLeft;
	}
	else
	{
		bp.m_textPoint--;
		if (bp.m_textPoint < 0)
			bp.m_textPoint = 0;
	}
	bp.m_textIfMarked = false;
}

void ImageTextOperation::TEXT_KeyDown_Right(CBTextPtr &bp)
{
	if (bp.m_textIfMarked == true)
	{
		bp.m_textPoint = bp.m_textMarkRight;
	}
	else
	{
		bp.m_textPoint++;
		if (bp.m_textPoint > bp.m_textString.GetLength())
			bp.m_textPoint = bp.m_textString.GetLength();
	}
	bp.m_textIfMarked = false;
}

void ImageTextOperation::TEXT_KeyDown_Escape(CBTextPtr& bp)
{
	bp.m_textIfMarked = false;
}

void ImageTextOperation::TEXT_KeyDown_Delete(CBTextPtr &bp)
{
	if (bp.m_textIfMarked == true && bp.m_textMarkLeft != bp.m_textMarkRight)
	{
		CString left = bp.m_textString.Left(bp.m_textMarkLeft);
		CString right = bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textMarkRight);
		bp.m_textString = left + right;
		bp.m_textPoint = bp.m_textMarkLeft;
	}
	else
	{
		CString s1 = bp.m_textString.Left(bp.m_textPoint);
		int rlen = bp.m_textString.GetLength() - bp.m_textPoint - 1;
		if (rlen < 0)
			rlen = 0;
		CString s2 = bp.m_textString.Right(rlen);
		bp.m_textString = s1 + s2;
	}
	bp.m_textIfMarked = false;
	TEXT_FrameOutText(bp);
}

void ImageTextOperation::TEXT_KeyDown_Back(CBTextPtr &bp)
{
	if (bp.m_textIfMarked == true && bp.m_textMarkLeft != bp.m_textMarkRight)
	{
		CString left = bp.m_textString.Left(bp.m_textMarkLeft);
		CString right = bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textMarkRight);
		bp.m_textString = left + right;
		bp.m_textPoint = bp.m_textMarkLeft;
	}
	else
	{
		int left = bp.m_textPoint - 1;
		if (left < 0)
			left = 0;

		CString s1 = bp.m_textString.Left(left);
		CString s2 = bp.m_textString.Right(bp.m_textString.GetLength() - bp.m_textPoint);
		bp.m_textString = s1 + s2;
		bp.m_textPoint = left;
	}
	bp.m_textIfMarked = false;
	TEXT_FrameOutText(bp);
}