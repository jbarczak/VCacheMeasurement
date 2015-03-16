

float4 main( float4 l : TEXCOORD0 ) : SV_TARGET
{
    return float4(l.xxx,1);
}