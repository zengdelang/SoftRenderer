#pragma once

#include "CoreMinimal.h"
#include "KismetCompiler.h"
#include "KismetCompilerModule.h"

//////////////////////////////////////////////////////////////////////////
// FUIBlueprintCompiler 

class UUIBlueprint;

class UIBLUEPRINTEDITOR_API FUIBlueprintCompiler : public IBlueprintCompiler
{
public:
	FUIBlueprintCompiler();

	virtual bool CanCompile(const UBlueprint* Blueprint) override;
	virtual void PreCompile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions) override;
	virtual void Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results) override;
	virtual void PostCompile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions) override;
	virtual bool GetBlueprintTypesForClass(UClass* ParentClass, UClass*& OutBlueprintClass, UClass*& OutBlueprintGeneratedClass) const override;
	
};

//////////////////////////////////////////////////////////////////////////
// FUIBlueprintCompilerContext

class UIBLUEPRINTEDITOR_API FUIBlueprintCompilerContext : public FKismetCompilerContext
{
protected:
	typedef FKismetCompilerContext Super;

public:
	FUIBlueprintCompilerContext(UUIBlueprint* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions);
	virtual ~FUIBlueprintCompilerContext() override;

	virtual void PostCompile() override;

};
