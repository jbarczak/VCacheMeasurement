

float4 s0;
float4 s1;
float4 s2;
float4 s3;

float4 main( float4 l : TEXCOORD0,
             float4 x0 : TEXCOORD1,
             float4 x1 : TEXCOORD2, 
             float4 x2 : TEXCOORD3,
             float4 x3 : TEXCOORD4) : SV_TARGET
{
    float3 c = (l+x0*s0+x1*s1+x2*s2+x3*s3).xxx;
    return float4(c.xyz,1);
}