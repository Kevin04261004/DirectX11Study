#include <iostream>
#include <DirectXMath.h>

void printXMVector(DirectX::XMVECTOR in);

int main(void)
{
	using namespace DirectX;
	XMVECTOR q = XMVectorSet(0.221, 0.221, 0.221, 0.924);
	printXMVector(q);
	XMVECTOR qStar = XMQuaternionConjugate(q);
	printXMVector(qStar);
	XMVECTOR p = XMVectorSet(0, 0, 1, 0);
	printXMVector(p);
	
	XMVECTOR a = XMQuaternionMultiply(qStar, XMQuaternionMultiply(p, q));
	printXMVector(a);

	float Q = 45;
	float radianQ = XMConvertToRadians(Q);
	XMVECTOR b = XMVectorSet(1, 1, 1, 0);
	XMVECTOR n = XMVector3Normalize(b);
	XMVECTOR v = XMVectorSet(0, 0, 1, 0);

	float cosQ = std::cosf(radianQ);
	float sinQ = std::sinf(radianQ);

	XMVECTOR term1 = XMVectorScale(v, cosQ); // cos(Q) * v
	XMVECTOR term2 = XMVectorScale(n, (1.0f - cosQ) * XMVector3Dot(n, v).m128_f32[0]); // (1 - cos(Q))(n ¡¤ v)n
	XMVECTOR term3 = XMVectorScale(XMVector3Cross(n, v), sinQ); // sin(Q)(n ¡¿ v)

	XMVECTOR c = XMVectorAdd(term1, XMVectorAdd(term2, term3));
	printXMVector(c);

	return 0;
}


void printXMVector(DirectX::XMVECTOR in)
{
	std::cout << "(" << in.m128_f32[0] << ", " << in.m128_f32[1] << ", " << in.m128_f32[2] << ", " << in.m128_f32[3] << ")" << std::endl;
}