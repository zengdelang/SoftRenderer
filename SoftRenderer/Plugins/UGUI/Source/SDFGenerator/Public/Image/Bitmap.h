#pragma once

namespace SDFGenerator
{
	/**
	 * Reference to a 2D image bitmap or a buffer acting as one. Pixel storage not owned or managed by the object.
	 */
	template <typename T, int N = 1>
	struct FBitmapRef
	{
		T* Pixels;
		int32 Width;
		int32 Height;

		FBitmapRef() : Pixels(nullptr), Width(0), Height(0) {}
		FBitmapRef(T* InPixels, int32 InWidth, int32 InHeight) : Pixels(InPixels), Width(InWidth), Height(InHeight) {}

		 T* operator()(int32 X, int32 Y) const
		{
			return Pixels+ N * (Width * Y + X);
		}
	};

	/**
	 * Constant reference to a 2D image bitmap or a buffer acting as one. Pixel storage not owned or managed by the object.
	 */
	template <typename T, int N = 1>
	struct FBitmapConstRef
	{
		const T* Pixels;
		int32 Width;
		int32 Height;

		FBitmapConstRef() : Pixels(nullptr), Width(0), Height(0) {}
		FBitmapConstRef(const T* InPixels, int32 InWidth, int32 InHeight) : Pixels(InPixels), Width(InWidth), Height(InHeight) {}
		FBitmapConstRef(const FBitmapRef<T, N>& Orig) : Pixels(Orig.Pixels), Width(Orig.Width), Height(Orig.Height) {}

		 const T* operator()(int32 X, int32 Y) const
		{
			return Pixels+ N * (Width * Y + X);
		}
	};
	
	/**
	 * A 2D image bitmap with N channels of type T. Pixel memory is managed by the class.
	 */
	template <typename T, int N = 1>
	class FBitmap
	{
	public:
		FBitmap() : Pixels(nullptr), W(0), H(0)
		{
			
		}
		
		FBitmap(int32 Width, int32 Height) : W(Width), H(Height)
		{
			const uint32 Count = N * W * H;
			Pixels = new T[Count];
			FMemory::Memzero(Pixels, Count);
		}
		
		FBitmap(const FBitmapConstRef<T, N>& Orig) : W(Orig.Width), H(Orig.Height)
		{
			Pixels = new T[N * W * H];
			FMemory::Memcpy(Pixels, Orig.Pixels, sizeof(T) * N * W *H);
		}
		
		FBitmap(const FBitmap<T, N>& Orig) : W(Orig.Width), H(Orig.Height)
		{
			Pixels = new T[N * W * H];
			FMemory::Memcpy(Pixels, Orig.Pixels, sizeof(T) * N * W *H);
		}
		
		FBitmap(FBitmap<T, N>&& Orig) noexcept : Pixels(Orig.Pixels), W(Orig.W), H(Orig.H)
		{
			Orig.Pixels = nullptr;
			Orig.W = 0;
			Orig.H = 0;
		}

		~FBitmap()
		{
			delete[] Pixels;
		}
		
		FBitmap<T, N>& operator=(const FBitmapConstRef<T, N>& Orig)
		{
			if (Pixels != Orig.Pixels)
			{
				delete[] Pixels;
				W = Orig.Width;
				H = Orig.Height;
				Pixels = new T[N * W * H];
				FMemory::Memcpy(Pixels, Orig.Pixels, sizeof(T) * N * W *H);
			}
			return *this;
		}
		
		FBitmap<T, N>& operator=(const FBitmap<T, N>& Orig)
		{
			if (this != &Orig)
			{
				delete[] Pixels;
				W = Orig.W;
				H = Orig.H;
				Pixels = new T[N * W * H];
				FMemory::Memcpy(Pixels, Orig.Pixels, sizeof(T) * N * W *H);
			}
			return *this;
		}
		
		FBitmap<T, N>& operator=(FBitmap<T, N>&& Orig) noexcept
		{
			if (this != &Orig)
			{
				delete[] Pixels;
				Pixels = Orig.Pixels;
				W = Orig.W;
				H = Orig.H;
				Orig.Pixels = nullptr;
			}
			return *this;
		}

		/**
		 * Bitmap width in pixels.
		 */
		int32 Width() const
		{
			return W;
		}
		
		/**
		 * Bitmap height in pixels.
		 */
		int32 Height() const
		{
			return H;
		}
		
		T* operator()(int32 X, int32 Y)
		{
			return Pixels + N * (W * Y + X);
		}
		
		const T* operator()(int32 X, int32 Y) const
		{
			return Pixels + N * (W * Y + X);
		}

		explicit operator T*()
		{
			return Pixels;
		}
		
		explicit operator const T*() const
		{
			return Pixels;
		}

		operator FBitmapRef<T, N>()
		{
			return FBitmapRef<T, N>(Pixels, W, H);
		}
		
		operator FBitmapConstRef<T, N>() const
		{
			return FBitmapConstRef<T, N>(Pixels, W, H);
		}

	private:
		T* Pixels;
		int32 W, H;
	};
}
