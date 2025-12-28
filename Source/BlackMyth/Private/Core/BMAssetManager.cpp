#include "Core/BMAssetManager.h"
#include "Engine/StreamableManager.h"

/*
 * @brief Get the asset manager
 * @return The asset manager
 */
UBMAssetManager& UBMAssetManager::Get()
{
    // AssetManager
    UBMAssetManager* This = Cast<UBMAssetManager>(GEngine->AssetManager);
    if (This)
    {
        return *This;
    }
    else
    {
        // Provide a fallback in case AssetManager is not set or of the wrong class
        UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManager Class! Check DefaultEngine.ini"));
        return *NewObject<UBMAssetManager>(); // Should not happen
    }
}

/*
 * @brief Async load asset, it async loads the asset
 * @param Path The path
 * @param OnLoaded The on loaded
 */
void UBMAssetManager::AsyncLoadAsset(const FSoftObjectPath& Path, FOnAssetLoaded OnLoaded)
{
    if (Path.IsNull())
    {
        OnLoaded.ExecuteIfBound(nullptr);
        return;
    }

    // Async Load
    FStreamableManager& Streamable = GetStreamableManager();

    Streamable.RequestAsyncLoad(Path, FStreamableDelegate::CreateLambda([Path, OnLoaded]()
        {
            UObject* LoadedAsset = Path.ResolveObject();
            OnLoaded.ExecuteIfBound(LoadedAsset);
        }));
}

/*
 * @brief Sync load asset, it sync loads the asset
 * @param Path The path
 * @return The object
 */
UObject* UBMAssetManager::SyncLoadAsset(const FSoftObjectPath& Path)
{
    if (Path.IsNull()) return nullptr;
    return Path.TryLoad();
}
