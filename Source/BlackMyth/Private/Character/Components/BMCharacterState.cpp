#include "Character/Components/BMCharacterState.h"
#include "Character/BMCharacterBase.h"

/*
 * @brief Initialize the character state
 * @param Owner The owner of the character state
 */
void UBMCharacterState::Init(ABMCharacterBase* Owner)
{
    Context = Owner;
}
