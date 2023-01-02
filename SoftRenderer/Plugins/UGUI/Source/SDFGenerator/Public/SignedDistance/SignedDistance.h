#pragma once

namespace SDFGenerator
{
	/**
	 * Represents a signed distance and alignment, which together can be compared to uniquely determine the closest edge segment.
	 */
	class FSignedDistance
	{
	public:
		static const FSignedDistance INFINITE;

		double Distance;
		double Dot;

		FSignedDistance();
		FSignedDistance(double InDist, double InDot);

		friend bool operator<(FSignedDistance A, FSignedDistance B);
		friend bool operator>(FSignedDistance A, FSignedDistance B);
		friend bool operator<=(FSignedDistance A, FSignedDistance B);
		friend bool operator>=(FSignedDistance A, FSignedDistance B);
	};
}
