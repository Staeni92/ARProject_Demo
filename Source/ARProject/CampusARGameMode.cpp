#include "CampusARGameMode.h"

#include "CampusARPawn.h"
#include "CampusARPlayerController.h"

ACampusARGameMode::ACampusARGameMode()
{
	PlayerControllerClass = ACampusARPlayerController::StaticClass();
	DefaultPawnClass = ACampusARPawn::StaticClass();
}
