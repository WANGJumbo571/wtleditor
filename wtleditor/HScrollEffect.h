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

// GUIDs for the shader, vertex buffers, and effect itself.
DEFINE_GUID(GUID_WaveTransformVertexShaderH,
            0x65fa2492, 0x9478, 0x4c4d, 0x95, 0x67, 0xf6, 0x96, 0x72, 0x6a, 0x83, 0xa4
            );
DEFINE_GUID(GUID_WaveTransformVertexBufferH,
            0xa9149e76, 0xc708, 0x4b3a, 0xad, 0x59, 0x56, 0x72, 0x3b, 0x65, 0x33, 0x44
            );
DEFINE_GUID(CLSID_CustomWaveEffectH,
            0x7b4c948b, 0xfcf2, 0x4c6b, 0xa0, 0x93, 0xdb, 0x4e, 0x59, 0x99, 0x42, 0x43
            );

enum WAVE_PROP
{
    HWAVE_PROP_OFFSET		= 0,
    HWAVE_PROP_SKEW_X		= 1,
    HWAVE_PROP_SKEW_Y		= 2,
	HWAVE_PROP_XRATIO		= 3,
    HWAVE_PROP_FORCE_DWORD = 0xFFFFFFFF
};

// Our effect contains one transform, which is simply a wrapper around a vertex shader.
// As such, we can simply make the effect itself act as the transform.
class HScrollEffect : public ID2D1EffectImpl, public ID2D1DrawTransform
{
public:
    // Declare effect registration methods.
    static HRESULT Register(_In_ ID2D1Factory1* pFactory);
    static HRESULT __stdcall CreateWaveImpl(_Outptr_ IUnknown** ppEffectImpl);

    // Declare property getter/setter methods.
    float GetWaveOffset() const;
    HRESULT SetWaveOffset(float waveOffset);

    float GetSkewX() const;
    HRESULT SetSkewX(float skew);

    float GetSkewY() const;
    HRESULT SetSkewY(float skew);

	float GetXRatio() const;
	HRESULT SetXRatio(float ratio);

    // Declare ID2D1EffectImpl implementation methods.
    IFACEMETHODIMP Initialize(
        _In_ ID2D1EffectContext* pEffectContext,
        _In_ ID2D1TransformGraph* pTransformGraph
        );

    IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType);

    IFACEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* pTransformGraph)
    {
        return E_NOTIMPL; // There's no need to implement this on effects with static input counts.
    }

    // Declare ID2D1DrawTransform implementation methods.
    IFACEMETHODIMP SetDrawInfo(_In_ ID2D1DrawInfo* pDrawInfo);

    // Declare ID2D1Transform implementation methods.
    IFACEMETHODIMP MapOutputRectToInputRects(
        _In_ const D2D1_RECT_L* pOutputRect,
        _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
        UINT32 inputRectCount
        ) const;

    IFACEMETHODIMP MapInputRectsToOutputRect(
        _In_reads_(inputRectCount) CONST D2D1_RECT_L *pInputRects,
        _In_reads_(inputRectCount) CONST D2D1_RECT_L *pInputOpaqueSubRects,
        UINT32 inputRectCount,
        _Out_ D2D1_RECT_L *pOutputRect,
        _Out_ D2D1_RECT_L *pOutputOpaqueSubRect
        );

    IFACEMETHODIMP MapInvalidRect(
        UINT32 inputIndex,
        D2D1_RECT_L invalidInputRect,
        _Out_ D2D1_RECT_L *pInvalidOutputRect
        ) const;


    // Declare ID2D1TransformNode implementation methods.
    IFACEMETHODIMP_(UINT32) GetInputCount() const { return 1; }

    // Declare IUnknown implementation methods.
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void** ppOutput);

private:
    HScrollEffect();

    inline static float Clamp(float v, float low, float high)
    {
        return (v < low) ? low : (v > high) ? high : v;
    }

    struct Vertex
    {
        float x;
        float y;
    };

    // This is a grid mesh in unit space, 0 to 1, with texture coordinates.
    // Deformations of this geometry will be performed in the vertex shader.
    HRESULT GenerateMesh(_Outptr_ Vertex** ppMesh);

    // Updates the constant buffer when effect properties are changed.
    HRESULT UpdateConstants();

    float                                       m_waveOffset;
    float                                       m_sizeX;
    float                                       m_sizeY;
    float                                       m_skewX;
    float                                       m_skewY;
	float										  m_xRatio;

    ID2D1VertexBuffer *				m_vertexBuffer;
	ID2D1DrawInfo*					m_drawInfo;
    UINT										m_numVertices;
    D2D_RECT_L							m_inputRect;

    LONG									m_refCount;

    // Used to update the constant buffer for the vertex shader.
    struct
    {
        D2D_MATRIX_4X4_F matrix;
        float sizeX;
        float sizeY;
        float waveOffset;
        float skewX;
        float skewY;
		float xRatio;
    } m_constants;
};
