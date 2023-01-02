#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Templates/SubclassOf.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Styling/SlateColor.h"
#include "Layout/Visibility.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"
#include "Engine/SCS_Node.h"
#include "BlueprintEditor.h"
#include "Widgets/SToolTip.h"
#include "SComponentClassCombo.h"
#include "ScopedTransaction.h"
#include "SUIComponentClassCombo.h"

class FMenuBuilder;
class UToolMenu;
class FSCSComponentEditorTreeNode;
class SSCSComponentEditor;
class UPrimitiveComponent;
struct EventData;
class ISCSEditorUICustomization;

// SCS editor tree node pointer types
using FSCSComponentEditorTreeNodePtrType = TSharedPtr<class FSCSComponentEditorTreeNode>;
using FSCSComponentEditorActorNodePtrType = TSharedPtr<class FSCSComponentEditorTreeNodeActorBase>;
using FSCSComponentEditorChildActorNodePtrType = TSharedPtr<class FSCSComponentEditorTreeNodeChildActor>;

/**
 * FSCSComponentEditorTreeNode
 *
 * Wrapper class for nodes displayed in the SCS (Simple Construction Script) editor tree widget.
 */
class UIBLUEPRINTEDITOR_API FSCSComponentEditorTreeNode : public TSharedFromThis<FSCSComponentEditorTreeNode>
{
public:
	/** Delegate for when the context menu requests a rename */
	DECLARE_DELEGATE(FOnRenameRequested);

	enum ENodeType
	{
		ComponentNode,
		RootActorNode,
		SeparatorNode,
		ChildActorNode,
	};

	/**
	 * Constructs an empty tree node.
	 */
	FSCSComponentEditorTreeNode(FSCSComponentEditorTreeNode::ENodeType InNodeType);

	/**
	* @return The name to identify this node.
	*/
	virtual FName GetNodeID() const;

	/**
	 * @return The name of the variable represented by this node.
	 */
	virtual FName GetVariableName() const;
	
	/**
	 * @return The string to be used in the tree display.
	 */
	virtual FString GetDisplayString() const;
	
	/**
	* @return The name of this node in text.
	*/
	virtual FText GetDisplayName() const;
	
	/**
	 * @return The SCS node that is represented by this object, or NULL if there is no associated SCS node.
	 */
	virtual class USCS_Node* GetSCSNode() const;
	
	/**
	 * @param ActualEditedBlueprint currently edited blueprint
	 * @note Derived classes should override GetOrCreateEditableObjectForBlueprint().
	 * @return The component template that can be editable for actual class.
	 */
	inline UActorComponent* GetOrCreateEditableComponentTemplate(UBlueprint* ActualEditedBlueprint) const
	{
		// @TODO - Deprecate this public API in favor of GetEditableObjectForBlueprint().
		return Cast<UActorComponent>(GetOrCreateEditableObjectForBlueprint(ActualEditedBlueprint));
	}
	
	/**
	 * Finds the component instance represented by this node contained within a given Actor instance.
	 *
	 * @param InActor The Actor instance to use as the container object for finding the component instance.
	 * @return The component instance represented by this node and contained within the given Actor instance, or NULL if not found.
	 */
	virtual UActorComponent* FindComponentInstanceInActor(const AActor* InActor) const;
	
	/**
	 * @return This object's parent node (or an invalid reference if no parent is assigned).
	 */
	FSCSComponentEditorTreeNodePtrType GetParent() const { return ParentNodePtr; }
	
	/**
	 * @return The set of nodes which are parented to this node (read-only).
	 */
	const TArray<FSCSComponentEditorTreeNodePtrType>& GetChildren() const { return Children; }
	
	/**
	 * @return The root of the actor subtree to which this node belongs.
	 */
	FSCSComponentEditorActorNodePtrType GetActorRootNode() const { return ActorRootNodePtr; }
	
	/**
	 * Sets the actor root to the given node for this node along with any children.
	 */
	void SetActorRootNode(FSCSComponentEditorActorNodePtrType InActorNode);
	
	/**
	 * @return Type of node
	 */
	ENodeType GetNodeType() const;
	
	/**
	 * @return True if this represents an actor node type
	 */
	bool IsActorNode() const
	{
		return NodeType == ENodeType::RootActorNode || NodeType == ENodeType::ChildActorNode;
	}
	
	/**
	 * @return True if this represents a component node type
	 */
	bool IsComponentNode() const
	{
		return NodeType == ENodeType::ComponentNode;
	}
	
	/**
	 * @param	bEvenIfPendingKill	If false, nullptr will be returned if the cached component template is pending kill.
	 *								If true, it will be returned regardless (this is used for recaching the component template if the objects
	 *								have been reinstanced following construction script execution).
	 *
	 * @note	Deliberately non-virtual, for performance reasons.
	 * @warning This will not return the right component for components overridden by the inherited component handler, you need to call GetOrCreateEditableComponentTemplate instead
	 * @return	The component template or instance represented by this node, if it's a component node.
	 */
	inline UActorComponent* GetComponentTemplate(bool bEvenIfPendingKill = false) const
	{
		// @todo - Deprecate this API? For backwards-compatibility, this continues to provide non-const access to the internal object instance specifically as a component reference.
		return Cast<UActorComponent>(WeakObjectPtr.Get(bEvenIfPendingKill));
	}
	
	/**
	 * @param	bEvenIfPendingKill	If false, nullptr will be returned if the cached object instance is pending kill.
	 *								If true, it will be returned regardless (this is used for recaching the object if the objects
	 *								have been reinstanced following construction script execution).
	 *
	 * @note	Deliberately non-virtual, for performance reasons.
	 * @return	A read-only reference to the object represented by this node.
	 */
	template<class T>
	inline const T* GetObject(bool bEvenIfPendingKill = false) const
	{
		return Cast<T>(WeakObjectPtr.Get(bEvenIfPendingKill));
	}
	
	/**
	 * @param	InBlueprint			The Blueprint in which the object will be edited.
	 *
	 * @note	May not be the same as the value returned by GetObject().
	 * @return	A reference to the object represented by this node that can be modified within the given Blueprint.
	 */
	template<class T>
	inline T* GetEditableObjectForBlueprint(UBlueprint* InBlueprint) const
	{
		return Cast<T>(GetOrCreateEditableObjectForBlueprint(InBlueprint));
	}
	
	/**
	 * Sets the internal object instance represented by this node.
	 */
	inline void SetObject(UObject* InObject)
	{
		WeakObjectPtr = InObject;
	}
	
	/**
	 * @return Whether or not this node is a direct child of the given node.
	 */
	bool IsDirectlyAttachedTo(FSCSComponentEditorTreeNodePtrType InNodePtr) const { return ParentNodePtr == InNodePtr; }
	
	/**
	 * @return Whether or not this node is a child (direct or indirect) of the given node.
	 */
	bool IsAttachedTo(FSCSComponentEditorTreeNodePtrType InNodePtr) const;	

	/**
	 * Finds the closest ancestor node in the given node set.
	 *
	 * @param InNodes The given node set.
	 * @return One of the nodes from the set, or an invalid node reference if the set does not contain any ancestor nodes.
	 */
	FSCSComponentEditorTreeNodePtrType FindClosestParent(TArray<FSCSComponentEditorTreeNodePtrType> InNodes);

	/**
	 * Adds the given node as a child node.
	 *
	 * @param InChildNodePtr The node to add as a child node.
	 */
	virtual void AddChild(FSCSComponentEditorTreeNodePtrType InChildNodePtr);

	/**
	 * Adds a child node for the given SCS node.
	 *
	 * @param InSCSNode The SCS node to for which to create a child node.
	 * @param bIsInheritedSCS Whether or not the given SCS node is inherited from a parent.
	 * @return A reference to the new child node or the existing child node if a match is found.
	 */
	FSCSComponentEditorTreeNodePtrType AddChild(USCS_Node* InSCSNode, bool bIsInheritedSCS);

	/**
	 * Adds a child node for the given component template.
	 *
	 * @param InComponentTemplate The component template for which to create a child node.
	 * @return A reference to the new child node or the existing child node if a match is found.
	 */
	FSCSComponentEditorTreeNodePtrType AddChildFromComponent(UActorComponent* InComponentTemplate);

