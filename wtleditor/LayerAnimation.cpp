#pragma once

#include "stdafx.h"
#include "LayerAnimation.h"
#include "AnimationUtility2.h"
#include "CBRotatablePtr.h"
#include "ImageLayerOperation.h"
#include "ImageTextOperation.h"
#include "ImagePicsOperation.h"
#include "ImageTransformationOperation.h"

using namespace Hilo::AnimationHelpers;
using namespace Hilo::Direct2DHelpers;

void ILayerAnimationRota::Cleanup()
{
	ComPtr<IImageOperation> opera = m_rotaOperation;
	ComPtr<IImageLayerOperation> layerOperation;
	if (SUCCEEDED(opera->QueryInterface(&layerOperation)))
	{
		CBLayerPtr bp;
		layerOperation->GetLayerEquipment(&bp);

		ComPtr<IImageOperation> operation;
		if (SUCCEEDED(SharedObject<ImageTransformationOperation>::Create(m_rotaOperationType, &operation)))
		{
			bp.m_rotatableOperations.push_back(operation);
		}

		layerOperation->SetLayerEquipment(bp);
	}

	ComPtr<IImageTextOperation> textOperation;
	if (SUCCEEDED(opera->QueryInterface(&textOperation)))
	{
		CBTextPtr bp;
		textOperation->GetTextEquipment(&bp);

		ComPtr<IImageOperation> operation;
		if (SUCCEEDED(SharedObject<ImageTransformationOperation>::Create(m_rotaOperationType, &operation)))
		{
			bp.m_rotatableOperations.push_back(operation);
		}

		textOperation->SetTextEquipment(bp);
	}
}

