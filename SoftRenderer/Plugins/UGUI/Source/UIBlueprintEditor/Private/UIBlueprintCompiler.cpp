#include "UIBlueprintCompiler.h"
#include "K2Node_FunctionEntry.h"
#include "UISequenceCompiler.h"
#include "Core/UIBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"

FUIBlueprintCompiler::FUIBlueprintCompiler()
{

}

bool FUIBlueprintCompiler::CanCompile(const UBlueprint* Blueprint)
{
	return false;
}

void FUIBlueprintCompiler::PreCompile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions)
{

}

void FUIBlueprintCompiler::Compile(UBlueprint * Blueprint, const FKismetCompilerOptions & CompileOptions, FCompilerResultsLog & Results)
{

}

void FUIBlueprintCompiler::PostCompile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions)
{

}

bool FUIBlueprintCompiler::GetBlueprintTypesForClass(UClass* ParentClass, UClass*& OutBlueprintClass, UClass*& OutBlueprintGeneratedClass) const
{
	return false;
}

FUIBlueprintCompilerContext::FUIBlueprintCompilerContext(UUIBlueprint* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions)
	: Super(SourceSketch, InMessageLog, InCompilerOptions)
{
}

FUIBlueprintCompilerContext::~FUIBlueprintCompilerContext()
{
	
}

void FUIBlueprintCompilerContext::PostCompile()
{
	FUISequenceCompiler::CompileUISequenceData(Blueprint, &MessageLog);
	FKismetCompilerContext::PostCompile();
}