	/**
	 * Attempts to find a reference to the child node that matches the given SCS node.
	 *
	 * @param InSCSNode The SCS node to match.
	 * @param bRecursiveSearch Whether or not to recursively search child nodes (default == false).
	 * @param OutDepth If non-NULL, the depth of the child node will be returned in this parameter on success (default == NULL).
	 * @return The child node that matches the given SCS node, or an invalid node reference if no match was found.
	 */
	FSCSComponentEditorTreeNodePtrType FindChild(const USCS_Node* InSCSNode, bool bRecursiveSearch = false, uint32* OutDepth = NULL) const;

	/**
	 * Attempts to find a reference to the child node that matches the given component template.
	 *
	 * @param InComponentTemplate The component template instance to match.
	 * @param bRecursiveSearch Whether or not to recursively search child nodes (default == false).
	 * @param OutDepth If non-NULL, the depth of the child node will be returned in this parameter on success (default == NULL).
	 * @return The child node with a component template that matches the given component template instance, or an invalid node reference if no match was found.
	 */
	FSCSComponentEditorTreeNodePtrType FindChild(const UActorComponent* InComponentTemplate, bool bRecursiveSearch = false, uint32* OutDepth = NULL) const;

	/**
	 * Attempts to find a reference to the child node that matches the given component variable or instance name.
	 *
	 * @param InVariableOrInstanceName The component variable or instance name to match.
	 * @param bRecursiveSearch Whether or not to recursively search child nodes (default == false).
	 * @param OutDepth If non-NULL, the depth of the child node will be returned in this parameter on success (default == NULL).
	 * @return The child node with a component variable or instance name that matches the given name, or an invalid node reference if no match was found.
	 */
	FSCSComponentEditorTreeNodePtrType FindChild(const FName& InVariableOrInstanceName, bool bRecursiveSearch = false, uint32* OutDepth = NULL) const;

	/**
	 * Removes the given node from the list of child nodes.
	 *
	 * @param InChildNodePtr The child node to remove.
	 */
	virtual void RemoveChild(FSCSComponentEditorTreeNodePtrType InChildNodePtr);

	bool IsSceneComponent() const
	{
		return Cast<USceneComponent>(GetComponentTemplate()) != nullptr;
	}

	/** Returns the associated child actor node if applicable to this node type. */
	virtual FSCSComponentEditorChildActorNodePtrType GetChildActorNode() { return nullptr; }

	// Tries to find a SCS node that was likely responsible for creating the specified instance component.  Note: This is not always possible to do!
	static USCS_Node* FindSCSNodeForInstance(const UActorComponent* InstanceComponent, UClass* ClassToSearch);

	/**
	 * Creates the correct type of node based on the component (instanced or not, etc...)
	 */
	static FSCSComponentEditorTreeNodePtrType FactoryNodeFromComponent(UActorComponent* InComponent);

	// Destructor
	virtual ~FSCSComponentEditorTreeNode() {}

	/**
	 * Ends the 'Create + enter initial name' transaction of this node. The creation of a node is 'ongoing' as long as the initial name of
	 * the node is in edition mode. When the text is not in edit mode anymore, the ongoing create transaction ends and the node
	 * is considered fully created.
	 */
	void CloseOngoingCreateTransaction();

protected:
	// Called when this node is being removed via a RemoveChild call
	virtual void RemoveMeAsChild() {}

	// Provides derived classes with non-const access to the object represented by this node (e.g. for rename operations, etc.). This should not be made public.
	template<class T>
	inline T* GetMutableObject() const
	{
		return Cast<T>(WeakObjectPtr.Get());
	}

	// Derived classes can override to create and/or return a reference to an alternate editable object.
	virtual UObject* GetOrCreateEditableObjectForBlueprint(UBlueprint* InBlueprint) const;

public:
	/**
	* @return The Blueprint to which the object represented by this node belongs (requires implementation in subclass).
	*/
	virtual UBlueprint* GetBlueprint() const = 0;

	/**
	 * @return Whether or not this object represents a "native" component template (i.e. one that is not found in the SCS tree).
	 */
	virtual bool IsNativeComponent() const { return false; }

	/**
	 * @return Whether or not this object represents a root component.
	 */
	virtual bool IsRootComponent() const { return false; }

	/**
	 * @return Whether or not this object represents an inherited SCS node (one from a SCS node in a parent Blueprint).
	 */
	virtual bool IsInheritedSCSNode() const { return false; }

	/**
	 * @return Whether or not this object was declared in the current class (or instance).  Anything inherited cannot be reorganized (renamed, reparented, etc...).
	 */
	virtual bool IsInheritedComponent() const
	{
		return IsNativeComponent() || IsInheritedSCSNode();
	}

	/**
	 * @return Whether or not this node represents an instanced object (i.e. not a template).
	 */
	virtual bool IsInstanced() const { return false; }

	/**
	 * @return Whether or not this node represents an instanced actor object.
	 */
	bool IsInstancedActor() const
	{
		return IsInstanced() && IsActorNode();
	}

	/**
	 * @return Whether or not this node represents an instanced component object.
	 */
	bool IsInstancedComponent() const
	{
		return IsInstanced() && IsComponentNode();
	}

	/**
	 * @return Whether or not this node represents a Blueprint (i.e. non-native) component object.
	 */
	bool IsBlueprintComponent() const
	{
		return IsComponentNode() && !IsNativeComponent();
	}

	/**
	 * @return Whether or not this object represents a component instance that was created by the user and not by a native or Blueprint-generated class.
	 */
	virtual bool IsUserInstancedComponent() const { return false; }

	/**
	* @return Whether or not this object represents the default SCS scene root component.
	*/
	virtual bool IsDefaultSceneRoot() const { return false; }

	/**
	 * @return Whether or not this object represents a node that can be deleted from the SCS tree.
	 */
	virtual bool CanDelete() const { return false; }

	/**
	 * @return Whether or not this object represents a node that can be reparented to other nodes based on its context.
	 */
	virtual bool CanReparent() const { return false; }

	/**
	 * @return Whether or not we can edit properties for the object represented by this node.
	 */
	virtual bool CanEdit() const { return false; }

	/**
	 * @return Whether or not we can rename the object or variable represented by this node.
	 */
	virtual bool CanRename() const { return false; }

	/**
	 * Requests a rename on the node.
	 * @param OngoingCreateTransaction The transaction scoping the node creation which will end once the node is named by the user or null if the rename is not part of a the creation process.
	 */
	void OnRequestRename(TUniquePtr<FScopedTransaction> OngoingCreateTransaction);

	/** Renames the object or variable represented by this node */
	virtual void OnCompleteRename(const FText& InNewName);

	/** Sets up the delegate for a rename operation */
	void SetRenameRequestedDelegate(FOnRenameRequested InRenameRequested) { RenameRequestedDelegate = InRenameRequested; }

	/** Query that determines if this item should be filtered out or not */
	virtual bool IsFlaggedForFiltration() const 
	{
		return ensureMsgf(FilterFlags != EFilteredState::Unknown, TEXT("Querying a bad filtration state.")) ? 
			(FilterFlags & EFilteredState::FilteredInMask) == 0 : false; 
	}

	/** Returns whether the node will match the given type (for filtering) */
	virtual bool MatchesFilterType(const UClass* InFilterType) const;

	/** Refreshes this item's filtration state. Set bRecursive to 'true' to refresh any child nodes as well */
	virtual bool RefreshFilteredState(const UClass* InFilterType, const TArray<FString>& InFilterTerms, bool bRecursive);

protected:
	/** Sets this item's filtration state. Use bUpdateParent to make sure the parent's EFilteredState::ChildMatches flag is properly updated based off the new state */
	void SetCachedFilterState(bool bMatchesFilter, bool bUpdateParent);
	
	/** Updates the EFilteredState::ChildMatches flag, based off of children's current state */
	void RefreshCachedChildFilterState(bool bUpdateParent);
	
	/** Used to update the EFilteredState::ChildMatches flag for parent nodes, when this item's filtration state has changed */
	void ApplyFilteredStateToParent();
	
	// Scope the creation of a node which ends when the initial 'name' is given/accepted by the user, which can be several frames after the node was actually created.
	TUniquePtr<FScopedTransaction> OngoingCreateTransaction;

private:
	// The type of tree node
	ENodeType NodeType;

	// Weak ptr to the object instance represented by this node (e.g. component template)
	TWeakObjectPtr<UObject> WeakObjectPtr;

