
#define _CRT_SECURE_NO_WARNINGS

#include "Simpleton.h"
#include "DX11/SimpletonDX11.h"
#include "PlyLoader.h"

#include "ShaderAutoGen/PS_Measure.h"
#include "ShaderAutoGen/VS_Measure.h"
#include "ShaderAutoGen/PS_Show.h"
#include "ShaderAutoGen/VS_Show.h"

#include "CounterBuffer.h"
#include "BufferStreamer.h"


float MeasureACMR_FIFO( uint nCacheSize, const uint* pIndices, uint nIndices )
{
    uint* pCache = new uint[nCacheSize];
    for( uint i=0; i<nCacheSize; i++ )
        pCache[i] = 0xffffffff;

    uint nMissCount=0;
    for( uint i=0; i<nIndices; i++ )
    {
        uint idx = pIndices[i];
        uint c=0;
        while( c < nCacheSize && pCache[c] != idx )
            c++;

        if( c < nCacheSize )
            continue; // hit
        
        // miss
        pCache[nMissCount%nCacheSize] = idx;
        nMissCount++;
    }

    delete[] pCache;
    return nMissCount / (float)(nIndices/3);
}

float MeasureACMR_LRU( uint nCacheSize, const uint* pIndices, uint nIndices )
{
    uint* pCache = new uint[nCacheSize];
    for( uint i=0; i<nCacheSize; i++ )
        pCache[i] = 0xffffffff;

    uint nMissCount=0;
    for( uint i=0; i<nIndices; i++ )
    {
        uint idx = pIndices[i];
        uint c=0;
        while( c < nCacheSize && pCache[c] != idx )
            c++;

        if( c < nCacheSize )
        {
            // hit.  Move this one to the front and shift others back
            for( uint k=c+1; k < nCacheSize; k++ )
                pCache[k-1] = pCache[k];
            pCache[nCacheSize-1] = idx;
        }
        else
        {
           for( uint k=1; k < nCacheSize; k++ )
                pCache[k-1] = pCache[k];
            pCache[nCacheSize-1] = idx;
            nMissCount++;
        }
        
    }

    delete[] pCache;
    return nMissCount / (float)(nIndices/3);
}




class VCacheWindow : public Simpleton::DX11WindowController
{
public:
    Simpleton::DX11Mesh    m_Mesh;
    Simpleton::DX11Texture m_VertexValence;

    Simpleton::DX11PipelineState m_MeasurePSO;
    Simpleton::DX11PipelineState m_ShowPSO;
    Simpleton::DX11PipelineStateBuilder m_PSOBuilder;
    Simpleton::DX11PipelineResourceSet m_MeasureResources;
    Simpleton::DX11PipelineResourceSet m_ShowResources;

    
    CounterBuffer m_CounterBuff;
    BufferStreamer m_Streamer;

    virtual bool OnCreate( Simpleton::DX11Window* pWindow ) 
    {
        Simpleton::PlyMesh mesh;
        Simpleton::LoadPly( "dragon_vrip.ply", mesh, 
                            Simpleton::PF_STANDARDIZE_POSITIONS|
                            Simpleton::PF_IGNORE_COLORS|
                            Simpleton::PF_IGNORE_UVS|
                            Simpleton::PF_REQUIRE_NORMALS);

        m_Mesh.InitFromPly( pWindow->GetDevice(),mesh );

        //for( uint i=48; i<64; i++ )
        {
            float f0 = MeasureACMR_FIFO(128,mesh.pVertexIndices, mesh.nTriangles*3 );
            printf("Cachesize: %u.  FIFO acmr: %f \n", 128,f0);
        }

        printf("ideal acmr: %f\n", mesh.nVertices / (float)mesh.nTriangles );
       
        uint* pValence = new uint[mesh.nVertices];
        memset(pValence,0,sizeof(uint)*mesh.nVertices);
        for( uint i=0; i<mesh.nTriangles*3; i++ )
            pValence[ mesh.pVertexIndices[i]]++;

        
        Simpleton::FreePly( mesh );
        
        m_VertexValence.InitTBuffer( pWindow->GetDevice(), DXGI_FORMAT_R32_UINT, pValence, mesh.nVertices );

        delete[] pValence;

        m_CounterBuff.Init( pWindow->GetDevice(), mesh.nVertices );
        m_Streamer.Init( m_CounterBuff.GetBuffer(), 3 );

        m_PSOBuilder.BeginState(pWindow->GetDevice());
        m_PSOBuilder.SetInputLayout( m_Mesh.GetVertexElements(), m_Mesh.GetVertexElementCount() );
        m_PSOBuilder.SetPixelShader( PS_Measure, sizeof(PS_Measure) );
        m_PSOBuilder.SetVertexShader( VS_Measure, sizeof(VS_Measure));
        m_PSOBuilder.SetCullMode( D3D11_CULL_NONE );
        m_PSOBuilder.EndState( &m_MeasurePSO );

        m_PSOBuilder.BeginState(pWindow->GetDevice());
        m_PSOBuilder.SetInputLayout( m_Mesh.GetVertexElements(), m_Mesh.GetVertexElementCount() );
        m_PSOBuilder.SetPixelShader( PS_Show, sizeof(PS_Show) );
        m_PSOBuilder.SetVertexShader( VS_Show, sizeof(VS_Show));
        m_PSOBuilder.SetCullMode( D3D11_CULL_NONE );
        m_PSOBuilder.EndState( &m_ShowPSO );


        m_MeasurePSO.GetResourceSchema()->CreateResourceSet( &m_MeasureResources, pWindow->GetDevice() );
        m_ShowPSO.GetResourceSchema()->CreateResourceSet( &m_ShowResources, pWindow->GetDevice() );

        return true;
    }

