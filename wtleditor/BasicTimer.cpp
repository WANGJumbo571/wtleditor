//********************************************************* 
// 
// Copyright (c) Microsoft. All rights reserved. 
// This code is licensed under the MIT License (MIT). 
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY 
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR 
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT. 
// 
//*********************************************************

#include "stdafx.h"
#include "BasicTimer.h"

BasicTimer::BasicTimer()
{
	QueryPerformanceFrequency(&m_frequency);
    Reset();
}

void BasicTimer::Reset()
{
    Update();
    m_startTime = m_currentTime;
    m_total = 0.0f;
    m_delta = 1.0f / 60.0f;
}

void BasicTimer::Update()
{
    if (!QueryPerformanceCounter(&m_currentTime))
    {
		return;
    }

    m_total = static_cast<float>(
        static_cast<double>(m_currentTime.QuadPart-m_startTime.QuadPart) /
        static_cast<double>(m_frequency.QuadPart)
        );

    if (m_lastTime.QuadPart == m_startTime.QuadPart)
    {
        // If the timer was just reset, report a time delta equivalent to 60Hz frame time.
        m_delta = 1.0f / 60.0f;
    }
    else
    {
        m_delta = static_cast<float>(
            static_cast<double>(m_currentTime.QuadPart-m_lastTime.QuadPart) /
            static_cast<double>(m_frequency.QuadPart)
            );
    }

    m_lastTime = m_currentTime;
}