	// Actual tree structure
	FSCSComponentEditorTreeNodePtrType ParentNodePtr;
	FSCSComponentEditorActorNodePtrType ActorRootNodePtr;
	TArray<FSCSComponentEditorTreeNodePtrType> Children;

	/** Handles rename requests */
	FOnRenameRequested RenameRequestedDelegate;

	enum EFilteredState
	{
		FilteredOut    = 0x00,
		MatchesFilter  = (1 << 0),
		ChildMatches   = (1 << 1),

		FilteredInMask = (MatchesFilter | ChildMatches),
		Unknown = 0xFC // ~FilteredInMask
	};
	uint8 FilterFlags;

public:
	UE_DEPRECATED(4.26, "Use IsNativeComponent() instead.")
	bool IsNative() const { return IsNativeComponent(); }

	UE_DEPRECATED(4.26, "Use IsInheritedSCSNode() instead.")
	bool IsInheritedSCS() const { return IsInheritedSCSNode(); }

	UE_DEPRECATED(4.26, "Use IsInheritedComponent() instead.")
	bool IsInherited() const { return IsInheritedComponent(); }

	UE_DEPRECATED(4.26, "Use IsUserInstancedComponent() instead.")
	bool IsUserInstanced() const { return IsUserInstancedComponent(); }

	UE_DEPRECATED(4.26, "Use CanEdit() instead.")
	bool CanEditDefaults() const { return CanEdit(); }

	UE_DEPRECATED(4.26, "Use SetObject() instead.")
	void SetComponentTemplate(UActorComponent* Component) { SetObject(Component); }

	UE_DEPRECATED(4.26, "Use RefreshFilteredState() instead. This API has been changed to an internal-only helper method.")
	void UpdateCachedFilterState(bool bMatchesFilter, bool bUpdateParent) { SetCachedFilterState(bMatchesFilter, bUpdateParent); }
};

//////////////////////////////////////////////////////////////////////////
// FSCSComponentEditorTreeNodeComponentBase

class UIBLUEPRINTEDITOR_API FSCSComponentEditorTreeNodeComponentBase : public FSCSComponentEditorTreeNode
{
protected:
	FSCSComponentEditorTreeNodeComponentBase()
		: FSCSComponentEditorTreeNode(FSCSComponentEditorTreeNode::ComponentNode)
	{
	}

public:
	// FSCSEditorTreeNode interface
	virtual FName GetVariableName() const override;
	virtual FString GetDisplayString() const override;
	virtual bool CanRename() const override { return !IsInheritedComponent() && !IsDefaultSceneRoot(); }
	virtual bool CanDelete() const override { return !IsInheritedComponent() && !IsDefaultSceneRoot(); }
	virtual bool CanReparent() const override;
	virtual UBlueprint* GetBlueprint() const override;
	virtual FSCSComponentEditorChildActorNodePtrType GetChildActorNode();
	virtual bool MatchesFilterType(const UClass* InFilterType) const override;
	// End of FSCSEditorTreeNode interface

private:
	/** Child actor node associated with this component node, if applicable. */
	FSCSComponentEditorChildActorNodePtrType ChildActorNodePtr;
};


//////////////////////////////////////////////////////////////////////////
// FSCSComponentEditorTreeNodeInstancedInheritedComponent - A inherited component in the instanced case (either an inherited SCS node or an inherited native component)

class UIBLUEPRINTEDITOR_API FSCSComponentEditorTreeNodeInstancedInheritedComponent : public FSCSComponentEditorTreeNodeComponentBase
{
public:
	/**
	 * Constructs a wrapper around a component template not contained within an SCS tree node (e.g. "native" components).
	 *
	 * @param InComponentTemplate The component template represented by this object.
	 */
	FSCSComponentEditorTreeNodeInstancedInheritedComponent(AActor* Owner, UActorComponent* InComponentTemplate);

	// FSCSEditorTreeNode public interface
	virtual bool IsNativeComponent() const override;
	virtual bool IsRootComponent() const override;
	virtual bool IsInheritedSCSNode() const override;
	virtual bool IsInstanced() const override { return true; }
	virtual bool IsInheritedComponent() const override { return true; }
	virtual bool IsDefaultSceneRoot() const override;
	virtual bool CanEdit() const override;
	virtual FText GetDisplayName() const override;
	// End of FSCSEditorTreeNode public interface

private:
	TWeakObjectPtr<AActor> InstancedComponentOwnerPtr;
};

//////////////////////////////////////////////////////////////////////////
// FSCSEditorComponentTreeNodeInstanceAddedComponent - A unique-to-this instance component

class UIBLUEPRINTEDITOR_API FSCSEditorComponentTreeNodeInstanceAddedComponent : public FSCSComponentEditorTreeNodeComponentBase
{
public:
	/**
	* Constructs a wrapper around a component template not contained within an SCS tree node (e.g. "native" components).
	*
	* @param InComponentTemplate The component template represented by this object.
	*/
	FSCSEditorComponentTreeNodeInstanceAddedComponent(AActor* Owner, UActorComponent* InComponentTemplate);

	// FSCSEditorTreeNode public interface
	virtual bool IsRootComponent() const override;
	virtual bool IsInstanced() const override { return true; }
	virtual bool IsUserInstancedComponent() const override { return true; }
	virtual bool IsDefaultSceneRoot() const override;
	virtual bool CanEdit() const override { return true; }
	virtual FName GetVariableName() const override { return NAME_None; }
	virtual FString GetDisplayString() const override;
	virtual FText GetDisplayName() const override;
	virtual void OnCompleteRename(const FText& InNewName) override;
	// End of FSCSEditorTreeNode public interface

protected:
	// FSCSEditorTreeNode protected interface
	virtual void RemoveMeAsChild() override;
	// End of FSCSEditorTreeNode protected interface

private:
	FName InstancedComponentName;
	TWeakObjectPtr<AActor> InstancedComponentOwnerPtr;
};

//////////////////////////////////////////////////////////////////////////
// FSCSEditorComponentTreeNodeComponent - A generic component in the non-instanced case (either a SCS node or an inherited native component)

class UIBLUEPRINTEDITOR_API FSCSEditorComponentTreeNodeComponent : public FSCSComponentEditorTreeNodeComponentBase
{
public:
	/**
	 * Constructs a wrapper around a component template contained within an SCS tree node.
	 *
	 * @param InSCSNode The SCS tree node represented by this object.
	 * @param bInIsInherited Whether or not the SCS tree node is inherited from a parent Blueprint class.
	 */
	FSCSEditorComponentTreeNodeComponent(class USCS_Node* InSCSNode, bool bInIsInherited = false);

	/**
	 * Constructs a wrapper around a component template not contained within an SCS tree node (e.g. "native" components).
	 *
	 * @param InComponentTemplate The component template represented by this object.
	 */
	FSCSEditorComponentTreeNodeComponent(UActorComponent* InComponentTemplate);


	// FSCSEditorTreeNode public interface
	virtual bool IsNativeComponent() const override;
	virtual bool IsRootComponent() const override;
	virtual bool IsInheritedSCSNode() const override;
	virtual bool IsDefaultSceneRoot() const override;
	virtual bool CanEdit() const override;
	virtual FText GetDisplayName() const override;
	virtual class USCS_Node* GetSCSNode() const override;
	virtual void OnCompleteRename(const FText& InNewName) override;
	// End of FSCSEditorTreeNode public interface

protected:
	// FSCSEditorTreeNode protected interface
	virtual void RemoveMeAsChild() override;
	virtual UObject* GetOrCreateEditableObjectForBlueprint(UBlueprint* InBlueprint) const override;
	// End of FSCSEditorTreeNode protected interface

	/** Get overridden template component, specialized in given blueprint */
	UActorComponent* INTERNAL_GetOverridenComponentTemplate(UBlueprint* Blueprint) const;

private:
	// Was this component inherited from a parent class or introduced in this class?
	bool bIsInheritedSCS;

	// Is this the template coming from an SCS node?
	TWeakObjectPtr<class USCS_Node> SCSNodePtr;
};

class UIBLUEPRINTEDITOR_API FSCSComponentEditorTreeNodeActorBase : public FSCSComponentEditorTreeNode
{
public:
	FSCSComponentEditorTreeNodeActorBase(FSCSComponentEditorTreeNode::ENodeType InNodeType, AActor* InActor)
		: FSCSComponentEditorTreeNode(InNodeType)
	{
		SetObject(InActor);
	}

