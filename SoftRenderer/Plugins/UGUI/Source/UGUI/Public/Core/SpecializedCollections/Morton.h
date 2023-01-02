#pragma once

class FMortonUtility
{
private:
	FORCEINLINE static uint32 Part1By_32Bit(uint32 X)
	{
		X &= 0x0000FFFF;                   // x = ---- ---- ---- ---- fedc ba98 7654 3210
		X = (X ^ (X <<  8)) & 0x00FF00FF;  // x = ---- ---- fedc ba98 ---- ---- 7654 3210
		X = (X ^ (X <<  4)) & 0x0F0F0F0F;  // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
		X = (X ^ (X <<  2)) & 0x33333333;  // x = --fe --dc --ba --98 --76 --54 --32 --10
		X = (X ^ (X <<  1)) & 0x55555555;  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
		return X;
	}

	FORCEINLINE static uint64 Part1By_64Bit(uint64 X)
	{
		X &= 0x00000000FFFFFFFF;
		X = (X ^ (X << 16)) & 0x0000FFFF0000FFFF;
		X = (X ^ (X <<  8)) & 0x00FF00FF00FF00FF;
		X = (X ^ (X <<  4)) & 0x0F0F0F0F0F0F0F0F;
		X = (X ^ (X <<  2)) & 0x3333333333333333;
		X = (X ^ (X <<  1)) & 0x5555555555555555;
		return X;
	}

	FORCEINLINE static uint32 Part1By2_32Bit(uint32 x)
	{
		x &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
		x = (x ^ (x << 16)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
		x = (x ^ (x <<  8)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
		x = (x ^ (x <<  4)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
		x = (x ^ (x <<  2)) & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
		return x;
	}

	FORCEINLINE static uint64 Part1By2_64Bit(uint64 X)
	{
		X &= 0x1fffff;
		X = (X ^ (X << 32)) & 0x1f00000000ffff;
		X = (X ^ (X << 16)) & 0x1f0000ff0000ff;
		X = (X ^ (X <<  8)) & 0x100f00f00f00f00f;
		X = (X ^ (X <<  4)) & 0x10c30c30c30c30c3;
		X = (X ^ (X <<  2)) & 0x1249249249249249;
		return X;
	}

	FORCEINLINE static uint32 Compact1By1_32Bit(uint32 X)
	{
		X &= 0x55555555;                  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
		X = (X ^ (X >>  1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
		X = (X ^ (X >>  2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
		X = (X ^ (X >>  4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
		X = (X ^ (X >>  8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
		return X;
	}

	FORCEINLINE static uint64 Compact1By1_64Bit(uint64 X)
	{
		X &= 0x5555555555555555;                 
		X = (X ^ (X >>  1)) & 0x3333333333333333;
		X = (X ^ (X >>  2)) & 0x0F0F0F0F0F0F0F0F;
		X = (X ^ (X >>  4)) & 0x00FF00FF00FF00FF;
		X = (X ^ (X >>  8)) & 0x0000FFFF0000FFFF;
		X = (X ^ (X >> 16)) & 0x00000000FFFFFFFF;
		return X;
	}

	FORCEINLINE static uint32 Compact1By2_32Bit(uint32 X)
	{
		X &= 0x09249249;                  // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
		X = (X ^ (X >>  2)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
		X = (X ^ (X >>  4)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
		X = (X ^ (X >>  8)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
		X = (X ^ (X >> 16)) & 0x000003ff; // x = ---- ---- ---- ---- ---- --98 7654 3210
		return X;
	}

	FORCEINLINE static uint64 Compact1By2_64Bit(uint64 X)
	{
		X &= 0x1249249249249249;              
		X = (X ^ (X >>  2)) & 0x10c30c30c30c30c3;
		X = (X ^ (X >>  4)) & 0x100f00f00f00f00f;
		X = (X ^ (X >>  8)) & 0x1f0000ff0000ff;
		X = (X ^ (X >> 16)) & 0x1f00000000ffff;
		X = (X ^ (X >> 32)) & 0x1fffff;
		return X;
	}
	
public:
	FORCEINLINE static uint32 EncodeMorton2(uint32 X, uint32 Y)
	{
		return (Part1By_32Bit(Y) << 1) + Part1By_32Bit(X);
	}

	FORCEINLINE static uint64 EncodeMorton2_64bit(uint32 X, uint32 Y)
	{
		return (Part1By_64Bit(Y) << 1) + Part1By_64Bit(X);
	}

public:
	FORCEINLINE static uint32 EncodeMorton3(uint32 X, uint32 Y, uint32 Z)
	{
		return (Part1By2_32Bit(Z) << 2) + (Part1By2_32Bit(Y) << 1) + Part1By2_32Bit(X);
	}

	FORCEINLINE static uint64 EncodeMorton3_64Bit(uint32 X, uint32 Y, uint32 Z)
	{
		return (Part1By2_64Bit(Z) << 2) + (Part1By2_64Bit(Y) << 1) + Part1By2_64Bit(X);
	}

public:
	FORCEINLINE static uint32 DecodeMorton2X(uint32 Code)
	{
		return Compact1By1_32Bit(Code);
	}

	FORCEINLINE static uint32 DecodeMorton2Y(uint32 Code)
	{
		return Compact1By1_32Bit(Code >> 1);
	}

	FORCEINLINE static void DecodeMortonXY(uint32 Code, uint32& X, uint32& Y)
	{
		X = DecodeMorton2X(Code);
		Y = DecodeMorton2Y(Code);
	}

public:
	FORCEINLINE static uint32 DecodeMorton2X(uint64 Code)
	{
		return Compact1By1_64Bit(Code);
	}

	FORCEINLINE static uint32 DecodeMorton2Y(uint64 Code)
	{
		return Compact1By1_64Bit(Code >> 1);
	}

	FORCEINLINE static void DecodeMortonXY(uint64 Code, uint32& X, uint32& Y)
	{
		X = DecodeMorton2X(Code);
		Y = DecodeMorton2Y(Code);
	}

public:
	FORCEINLINE static uint32 DecodeMorton3X(uint32 Code)
	{
		return Compact1By2_32Bit(Code);
	}

	FORCEINLINE static uint32 DecodeMorton3Y(uint32 Code)
	{
		return Compact1By2_32Bit(Code >> 1);
	}

	FORCEINLINE static uint32 DecodeMorton3Z(uint32 Code)
	{
		return Compact1By2_32Bit(Code >> 2);
	}

	FORCEINLINE static void DecodeMortonXYZ(uint32 Code, uint32& X, uint32& Y, uint32& Z)
	{
		X = DecodeMorton3X(Code);
		Y = DecodeMorton3Y(Code);
		Z = DecodeMorton3Z(Code);
	}

public:
	FORCEINLINE static uint32 DecodeMorton3X(uint64 Code)
	{
		return Compact1By2_64Bit(Code);
	}

	FORCEINLINE static uint32 DecodeMorton3Y(uint64 Code)
	{
		return Compact1By2_64Bit(Code >> 1);
	}

	FORCEINLINE static uint32 DecodeMorton3Z(uint64 Code)
	{
		return Compact1By2_64Bit(Code >> 2);
	}

	FORCEINLINE static void DecodeMortonXYZ(uint64 Code, uint32& X, uint32& Y, uint32& Z)
	{
		X = DecodeMorton3X(Code);
		Y = DecodeMorton3Y(Code);
		Z = DecodeMorton3Z(Code);
	}
};
