//===================================================================================
// Copyright (c) Microsoft Corporation.  All rights reserved.                        
//                                                                                   
// THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY                    
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT                       
// LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND                          
// FITNESS FOR A PARTICULAR PURPOSE.                                                 
//===================================================================================


#pragma once

#include "UIAnimation.h"
#include "Animation.h"
#include <vector>

namespace Hilo
{
	namespace AnimationHelpers
	{
		class DX_DECLARE_INTERFACE("43D782E3-A59B-464D-9BBC-2C79391A7F60") AnimationPackage2 : public IUnknown
		{
		public:
			AnimationPackage2() {}
			~AnimationPackage2() {}

			ComPtr<IUIAnimationManager> actorManager;
			ComPtr<IUIAnimationTimer> m_animationTimer;
			ComPtr<IUIAnimationTransitionLibrary> m_transitionLibrary;
			ComPtr<ILayerAnimation> m_animation;
			void * key = nullptr;

			bool QueryInterfaceHelper(const IID &iid, void **object)
			{
				return CastHelper<AnimationPackage2>::CastTo(iid, this, object);
			}
		};

		//
		// This utility class provides several utility functions
		// to work with Window actor manager, including the
		// actor manager, timer and transitions libraty.
		//
		class AnimationUtility2
		{
		private:
			AnimationUtility2();
			virtual ~AnimationUtility2();

		public:
			static HRESULT GetAnimationManager(__in void * animation, __out IUIAnimationManager **animationManager);
			static HRESULT GetAnimationTimer(__in void * animation, __out IUIAnimationTimer **animationTimer);
			static HRESULT GetTransitionLibrary(__in void * animation, __out IUIAnimationTransitionLibrary **transitionLibrary);
			static HRESULT GetAnimationTimerTime(__in void * animation, __out UI_ANIMATION_SECONDS* animationSeconds);
			static HRESULT UpdateAnimationManagerTime();
			static HRESULT ScheduleStoryboard(__in void * animation, IUIAnimationStoryboard* storyboard);
			static HRESULT IsAnimationManagerBusy(__in void * animation, __out bool* isBusy);
			static HRESULT IsAnimationManagerBusy(__out bool* isBusy);
			static HRESULT DeleteAnimationPackage(__in void * animation);
			static ComPtr<AnimationPackage2> GetAnUnbusyAnimationPackage(void ** animationKey);
			static ComPtr<AnimationPackage2> Initialize(__in void * animation);
			static std::vector<ComPtr<AnimationPackage2>> GetVectorOfPackages();
		};
	}
}
