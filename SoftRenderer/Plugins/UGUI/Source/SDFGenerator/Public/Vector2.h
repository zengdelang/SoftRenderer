#pragma once

namespace SDFGenerator
{
	/**
	* A 2-dimensional euclidean vector with double precision.
	* Implementation based on the Vector2 template from Artery Engine.
	*/
	struct FVector2
	{
	public:
		double X, Y;

	public:
		FVector2(double Val = 0);
		FVector2(double InX, double InY);
		
		/**
		 * Sets the vector to zero.
		 */
		void Rest();

		/**
		 * Sets individual elements of the vector.
		 */
		void Set(double NewX, double NewY);

		/**
		 * Returns the vector's length.
		 */
		double Length() const;

		/**
		 * Returns the angle of the vector in radians (atan2).
		 */
		double Direction() const;
		
		/**
		 * Returns the normalized vector - one that has the same direction but unit length.
		 */
		FVector2 Normalize(bool bAllowZero = false) const;
		
		/**
		 * Returns a vector with the same length that is orthogonal to this one.
		 */
		FVector2 GetOrthogonal(bool bPolarity = true) const;
		
		/**
		 * Returns a vector with unit length that is orthogonal to this one.
		 */
		FVector2 GetOrthonormal(bool bPolarity = true, bool bAllowZero = false) const;
		
		/**
		 * Returns a vector projected along this one.
		 */
		FVector2 Project(const FVector2& Vector, bool bPositive = false) const;

		operator const void*() const;
		bool operator!() const;
		bool operator==(const FVector2& Other) const;
		bool operator!=(const FVector2& Other) const;

		FVector2 operator+() const;
		FVector2 operator-() const;
		FVector2 operator+(const FVector2& Other) const;
		FVector2 operator-(const FVector2& Other) const;
		FVector2 operator*(const FVector2& Other) const;
		FVector2 operator/(const FVector2& Other) const;
		FVector2 operator*(double Value) const;
		FVector2 operator/(double Value) const;

		FVector2& operator+=(const FVector2& Other);
		FVector2& operator-=(const FVector2& Other);
		FVector2& operator*=(const FVector2& Other);
		FVector2& operator/=(const FVector2& Other);
		FVector2& operator*=(double Value);
		FVector2& operator/=(double Value);
		
		/**
		 * Dot product of two vectors.
		 */
		friend double DotProduct(const FVector2& A, const FVector2& B);
		
		/**
		 * A special version of the cross product for 2D vectors (returns scalar value).
		 */
		friend double CrossProduct(const FVector2& A, const FVector2& B);
		
		friend FVector2 operator*(double Value, const FVector2& Vector);
		friend FVector2 operator/(double Value, const FVector2& Vector);
	};

	/// A vector may also represent a point, which shall be differentiated semantically using the alias FPoint2D.
	typedef FVector2 FPoint2D;
}
