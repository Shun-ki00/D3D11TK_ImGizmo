

struct MatrixPair
{
    float4x4 Matrix1;
    float4x4 Matrix2;
};

StructuredBuffer<MatrixPair> Input : register(t0);

// 出力：掛け算結果1つの行列
RWStructuredBuffer<float4x4> Output : register(u0);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{ 
    
    // 範囲チェックを追加：ビューが大きくてもアクセスしないようにする
    if (DTid.x >= 10000)
        return;
    
    uint i = DTid.x;
    Output[i] = mul(Input[i].Matrix1, Input[i].Matrix2);
    
}