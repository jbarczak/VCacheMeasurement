
uniform float4x4 mViewProj;
uniform float3 vEye;

RWBuffer<uint> tCounters : register(u1);

struct VSOut
{
    float4 Lighting : TEXCOORD0;    
    float4 x0 : TEXCOORD1;
    float4 x1 : TEXCOORD2;
    float4 x2 : TEXCOORD3;    
    float4 x3 : TEXCOORD4;
    float4 vpos : SV_POSITION;
     
};

VSOut  main( float4 vpos : POSITION, float3 n : NORMAL, uint vid : SV_VertexID ) 
{
    VSOut o;
    o.vpos     = mul( mViewProj,  float4(vpos.xyz,1));
    o.Lighting = saturate( dot( normalize(vEye - vpos.xyz ),n ) );
    o.x0 = 0;
    o.x1 = 0;
    o.x2 = 0;
    o.x3 = 0;
    o.x0.xyz = n;
    o.x1.xyz = 2*n;
    o.x2.xyz = 4*n;
    o.x3.xyz = 8*n;
    InterlockedAdd( tCounters[vid], 1 );

    return o;
}