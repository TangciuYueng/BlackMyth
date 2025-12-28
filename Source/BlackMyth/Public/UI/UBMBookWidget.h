// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "UBMBookWidget.generated.h"

class UTextBlock;
class UFont;

/**
 * @brief Define the UBMBookWidget class, story book widget
 * @param UBMBookWidget The name of the class
 * @param UBMWidgetBase The parent class
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

	// Show next page, called when the user presses the Enter key
	UFUNCTION(BlueprintCallable, Category = "BM|UI|Book")
	void ShowNextPage();

	// Close the book, called when the user presses the Escape key
	UFUNCTION(BlueprintCallable, Category = "BM|UI|Book")
	void CloseBook();

protected:
	// Native construct
	virtual void NativeConstruct() override;
	// Native on key down
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	// Native on initialized
	virtual void NativeOnInitialized() override;

private:
	// Update the page content
	void UpdatePageContent();

	// Pages array
	TArray<FText> Pages;
	// Current page index
	int32 CurrentPageIndex = 0;
};
