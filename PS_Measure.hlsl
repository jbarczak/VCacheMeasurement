

#define NUM_ATTRS 1 // NOTE: Must match VS_Measure.hlsl

float4 s0;
float4 s1;
float4 s2;
float4 s3;

float4 main( float4 l : TEXCOORD0,
             float4 ExtraAttrs[NUM_ATTRS] : TEXCOORD1 ) : SV_TARGET
{
    float4 c = l.xyzw;
    
    [unroll]
    for( uint i=0; i<NUM_ATTRS; i++ )
        c.xyzw += ExtraAttrs[i].xyzw*s0;

    return float4(c.xyzw);
}