#include "Shape/EquationSolver.h"

#define TOO_LARGE_RATIO 1e12

namespace SDFGenerator
{
	int32 SolveQuadratic(double X[2], double A, double B, double C)
	{
		// a = 0 -> linear equation
		if (A == 0 || fabs(B) + fabs(C) > TOO_LARGE_RATIO * fabs(A))
		{
			// a, b = 0 -> no solution
			if (B == 0 || fabs(C) > TOO_LARGE_RATIO * fabs(B))
			{
				if (C == 0)
					return -1; // 0 = 0
				return 0;
			}
			X[0] = -C / B;
			return 1;
		}
		
		double Dscr = B * B - 4 * A * C;
		if (Dscr > 0)
		{
			Dscr = sqrt(Dscr);
			X[0] = (-B + Dscr) / (2 * A);
			X[1] = (-B - Dscr) / (2 * A);
			return 2;
		}
		else if (Dscr == 0)
		{
			X[0] = -B / (2 * A);
			return 1;
		}
		else
		{
			return 0;
		}
	}

	static int32 SolveCubicNormed(double X[3], double InA, double InB, double InC)
	{
		const double A2 = InA * InA;
		double Q = (A2 - 3 * InB) / 9;
		const double R = (InA * (2 * A2 - 9 * InB) + 27 * InC) / 54;
		const double R2 = R * R;
		const double Q3 = Q * Q * Q;

		if (R2 < Q3)
		{
			double T = R / sqrt(Q3);
			
			if (T < -1)
				T = -1;
			
			if (T > 1)
				T = 1;
			
			T = acos(T);
			InA /= 3;
			Q = -2 * sqrt(Q);
			X[0] = Q * cos(T / 3) - InA;
			X[1] = Q * cos((T + 2 * PI) / 3) - InA;
			X[2] = Q * cos((T - 2 * PI) / 3) - InA;
			return 3;
		}
		else
		{
			double A = -pow(fabs(R) + sqrt(R2 - Q3), 1 / 3.);
			if (R < 0)
				A = -A;

			const double B = A == 0 ? 0 : Q / A;
			
			InA /= 3;
			X[0] = (A + B) - InA;
			X[1] = -0.5 * (A + B) - InA;
			X[2] = 0.5 * sqrt(3.) * (A - B);
			
			if (fabs(X[2]) < 1e-14)
				return 2;
			return 1;
		}
	}

	int32 SolveCubic(double X[3], double A, double B, double C, double D)
	{
		if (A != 0)
		{
			const double Bn = B / A, Cn = C / A, Dn = D / A;
			// Check that a isn't "almost zero"
			if (fabs(Bn) < TOO_LARGE_RATIO && fabs(Cn) < TOO_LARGE_RATIO && fabs(Dn) < TOO_LARGE_RATIO)
				return SolveCubicNormed(X, Bn, Cn, Dn);
		}
		return SolveQuadratic(X, B, C, D);
	}
}
