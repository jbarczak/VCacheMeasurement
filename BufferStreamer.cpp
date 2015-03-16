
#include "BufferStreamer.h"

BufferStreamer::BufferStreamer() : m_pHostMemory(0)
{
}

BufferStreamer::~BufferStreamer()
{
    _aligned_free(m_pHostMemory);
    m_pHostMemory=0;
}

bool BufferStreamer::Init( ID3D11Buffer* pBuffer, uint nStreamBuffers )
{
    D3D11_BUFFER_DESC bd;
    pBuffer->GetDesc(&bd);
    bd.BindFlags = 0;
    bd.Usage = D3D11_USAGE_STAGING;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ID3D11Device* pDev=0;
    pBuffer->GetDevice(&pDev);

    Simpleton::ComPtr<ID3D11Device> pManaged(pDev);

    for( uint i=0; i<nStreamBuffers; i++ )
    {
        ID3D11Buffer* pBuffer;
        if( FAILED(pDev->CreateBuffer(&bd,0,&pBuffer)) )
            goto fail;

        m_Available.push_back(pBuffer);
        m_AllBuffers.push_back(pBuffer);
    }

    m_Pending.reserve(m_Available.size());
    m_pSourceBuffer = pBuffer;
    m_AllBuffers.push_back(pBuffer);

    m_pHostMemory = _aligned_malloc( bd.ByteWidth, 16 );
    m_nBufferSize = bd.ByteWidth;
    return true;

fail:
    m_Available.clear();
    m_Pending.clear();
    m_AllBuffers.clear();
    return false;
}

bool BufferStreamer::QueueReadback(  ID3D11DeviceContext* pCtx )
{
    // queue a new staging buffer if one is available
    if( !m_Available.empty() )
    {
        ID3D11Buffer* pBuf = m_Available.back();
        m_Available.pop_back();
        pCtx->CopyResource( pBuf, m_pSourceBuffer );
        m_Pending.push_back(pBuf);
        return true;
    }

    return false;
}

bool BufferStreamer::Poll( ID3D11DeviceContext* pCtx )
{
    // purge pending staging buffers
    uint nPending=0;
    for( uint i=0; i<m_Pending.size(); i++ )
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = pCtx->Map( m_Pending[i],0,D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &mapped );
        if( hr == S_OK )
        {
            memcpy( m_pHostMemory, mapped.pData, m_nBufferSize );
            pCtx->Unmap( m_Pending[i], 0 );
            m_Available.push_back(m_Pending[i]);
        }
        else
        {
            m_Pending[nPending++] = m_Pending[i];
        }
    }
    uint nOldPending = m_Pending.size();
    m_Pending.resize(nPending);
    return nOldPending != nPending;
}


