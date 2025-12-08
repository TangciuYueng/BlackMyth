#include "Character/Components/BMCharacterState.h"
#include "Character/BMCharacterBase.h"

void UBMCharacterState::Init(ABMCharacterBase* Owner)
{
    Context = Owner;
}
