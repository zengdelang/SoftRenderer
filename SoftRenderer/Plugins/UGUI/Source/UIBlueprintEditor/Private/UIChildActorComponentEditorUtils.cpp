#include "UIChildActorComponentEditorUtils.h"
#include "Settings/EditorProjectSettings.h"
#include "ToolMenus.h"
#include "SSCSComponentEditor.h"
#include "SSCSComponentEditorMenuContext.h"

#define LOCTEXT_NAMESPACE "ChildActorComponentEditorUtils"

struct FLocalChildActorComponentEditorUtils
{
	static bool IsChildActorTreeViewVisualizationModeSet(UChildActorComponent* InChildActorComponent, EChildActorComponentTreeViewVisualizationMode InMode)
	{
		if (!InChildActorComponent)
		{
			return false;
		}

		const EChildActorComponentTreeViewVisualizationMode CurrentMode = InChildActorComponent->GetEditorTreeViewVisualizationMode();
		if (CurrentMode == EChildActorComponentTreeViewVisualizationMode::UseDefault)
		{
			return InMode == FUIChildActorComponentEditorUtils::GetProjectDefaultTreeViewVisualizationMode();
		}

		return InMode == CurrentMode;
	}

	static void OnSetChildActorTreeViewVisualizationMode(UChildActorComponent* InChildActorComponent, EChildActorComponentTreeViewVisualizationMode InMode, TWeakPtr<SSCSComponentEditor> InWeakSCSEditorPtr)
	{
		if (!InChildActorComponent)
		{
			return;
		}

		InChildActorComponent->SetEditorTreeViewVisualizationMode(InMode);

		TSharedPtr<SSCSComponentEditor> SCSEditorPtr = InWeakSCSEditorPtr.Pin();
		if (SCSEditorPtr.IsValid())
		{
			SCSEditorPtr->UpdateTree();
		}
	}

	static void CreateChildActorVisualizationModesSubMenu(UToolMenu* InSubMenu, UChildActorComponent* InChildActorComponent, TWeakPtr<SSCSComponentEditor> InWeakSCSEditorPtr)
	{
		FToolMenuSection& SubMenuSection = InSubMenu->AddSection("ExpansionModes");
		SubMenuSection.AddMenuEntry(
			"ComponentOnly",
			LOCTEXT("ChildActorVisualizationModeLabel_ComponentOnly", "Component Only"),
			LOCTEXT("ChildActorVisualizationModeToolTip_ComponentOnly", "Visualize this child actor as a single component node. The child actor template/instance will not be included in the tree view."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateStatic(&FLocalChildActorComponentEditorUtils::OnSetChildActorTreeViewVisualizationMode, InChildActorComponent, EChildActorComponentTreeViewVisualizationMode::ComponentOnly, InWeakSCSEditorPtr),
				FCanExecuteAction(),
				FIsActionChecked::CreateStatic(&FLocalChildActorComponentEditorUtils::IsChildActorTreeViewVisualizationModeSet, InChildActorComponent, EChildActorComponentTreeViewVisualizationMode::ComponentOnly)
			),
			EUserInterfaceActionType::Check);
		SubMenuSection.AddMenuEntry(
			"ChildActorOnly",
			LOCTEXT("ChildActorVisualizationModeLabel_ChildActorOnly", "Child Actor Only"),
			LOCTEXT("ChildActorVisualizationModeToolTip_ChildActorOnly", "Visualize this child actor's template/instance as a subtree with a root actor node in place of the component node."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateStatic(&FLocalChildActorComponentEditorUtils::OnSetChildActorTreeViewVisualizationMode, InChildActorComponent, EChildActorComponentTreeViewVisualizationMode::ChildActorOnly, InWeakSCSEditorPtr),
				FCanExecuteAction(),
				FIsActionChecked::CreateStatic(&FLocalChildActorComponentEditorUtils::IsChildActorTreeViewVisualizationModeSet, InChildActorComponent, EChildActorComponentTreeViewVisualizationMode::ChildActorOnly)
			),
			EUserInterfaceActionType::Check);
		SubMenuSection.AddMenuEntry(
			"ComponentWithChildActor",
			LOCTEXT("ChildActorVisualizationModeLabel_ComponentWithChildActor", "Component with Attached Child Actor"),
			LOCTEXT("ChildActorVisualizationModeToolTip_ComponentWithChildActor", "Visualize this child actor's template/instance as a subtree with a root actor node that's parented to the component node."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateStatic(&FLocalChildActorComponentEditorUtils::OnSetChildActorTreeViewVisualizationMode, InChildActorComponent, EChildActorComponentTreeViewVisualizationMode::ComponentWithChildActor, InWeakSCSEditorPtr),
				FCanExecuteAction(),
				FIsActionChecked::CreateStatic(&FLocalChildActorComponentEditorUtils::IsChildActorTreeViewVisualizationModeSet, InChildActorComponent, EChildActorComponentTreeViewVisualizationMode::ComponentWithChildActor)
			),
			EUserInterfaceActionType::Check);
	}
};

