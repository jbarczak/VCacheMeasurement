
uniform float4x4 mViewProj;
uniform float3 vEye;

Buffer<uint> Valences;
Buffer<uint> VSCounts;

struct VSOut
{
    float3 Lighting : TEXCOORD0;
    float4 vpos : SV_POSITION;

};

VSOut  main( float4 vpos : POSITION, float3 n : NORMAL, uint vid : SV_VertexID ) 
{
    uint nTimesProcessed = VSCounts[vid];
    uint nValence = Valences[vid];

    float fCost = (float)(nTimesProcessed-1) / ((float)nValence-1);

    VSOut o;
    o.vpos     = mul( mViewProj,  float4(vpos.xyz,1));
    o.Lighting = fCost*saturate( dot( normalize(vEye - vpos.xyz ),n ) );



    return o;
}