    virtual void OnFrame( Simpleton::DX11Window* pWindow )
    {
        float black[4] = {0.2,0.2,0.2,1};
        ID3D11Device* pDev = pWindow->GetDevice();
        ID3D11DeviceContext* pCtx = pWindow->GetDeviceContext();
        ID3D11RenderTargetView* pRTV = pWindow->GetBackbufferRTV();

        D3D11_VIEWPORT vp = pWindow->BuildViewport();
        D3D11_RECT scissor = pWindow->BuildScissorRect();

        pCtx->RSSetViewports(1,&vp);
        pCtx->RSSetScissorRects(1,&scissor);
        pCtx->ClearRenderTargetView( pWindow->GetBackbufferRTV(), black );
        pCtx->ClearDepthStencilView( pWindow->GetBackbufferDSV(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
        
        Simpleton::Vec3f eye(0,0.5,-5);
        Simpleton::Matrix4f mView = Simpleton::MatrixLookAtLH( eye, Simpleton::Vec3f(0,0.5,0), Simpleton::Vec3f(0,1,0) );
        Simpleton::Matrix4f mProj = Simpleton::MatrixOrthoLH( 1,1,1,1000 );

        Simpleton::Matrix4f viewProj = mProj * mView;
        
        m_MeasureResources.BeginUpdate(pCtx);
        m_MeasureResources.BindConstant( "mViewProj", &viewProj, sizeof(viewProj));
        m_MeasureResources.BindConstant( "vEye", &eye, sizeof(eye));
        m_MeasureResources.EndUpdate(pCtx);

        m_ShowResources.BeginUpdate(pCtx);
        m_ShowResources.BindConstant( "mViewProj", &viewProj, sizeof(viewProj));
        m_ShowResources.BindConstant( "vEye", &eye, sizeof(eye));
        m_ShowResources.BindSRV( "Valences", m_VertexValence.GetSRV() );
        m_ShowResources.BindSRV( "VSCounts", m_CounterBuff.GetSRV() );
        m_ShowResources.EndUpdate(pCtx);

        ID3D11UnorderedAccessView* pUAV = m_CounterBuff.GetUAV();
        uint zeros[] = {0};
        pCtx->OMSetRenderTargetsAndUnorderedAccessViews( 1, &pRTV, pWindow->GetBackbufferDSV(), 1, 1, &pUAV, zeros ); 
        m_CounterBuff.Clear( pCtx );

        m_MeasurePSO.Apply(pCtx);
        m_MeasureResources.Apply(pCtx);
        m_Mesh.Draw(pCtx);

        pCtx->OMSetRenderTargets( 1, &pRTV, pWindow->GetBackbufferDSV() ); 
        pCtx->ClearRenderTargetView( pWindow->GetBackbufferRTV(), black );
        pCtx->ClearDepthStencilView( pWindow->GetBackbufferDSV(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);
        
        m_ShowPSO.Apply(pCtx);
        m_ShowResources.Apply(pCtx);
        m_Mesh.Draw(pCtx);

        m_Streamer.QueueReadback(pCtx);
        if( m_Streamer.Poll(pCtx) )
        {
            const uint* pCounts = (const uint*) m_Streamer.GetHostBuffer();
            
            uint nVerts = m_CounterBuff.GetCounterCount();
            uint nSum=0;
            for( uint i=0; i<nVerts; i++ )
                nSum += pCounts[i];
            
            float acmr = nSum / (float)(m_Mesh.GetIndexCount()/3);

            char text[2048];
            sprintf( text, "acmr: %f", acmr);
            pWindow->SetCaption( text );
        }
    }
};

int main()
{
    VCacheWindow wc;
    Simpleton::DX11Window* pWin = Simpleton::DX11Window::Create( 512,512,Simpleton::DX11Window::USE_DEBUG_LAYER,&wc);
    while( pWin->DoEvents() )
        pWin->DoFrame();

    return 0;
}