bool FUIChildActorComponentEditorUtils::IsChildActorNode(TSharedPtr<const FSCSComponentEditorTreeNode> InNodePtr)
{
	return InNodePtr.IsValid() && InNodePtr->GetNodeType() == FSCSComponentEditorTreeNode::ENodeType::ChildActorNode;
}

bool FUIChildActorComponentEditorUtils::IsChildActorSubtreeNode(TSharedPtr<const FSCSComponentEditorTreeNode> InNodePtr)
{
	return InNodePtr.IsValid() && IsChildActorNode(InNodePtr->GetActorRootNode());
}

bool FUIChildActorComponentEditorUtils::ContainsChildActorSubtreeNode(const TArray<TSharedPtr<FSCSComponentEditorTreeNode>>& InNodePtrs)
{
	for (TSharedPtr<FSCSComponentEditorTreeNode> NodePtr : InNodePtrs)
	{
		if (IsChildActorSubtreeNode(NodePtr))
		{
			return true;
		}
	}

	return false;
}

TSharedPtr<FSCSComponentEditorTreeNode> FUIChildActorComponentEditorUtils::GetOuterChildActorComponentNode(TSharedPtr<const FSCSComponentEditorTreeNode> InNodePtr)
{
	if (InNodePtr.IsValid())
	{
		FSCSComponentEditorActorNodePtrType ActorTreeRootNode = InNodePtr->GetActorRootNode();
		if (IsChildActorNode(ActorTreeRootNode))
		{
			return ActorTreeRootNode->GetOwnerNode();
		}
	}

	return nullptr;
}

bool FUIChildActorComponentEditorUtils::IsChildActorTreeViewExpansionEnabled()
{
	const UBlueprintEditorProjectSettings* EditorProjectSettings = GetDefault<UBlueprintEditorProjectSettings>();
	return EditorProjectSettings->bEnableChildActorExpansionInTreeView;
}

EChildActorComponentTreeViewVisualizationMode FUIChildActorComponentEditorUtils::GetProjectDefaultTreeViewVisualizationMode()
{
	const UBlueprintEditorProjectSettings* EditorProjectSettings = GetDefault<UBlueprintEditorProjectSettings>();
	return EditorProjectSettings->DefaultChildActorTreeViewMode;
}

EChildActorComponentTreeViewVisualizationMode FUIChildActorComponentEditorUtils::GetChildActorTreeViewVisualizationMode(UChildActorComponent* ChildActorComponent, EChildActorComponentTreeViewVisualizationMode DefaultVisOverride)
{
	if (ChildActorComponent)
	{
		EChildActorComponentTreeViewVisualizationMode CurrentMode = ChildActorComponent->GetEditorTreeViewVisualizationMode();
		if (CurrentMode != EChildActorComponentTreeViewVisualizationMode::UseDefault)
		{
			return CurrentMode;
		}
	}

	return DefaultVisOverride == EChildActorComponentTreeViewVisualizationMode::UseDefault ? GetProjectDefaultTreeViewVisualizationMode() : DefaultVisOverride;
}

