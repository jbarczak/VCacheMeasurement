
#ifndef _BUFFER_STREAMER_H_
#define _BUFFER_STREAMER_H_

#include <d3d11.h>
#include "ComPtr.h"
#include "Simpleton.h"
#include <vector>

class BufferStreamer
{
public:
    BufferStreamer();
    ~BufferStreamer();

    bool Init( ID3D11Buffer* pBuffer, uint nStreamBuffers );

    bool QueueReadback( ID3D11DeviceContext* pCtx );
    bool Poll( ID3D11DeviceContext* pCtx );

    const void* GetHostBuffer() { return m_pHostMemory; }

private:

    std::vector<ID3D11Buffer*> m_Pending;
    std::vector<ID3D11Buffer*> m_Available;
    std::vector<Simpleton::ComPtr<ID3D11Buffer>> m_AllBuffers;

    ID3D11Buffer* m_pSourceBuffer;
    void* m_pHostMemory;    ///< Our current working copy
    size_t m_nBufferSize;
};

#endif