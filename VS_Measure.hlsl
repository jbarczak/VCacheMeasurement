
uniform float4x4 mViewProj;
uniform float3 vEye;

RWBuffer<uint> tCounters : register(u1);

#define NUM_ATTRS 1 // NOTE: Must match PS_Measure.hlsl

struct VSOut
{
    float4 Lighting : TEXCOORD0;    
    float4 ExtraAttrs[NUM_ATTRS] : TEXCOORD1;
    float4 vpos : SV_POSITION;
     
};

VSOut  main( float4 vpos : POSITION, float3 n : NORMAL, uint vid : SV_VertexID ) 
{
    VSOut o;
    o.vpos     = mul( mViewProj,  float4(vpos.xyz,1));
    o.Lighting = saturate( dot( normalize(vEye - vpos.xyz ),n ) );

    // Increment this vert's invocation count
    InterlockedAdd( tCounters[vid], 1 );

    // generate spurious extra attributes
    [unroll]
    for( uint i=0; i<NUM_ATTRS; i++ )
        o.ExtraAttrs[i] = float4(i*vpos);

    
    return o;
}