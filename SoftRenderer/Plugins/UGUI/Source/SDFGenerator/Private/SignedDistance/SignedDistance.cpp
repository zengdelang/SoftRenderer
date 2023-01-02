#include "SignedDistance/SignedDistance.h"

namespace SDFGenerator
{
	const FSignedDistance FSignedDistance::INFINITE(-1e240, 1);

	FSignedDistance::FSignedDistance() : Distance(-1e240), Dot(1)
	{
	}

	FSignedDistance::FSignedDistance(double InDist, double InDot) : Distance(InDist), Dot(InDot)
	{
	}

	bool operator<(FSignedDistance A, FSignedDistance B)
	{
		return fabs(A.Distance) < fabs(B.Distance) || (fabs(A.Distance) == fabs(B.Distance) && A.Dot < B.Dot);
	}

	bool operator>(FSignedDistance A, FSignedDistance B)
	{
		return fabs(A.Distance) > fabs(B.Distance) || (fabs(A.Distance) == fabs(B.Distance) && A.Dot > B.Dot);
	}

	bool operator<=(FSignedDistance A, FSignedDistance B)
	{
		return fabs(A.Distance) < fabs(B.Distance) || (fabs(A.Distance) == fabs(B.Distance) && A.Dot <= B.Dot);
	}

	bool operator>=(FSignedDistance A, FSignedDistance B)
	{
		return fabs(A.Distance) > fabs(B.Distance) || (fabs(A.Distance) == fabs(B.Distance) && A.Dot >= B.Dot);
	}
}
