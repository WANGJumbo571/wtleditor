//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================


#include "stdafx.h"
#include "AnimationUtility2.h"
#include "ImageRenderer.h"
#include <map>
#include <set>

using namespace Hilo::AnimationHelpers;

static std::map<void *, ComPtr<AnimationPackage2> > animationPackages;
static std::set<void *> animationKeys;

AnimationUtility2::AnimationUtility2()
{
}

AnimationUtility2::~AnimationUtility2()
{
}

HRESULT AnimationUtility2::GetAnimationTimer(void * animation, IUIAnimationTimer **animationTimer)
{
	ComPtr<AnimationPackage2> pack = Initialize(animation);
	HRESULT hr = S_OK;
	reportError(156, (hr = AssignToOutputPointer(animationTimer, pack->m_animationTimer)));
	return hr;
}

HRESULT AnimationUtility2::GetAnimationManager(void * animation, IUIAnimationManager **animationManager)
{
	ComPtr<AnimationPackage2> pack = Initialize(animation);
	HRESULT hr = S_OK;
	reportError(157, (hr = AssignToOutputPointer(animationManager, pack->actorManager)));
	return hr;
}

HRESULT AnimationUtility2::GetTransitionLibrary(void * animation, IUIAnimationTransitionLibrary **transitionLibrary)
{
	ComPtr<AnimationPackage2> pack = Initialize(animation);
	HRESULT hr = S_OK;
	reportError(158, (hr = AssignToOutputPointer(transitionLibrary, pack->m_transitionLibrary)));
	return hr;
}

HRESULT AnimationUtility2::GetAnimationTimerTime(void * animation, UI_ANIMATION_SECONDS* animationSeconds)
{
	ComPtr<AnimationPackage2> pack = Initialize(animation);
	HRESULT hr = S_OK;
	reportError(159, (hr = pack->m_animationTimer->GetTime(animationSeconds)));
	return hr;
}

HRESULT AnimationUtility2::UpdateAnimationManagerTime()
{
	UI_ANIMATION_SECONDS secondsNow = 0;

	HRESULT hr = S_OK;

	for (void * key : animationKeys)
	{
		ComPtr<AnimationPackage2> pack = Initialize(key);
		reportError(160, (hr = pack->m_animationTimer->GetTime(&secondsNow)));

		if (SUCCEEDED(hr))
		{
			reportError(161, (hr = pack->actorManager->Update(secondsNow)));
		}
	}
	return hr;
}

ComPtr<AnimationPackage2> AnimationUtility2::Initialize(void * animationKey)
{
	// Create actor Manager
	HRESULT hr = S_OK;

	animationKeys.insert(animationKey);

	if (animationPackages.find(animationKey) != animationPackages.end())
	{
		return animationPackages[animationKey];
	}

	ComPtr<IUIAnimationManager>				actorManager;
	ComPtr<IUIAnimationTimer>					animationTimer;
	ComPtr<IUIAnimationTransitionLibrary>	transitionLibrary;

	if (!actorManager)
	{
		reportError(162, (hr = CoCreateInstance(
			CLSID_UIAnimationManager,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&actorManager))));
	}

	if (SUCCEEDED(hr) && !animationTimer)
	{
		// Create actor Timer
		reportError(163, (hr = CoCreateInstance(
			CLSID_UIAnimationTimer,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&animationTimer))));
	}

	if (SUCCEEDED(hr) && !transitionLibrary)
	{
		// Create actor Transition Library
		reportError(164, (hr = CoCreateInstance(
			CLSID_UIAnimationTransitionLibrary,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&transitionLibrary))));
	}

	ComPtr<AnimationPackage2> package;
	SharedObject<AnimationPackage2>::Create(&package);
	package->actorManager			= actorManager;
	package->m_animationTimer	= animationTimer;
	package->m_transitionLibrary	= transitionLibrary;
	package->key							= animationKey;

	animationPackages[animationKey] = package;
	return package;
}

HRESULT AnimationUtility2::ScheduleStoryboard(void * animation, IUIAnimationStoryboard* storyboard)
{
	UI_ANIMATION_SECONDS secondsNow = static_cast<UI_ANIMATION_SECONDS>(0);

	ComPtr<AnimationPackage2> pack = Initialize(animation);
	HRESULT hr S_OK;
	reportError(165, (hr = pack->m_animationTimer->GetTime(&secondsNow)));

	if (SUCCEEDED(hr))
	{
		reportError(166, (hr = storyboard->Schedule(secondsNow)));
	}

	return hr;
}

HRESULT AnimationUtility2::IsAnimationManagerBusy(void * animation, bool* isBusy)
{
	UI_ANIMATION_MANAGER_STATUS status = UI_ANIMATION_MANAGER_IDLE;

	ComPtr<AnimationPackage2> pack = Initialize(animation);
	HRESULT hr = S_OK;
	reportError(167, (hr = pack->actorManager->GetStatus(&status)));

	if (SUCCEEDED(hr))
	{
		*isBusy = (status == UI_ANIMATION_MANAGER_BUSY);
	}

	return hr;
}

HRESULT AnimationUtility2::IsAnimationManagerBusy(bool* isBusy)
{
	for (void * key : animationKeys)
	{
		ComPtr<AnimationPackage2> pack = Initialize(key);
		bool thisOne = false;
		if (SUCCEEDED(IsAnimationManagerBusy(key, &thisOne))) 
		{
			if (thisOne == true)
			{
				*isBusy = true;
				return S_OK;
			}
		}
	}

	*isBusy = false;
	return S_OK;
}

HRESULT AnimationUtility2::DeleteAnimationPackage(__in void * animation)
{
	animationKeys.erase(animation);
	animationPackages.erase(animation);
	return S_OK;
}

ComPtr<AnimationPackage2> AnimationUtility2::GetAnUnbusyAnimationPackage(void ** animationKey)
{
	for (void * key : animationKeys)
	{
		ComPtr<AnimationPackage2> pack = Initialize(key);
		bool thisOne = false;
		if (SUCCEEDED(IsAnimationManagerBusy(key, &thisOne)))
		{
			if (thisOne == false)
			{
				*animationKey = key;
				return pack;
			}
		}
	}
	*animationKey = nullptr;
	return nullptr;
}

std::vector<ComPtr<AnimationPackage2>> AnimationUtility2::GetVectorOfPackages()
{
	static std::vector<ComPtr<AnimationPackage2>> vec;
	vec.clear();
	for (void * key : animationKeys)
	{
		ComPtr<AnimationPackage2> pack = Initialize(key);
		vec.push_back(pack);
	}
	return vec;
}