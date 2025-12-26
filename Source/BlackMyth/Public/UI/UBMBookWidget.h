// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "UBMBookWidget.generated.h"

class UTextBlock;
class UFont;

/**
 * Widget for displaying story book content with page turning.
 */
UCLASS()
class BLACKMYTH_API UBMBookWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    // Bound from WBP_Book (optional so widget blueprint can omit it during testing)
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* TextBook = nullptr;

    // Optional: specify a font asset that supports Chinese (set this in the Editor)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BM|UI|Book")
    TSoftObjectPtr<UFont> DefaultChineseFont;

	UFUNCTION(BlueprintCallable, Category = "BM|UI|Book")
	void ShowNextPage();

	UFUNCTION(BlueprintCallable, Category = "BM|UI|Book")
	void CloseBook();

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeOnInitialized() override;

private:
	void UpdatePageContent();

	TArray<FText> Pages;
	int32 CurrentPageIndex = 0;
};