	FSCSComponentEditorTreeNodePtrType GetOwnerNode() const;
	void SetOwnerNode(FSCSComponentEditorTreeNodePtrType NewOwnerNode);

	FSCSComponentEditorTreeNodePtrType GetSceneRootNode() const;
	void SetSceneRootNode(FSCSComponentEditorTreeNodePtrType NewSceneRootNode);

	/** Returns the set of root nodes */
	const TArray<FSCSComponentEditorTreeNodePtrType>& GetComponentNodes() const;

	// FSCSEditorTreeNode public interface
	virtual FName GetNodeID() const override;
	virtual bool IsInstanced() const override;
	virtual bool CanEdit() const override { return true; }
	virtual void AddChild(FSCSComponentEditorTreeNodePtrType InChildNodePtr) override;
	virtual void RemoveChild(FSCSComponentEditorTreeNodePtrType InChildNodePtr) override;
	virtual UBlueprint* GetBlueprint() const override;
	// End of FSCSEditorTreeNode public interface

protected:
	using Super = FSCSComponentEditorTreeNode;

private:
	/** The actor's subtree owner (if valid) */
	FSCSComponentEditorTreeNodePtrType OwnerNodePtr;
	/** The actor's scene root node (if valid) */
	FSCSComponentEditorTreeNodePtrType SceneRootNodePtr;
	/** Root set of components (contains the root scene component and any non-scene component nodes) */
	TArray<FSCSComponentEditorTreeNodePtrType> ComponentNodes;
};

class UIBLUEPRINTEDITOR_API FSCSComponentEditorTreeNodeRootActor : public FSCSComponentEditorTreeNodeActorBase
{
public:
	FSCSComponentEditorTreeNodeRootActor(AActor* InActor, bool bInAllowRename)
		: FSCSComponentEditorTreeNodeActorBase(FSCSComponentEditorTreeNode::RootActorNode, InActor)
		, bAllowRename(bInAllowRename)
		, CachedFilterType(nullptr)
	{
	}

	// FSCSEditorTreeNode public interface
	virtual bool CanRename() const override { return bAllowRename; }
	virtual void OnCompleteRename(const FText& InNewName) override;
	virtual void AddChild(FSCSComponentEditorTreeNodePtrType InChildNodePtr) override;
	virtual void RemoveChild(FSCSComponentEditorTreeNodePtrType InChildNodePtr) override;
	virtual bool RefreshFilteredState(const UClass* InFilterType, const TArray<FString>& InFilterTerms, bool bRecursive) override;
	// End of FSCSEditorTreeNode public interface

private:
	bool bAllowRename;
	const UClass* CachedFilterType;
	TArray<FString> CachedFilterTerms;
	TSharedPtr<class FSCSComponentEditorTreeNodeSeparator> SceneComponentSeparatorNodePtr;
	TSharedPtr<class FSCSComponentEditorTreeNodeSeparator> NonSceneComponentSeparatorNodePtr;
};

class UIBLUEPRINTEDITOR_API FSCSComponentEditorTreeNodeChildActor : public FSCSComponentEditorTreeNodeActorBase
{
public:
	FSCSComponentEditorTreeNodeChildActor(AActor* InActor)
		: FSCSComponentEditorTreeNodeActorBase(FSCSComponentEditorTreeNode::ChildActorNode, InActor)
	{
	}

	// FSCSEditorTreeNode public interface
	virtual bool IsFlaggedForFiltration() const override;
	// End of FSCSEditorTreeNode public interface

	UChildActorComponent* GetChildActorComponent() const;
};

class UIBLUEPRINTEDITOR_API FSCSComponentEditorTreeNodeSeparator : public FSCSComponentEditorTreeNode
{
public:
	FSCSComponentEditorTreeNodeSeparator()
		: FSCSComponentEditorTreeNode(FSCSComponentEditorTreeNode::SeparatorNode)
	{
	}

	// FSCSEditorTreeNode public interface
	virtual UBlueprint* GetBlueprint() const override { return nullptr; }
	virtual bool MatchesFilterType(const UClass* InFilterType) const override;
	// End of FSCSEditorTreeNode public interface

	/** If the given type matches the tree view filter, the separator will also be flagged for filtration. */
	void AddFilteredComponentType(const TSubclassOf<UActorComponent>& InFilteredType);

private:
	TArray<const UClass*> FilteredTypes;
};

//////////////////////////////////////////////////////////////////////////
// SSCS_UI_RowWidget

class SSCS_UI_RowWidget : public SMultiColumnTableRow<FSCSComponentEditorTreeNodePtrType>
{
public:
	SLATE_BEGIN_ARGS( SSCS_UI_RowWidget ){}
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, TSharedPtr<SSCSComponentEditor> InSCSEditor, FSCSComponentEditorTreeNodePtrType InNodePtr, TSharedPtr<STableViewBase> InOwnerTableView  );
	virtual ~SSCS_UI_RowWidget() override;

	// SMultiColumnTableRow<T> interface
	virtual TSharedRef<SWidget> GenerateWidgetForColumn( const FName& ColumnName ) override;
	// End of SMultiColumnTableRow<T>

	// SWidget interface
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End of SWidget interface

	/** Get the blueprint we are editing */
	UBlueprint* GetBlueprint() const;

	FText GetNameLabel() const;
	FSlateColor GetColorForNameLabel() const;
	FText GetTooltipText() const;
	FSlateColor GetColorTintForIcon() const;
	FSlateColor GetColorTintForText() const;
	FString GetDocumentationLink() const;
	FString GetDocumentationExcerptName() const;
	
	UIBLUEPRINTEDITOR_API static FLinearColor GetColorTintForIcon(FSCSComponentEditorTreeNodePtrType InNode);

	bool IsNodeEnabled() const;
	
	FText GetAssetName() const;
	FText GetAssetPath() const;
	EVisibility GetAssetVisibility() const;

	/* Get the node used by the row Widget */
	virtual FSCSComponentEditorTreeNodePtrType GetNode() const { return TreeNodePtr; };

protected:
	virtual ESelectionMode::Type GetSelectionMode() const override;

	virtual const FSlateBrush* GetIconBrush() const;

	static void AddToToolTipInfoBox(const TSharedRef<SVerticalBox>& InfoBox, const FText& Key, TSharedRef<SWidget> ValueIcon, const TAttribute<FText>& Value, bool bImportant);

	/** Commits the new name of the component */
	void OnNameTextCommit(const FText& InNewName, ETextCommit::Type InTextCommit);

	/** Handles clicking the visibility toggle */
	FReply OnToggleVisibility();

	void ToggleVisibilityForChildren(FSCSComponentEditorTreeNode* ChildTreeNode, bool bIsVisible);
	
	EVisibility GetLockBrushVisibility() const;

	/** Handles clicking the locked toggle */
	FReply OnToggleLockedInDesigner();

	void ToggleLockedInDesignerForChildren(FSCSComponentEditorTreeNode* ChildTreeNode, bool bLock);
	
	/** Returns a brush representing the lock item of the widget's lock button */
	FText GetLockBrushForWidget() const;

	/** Returns a brush representing the visibility item of the widget's visibility button */
	FText GetVisibilityBrushForWidget() const;

	EVisibility GetVisibilityBrushVisibility() const;

