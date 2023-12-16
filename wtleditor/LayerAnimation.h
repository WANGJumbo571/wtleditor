#pragma once

#include "UIAnimation.h"
#include "Animation.h"
#include "ImageOperation.h"
#include "ComPtr.h"
#include "AnimationUtility2.h"

namespace Hilo
{
	namespace AnimationHelpers
	{
		enum start_from { start_from_above, start_from_low };
		enum write_style { normal, opacity, punch, wave };

		class DX_DECLARE_INTERFACE("F45A0A5F-698A-BF42-A6A0-653FEC496EE6") ILayerAnimationRota : public ILayerAnimation
		{
		public:
			ComPtr<IImageOperation>			m_rotaOperation;
			ImageOperationType					m_rotaOperationType;
			ComPtr<IUIAnimationVariable>	m_rotaAnimationVariable;

			void __stdcall Cleanup();
			void __stdcall Setup(ComPtr<IImageOperation> rotaOperation,
				ImageOperationType rtype,
				ComPtr<ILayerAnimation> animation);

			bool QueryInterfaceHelper(const IID& iid, void** object)
			{
				return CastHelper<ILayerAnimation>::CastTo(iid, this, object) ||
					CastHelper<ILayerAnimationRota>::CastTo(iid, this, object);
			}

		};

		class DX_DECLARE_INTERFACE("F45A6A5F-698A-BF32-A6A0-653BEC4968E7") ILayerAnimationRollup : public ILayerAnimation
		{
		public:
			ComPtr<IImageOperation>			m_rollupOperation;
			ComPtr<IUIAnimationVariable>	m_rollupAnimationVariable;
			FLOAT m_rollupMost = 0;
			FLOAT m_frame_height = 0;

			void __stdcall Setup(ComPtr<IImageOperation> rollupOperation,
				ComPtr<ILayerAnimation> animation,
				start_from from,
				FLOAT duration);

			bool QueryInterfaceHelper(const IID& iid, void** object)
			{
				return CastHelper<ILayerAnimation>::CastTo(iid, this, object) ||
					CastHelper<ILayerAnimationRollup>::CastTo(iid, this, object);
			}

		};

		class DX_DECLARE_INTERFACE("F35A6A5F-698A-BF32-A6B0-653BEC496EE7") ILayerAnimationWrite : public ILayerAnimation
		{
		public:
			ComPtr<IImageOperation>			m_writeOperation;
			ComPtr<IUIAnimationVariable>	m_writeAnimationVariable;
			write_style										m_writeStyle;

			void __stdcall Setup(ComPtr<IImageOperation> writeOperation,
				ComPtr<ILayerAnimation> animation,
				FLOAT duration,
				write_style wStyle);

			bool QueryInterfaceHelper(const IID& iid, void** object)
			{
				return CastHelper<ILayerAnimation>::CastTo(iid, this, object) ||
					CastHelper<ILayerAnimationWrite>::CastTo(iid, this, object);
			}

		};


		class DX_DECLARE_INTERFACE("F45A0A3F-698A-BF42-A6A1-653FEB492EB6") ILayerAnimationPast : public ILayerAnimation
		{
		public:
			ILayerAnimationPast(ComPtr<IImageOperation> pastOperation);
			virtual ~ILayerAnimationPast();

			ComPtr<IImageOperation>			m_pastOperation;

			HRESULT __stdcall GetCurrentPoint(__out D2D1_POINT_2F* point);
			HRESULT __stdcall Setup(D2D1_POINT_2F initialPoint, D2D1_POINT_2F targetPoint, double duration, ComPtr<ILayerAnimation> animation);

			bool QueryInterfaceHelper(const IID& iid, void** object)
			{
				return
					CastHelper<ILayerAnimation>::CastTo(iid, this, object) ||
					CastHelper<ILayerAnimationPast>::CastTo(iid, this, object);
			}

		private:
			D2D1_POINT_2F	m_initialPoint;
			D2D1_POINT_2F	m_targetPoint;
			D2D1_POINT_2F   m_endPoint;
			double					m_duration;

			// Animation variables
			ComPtr<IUIAnimationVariable> m_pointX;
			ComPtr<IUIAnimationVariable> m_pointY;
			ComPtr<IUIAnimationVariable> m_ratio;
		};

		class DX_DECLARE_INTERFACE("F33A6A5F-698A-BF38-A6B0-653BEC495EE7") ILayerAnimationPics : public ILayerAnimation
		{
		public:
			ComPtr<IImageOperation>			m_picsOperation;
			ComPtr<IUIAnimationVariable>	m_picsAnimationVariable;
			FLOAT											m_picsAnimationDuration;

			void __stdcall Setup(ComPtr<IImageOperation> picsOperation,
				ComPtr<ILayerAnimation> animation,
				FLOAT duration);

			bool QueryInterfaceHelper(const IID& iid, void** object)
			{
				return CastHelper<ILayerAnimation>::CastTo(iid, this, object) ||
					CastHelper<ILayerAnimationPics>::CastTo(iid, this, object);
			}
		};

		class DX_DECLARE_INTERFACE("F45A0A6F-698A-BF42-A6A1-353FEB992EB6") ILayerAnimationWWave : public ILayerAnimation
		{
		public:
			ComPtr<IImageOperation>			m_textOperation;

			HRESULT __stdcall GetCurrentAngle(__out double * angle);
			HRESULT __stdcall Setup(ComPtr<IImageOperation> textOperation, ComPtr<ILayerAnimation> animation);

			bool QueryInterfaceHelper(const IID & iid, void** object)
			{
				return
					CastHelper<ILayerAnimation>::CastTo(iid, this, object) ||
					CastHelper<ILayerAnimationWWave>::CastTo(iid, this, object);
			}

		private:
			// Animation variables
			ComPtr<IUIAnimationVariable> m_anglelinear;
		};
	}
}