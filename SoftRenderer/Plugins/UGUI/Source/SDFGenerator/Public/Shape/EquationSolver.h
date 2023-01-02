#pragma once

namespace SDFGenerator
{
	// ax^2 + bx + c = 0
	int32 SolveQuadratic(double X[2], double A, double B, double C);

	// ax^3 + bx^2 + cx + d = 0
	int32 SolveCubic(double X[3], double A, double B, double C, double D);
}