private:
	/** Verifies the name of the component when changing it */
	bool OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage);

	/** Builds a context menu popup for dropping a child node onto the scene root node */
	TSharedPtr<SWidget> BuildSceneRootDropActionMenu(FSCSComponentEditorTreeNodePtrType DroppedNodePtr);

	/** Creates a tooltip for this row */
	TSharedRef<SToolTip> CreateToolTipWidget() const;

	/** Drag-drop handlers */
	void HandleOnDragEnter(const FDragDropEvent& DragDropEvent);
	void HandleOnDragLeave(const FDragDropEvent& DragDropEvent);
	FReply HandleOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	TOptional<EItemDropZone> HandleOnCanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, FSCSComponentEditorTreeNodePtrType TargetItem);
	FReply HandleOnAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, FSCSComponentEditorTreeNodePtrType TargetItem);

	/** Handler for attaching a single node to this node */
	void OnAttachToDropAction(FSCSComponentEditorTreeNodePtrType DroppedNodePtr)
	{
		TArray<FSCSComponentEditorTreeNodePtrType> DroppedNodePtrs;
		DroppedNodePtrs.Add(DroppedNodePtr);
		OnAttachToDropAction(DroppedNodePtrs, EItemDropZone::OntoItem);
	}

	/** Handler for attaching one or more nodes to this node */
	void OnAttachToDropAction(const TArray<FSCSComponentEditorTreeNodePtrType>& DroppedNodePtrs, EItemDropZone DropZone);

	/** Handler for detaching one or more nodes from the current parent and reattaching to the existing scene root node */
	void OnDetachFromDropAction(const TArray<FSCSComponentEditorTreeNodePtrType>& DroppedNodePtrs);

	/** Handler for making the given node the new scene root node */
	void OnMakeNewRootDropAction(FSCSComponentEditorTreeNodePtrType DroppedNodePtr);
	
	/** Tasks to perform after handling a drop action */
	void PostDragDropAction(bool bRegenerateTreeNodes);

	/**
	 * Retrieves an image brush signifying the specified component's mobility (could sometimes be NULL).
	 * 
	 * @returns A pointer to the FSlateBrush to use (NULL for Static and Non-SceneComponents)
	 */
	FSlateBrush const* GetMobilityIconImage() const;

	/**
	 * Retrieves tooltip text describing the specified component's mobility.
	 * 
	 * @returns An FText object containing a description of the component's mobility
	 */
	FText GetMobilityToolTipText() const;

	/**
	 * Retrieves tooltip text describing where the component was first introduced (for inherited components).
	 * 
	 * @returns An FText object containing a description of when the component was first introduced
	 */
	FText GetIntroducedInToolTipText() const;

	/**
	 * Retrieves tooltip text describing how the component was introduced
	 * 
	 * @returns An FText object containing a description of when the component was first introduced
	 */
	FText GetComponentAddSourceToolTipText() const;

	/**
	 * Retrieves tooltip text for the specified Native Component's underlying Name
	 *
	 * @returns An FText object containing the Component's Name
	 */
	FText GetNativeComponentNameToolTipText() const;

public:
	/** Pointer back to owning SCSEditor 2 tool */
	TWeakPtr<SSCSComponentEditor> SCSEditor;
	TSharedPtr<SInlineEditableTextBlock> InlineWidget;
private:
	/** Pointer to node we represent */
	FSCSComponentEditorTreeNodePtrType TreeNodePtr;
};

class SSCS_UI_RowWidget_ActorRoot : public SSCS_UI_RowWidget
{
public:

	// SMultiColumnTableRow<T> interface
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	// End of SMultiColumnTableRow<T>

protected:
	/** Gets the associated actor node */
	FSCSComponentEditorActorNodePtrType GetActorNode() const;

	/** Data accessors */
	virtual const FSlateBrush* GetIconBrush() const override;

private:
	/** Creates a tooltip for this row */
	TSharedRef<SToolTip> CreateToolTipWidget() const;

	/** Called to validate the actor name */
	bool OnVerifyActorLabelChanged(const FText& InLabel, FText& OutErrorMessage);

	/** Data accessors */
	FText GetActorDisplayText() const;
	FText GetActorContextText() const;
	FText GetActorClassNameText() const;
	FText GetActorSuperClassNameText() const;
	FText GetActorMobilityText() const;
};

class SSCS_UI_RowWidget_Separator : public SSCS_UI_RowWidget
{
public:

	// SMultiColumnTableRow<T> interface
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	// End of SMultiColumnTableRow<T>

private:

};

//////////////////////////////////////////////////////////////////////////
// SSCSComponentEditorDragDropTree - implements STreeView for our specific node type and adds drag/drop functionality
class SSCSComponentEditorDragDropTree : public STreeView<FSCSComponentEditorTreeNodePtrType>
{
public:
	SLATE_BEGIN_ARGS( SSCSComponentEditorDragDropTree )
		: _SCSEditor( NULL )
		, _OnGenerateRow()
		, _OnGetChildren()
		, _OnSetExpansionRecursive()
		, _TreeItemsSource( static_cast< TArray<FSCSComponentEditorTreeNodePtrType>* >(NULL) ) //@todo Slate Syntax: Initializing from NULL without a cast
		, _ItemHeight(16)
		, _OnContextMenuOpening()
		, _OnMouseButtonDoubleClick()
		, _OnSelectionChanged()
		, _OnExpansionChanged()
		, _SelectionMode(ESelectionMode::Multi)
		, _ClearSelectionOnClick(true)
		, _ExternalScrollbar()
		, _OnTableViewBadState()
		{
			_Clipping = EWidgetClipping::ClipToBounds;
		}

		SLATE_ARGUMENT( SSCSComponentEditor*, SCSEditor )

		SLATE_EVENT( FOnGenerateRow, OnGenerateRow )

		SLATE_EVENT( FOnItemScrolledIntoView, OnItemScrolledIntoView )

		SLATE_EVENT( FOnGetChildren, OnGetChildren )

		SLATE_EVENT( FOnSetExpansionRecursive, OnSetExpansionRecursive )

		SLATE_ARGUMENT( TArray<FSCSComponentEditorTreeNodePtrType>* , TreeItemsSource )

		SLATE_ATTRIBUTE( float, ItemHeight )

		SLATE_EVENT( FOnContextMenuOpening, OnContextMenuOpening )

		SLATE_EVENT( FOnMouseButtonDoubleClick, OnMouseButtonDoubleClick )

		SLATE_EVENT( FOnSelectionChanged, OnSelectionChanged )

		SLATE_EVENT( FOnExpansionChanged, OnExpansionChanged )

		SLATE_ATTRIBUTE( ESelectionMode::Type, SelectionMode )

		SLATE_ARGUMENT( TSharedPtr<SHeaderRow>, HeaderRow )

		SLATE_ARGUMENT ( bool, ClearSelectionOnClick )

		SLATE_ARGUMENT( TSharedPtr<SScrollBar>, ExternalScrollbar )

		SLATE_EVENT( FOnTableViewBadState, OnTableViewBadState )

	SLATE_END_ARGS()
	/** Object construction - mostly defers to the base STreeView */
	void Construct( const FArguments& InArgs );

