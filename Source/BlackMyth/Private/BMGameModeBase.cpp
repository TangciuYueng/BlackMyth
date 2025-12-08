#include "BMGameModeBase.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/BMPlayerController.h"

ABMGameModeBase::ABMGameModeBase()
{
    DefaultPawnClass = ABMPlayerCharacter::StaticClass();
    PlayerControllerClass = ABMPlayerController::StaticClass();
}
