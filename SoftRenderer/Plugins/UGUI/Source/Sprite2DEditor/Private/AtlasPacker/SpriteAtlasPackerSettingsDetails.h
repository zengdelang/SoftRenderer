#pragma once

#include "IDetailCustomization.h"

class FSpriteAtlasPackerSettingsDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder )  override;

protected:
	FReply OnRefreshImportPath(TSharedRef<IPropertyHandle> Handle) const;
	FReply OnRefreshImportPathBySelectedFolderPath(TSharedRef<IPropertyHandle> Handle) const;
	
private:
	TWeakObjectPtr<class USpriteAtlasPackerSettings> TargetScriptPtr;

};
