#include "Vector2.h"

namespace SDFGenerator
{
	FVector2::FVector2(double Val) : X(Val), Y(Val)
	{
	}

	FVector2::FVector2(double InX, double InY) : X(InX), Y(InY)
	{
	}

	void FVector2::Rest()
	{
		X = 0, Y = 0;
	}

	void FVector2::Set(double InX, double InY)
	{
		X = InX, Y = InY;
	}

	double FVector2::Length() const
	{
		return sqrt(X * X + Y * Y);
	}

	double FVector2::Direction() const
	{
		return atan2(Y, X);
	}

	FVector2 FVector2::Normalize(bool bAllowZero) const
	{
		const double Len = Length();
		if (Len == 0)
			return FVector2(0, !bAllowZero);
		return FVector2(X / Len, Y / Len);
	}

	FVector2 FVector2::GetOrthogonal(bool bPolarity) const
	{
		return bPolarity ? FVector2(-Y, X) : FVector2(Y, -X);
	}

	FVector2 FVector2::GetOrthonormal(bool bPolarity, bool bAllowZero) const
	{
		const double Len = Length();
		if (Len == 0)
			return bPolarity ? FVector2(0, !bAllowZero) : FVector2(0, -!bAllowZero);
		return bPolarity ? FVector2(-Y / Len, X / Len) : FVector2(Y / Len, -X / Len);
	}

	FVector2 FVector2::Project(const FVector2& Vector, bool bPositive) const
	{
		const FVector2 N = Normalize(true);
		const double T = DotProduct(Vector, N);
		if (bPositive && T <= 0)
			return FVector2();
		return T * N;
	}

	FVector2::operator const void*() const
	{
		return X || Y ? this : nullptr;
	}

	bool FVector2::operator!() const
	{
		return !X && !Y;
	}

	bool FVector2::operator==(const FVector2& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

	bool FVector2::operator!=(const FVector2& Other) const
	{
		return X != Other.X || Y != Other.Y;
	}

	FVector2 FVector2::operator+() const
	{
		return *this;
	}

	FVector2 FVector2::operator-() const
	{
		return FVector2(-X, -Y);
	}

	FVector2 FVector2::operator+(const FVector2& Other) const
	{
		return FVector2(X + Other.X, Y + Other.Y);
	}

	FVector2 FVector2::operator-(const FVector2& Other) const
	{
		return FVector2(X - Other.X, Y - Other.Y);
	}

	FVector2 FVector2::operator*(const FVector2& Other) const
	{
		return FVector2(X * Other.X, Y * Other.Y);
	}

	FVector2 FVector2::operator/(const FVector2& Other) const
	{
		return FVector2(X / Other.X, Y / Other.Y);
	}

	FVector2 FVector2::operator*(double Value) const
	{
		return FVector2(X * Value, Y * Value);
	}

	FVector2 FVector2::operator/(double Value) const
	{
		return FVector2(X / Value, Y / Value);
	}

	FVector2& FVector2::operator+=(const FVector2& Other)
	{
		X += Other.X, Y += Other.Y;
		return *this;
	}

	FVector2& FVector2::operator-=(const FVector2& Other)
	{
		X -= Other.X, Y -= Other.Y;
		return *this;
	}

	FVector2& FVector2::operator*=(const FVector2& Other)
	{
		X *= Other.X, Y *= Other.Y;
		return *this;
	}

	FVector2& FVector2::operator/=(const FVector2& Other)
	{
		X /= Other.X, Y /= Other.Y;
		return *this;
	}

	FVector2& FVector2::operator*=(double Value)
	{
		X *= Value, Y *= Value;
		return *this;
	}

	FVector2& FVector2::operator/=(double Value)
	{
		X /= Value, Y /= Value;
		return *this;
	}

	double DotProduct(const FVector2& A, const FVector2& B)
	{
		return A.X * B.X + A.Y * B.Y;
	}

	double CrossProduct(const FVector2& A, const FVector2& B)
	{
		return A.X * B.Y - A.Y * B.X;
	}

	FVector2 operator*(double Value, const FVector2& Vector)
	{
		return FVector2(Value * Vector.X, Value * Vector.Y);
	}

	FVector2 operator/(double Value, const FVector2& Vector)
	{
		return FVector2(Value / Vector.X, Value / Vector.Y);
	}
}
