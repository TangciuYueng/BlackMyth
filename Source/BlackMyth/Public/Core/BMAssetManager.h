#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "BMAssetManager.generated.h"

// Delegate for asset loaded callback
DECLARE_DELEGATE_OneParam(FOnAssetLoaded, UObject*);

UCLASS()
class BLACKMYTH_API UBMAssetManager : public UAssetManager
{
    GENERATED_BODY()

public:
    static UBMAssetManager& Get();

    /**
     * @param Path     The soft object path of the asset to load     
     * @param OnLoaded Delegate to call when the asset is loaded 
     */
    void AsyncLoadAsset(const FSoftObjectPath& Path, FOnAssetLoaded OnLoaded);

    /**
     * @param Path The soft object path of the asset to load
     */
    UObject* SyncLoadAsset(const FSoftObjectPath& Path);
};
