#include "Shape/Projection.h"

namespace SDFGenerator
{
	FProjection::FProjection() : Scale(1), Translate(0)
	{
	}

	FProjection::FProjection(const FVector2& InScale, const FVector2& InTranslate) : Scale(InScale), Translate(InTranslate)
	{
	}

	FPoint2D FProjection::Project(const FPoint2D& Coord) const
	{
		return Scale * (Coord + Translate);
	}

	FPoint2D FProjection::Unproject(const FPoint2D& Coord) const
	{
		return Coord / Scale - Translate;
	}

	FVector2 FProjection::ProjectVector(const FVector2& Vector) const
	{
		return Scale * Vector;
	}

	FVector2 FProjection::UnprojectVector(const FVector2& Vector) const
	{
		return Vector / Scale;
	}

	double FProjection::ProjectX(double X) const
	{
		return Scale.X * (X + Translate.X);
	}

	double FProjection::ProjectY(double Y) const
	{
		return Scale.Y * (Y + Translate.Y);
	}

	double FProjection::UnprojectX(double X) const
	{
		return X / Scale.X - Translate.X;
	}

	double FProjection::UnprojectY(double Y) const
	{
		return Y / Scale.Y - Translate.Y;
	}
}
