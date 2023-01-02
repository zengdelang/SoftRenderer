#include "Shape/EdgetSegments.h"
#include "Arithmetics.h"
#include "Shape/EquationSolver.h"

namespace SDFGenerator
{
	static void PointBounds(FPoint2D Point, double& Left, double& Bottom, double& Right, double& Top)
	{
		if (Point.X < Left) Left = Point.X;
		if (Point.Y < Bottom) Bottom = Point.Y;
		if (Point.X > Right) Right = Point.X;
		if (Point.Y > Top) Top = Point.Y;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// FEdgeSegment
	/// 
	void FEdgeSegment::DistanceToPseudoDistance(FSignedDistance& Distance, FPoint2D Origin, double Param) const
	{
		if (Param < 0)
		{
			const FVector2 Dir = Direction(0).Normalize();
			const FVector2 Aq = Origin - Point(0);
			const double Ts = DotProduct(Aq, Dir);
			if (Ts < 0)
			{
				const double PseudoDistance = CrossProduct(Aq, Dir);
				if (fabs(PseudoDistance) <= fabs(Distance.Distance))
				{
					Distance.Distance = PseudoDistance;
					Distance.Dot = 0;
				}
			}
		}
		else if (Param > 1)
		{
			const FVector2 Dir = Direction(1).Normalize();
			const FVector2 Bq = Origin - Point(1);
			const double Ts = DotProduct(Bq, Dir);
			if (Ts > 0)
			{
				const double PseudoDistance = CrossProduct(Bq, Dir);
				if (fabs(PseudoDistance) <= fabs(Distance.Distance))
				{
					Distance.Distance = PseudoDistance;
					Distance.Dot = 0;
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// FLinearSegment
	
	FLinearSegment::FLinearSegment(FPoint2D P0, FPoint2D P1, EEdgeColor EdgeColor) : FEdgeSegment(EdgeColor)
	{
		Points[0] = P0;
		Points[1] = P1;
	}

	FLinearSegment* FLinearSegment::Clone() const
	{
		return new FLinearSegment(Points[0], Points[1], Color);
	}

	FPoint2D FLinearSegment::Point(double Param) const
	{
		return Mix(Points[0], Points[1], Param);
	}

	FVector2 FLinearSegment::Direction(double Param) const
	{
		return Points[1] - Points[0];
	}

	FVector2 FLinearSegment::DirectionChange(double Param) const
	{
		return FVector2();
	}

	FSignedDistance FLinearSegment::SignedDistance(FPoint2D Origin, double& Param) const
	{
		const FVector2 Aq = Origin - Points[0];
		const FVector2 Ab = Points[1] - Points[0];
		Param = DotProduct(Aq, Ab) / DotProduct(Ab, Ab);
		const FVector2 Eq = Points[Param > .5] - Origin;
		const double EndpointDistance = Eq.Length();
		if (Param > 0 && Param < 1)
		{
			const double OrthoDistance = DotProduct(Ab.GetOrthonormal(false), Aq);
			if (fabs(OrthoDistance) < EndpointDistance)
				return FSignedDistance(OrthoDistance, 0);
		}
		return FSignedDistance(NonZeroSign(CrossProduct(Aq, Ab)) * EndpointDistance,
							   fabs(DotProduct(Ab.Normalize(), Eq.Normalize())));
	}

	void FLinearSegment::Bound(double& Left, double& Bottom, double& Right, double& Top) const
	{
		PointBounds(Points[0], Left, Bottom, Right, Top);
		PointBounds(Points[1], Left, Bottom, Right, Top);
	}

	void FLinearSegment::Reverse()
	{
		const FPoint2D Temp = Points[0];
		Points[0] = Points[1];
		Points[1] = Temp;
	}

	void FLinearSegment::MoveStartPoint(FPoint2D To)
	{
		Points[0] = To;
	}

	void FLinearSegment::MoveEndPoint(FPoint2D To)
	{
		Points[1] = To;
	}

	void FLinearSegment::SplitInThirds(FEdgeSegment*& Part1, FEdgeSegment*& Part2, FEdgeSegment*& Part3) const
	{
		Part1 = new FLinearSegment(Points[0], Point(1 / 3.), Color);
		Part2 = new FLinearSegment(Point(1 / 3.), Point(2 / 3.), Color);
		Part3 = new FLinearSegment(Point(2 / 3.), Points[1], Color);
	}
	
	double FLinearSegment::Length() const
	{
		return (Points[1] - Points[0]).Length();
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// FQuadraticSegment

	FQuadraticSegment::FQuadraticSegment(FPoint2D P0, FPoint2D P1, FPoint2D P2, EEdgeColor EdgeColor) : FEdgeSegment(EdgeColor)
	{
		if (P1 == P0 || P1 == P2)
			P1 = 0.5 * (P0 + P2);
		Points[0] = P0;
		Points[1] = P1;
		Points[2] = P2;
	}

	FQuadraticSegment* FQuadraticSegment::Clone() const
	{
		return new FQuadraticSegment(Points[0], Points[1], Points[2], Color);
	}

	FPoint2D FQuadraticSegment::Point(double Param) const
	{
		return Mix(Mix(Points[0], Points[1], Param), Mix(Points[1], Points[2], Param), Param);
	}

	FVector2 FQuadraticSegment::Direction(double Param) const
	{
		const FVector2 Tangent = Mix(Points[1] - Points[0], Points[2] - Points[1], Param);
		if (!Tangent)
			return Points[2] - Points[0];
		return Tangent;
	}

	FVector2 FQuadraticSegment::DirectionChange(double Param) const
	{
		return (Points[2] - Points[1]) - (Points[1] - Points[0]);
	}

	FSignedDistance FQuadraticSegment::SignedDistance(FPoint2D Origin, double& Param) const
	{
		const FVector2 Qa = Points[0] - Origin;
		const FVector2 Ab = Points[1] - Points[0];
		const FVector2 Br = Points[2] - Points[1] - Ab;

		const double A = DotProduct(Br, Br);
		const double B = 3 * DotProduct(Ab, Br);
		const double C = 2 * DotProduct(Ab, Ab) + DotProduct(Qa, Br);
		const double D = DotProduct(Qa, Ab);
		
		double T[3];
		const int32 Solutions = SolveCubic(T, A, B, C, D);

		FVector2 EpDir = Direction(0);
		double MinDistance = NonZeroSign(CrossProduct(EpDir, Qa)) * Qa.Length(); // distance from A
		Param = -DotProduct(Qa, EpDir) / DotProduct(EpDir, EpDir);
		{
			EpDir = Direction(1);
			const double Distance = (Points[2] - Origin).Length(); // distance from B
			if (Distance < fabs(MinDistance))
			{
				MinDistance = NonZeroSign(CrossProduct(EpDir, Points[2] - Origin)) * Distance;
				Param = DotProduct(Origin - Points[1], EpDir) / DotProduct(EpDir, EpDir);
			}
		}
		
		for (int32 Index = 0; Index < Solutions; ++Index)
		{
			if (T[Index] > 0 && T[Index] < 1)
			{
				FPoint2D Qe = Qa + 2 * T[Index] * Ab + T[Index] * T[Index] * Br;
				const double Distance = Qe.Length();
				if (Distance <= fabs(MinDistance))
				{
					MinDistance = NonZeroSign(CrossProduct(Ab + T[Index] * Br, Qe)) * Distance;
					Param = T[Index];
				}
			}
		}

		if (Param >= 0 && Param <= 1)
			return FSignedDistance(MinDistance, 0);
		if (Param < .5)
			return FSignedDistance(MinDistance, fabs(DotProduct(Direction(0).Normalize(), Qa.Normalize())));
		else
			return FSignedDistance(MinDistance,fabs(DotProduct(Direction(1).Normalize(), (Points[2] - Origin).Normalize())));
	}

	int32 FLinearSegment::ScanlineIntersections(double X[3], int32 Dy[3], double Y) const
	{
		if ((Y >= Points[0].Y && Y < Points[1].Y) || (Y >= Points[1].Y && Y < Points[0].Y))
		{
			const double Param = (Y - Points[0].Y) / (Points[1].Y - Points[0].Y);
			X[0] = Mix(Points[0].X, Points[1].X, Param);
			Dy[0] = Sign(Points[1].Y - Points[0].Y);
			return 1;
		}
		return 0;
	}

	int32 FQuadraticSegment::ScanlineIntersections(double X[3], int32 Dy[3], double Y) const
	{
		int32 Total = 0;
		int32 NextDy = Y > Points[0].Y ? 1 : -1;
		X[Total] = Points[0].X;
		if (Points[0].Y == Y)
		{
			if (Points[0].Y < Points[1].Y || (Points[0].Y == Points[1].Y && Points[0].Y < Points[2].Y))
				Dy[Total++] = 1;
			else
				NextDy = 1;
		}
		
		{
			const FVector2 Ab = Points[1] - Points[0];
			const FVector2 Br = Points[2] - Points[1] - Ab;
			double T[2];
			const int32 Solutions = SolveQuadratic(T, Br.Y, 2 * Ab.Y, Points[0].Y - Y);

			// Sort solutions
			double Tmp;
			if (Solutions >= 2 && T[0] > T[1])
				Tmp = T[0], T[0] = T[1], T[1] = Tmp;
			
			for (int32 Index = 0; Index < Solutions && Total < 2; ++Index)
			{
				if (T[Index] >= 0 && T[Index] <= 1)
				{
					X[Total] = Points[0].X + 2 * T[Index] * Ab.X + T[Index] * T[Index] * Br.X;
					if (NextDy * (Ab.Y + T[Index] * Br.Y) >= 0)
					{
						Dy[Total++] = NextDy;
						NextDy = -NextDy;
					}
				}
			}
		}
		
		if (Points[2].Y == Y)
		{
			if (NextDy > 0 && Total > 0)
			{
				--Total;
				NextDy = -1;
			}
			if ((Points[2].Y < Points[1].Y || (Points[2].Y == Points[1].Y && Points[2].Y < Points[0].Y)) && Total < 2)
			{
				X[Total] = Points[2].X;
				if (NextDy < 0)
				{
					Dy[Total++] = -1;
					NextDy = 1;
				}
			}
		}
		
		if (NextDy != (Y >= Points[2].Y ? 1 : -1))
		{
			if (Total > 0)
				--Total;
			else
			{
				if (fabs(Points[2].Y - Y) < fabs(Points[0].Y - Y))
					X[Total] = Points[2].X;
				Dy[Total++] = NextDy;
			}
		}
		
		return Total;
	}

	void FQuadraticSegment::Bound(double& Left, double& Bottom, double& Right, double& Top) const
	{
		PointBounds(Points[0], Left, Bottom, Right, Top);
		PointBounds(Points[2], Left, Bottom, Right, Top);
		
		const FVector2 Bot = (Points[1] - Points[0]) - (Points[2] - Points[1]);
		if (Bot.X)
		{
			const double Param = (Points[1].X - Points[0].X) / Bot.X;
			if (Param > 0 && Param < 1)
				PointBounds(Point(Param), Left, Bottom, Right, Top);
		}
		
		if (Bot.Y)
		{
			const double Param = (Points[1].Y - Points[0].Y) / Bot.Y;
			if (Param > 0 && Param < 1)
				PointBounds(Point(Param), Left, Bottom, Right, Top);
		}
	}
	
	void FQuadraticSegment::Reverse()
	{
		const FPoint2D Temp = Points[0];
		Points[0] = Points[2];
		Points[2] = Temp;
	}
	
	void FQuadraticSegment::MoveStartPoint(FPoint2D To)
	{
		const FVector2 OrigSDir = Points[0] - Points[1];
		const FPoint2D OrigP1 = Points[1];
		
		Points[1] += CrossProduct(Points[0] - Points[1], To - Points[0]) / CrossProduct(Points[0] - Points[1], Points[2] - Points[1]) * (Points[2] - Points[1]);
		Points[0] = To;
		
		if (DotProduct(OrigSDir, Points[0] - Points[1]) < 0)
		{
			Points[1] = OrigP1;
		}
	}

	void FQuadraticSegment::MoveEndPoint(FPoint2D To)
	{
		const FVector2 OrigEDir = Points[2] - Points[1];
		const FPoint2D OrigP1 = Points[1];
		
		Points[1] += CrossProduct(Points[2] - Points[1], To - Points[2]) / CrossProduct(Points[2] - Points[1], Points[0] - Points[1]) * (Points[0] - Points[1]);
		Points[2] = To;
		
		if (DotProduct(OrigEDir, Points[2] - Points[1]) < 0)
		{
			Points[1] = OrigP1;
		}
	}

	void FQuadraticSegment::SplitInThirds(FEdgeSegment*& Part1, FEdgeSegment*& Part2, FEdgeSegment*& Part3) const
	{
		Part1 = new FQuadraticSegment(Points[0], Mix(Points[0], Points[1], 1 / 3.), Point(1 / 3.), Color);
		Part2 = new FQuadraticSegment(Point(1 / 3.), Mix(Mix(Points[0], Points[1], 5 / 9.), Mix(Points[1], Points[2], 4 / 9.), .5),Point(2 / 3.), Color);
		Part3 = new FQuadraticSegment(Point(2 / 3.), Mix(Points[1], Points[2], 2 / 3.), Points[2], Color);
	}
	
	double FQuadraticSegment::Length() const
	{
		const FVector2 Ab = Points[1] - Points[0];
		const FVector2 Br = Points[2] - Points[1] - Ab;
		const double AbAb = DotProduct(Ab, Ab);
		const double AbBr = DotProduct(Ab, Br);
		const double BrBr = DotProduct(Br, Br);
		const double abLen = sqrt(AbAb);
		const double brLen = sqrt(BrBr);
		const double Crs = CrossProduct(Ab, Br);
		const double H = sqrt(AbAb + AbBr + AbBr + BrBr);
		return (brLen * ((AbBr + BrBr) * H - AbBr * abLen) + Crs * Crs * log((brLen * H + AbBr + BrBr) / (brLen * abLen + AbBr))) / (BrBr * brLen);
	}

	FEdgeSegment* FQuadraticSegment::ConvertToCubic() const
	{
		return new FCubicSegment(Points[0], Mix(Points[0], Points[1], 2 / 3.), Mix(Points[1], Points[2], 1 / 3.), Points[2], Color);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// FCubicSegment

	FCubicSegment::FCubicSegment(FPoint2D P0, FPoint2D P1, FPoint2D P2, FPoint2D P3, EEdgeColor EdgeColor) : FEdgeSegment(EdgeColor)
	{
		if ((P1 == P0 || P1 == P3) && (P2 == P0 || P2 == P3))
		{
			P1 = Mix(P0, P3, 1 / 3.);
			P2 = Mix(P0, P3, 2 / 3.);
		}
		Points[0] = P0;
		Points[1] = P1;
		Points[2] = P2;
		Points[3] = P3;
	}

	FCubicSegment* FCubicSegment::Clone() const
	{
		return new FCubicSegment(Points[0], Points[1], Points[2], Points[3], Color);
	}
	
	FPoint2D FCubicSegment::Point(double Param) const
	{
		const FVector2 P12 = Mix(Points[1], Points[2], Param);
		return Mix(Mix(Mix(Points[0], Points[1], Param), P12, Param), Mix(P12, Mix(Points[2], Points[3], Param), Param), Param);
	}
	
	FVector2 FCubicSegment::Direction(double Param) const
	{
		const FVector2 Tangent = Mix(Mix(Points[1] - Points[0], Points[2] - Points[1], Param), Mix(Points[2] - Points[1], Points[3] - Points[2], Param), Param);
		if (!Tangent)
		{
			if (Param == 0) return Points[2] - Points[0];
			if (Param == 1) return Points[3] - Points[1];
		}
		return Tangent;
	}
	
	FVector2 FCubicSegment::DirectionChange(double Param) const
	{
		return Mix((Points[2] - Points[1]) - (Points[1] - Points[0]), (Points[3] - Points[2]) - (Points[2] - Points[1]), Param);
	}

	FSignedDistance FCubicSegment::SignedDistance(FPoint2D Origin, double& Param) const
	{
		const FVector2 Qa = Points[0] - Origin;
		const FVector2 Ab = Points[1] - Points[0];
		const FVector2 Br = Points[2] - Points[1] - Ab;
		const FVector2 As = (Points[3] - Points[2]) - (Points[2] - Points[1]) - Br;

		FVector2 EpDir = Direction(0);
		double MinDistance = NonZeroSign(CrossProduct(EpDir, Qa)) * Qa.Length(); // distance from A
		Param = -DotProduct(Qa, EpDir) / DotProduct(EpDir, EpDir);
		{
			EpDir = Direction(1);
			const double Distance = (Points[3] - Origin).Length(); // distance from B
			if (Distance < fabs(MinDistance))
			{
				MinDistance = NonZeroSign(CrossProduct(EpDir, Points[3] - Origin)) * Distance;
				Param = DotProduct(EpDir - (Points[3] - Origin), EpDir) / DotProduct(EpDir, EpDir);
			}
		}
		
		// Iterative minimum distance search
		for (int32 Index = 0; Index <= SDF_GENERATOR_CUBIC_SEARCH_STARTS; ++Index)
		{
			double T = static_cast<double>(Index) / SDF_GENERATOR_CUBIC_SEARCH_STARTS;
			FVector2 Qe = Qa + 3 * T * Ab + 3 * T * T * Br + T * T * T * As;
			
			for (int32 Step = 0; Step < SDF_GENERATOR_CUBIC_SEARCH_STEPS; ++Step)
			{
				// Improve t
				FVector2 D1 = 3 * Ab + 6 * T * Br + 3 * T * T * As;
				FVector2 D2 = 6 * Br + 6 * T * As;
				T -= DotProduct(Qe, D1) / (DotProduct(D1, D1) + DotProduct(Qe, D2));
				if (T <= 0 || T >= 1)
					break;
				
				Qe = Qa + 3 * T * Ab + 3 * T * T * Br + T * T * T * As;
				const double Distance = Qe.Length();
				if (Distance < fabs(MinDistance))
				{
					MinDistance = NonZeroSign(CrossProduct(D1, Qe)) * Distance;
					Param = T;
				}
			}
		}

		if (Param >= 0 && Param <= 1)
			return FSignedDistance(MinDistance, 0);
		if (Param < .5)
			return FSignedDistance(MinDistance, fabs(DotProduct(Direction(0).Normalize(), Qa.Normalize())));
		else
			return FSignedDistance(MinDistance,fabs(DotProduct(Direction(1).Normalize(), (Points[3] - Origin).Normalize())));
	}

	int32 FCubicSegment::ScanlineIntersections(double X[3], int32 Dy[3], double Y) const
	{
		int32 Total = 0;
		int32 NextDy = Y > Points[0].Y ? 1 : -1;
		X[Total] = Points[0].X;
		if (Points[0].Y == Y)
		{
			if (Points[0].Y < Points[1].Y || (Points[0].Y == Points[1].Y && (Points[0].Y < Points[2].Y || (Points[0].Y == Points[2].Y && Points[0].Y < Points[3].Y))))
				Dy[Total++] = 1;
			else
				NextDy = 1;
		}
		
		{
			const FVector2 Ab = Points[1] - Points[0];
			const FVector2 Br = Points[2] - Points[1] - Ab;
			const FVector2 As = (Points[3] - Points[2]) - (Points[2] - Points[1]) - Br;
			
			double T[3];
			const int32 Solutions = SolveCubic(T, As.Y, 3 * Br.Y, 3 * Ab.Y, Points[0].Y - Y);
			// Sort solutions
			if (Solutions >= 2)
			{
				double Tmp;
				if (T[0] > T[1])
					Tmp = T[0], T[0] = T[1], T[1] = Tmp;
				if (Solutions >= 3 && T[1] > T[2])
				{
					Tmp = T[1], T[1] = T[2], T[2] = Tmp;
					if (T[0] > T[1])
						Tmp = T[0], T[0] = T[1], T[1] = Tmp;
				}
			}
			
			for (int32 Index = 0; Index < Solutions && Total < 3; ++Index)
			{
				if (T[Index] >= 0 && T[Index] <= 1)
				{
					X[Total] = Points[0].X + 3 * T[Index] * Ab.X + 3 * T[Index] * T[Index] * Br.X + T[Index] * T[Index] * T[Index] * As.X;
					if (NextDy * (Ab.Y + 2 * T[Index] * Br.Y + T[Index] * T[Index] * As.Y) >= 0)
					{
						Dy[Total++] = NextDy;
						NextDy = -NextDy;
					}
				}
			}
		}
		
		if (Points[3].Y == Y)
		{
			if (NextDy > 0 && Total > 0)
			{
				--Total;
				NextDy = -1;
			}
			if ((Points[3].Y < Points[2].Y || (Points[3].Y == Points[2].Y && (Points[3].Y < Points[1].Y || (Points[3].Y == Points[1].Y && Points[3].Y < Points[0].Y)))) &&
				Total < 3)
			{
				X[Total] = Points[3].X;
				if (NextDy < 0)
				{
					Dy[Total++] = -1;
					NextDy = 1;
				}
			}
		}
		
		if (NextDy != (Y >= Points[3].Y ? 1 : -1))
		{
			if (Total > 0)
				--Total;
			else
			{
				if (fabs(Points[3].Y - Y) < fabs(Points[0].Y - Y))
					X[Total] = Points[3].X;
				Dy[Total++] = NextDy;
			}
		}
		return Total;
	}
	
	void FCubicSegment::Bound(double& Left, double& Bottom, double& Right, double& Top) const
	{
		PointBounds(Points[0], Left, Bottom, Right, Top);
		PointBounds(Points[3], Left, Bottom, Right, Top);
		
		const FVector2 A0 = Points[1] - Points[0];
		const FVector2 A1 = 2 * (Points[2] - Points[1] - A0);
		const FVector2 A2 = Points[3] - 3 * Points[2] + 3 * Points[1] - Points[0];
		
		double Params[2];
		int32 Solutions = SolveQuadratic(Params, A2.X, A1.X, A0.X);
		
		for (int32 Index = 0; Index < Solutions; ++Index)
		{
			if (Params[Index] > 0 && Params[Index] < 1)
			{
				PointBounds(Point(Params[Index]), Left, Bottom, Right, Top);
			}
		}
		
		Solutions = SolveQuadratic(Params, A2.Y, A1.Y, A0.Y);
		for (int32 Index = 0; Index < Solutions; ++Index)
		{
			if (Params[Index] > 0 && Params[Index] < 1)
			{
				PointBounds(Point(Params[Index]), Left, Bottom, Right, Top);
			}
		}
	}
	
	void FCubicSegment::Reverse()
	{
		FPoint2D Temp = Points[0];
		Points[0] = Points[3];
		Points[3] = Temp;
		Temp = Points[1];
		Points[1] = Points[2];
		Points[2] = Temp;
	}
	
	void FCubicSegment::MoveStartPoint(FPoint2D To)
	{
		Points[1] += To - Points[0];
		Points[0] = To;
	}
	
	void FCubicSegment::MoveEndPoint(FPoint2D To)
	{
		Points[2] += To - Points[3];
		Points[3] = To;
	}
	
	void FCubicSegment::SplitInThirds(FEdgeSegment*& Part1, FEdgeSegment*& Part2, FEdgeSegment*& Part3) const
	{
		Part1 = new FCubicSegment(Points[0], Points[0] == Points[1] ? Points[0] : Mix(Points[0], Points[1], 1 / 3.),
		                         Mix(Mix(Points[0], Points[1], 1 / 3.), Mix(Points[1], Points[2], 1 / 3.), 1 / 3.), Point(1 / 3.), Color);
		Part2 = new FCubicSegment(Point(1 / 3.),
		                         Mix(Mix(Mix(Points[0], Points[1], 1 / 3.), Mix(Points[1], Points[2], 1 / 3.), 1 / 3.),
		                             Mix(Mix(Points[1], Points[2], 1 / 3.), Mix(Points[2], Points[3], 1 / 3.), 1 / 3.), 2 / 3.),
		                         Mix(Mix(Mix(Points[0], Points[1], 2 / 3.), Mix(Points[1], Points[2], 2 / 3.), 2 / 3.),
		                             Mix(Mix(Points[1], Points[2], 2 / 3.), Mix(Points[2], Points[3], 2 / 3.), 2 / 3.), 1 / 3.),
		                         Point(2 / 3.), Color);
		Part3 = new FCubicSegment(Point(2 / 3.), Mix(Mix(Points[1], Points[2], 2 / 3.), Mix(Points[2], Points[3], 2 / 3.), 2 / 3.),
		                         Points[2] == Points[3] ? Points[3] : Mix(Points[2], Points[3], 2 / 3.), Points[3], Color);
	}
	
	void FCubicSegment::Deconverge(int32 Param, double Amount)
	{
		const FVector2 Dir = Direction(Param);
		const FVector2 Normal = Dir.GetOrthonormal();
		const double H = DotProduct(DirectionChange(Param) - Dir, Normal);
		switch (Param)
		{
		case 0:
			Points[1] += Amount * (Dir + Sign(H) * sqrt(fabs(H)) * Normal);
			break;
		case 1:
			Points[2] -= Amount * (Dir - Sign(H) * sqrt(fabs(H)) * Normal);
			break;
		default: ;
		}
	}
}
