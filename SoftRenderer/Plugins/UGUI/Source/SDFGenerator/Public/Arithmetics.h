#pragma once

namespace SDFGenerator
{
	/**
	 * Returns the smaller of the arguments.
	 */
	template <typename T>
	T Min(T A, T B)
	{
		return B < A ? B : A;
	}

	/**
	 * Returns the larger of the arguments.
	 */
	template <typename T>
	T Max(T A, T B)
	{
		return A < B ? B : A;
	}

	/**
	 * Returns the middle out of three values
	 */
	template <typename T>
	T Median(T A, T B, T C)
	{
		return Max(Min(A, B), Min(Max(A, B), C));
	}

	/**
	 * Returns the weighted average of a and b.
	 */
	template <typename T, typename S>
	T Mix(T A, T B, S Weight)
	{
		return T((S(1) - Weight) * A + Weight * B);
	}

	/**
	 * Clamps the number to the interval from 0 to 1.
	 */
	template <typename T>
	T Clamp(T V)
	{
		return V >= T(0) && V <= T(1) ? V : T(V > T(0));
	}

	/**
	 * Clamps the number to the interval from 0 to b.
	 */
	template <typename T>
	T Clamp(T V, T B)
	{
		return V >= T(0) && V <= B ? V : T(V > T(0)) * B;
	}

	/**
	 * Clamps the number to the interval from a to b.
	 */
	template <typename T>
	T Clamp(T V, T A, T B)
	{
		return V >= A && V <= B ? V : V < A ? A : B;
	}

	/**
	 * Returns 1 for positive values, -1 for negative values, and 0 for zero.
	 */
	template <typename T>
	int32 Sign(T V)
	{
		return (T(0) < V) - (V < T(0));
	}

	/**
	 * Returns 1 for non-negative values and -1 for negative values.
	 */
	template <typename T>
	int32 NonZeroSign(T V)
	{
		return 2 * (V > T(0)) - 1;
	}
}