bool FUIChildActorComponentEditorUtils::ShouldExpandChildActorInTreeView(UChildActorComponent* ChildActorComponent, EChildActorComponentTreeViewVisualizationMode DefaultVisOverride)
{
	if (!ChildActorComponent)
	{
		return false;
	}

	if ((DefaultVisOverride == EChildActorComponentTreeViewVisualizationMode::UseDefault) && !IsChildActorTreeViewExpansionEnabled())
	{
		return false;
	}

	EChildActorComponentTreeViewVisualizationMode CurrentMode = GetChildActorTreeViewVisualizationMode(ChildActorComponent, DefaultVisOverride);
	return CurrentMode != EChildActorComponentTreeViewVisualizationMode::ComponentOnly;
}

bool FUIChildActorComponentEditorUtils::ShouldShowChildActorNodeInTreeView(UChildActorComponent* ChildActorComponent, EChildActorComponentTreeViewVisualizationMode DefaultVisOverride)
{
	if (!ShouldExpandChildActorInTreeView(ChildActorComponent, DefaultVisOverride))
	{
		return false;
	}

	EChildActorComponentTreeViewVisualizationMode CurrentMode = GetChildActorTreeViewVisualizationMode(ChildActorComponent, DefaultVisOverride);
	return CurrentMode == EChildActorComponentTreeViewVisualizationMode::ComponentWithChildActor;
}

void FUIChildActorComponentEditorUtils::FillComponentContextMenuOptions(UToolMenu* Menu, UChildActorComponent* ChildActorComponent)
{
	if (!ChildActorComponent)
	{
		return;
	}

	if (!IsChildActorTreeViewExpansionEnabled())
	{
		return;
	}

	FToolMenuSection& Section = Menu->AddSection("ChildActorComponent", LOCTEXT("ChildActorComponentHeading", "Child Actor Component"));
	{
		TWeakPtr<SSCSComponentEditor> WeakEditorPtr;
		if (USSCSComponentEditorMenuContext* MenuContext = Menu->FindContext<USSCSComponentEditorMenuContext>())
		{
			WeakEditorPtr = MenuContext->SCSComponentEditor;
		}

		Section.AddSubMenu(
			"ChildActorVisualizationModes",
			LOCTEXT("ChildActorVisualizationModesSubMenu_Label", "Visualization Mode"),
			LOCTEXT("ChildActorVisualizationModesSubMenu_ToolTip", "Choose how to visualize this child actor in the tree view."),
			FNewToolMenuDelegate::CreateStatic(&FLocalChildActorComponentEditorUtils::CreateChildActorVisualizationModesSubMenu, ChildActorComponent, WeakEditorPtr));
	}
}

void FUIChildActorComponentEditorUtils::FillChildActorContextMenuOptions(UToolMenu* Menu, TSharedPtr<const FSCSComponentEditorTreeNode> InNodePtr)
{
	if (!IsChildActorTreeViewExpansionEnabled())
	{
		return;
	}

	if (!IsChildActorNode(InNodePtr))
	{
		return;
	}

	TSharedPtr<const FSCSComponentEditorTreeNodeChildActor> ChildActorNodePtr = StaticCastSharedPtr<const FSCSComponentEditorTreeNodeChildActor>(InNodePtr);
	check(ChildActorNodePtr.IsValid());

	UChildActorComponent* ChildActorComponent = ChildActorNodePtr->GetChildActorComponent();
	if (!ChildActorComponent)
	{
		return;
	}

	FToolMenuSection& Section = Menu->AddSection("ChildActor", LOCTEXT("ChildActorHeading", "Child Actor"));
	{
		TWeakPtr<SSCSComponentEditor> WeakEditorPtr;
		if (USSCSComponentEditorMenuContext* MenuContext = Menu->FindContext<USSCSComponentEditorMenuContext>())
		{
			WeakEditorPtr = MenuContext->SCSComponentEditor;
		}

		Section.AddMenuEntry(
			"SetChildActorOnlyMode",
			LOCTEXT("SetChildActorOnlyMode_Label", "Switch to Child Actor Only Mode"),
			LOCTEXT("SetChildActorOnlyMode_ToolTip", "Visualize this child actor's template/instance subtree in place of its parent component node."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateStatic(&FLocalChildActorComponentEditorUtils::OnSetChildActorTreeViewVisualizationMode, ChildActorComponent, EChildActorComponentTreeViewVisualizationMode::ChildActorOnly, WeakEditorPtr),
				FCanExecuteAction()
			)
		);
	}
}

#undef LOCTEXT_NAMESPACE