void ILayerAnimationRota::Setup(	ComPtr<IImageOperation> rotaOperation, 
													ImageOperationType rtype, 
													ComPtr<ILayerAnimation> animation)
{
	HRESULT hr = S_OK;
	ComPtr<IUIAnimationManager> animationManager;

	hr = AnimationUtility2::GetAnimationManager(this, &animationManager);

	ComPtr<AnimationPackage2> pack = AnimationUtility2::Initialize(this);
	pack->m_animation = animation;

	m_rotaOperation = rotaOperation;
	m_rotaOperationType = rtype;
	m_rotaAnimationVariable = nullptr;

	if (m_rotaOperationType == ImageOperationTypeFlipHorizontal ||
		m_rotaOperationType == ImageOperationTypeFlipVertical)
	{
		hr = animationManager->CreateAnimationVariable(-1, &m_rotaAnimationVariable);
	}
	else if (	m_rotaOperationType == ImageOperationTypeSizeBigger ||
				m_rotaOperationType == ImageOperationTypeSizeSmaller)
	{
		hr = animationManager->CreateAnimationVariable(1, &m_rotaAnimationVariable);
	}
	else
	{
		hr = animationManager->CreateAnimationVariable(0, &m_rotaAnimationVariable);
	}

	ComPtr<IUIAnimationStoryboard> storyboard;
	ComPtr<IUIAnimationTransition> transition;

	float duration = 1.0f;
	DOUBLE finalValue;
	switch (m_rotaOperationType)
	{
	case ImageOperationTypeFlipHorizontal:
	case ImageOperationTypeFlipVertical:
		finalValue = 1;
		break;
	case ImageOperationTypeRotateClockwise:
		finalValue = 90;
		break;
	case ImageOperationTypeRotate360:
		duration = rotaOperation->GetDuration();
		finalValue = 360;
		break;
	case ImageOperationTypeRotate30:
		finalValue = 30;
		break;
	case ImageOperationTypeSizeBigger:
		finalValue = (double)(1 / 0.9);
		break;
	case ImageOperationTypeSizeSmaller:
		finalValue = (double)0.9;
		break;
	case ImageOperationTypeRotateCounterClockwise:
		finalValue = -90;
		break;
	default:
		finalValue = 45;
		break;
	}

	if (SUCCEEDED(hr))
	{
		hr = animationManager->CreateStoryboard(&storyboard);
	}

	ComPtr<IUIAnimationTransitionLibrary> transitionLibrary;
	hr = AnimationUtility2::GetTransitionLibrary(this, &transitionLibrary);

	if (SUCCEEDED(hr))
	{
		hr = transitionLibrary->CreateLinearTransition(duration, finalValue, &transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = storyboard->AddTransition(m_rotaAnimationVariable, transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = AnimationUtility2::ScheduleStoryboard(this, storyboard);
	}
}

void ILayerAnimationWrite::Setup(	ComPtr<IImageOperation> writeOperation,
														ComPtr<ILayerAnimation> animation,
														FLOAT duration,
														write_style wStyle)
{
	HRESULT hr = S_OK;

	DOUBLE finalValue = 110;
	m_writeStyle = wStyle;

	ComPtr<IImageTextOperation> text;
	if (SUCCEEDED(writeOperation->QueryInterface(&text)))
	{
		CBTextPtr bp;
		text->GetTextEquipment(&bp);
		finalValue = bp.m_textLayout.size() + 2;
	}

	ComPtr<IUIAnimationManager> animationManager;

	hr = AnimationUtility2::GetAnimationManager(this, &animationManager);

	ComPtr<AnimationPackage2> pack = AnimationUtility2::Initialize(this);
	pack->m_animation = animation;

	m_writeOperation = writeOperation;
	m_writeAnimationVariable = nullptr;

	hr = animationManager->CreateAnimationVariable(0, &m_writeAnimationVariable);

	ComPtr<IUIAnimationStoryboard> storyboard;
	ComPtr<IUIAnimationTransition> transition;

	if (SUCCEEDED(hr))
	{
		hr = animationManager->CreateStoryboard(&storyboard);
	}

	ComPtr<IUIAnimationTransitionLibrary> transitionLibrary;
	hr = AnimationUtility2::GetTransitionLibrary(this, &transitionLibrary);

	if (SUCCEEDED(hr))
	{
		hr = transitionLibrary->CreateLinearTransition(duration, finalValue, &transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = storyboard->AddTransition(m_writeAnimationVariable, transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = AnimationUtility2::ScheduleStoryboard(this, storyboard);
	}
}

void ILayerAnimationRollup::Setup(ComPtr<IImageOperation> rollupOperation,
													ComPtr<ILayerAnimation> animation,
													start_from from,
													FLOAT duration)
{
	HRESULT hr = S_OK;
	
	DOUBLE finalValue = 0;
	m_rollupMost = 1;
	m_frame_height = 0;
	text_style style = style_f;

	ComPtr<IImageTextOperation> text;
	if (SUCCEEDED(rollupOperation->QueryInterface(&text)))
	{
		CBTextPtr bp;
		text->GetTextEquipment(&bp);
		style = bp.m_textStyle;
		if (bp.m_textStyle == style_b)
		{
			int size = bp.m_textLayout.size();
			if (size >= 1)
			{
				m_rollupMost =	bp.m_textLayout.at(0)->m_charLayoutRect.right - 
											bp.m_textLayout.at(size - 1)->m_charLayoutRect.left + 
											bp.m_textSingleCharWidth + 
											DISTANCE;
				m_frame_height = bp.m_rotatableBorderWidth;
			}
		}
		else {
			int size = bp.m_textLayout.size();
			if (size >= 1)
			{
				m_rollupMost = 	bp.m_textLayout.at(size - 1)->m_charLayoutRect.bottom - 
											bp.m_textLayout.at(0)->m_charLayoutRect.top + 10;
				m_frame_height = bp.m_rotatableBorderHeight;
			}
		}
	}

	if (from == start_from_low)
	{
		duration *= (m_frame_height + m_rollupMost) / m_rollupMost;
	}

	ComPtr<IUIAnimationManager> animationManager;

	hr = AnimationUtility2::GetAnimationManager(this, &animationManager);

	ComPtr<AnimationPackage2> pack = AnimationUtility2::Initialize(this);
	pack->m_animation = animation;

	m_rollupOperation = rollupOperation;
	m_rollupAnimationVariable = nullptr;

	if (from == start_from_above)
	{
		hr = animationManager->CreateAnimationVariable(0, &m_rollupAnimationVariable);
	}
	else
	{
		if (style == style_b)
		{
			hr = animationManager->CreateAnimationVariable(-m_frame_height, &m_rollupAnimationVariable);
		}
		else
		{
			hr = animationManager->CreateAnimationVariable(m_frame_height, &m_rollupAnimationVariable);
		}
	}

	ComPtr<IUIAnimationStoryboard> storyboard;
	ComPtr<IUIAnimationTransition> transition;

	if (SUCCEEDED(hr))
	{
		hr = animationManager->CreateStoryboard(&storyboard);
	}

	ComPtr<IUIAnimationTransitionLibrary> transitionLibrary;
	hr = AnimationUtility2::GetTransitionLibrary(this, &transitionLibrary);

	if (SUCCEEDED(hr))
	{
		if (style == style_b)
		{
			hr = transitionLibrary->CreateLinearTransition(duration, m_rollupMost, &transition);
		}
		else
		{
			hr = transitionLibrary->CreateLinearTransition(duration, -m_rollupMost, &transition);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = storyboard->AddTransition(m_rollupAnimationVariable, transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = AnimationUtility2::ScheduleStoryboard(this, storyboard);
	}
}

ILayerAnimationPast::ILayerAnimationPast(ComPtr<IImageOperation> pastOperation) : 
																m_pastOperation(pastOperation)
{
}

ILayerAnimationPast::~ILayerAnimationPast()
{
}

//
// Retrieve the current animation values for x and y
//
HRESULT ILayerAnimationPast::GetCurrentPoint(D2D1_POINT_2F* point)
{
	HRESULT hr = S_OK;

	double pointX = 0;
	double pointY = 0;
	double ratio = 0;

	if (nullptr != point)
	{
		hr = m_pointX->GetValue(&pointX);

		if (SUCCEEDED(hr))
		{
			hr = m_pointY->GetValue(&pointY);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_ratio->GetValue(&ratio);
		}

		if (ratio > 0.5)
		{
			pointX = m_targetPoint.x - (pointX - m_targetPoint.x);
			pointY = m_targetPoint.y - (pointY - m_targetPoint.y);
		}

		*point = D2D1::Point2F(static_cast<float>(pointX), static_cast<float>(pointY));
	}

	return hr;
}

//
// Setup a new animation that animation from the current point to the specified target point
//
HRESULT ILayerAnimationPast::Setup(D2D1_POINT_2F initialPoint, 
														D2D1_POINT_2F targetPoint, 
														double duration, 
														ComPtr<ILayerAnimation> animation)
{
	m_initialPoint = initialPoint;
	m_targetPoint = targetPoint;
	m_endPoint.x = initialPoint.x + 2 * (targetPoint.x - initialPoint.x);
	m_endPoint.y = initialPoint.y + 2 * (targetPoint.y - initialPoint.y);
	m_duration = duration;

	// Animation objects
	ComPtr<IUIAnimationManager> animationManager;
	ComPtr<IUIAnimationTransitionLibrary> transitionLibrary;
	ComPtr<IUIAnimationStoryboard> storyboard;

	// Transition objects
	ComPtr<IUIAnimationTransition> pointXTransition;
	ComPtr<IUIAnimationTransition> pointYTransition;
	ComPtr<IUIAnimationTransition> pointRatioTransition;

	// Retrieve animation objects
	HRESULT hr = AnimationUtility2::GetAnimationManager(this, &animationManager);

	ComPtr<AnimationPackage2> pack = AnimationUtility2::Initialize(this);
	pack->m_animation = animation;

	if (SUCCEEDED(hr))
	{
		m_pointX = nullptr;
		hr = animationManager->CreateAnimationVariable(m_initialPoint.x, &m_pointX);
	}

	if (SUCCEEDED(hr))
	{
		m_pointY = nullptr;
		hr = animationManager->CreateAnimationVariable(m_initialPoint.y, &m_pointY);
	}
	
	if (SUCCEEDED(hr))
	{
		m_ratio = nullptr;
		hr = animationManager->CreateAnimationVariable(0, &m_ratio);
	}

	if (SUCCEEDED(hr))
	{
		hr = AnimationUtility2::GetTransitionLibrary(this, &transitionLibrary);
	}

	// Initialize storyboard
	if (SUCCEEDED(hr))
	{
		hr = animationManager->CreateStoryboard(&storyboard);
		
		// Create one transition each coordinate
		if (SUCCEEDED(hr))
		{
			hr = transitionLibrary->CreateLinearTransition(duration, m_endPoint.x, &pointXTransition);
			if (SUCCEEDED(hr))
			{
				hr = storyboard->AddTransition(m_pointX, pointXTransition);
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = transitionLibrary->CreateLinearTransition(duration, m_endPoint.y, &pointYTransition);
			if (SUCCEEDED(hr))
			{
				hr = storyboard->AddTransition(m_pointY, pointYTransition);
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = transitionLibrary->CreateLinearTransition(duration, 1, &pointRatioTransition);
			if (SUCCEEDED(hr))
			{
				hr = storyboard->AddTransition(m_ratio, pointRatioTransition);
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = AnimationUtility2::ScheduleStoryboard(this, storyboard);
		}
	}
	return hr;
}

void ILayerAnimationPics::Setup(ComPtr<IImageOperation> picsOperation,
												ComPtr<ILayerAnimation> animation,
												FLOAT duration)
{
	HRESULT hr = S_OK;

	DOUBLE finalValue = (DOUBLE)duration;
	m_picsAnimationDuration = duration;

	ComPtr<IImagePicsOperation> pics;
	if (SUCCEEDED(picsOperation->QueryInterface(&pics)))
	{
		CBPicsPtr bp;
		pics->GetPicsEquipment(&bp);
		int picsCnt = (int)bp.m_picsFileNames.size();
		FLOAT totalLen = picsCnt * 2.0f;
		if (totalLen > duration)
		{
			m_picsAnimationDuration = totalLen + 10;
			finalValue = (DOUBLE)m_picsAnimationDuration;
		}
	}

	ComPtr<IUIAnimationManager> animationManager;

	hr = AnimationUtility2::GetAnimationManager(this, &animationManager);

	ComPtr<AnimationPackage2> pack = AnimationUtility2::Initialize(this);
	pack->m_animation = animation;

	m_picsOperation = picsOperation;
	m_picsAnimationVariable = nullptr;

	hr = animationManager->CreateAnimationVariable(0, &m_picsAnimationVariable);

	ComPtr<IUIAnimationStoryboard> storyboard;
	ComPtr<IUIAnimationTransition> transition;

	if (SUCCEEDED(hr))
	{
		hr = animationManager->CreateStoryboard(&storyboard);
	}

	ComPtr<IUIAnimationTransitionLibrary> transitionLibrary;
	hr = AnimationUtility2::GetTransitionLibrary(this, &transitionLibrary);

	if (SUCCEEDED(hr))
	{
		hr = transitionLibrary->CreateLinearTransition(finalValue, finalValue, &transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = storyboard->AddTransition(m_picsAnimationVariable, transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = AnimationUtility2::ScheduleStoryboard(this, storyboard);
	}
}

HRESULT ILayerAnimationWWave::Setup(ComPtr<IImageOperation> textOperation, ComPtr<ILayerAnimation> animation)
{
	HRESULT hr = S_OK;
	
	m_textOperation = textOperation;

	ComPtr<IUIAnimationManager> animationManager;

	hr = AnimationUtility2::GetAnimationManager(this, &animationManager);

	ComPtr<AnimationPackage2> pack = AnimationUtility2::Initialize(this);
	pack->m_animation = animation;

	m_anglelinear = nullptr;
	hr = animationManager->CreateAnimationVariable(-30.0f, &m_anglelinear);

	ComPtr<IUIAnimationStoryboard> storyboard;
	ComPtr<IUIAnimationTransition> transition;

	if (SUCCEEDED(hr))
	{
		hr = animationManager->CreateStoryboard(&storyboard);
	}

	ComPtr<IUIAnimationTransitionLibrary> transitionLibrary;
	hr = AnimationUtility2::GetTransitionLibrary(this, &transitionLibrary);

	if (SUCCEEDED(hr))
	{
		hr = transitionLibrary->CreateLinearTransition(2.0f, 90.0f, &transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = storyboard->AddTransition(m_anglelinear, transition);
	}

	if (SUCCEEDED(hr))
	{
		hr = AnimationUtility2::ScheduleStoryboard(this, storyboard);
	}

	return hr;
}

HRESULT ILayerAnimationWWave::GetCurrentAngle(double* angle)
{
	double anglelinear;
	m_anglelinear->GetValue(&anglelinear);
	if (anglelinear < 0.0f)
	{
		*angle = anglelinear + 30.0f;
	}
	else if (anglelinear < 60.0f)
	{
		*angle = 30.0f - anglelinear;
	}
	else 
	{
		*angle = anglelinear - 90.0f;
	}
	return S_OK;
}