	// SWidget interface
	virtual FReply OnDragOver( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	virtual FReply OnDrop( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	// End SWidget interface

private:
	/** Pointer to the SSCSEditor that owns this widget */
	SSCSComponentEditor* SCSEditor;
};

//////////////////////////////////////////////////////////////////////////
// SSCSEditor

typedef SSCSComponentEditorDragDropTree SSCSUITreeType;

/* Component editor mode */
namespace EUIBlueprintComponentEditorMode
{
	enum Type
	{
		/* View/edit the SCS in a BGPC */
		BlueprintSCS,
		/* View/edit the Actor instance */
		ActorInstance
	};
};

class UIBLUEPRINTEDITOR_API SSCSComponentEditor : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(class USCS_Node*, FOnAddNewComponent, class UClass*);
	DECLARE_DELEGATE_RetVal_OneParam(class USCS_Node*, FOnAddExistingComponent, class UActorComponent*);
	DECLARE_DELEGATE_TwoParams(FOnSelectionUpdated, const TArray<FSCSComponentEditorTreeNodePtrType>&, bool);
	DECLARE_DELEGATE_OneParam(FOnItemDoubleClicked, const FSCSComponentEditorTreeNodePtrType);
	DECLARE_DELEGATE_OneParam(FOnHighlightPropertyInDetailsView, const class FPropertyPath&);

	SLATE_BEGIN_ARGS( SSCSComponentEditor )
		:_EditorMode(EUIBlueprintComponentEditorMode::BlueprintSCS)
		,_IsDiffing(false)
		,_ActorContext(nullptr)
		,_PreviewActor(nullptr)
		,_AllowEditing(true)
		,_HideComponentClassCombo(false)
		,_ComponentTypeFilter()
		,_OnSelectionUpdated()
		,_OnHighlightPropertyInDetailsView()
		{}

		SLATE_ARGUMENT(EUIBlueprintComponentEditorMode::Type, EditorMode)
		SLATE_ARGUMENT(bool, IsDiffing)
		SLATE_ATTRIBUTE(class AActor*, ActorContext)
		SLATE_ATTRIBUTE(class AActor*, PreviewActor)
		SLATE_ATTRIBUTE(bool, AllowEditing)
		SLATE_ATTRIBUTE(bool, HideComponentClassCombo)
		SLATE_ATTRIBUTE(TSubclassOf<UActorComponent>, ComponentTypeFilter)
		SLATE_EVENT(FOnSelectionUpdated, OnSelectionUpdated)
		SLATE_EVENT(FOnItemDoubleClicked, OnItemDoubleClicked)
		SLATE_ARGUMENT(TWeakPtr<class FUIBlueprintEditor>, BlueprintEditor)
		SLATE_EVENT(FOnHighlightPropertyInDetailsView, OnHighlightPropertyInDetailsView)

	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	virtual ~SSCSComponentEditor() override;

	/** SWidget interface */
	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;

	/** Used by tree control - make a widget for a table row from a node */
	TSharedRef<ITableRow> MakeTableRowWidget(FSCSComponentEditorTreeNodePtrType InNodePtr, const TSharedRef<STableViewBase>& OwnerTable);

	/** Used by tree control - get children for a specified node */
	void OnGetChildrenForTree(FSCSComponentEditorTreeNodePtrType InNodePtr, TArray<FSCSComponentEditorTreeNodePtrType>& OutChildren);

	/** Returns true if editing is allowed */
	bool IsEditingAllowed() const;

	/** gets the actor root */
	FSCSComponentEditorActorNodePtrType GetActorNode() const;

	/** get the root scene node */
	FSCSComponentEditorTreeNodePtrType GetSceneRootNode() const;

	void SetSceneRootNode(FSCSComponentEditorTreeNodePtrType NewSceneRootNode);

	/** Adds a component to the SCS tree */
	struct FAddNewComponentParams
	{
		FAddNewComponentParams()
			: bSkipMarkBlueprintModified(false)
			, bSetFocusToNewItem(true)
			, bConformTransformToParent(true)
		{
		}
		
		/** Optionally skip marking this blueprint as modified (e.g. if we're handling that externally */
		bool bSkipMarkBlueprintModified;
		/** Whether the newly created component should be focused */
		bool bSetFocusToNewItem;
		/** Whether the newly created component should keep its transform, or conform it to its parent */
		bool bConformTransformToParent;
	};

	/** Adds a component to the SCS Table
	   @param NewComponentClass				(In) The class to add
	   @param Asset       					(In) Optional asset to assign to the component
	   @param Params       					(In) Parameter block of optional behavior flags
	 */
	UActorComponent* AddNewComponent(UClass* NewComponentClass, UObject* Asset, const FAddNewComponentParams Params = FAddNewComponentParams());

	/** Adds a component to the SCS tree */
	UE_DEPRECATED(4.26, "Use version that takes parameter block")
	UActorComponent* AddNewComponent(UClass* NewComponentClass, UObject* Asset, const bool bSkipMarkBlueprintModified, bool bSetFocusToNewItem = true)
	{
		FAddNewComponentParams Params;
		Params.bSkipMarkBlueprintModified = bSkipMarkBlueprintModified;
		Params.bSetFocusToNewItem = bSetFocusToNewItem;
		return AddNewComponent(NewComponentClass, Asset, Params);
	}

	struct FAddedNodeDetails
	{
		FSCSComponentEditorTreeNodePtrType NewNodePtr;
		FSCSComponentEditorTreeNodePtrType ParentNodePtr;
	};

	/** Adds a new SCS Node to the component Table
	   @param OutNodeDetails (Out) Struct to be populated by the tree node pointers of the new node and its parent
	   @param OngoingCreateTransaction (In) The transaction containing the creation of the node. The transaction will remain ongoing until the node gets its initial name from user.
	   @param NewNode	(In) The SCS node to add
	   @param Asset		(In) Optional asset to assign to the component
	   @param bMarkBlueprintModified (In) Whether or not to mark the Blueprint as structurally modified
	   @param bSetFocusToNewItem (In) Select the new item and activate the inline rename widget (default is true) */
	void AddNewNode(FAddedNodeDetails& OutNodeDetails, TUniquePtr<FScopedTransaction> OngoingCreateTransaction, USCS_Node* NewNode, UObject* Asset, bool bMarkBlueprintModified, bool bSetFocusToNewItem = true);

	/** Adds a new SCS Node to the component Table
	   @param OngoingCreateTransaction (In) The transaction containing the creation of the node. The transaction will remain ongoing until the node gets its initial name from user.
	   @param NewNode	(In) The SCS node to add
	   @param Asset		(In) Optional asset to assign to the component
	   @param bMarkBlueprintModified (In) Whether or not to mark the Blueprint as structurally modified
	   @param bSetFocusToNewItem (In) Select the new item and activate the inline rename widget (default is true)
	   @return The reference of the newly created ActorComponent */
	UActorComponent* AddNewNode(TUniquePtr<FScopedTransaction> OngoingCreateTransaction, USCS_Node* NewNode, UObject* Asset, bool bMarkBlueprintModified, bool bSetFocusToNewItem = true);

	/** Adds a new component instance node to the component Table
		@param OngoingCreateTransaction (In) The transaction containing the creation of the node. The transaction will remain ongoing until the node gets its initial name from user.
		@param NewInstanceComponent	(In) The component being added to the actor instance
		@param InParentNodePtr (In) The node this component will be added to
		@param Asset (In) Optional asset to assign to the component
		@param bSetFocusToNewItem (In) Select the new item and activate the inline rename widget (default is true) */
	void AddNewNodeForInstancedComponent(TUniquePtr<FScopedTransaction> OngoingCreateTransaction, UActorComponent* NewInstanceComponent, FSCSComponentEditorTreeNodePtrType InParentNodePtr, UObject* Asset, bool bSetFocusToNewItem = true);
	
	/** Returns true if the specified component is currently selected */
	bool IsComponentSelected(const UPrimitiveComponent* PrimComponent) const;

	/** Assigns a selection override delegate to the specified component */
	void SetSelectionOverride(UPrimitiveComponent* PrimComponent) const;

	/** Cut selected node(s) */
	void CutSelectedNodes();
	bool CanCutNodes() const;

	/** Copy selected node(s) */
	void CopySelectedNodes();
	bool CanCopyNodes() const;

	/** Pastes previously copied node(s) */
	void PasteNodes();
	bool CanPasteNodes() const;

	/** Callbacks to duplicate the selected component */
	bool CanDuplicateComponent() const;
	void OnDuplicateComponent();

	/** Removes existing selected component nodes from the SCS */
	void OnDeleteNodes();
	bool CanDeleteNodes() const;

	/** Callbacks to find references of the selected component */
	void OnFindReferences();

	void ConvertActorSequences();

	/** Removes an existing component node from the tree */
	void RemoveComponentNode(FSCSComponentEditorTreeNodePtrType InNodePtr);

	/** Called when selection in the tree changes */
	void OnTreeSelectionChanged(FSCSComponentEditorTreeNodePtrType InSelectedNodePtr, ESelectInfo::Type SelectInfo);

	/** Called when the Actor is selected. */
	void OnActorSelected(const ECheckBoxState NewCheckedState);

	/** Called to determine if actor is selected. */
	ECheckBoxState OnIsActorSelected() const;

	/** Update any associated selection (e.g. details view) from the passed in nodes */
	void UpdateSelectionFromNodes(const TArray<FSCSComponentEditorTreeNodePtrType> &SelectedNodes, bool bUpdateDesigner = false);

	/** Refresh the tree control to reflect changes in the SCS */
	void UpdateTree(bool bRegenerateTreeNodes = true);

	/** Dumps out the tree view contents to the log (used to assist with debugging widget hierarchy issues) */
	void DumpTree();

	/** Forces the details panel to refresh on the same objects */
	void RefreshSelectionDetails(bool bUpdateDesigner = false);

	/** Clears the current selection */
	void ClearSelection();

	/** Get the currently selected tree nodes */
	TArray<FSCSComponentEditorTreeNodePtrType> GetSelectedNodes() const;

	/** Get the number of currently selected tree nodes */
	int32 GetNumSelectedNodes() const { return SCSTreeWidget->GetNumItemsSelected(); }

	/**
	 * Fills out an events section in ui.
	 * @param Menu								the menu to add the events section into
	 * @param Blueprint							the active blueprint context being edited
	 * @param SelectedClass						the common component class to build the events list from
	 * @param CanExecuteActionDelegate			the delegate to query whether or not to execute the UI action
	 * @param GetSelectedObjectsDelegate		the delegate to fill the currently select variables / components
	 */
	static void BuildMenuEventsSection( FMenuBuilder& Menu, UBlueprint* Blueprint, UClass* SelectedClass, FCanExecuteAction CanExecuteActionDelegate, FGetSelectedObjectsDelegate GetSelectedObjectsDelegate );

	/**
	 * Given an actor component, attempts to find an associated tree node.
	 *
	 * @param ActorComponent The component associated with the node.
	 * @param bIncludeAttachedComponents Whether or not to include components attached to each node in the search (default is true).
	 * @return A shared pointer to a tree node. The pointer will be invalid if no match could be found.
	 */
	FSCSComponentEditorTreeNodePtrType GetNodeFromActorComponent(const UActorComponent* ActorComponent, bool bIncludeAttachedComponents = true) const;

	/** Select the root of the tree */
	void SelectRoot();

	/** Select the given tree node */
	void SelectNode(FSCSComponentEditorTreeNodePtrType InNodeToSelect, bool IsCtrlDown);

	void SelectNodes(const TArray<FSCSComponentEditorTreeNodePtrType>& NodesToSelect);

	/**
	 * Set the expansion state of a node
	 *
	 * @param InNodeToChange	The node to be expanded/collapsed
	 * @param bIsExpanded		True to expand the node, false to collapse it
	 */
	void SetNodeExpansionState(FSCSComponentEditorTreeNodePtrType InNodeToChange, const bool bIsExpanded);

	/**
	 * Highlight a tree node and, optionally, a property with in it
	 *
	 * @param TreeNodeName		Name of the treenode to be highlighted
	 * @param Property	The name of the property to be highlighted in the details view
	 * @return True if the node was found in this Editor, otherwise false
	 */
	void HighlightTreeNode( FName TreeNodeName, const class FPropertyPath& Property );
	/**
	 * Highlight a tree node and, optionally, a property with in it
	 *
	 * @param Node		 A Reference to the Node SCS_Node to be highlighted
	 * @param Property	The name of the property to be highlighted in the details view
	 */
	void HighlightTreeNode( const USCS_Node* Node, FName Property );

	/**
	 * Function to save current state of SimpleConstructionScript and nodes associated with it.
	 *
	 * @param: SimpleContructionScript object reference.
	 */
	static void SaveSCSCurrentState( USimpleConstructionScript* SCSObj );

	/**
	 * Function to save the current state of SCS_Node and its children
	 *
	 * @param: Reference of the SCS_Node to be saved
	 */
	static void SaveSCSNode(USCS_Node* Node);

	/** Is this node still used by the Simple Construction Script */
	bool IsNodeInSimpleConstructionScript(USCS_Node* Node) const;

	/**
	 * Fills the supplied array with the currently selected objects
	 * @param OutSelectedItems The array to fill.
	 */
	void GetSelectedItemsForContextMenu(TArray<FComponentEventConstructionData>& OutSelectedItems) const;

	/** @return Array of the editable objects selected in the tree */
	TArray<UObject*> GetSelectedEditableObjects() const;

	/** Provides access to the Blueprint context that's being edited */
	class UBlueprint* GetBlueprint() const;

	/** @return The current editor mode (editing live actors or editing blueprints) */
	EUIBlueprintComponentEditorMode::Type GetEditorMode() const { return EditorMode; }

	/** Try to handle a drag-drop operation */
	FReply TryHandleAssetDragDropOperation(const FDragDropEvent& DragDropEvent);

	/** Handler for recursively expanding/collapsing items */
	void SetItemExpansionRecursive(FSCSComponentEditorTreeNodePtrType Model, bool bInExpansionState);

	/** Callback for the action trees to get the filter text */
	FText GetFilterText() const;

	/** Called at the end of each frame. */
	void OnPostTick(float);

	/** Sets UI customizations of this SCSEditor. */
	void SetUICustomization(TSharedPtr<ISCSEditorUICustomization> InUICustomization);

protected:
	FSCSComponentEditorTreeNodePtrType FindOrCreateParentForExistingComponent(UActorComponent* InActorComponent, FSCSComponentEditorActorNodePtrType ActorRootNode);
	FSCSComponentEditorTreeNodePtrType FindParentForNewComponent(UActorComponent* NewComponent) const;
	FSCSComponentEditorTreeNodePtrType FindParentForNewNode(USCS_Node* NewNode) const;

	/** Add a component from the selection in the combo box */
	UActorComponent* PerformComboAddClass(TSubclassOf<UActorComponent> ComponentClass, EUIComponentCreateAction::Type ComponentCreateAction, UObject* AssetOverride);

	/** Called to display context menu when right clicking on the widget */
	TSharedPtr< SWidget > CreateContextMenu();

	/** Registers context menu by name for later access */
	void RegisterContextMenu();

	/** Populate context menu on the fly */
	void PopulateContextMenu(UToolMenu* InMenu);

	/** Called when the level editor requests a component to be renamed. */
	void OnLevelComponentRequestRename(const UActorComponent* InComponent);

	/** Checks to see if renaming is allowed on the selected component */
	bool CanRenameComponent() const;

	/**
	 * Requests a rename on the selected component just after creation so that the user can provide the initial
	 * component name (overwriting the default generated one), which is considered part of the creation process.
	 * @param OngoingCreateTransaction The ongoing transaction started when the component was created.
	 */
	void OnRenameComponent(TUniquePtr<FScopedTransaction> OngoingCreateTransaction);

	/**
	 * Requests a rename on the selected component.
	 */
	void OnRenameComponent();

	/** Called when component objects are replaced following construction script execution */
	void OnObjectsReplaced(const TMap<UObject*, UObject*>& OldToNewInstanceMap);

	/** Helper method to update component pointers held by the given actor node's subtree */
	void ReplaceComponentReferencesInTree(FSCSComponentEditorActorNodePtrType InActorNode, const TMap<UObject*, UObject*>& OldToNewInstanceMap);

	/** Update component pointers held by tree nodes if components have been replaced following construction script execution */
	void ReplaceComponentReferencesInTree(const TArray<FSCSComponentEditorTreeNodePtrType>& Nodes, const TMap<UObject*, UObject*>& OldToNewInstanceMap);

	/**
	 * Function to create events for the current selection
	 * @param Blueprint						the active blueprint context
	 * @param EventName						the event to add
	 * @param GetSelectedObjectsDelegate	the delegate to gather information about current selection
	 * @param NodeIndex						an index to a specified node to add event for or < 0 for all selected nodes.
	 */
	static void CreateEventsForSelection(UBlueprint* Blueprint, FName EventName, FGetSelectedObjectsDelegate GetSelectedObjectsDelegate);

	/**
	 * Function to construct an event for a node
	 * @param Blueprint						the nodes blueprint
	 * @param EventName						the event to add
	 * @param EventData						the event data structure describing the node
	 */
	static void ConstructEvent(UBlueprint* Blueprint, const FName EventName, const FComponentEventConstructionData EventData);

	/**
	 * Function to view an event for a node
	 * @param Blueprint						the nodes blueprint
	 * @param EventName						the event to view
	 * @param EventData						the event data structure describing the node
	 */
	static void ViewEvent(UBlueprint* Blueprint, const FName EventName, const FComponentEventConstructionData EventData);

	/** Helper method to add a tree node for the given SCS node */
	FSCSComponentEditorTreeNodePtrType AddTreeNode(USCS_Node* InSCSNode, FSCSComponentEditorTreeNodePtrType InParentNodePtr, const bool bIsInheritedSCS);

	/** Helper method to add a tree node for the given actor component */
	FSCSComponentEditorTreeNodePtrType AddTreeNodeFromComponent(UActorComponent* InSceneComponent, FSCSComponentEditorTreeNodePtrType InParentTreeNode = FSCSComponentEditorTreeNodePtrType());
	
	/** Helper method to add a tree node for the given node's child actor node, if present */
	FSCSComponentEditorTreeNodePtrType AddTreeNodeFromChildActor(FSCSComponentEditorTreeNodePtrType InNodePtr);
	
	/** Helper method to recursively find a tree node for the given SCS node starting at the given tree node */
	FSCSComponentEditorTreeNodePtrType FindTreeNode(const USCS_Node* InSCSNode, FSCSComponentEditorTreeNodePtrType InStartNodePtr = FSCSComponentEditorTreeNodePtrType()) const;

	/** Helper method to recursively find a tree node for the given scene component starting at the given tree node */
	FSCSComponentEditorTreeNodePtrType FindTreeNode(const UActorComponent* InComponent, FSCSComponentEditorTreeNodePtrType InStartNodePtr = FSCSComponentEditorTreeNodePtrType()) const;

	/** Helper method to recursively find a tree node for the given variable or instance name starting at the given tree node */
	FSCSComponentEditorTreeNodePtrType FindTreeNode(const FName& InVariableOrInstanceName, FSCSComponentEditorTreeNodePtrType InStartNodePtr = FSCSComponentEditorTreeNodePtrType()) const;

	/** Callback when a component item is scrolled into view */
	void OnItemScrolledIntoView( FSCSComponentEditorTreeNodePtrType InItem, const TSharedPtr<ITableRow>& InWidget);

	/** Callback when a component item is double clicked. */
	void HandleItemDoubleClicked(FSCSComponentEditorTreeNodePtrType InItem);

	/** Recursively visits the given node + its children and invokes the given function for each. */
	void DepthFirstTraversal(const FSCSComponentEditorTreeNodePtrType& InNodePtr, TSet<FSCSComponentEditorTreeNodePtrType>& OutVisitedNodes, const TFunctionRef<void(const FSCSComponentEditorTreeNodePtrType&)> InFunction) const;

	/** Returns the set of expandable nodes that are currently collapsed in the UI */
	void GetCollapsedNodes(const FSCSComponentEditorTreeNodePtrType& InNodePtr, TSet<FSCSComponentEditorTreeNodePtrType>& OutCollapsedNodes) const;

	/** @return The visibility of the promote to blueprint button (only visible with an actor instance that is not created from a blueprint)*/
	EVisibility GetPromoteToBlueprintButtonVisibility() const;

	/** @return The visibility of the Edit Blueprint button (only visible with an actor instance that is created from a blueprint)*/
	EVisibility GetEditBlueprintButtonVisibility() const;

	/** @return The visibility of the Add Component combo button */
	EVisibility GetComponentClassComboButtonVisibility() const;

	/** @return The visibility of the components tree */
	EVisibility GetComponentsTreeVisibility() const;

	/** @return The visibility of the components filter box */
	EVisibility GetComponentsFilterBoxVisibility() const;

	/** @return the tooltip describing how many properties will be applied to the blueprint */
	FText OnGetApplyChangesToBlueprintTooltip() const;

	/** @return the tooltip describing how many properties will be reset to the blueprint default*/
	FText OnGetResetToBlueprintDefaultsTooltip() const;

	/** Opens the blueprint editor for the blueprint being viewed by the scseditor */
	void OnOpenBlueprintEditor(bool bForceCodeEditing) const;

	/** Propagates instance changes to the blueprint */
	void OnApplyChangesToBlueprint() const;

	/** Resets instance changes to the blueprint default */
	void OnResetToBlueprintDefaults();

	/** Converts the current actor instance to a blueprint */
	void PromoteToBlueprint() const;

	/** Called when the promote to blueprint button is clicked */
	FReply OnPromoteToBlueprintClicked();

	/** gets a root nodes of the tree */
	const TArray<FSCSComponentEditorTreeNodePtrType>& GetRootNodes() const;

	/**
	 * Creates a new C++ component from the specified class type
	 * The user will be prompted to pick a new subclass name and code will be recompiled
	 *
	 * @return The new class that was created
	 */
	UClass* CreateNewCPPComponent(TSubclassOf<UActorComponent> ComponentClass);
	
	/**
	 * Creates a new Blueprint component from the specified class type
	 * The user will be prompted to pick a new subclass name and a blueprint asset will be created
	 *
	 * @return The new class that was created
	 */
	UClass* CreateNewBPComponent(TSubclassOf<UActorComponent> ComponentClass);

	/** Recursively updates the filtered state for each component item */
	void OnFilterTextChanged(const FText& InFilterText);

	/** 
	 * Compares the filter bar's text with the item's component name. Use 
	 * bRecursive to refresh the state of child nodes as well. Returns true if 
	 * the node is set to be filtered out 
	 */
	bool RefreshFilteredState(FSCSComponentEditorTreeNodePtrType TreeNode, bool bRecursive);

	/** Helper method to construct the subtree for the given actor (root) node. */
	void BuildSubTreeForActorNode(FSCSComponentEditorActorNodePtrType InActorNode);

	/** @return Type of component to filter the tree view with or nullptr if there's no filter. */
	TSubclassOf<UActorComponent> GetComponentTypeFilterToApply() const;

#if SUPPORT_UI_BLUEPRINT_EDITOR
public:
	TArray<FSCSComponentEditorTreeNodePtrType> GetSelectedItemsWithoutChildren() const;

protected:
	TArray<FSCSComponentEditorTreeNodePtrType> GetSelectedItemsIncludeChildren() const;
	static void GetSelectedChildrenNode(TArray<FSCSComponentEditorTreeNodePtrType>& NodeList, const FSCSComponentEditorTreeNodePtrType& Node);

	static void ChangeChildComponentOrder(TMap<FName, FSCSComponentEditorTreeNodePtrType>& NodeMap, FSCSComponentEditorTreeNodePtrType& TreeNode);
#endif

public:
	/** Tree widget */
	TSharedPtr<SSCSUITreeType> SCSTreeWidget;

	/** Command list for handling actions in the SSCSEditor */
	TSharedPtr< FUICommandList > CommandList;

	/** Name of a node that has been requested to be renamed */
	FName DeferredRenameRequest;

	/** Scope the creation of a component which ends when the initial component 'name' is given/accepted by the user, which can be several frames after the component was actually created. */
	TUniquePtr<FScopedTransaction> DeferredOngoingCreateTransaction;

	/** Used to unregister from the post tick event. */
	FDelegateHandle PostTickHandle;

	/** Attribute that provides access to the Actor context for which we are viewing/editing the SCS. */
	TAttribute<class AActor*> ActorContext;

	/** Attribute that provides access to a "preview" Actor context (may not be same as the Actor context that's being edited. */
	TAttribute<class AActor*> PreviewActor;

	/** Attribute to indicate whether or not editing is allowed. */
	TAttribute<bool> AllowEditing;

	/** Attribute to indicate whether or not the "Add Component" button is visible. If true, new components cannot be added to the Blueprint. */
	TAttribute<bool> HideComponentClassCombo;

	/** Attribute to limit visible nodes to a particular component type when filtering the tree view. */
	TAttribute<TSubclassOf<UActorComponent>> ComponentTypeFilter;

	/** Delegate to invoke on selection update. */
	FOnSelectionUpdated OnSelectionUpdated;

	/** Delegate to invoke when an item in the tree is double clicked. */
	FOnItemDoubleClicked OnItemDoubleClicked;

	/** Delegate to invoke when the given property should be highlighted in the details view (e.g. diff). */
	FOnHighlightPropertyInDetailsView OnHighlightPropertyInDetailsView;

	/** Returns the Actor context for which we are viewing/editing the SCS.  Can return null.  Should not be cached as it may change from frame to frame. */
	class AActor* GetActorContext() const;
	
public:
	TWeakPtr<class FUIBlueprintEditor> BlueprintEditorPtr;
	
private:
	/** Indicates which editor mode we're in. */
	EUIBlueprintComponentEditorMode::Type EditorMode;

	/** Root set of tree */
	TArray<FSCSComponentEditorTreeNodePtrType> RootNodes;

	/* Root Tree Node*/
	TSharedPtr<FExtender> ActorMenuExtender;

	/** Flag to enable/disable component editing */
	bool bEnableComponentEditing;

	/** Gate to prevent changing the selection while selection change is being broadcast. */
	bool bUpdatingSelection;

	/** Controls whether or not to allow calls to UpdateTree() */
	bool bAllowTreeUpdates;

	/** TRUE if this SCSEditor is currently the target of a diff */
	bool bIsDiffing;

	/** The filter box that handles filtering for the tree. */
	TSharedPtr< SSearchBox > FilterBox;

	/** SCSEditor UI customizations */
	TSharedPtr<ISCSEditorUICustomization> UICustomization;

	/** SCSEditor UI extension */
	TSharedPtr<class SExtensionPanel> ExtensionPanel;
};
