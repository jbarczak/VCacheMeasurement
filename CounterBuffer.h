
#ifndef _COUNTER_BUFFER_H_
#define _COUNTER_BUFFER_H_

#include "Simpleton.h"
#include "ComPtr.h"
#include <d3d11.h>


class CounterBuffer
{
public:

    void Init( ID3D11Device* pDev, UINT nCounters );

    void Clear( ID3D11DeviceContext* pCtx );

    uint GetCounterCount() const { return m_nCounters; }

    ID3D11Buffer* GetBuffer() { return m_pBuffer; }
    ID3D11UnorderedAccessView* GetUAV() { return m_pUAV; }
    ID3D11ShaderResourceView* GetSRV() { return m_pSRV; }

private:

    Simpleton::ComPtr<ID3D11UnorderedAccessView> m_pUAV;
    Simpleton::ComPtr<ID3D11ShaderResourceView> m_pSRV;
    Simpleton::ComPtr<ID3D11Buffer> m_pBuffer;
    uint m_nCounters;
};

#endif