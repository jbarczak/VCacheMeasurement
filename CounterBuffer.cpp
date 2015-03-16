
#include "CounterBuffer.h"

void CounterBuffer::Init( ID3D11Device* pDev, UINT nCounters )
{
    ID3D11Buffer* pBuffer=0;
    ID3D11UnorderedAccessView* pUAV=0;
    ID3D11ShaderResourceView* pSRV=0;

    D3D11_BUFFER_DESC bd;
    bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS|D3D11_BIND_SHADER_RESOURCE;
    bd.ByteWidth = sizeof(UINT)*nCounters;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    pDev->CreateBuffer(&bd,0,&pBuffer);

    D3D11_UNORDERED_ACCESS_VIEW_DESC vd;
    vd.Buffer.FirstElement = 0;
    vd.Buffer.NumElements = nCounters;
    vd.Buffer.Flags = 0;
    vd.Format = DXGI_FORMAT_R32_UINT;
    vd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    pDev->CreateUnorderedAccessView( pBuffer, &vd, &pUAV );

    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Buffer.FirstElement = 0;
    srv.Buffer.NumElements = nCounters;
    srv.Format = DXGI_FORMAT_R32_UINT;
    srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    pDev->CreateShaderResourceView(pBuffer,&srv, &pSRV );

    m_pSRV.Owns(pSRV);
    m_pUAV.Owns(pUAV);
    m_pBuffer.Owns(pBuffer);
    this->m_nCounters = nCounters;

}

void CounterBuffer::Clear( ID3D11DeviceContext* pCtx )
{
    UINT zeros[] = {0,0,0,0};
    pCtx->ClearUnorderedAccessViewUint( m_pUAV, zeros );
}


