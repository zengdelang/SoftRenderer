#pragma once

#include "CoreMinimal.h"
#include "Core/SpecializedCollections/WeakObjectIndexedSet.h"

class IClipperInterface;

/**
 * Registry class to keep track of all IClippers that exist in the scene
 *
 * This is used during the CanvasUpdate loop to cull clippable elements. The clipping is called after layout, but before Graphic update.
 */
class FClipperRegistry
{
public:
	static void Initialize();
	static void Shutdown();

	/**
	 * The singleton instance of the clipper registry. Creates new if non exist.
	 */
	static FClipperRegistry* GetInstance()
	{
		if (!Instance.IsValid())
		{
			Initialize();
		}

		return Instance.Get();
	}

public:
	bool IsDirty() const { return bIsDirty.TryGetValue(true); }
	
	/**
	 * Perform the clipping on all registered IClipper
	 */
	void Cull();

	/**
	 * Register a unique IClipper element
	 * 
	 * @param  Clipper  The clipper element to add
	 */
	static void Register(IClipperInterface* Clipper);

	/**
	 * UnRegister a IClipper element
	 *
	 * @param  Clipper  The Element to try and remove.
	 */
	static void Unregister(IClipperInterface* Clipper);
	
private:
	/** Singleton instances of this clipper registry. */
	static TSharedPtr< class FClipperRegistry > Instance;

	FWeakObjectIndexedSet Clippers;

	TFrameValue<bool> bIsDirty;
	
};
