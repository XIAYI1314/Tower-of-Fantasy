#include "no_clip.hpp"
#include "../globals.hpp"
#include "../hooks.hpp"
#include "../logger/logger.hpp"
#include "hotkey.hpp"

namespace Feats {
    namespace NoClip {
        Config::field<bool> enabled;
        bool toggleInNextTick = false;

        void init() {
            enabled = Config::get<bool>(confEnabled, false);

            Hooks::registerHook(
                "Engine.ActorComponent.ReceiveTick",
                [](SDK::UObject *pObject, SDK::UFunction *pFunction, void *pParams) -> Hooks::ExecutionFlag {
                    if (toggleInNextTick) {
                        const auto character = Globals::getCharacter();

                        if (character != nullptr) {
                            character->SetActorEnableCollision(!*enabled);
                            toggleInNextTick = false;
                        }
                    }

                    return Hooks::ExecutionFlag::CONTINUE_EXECUTION;
                });
        }

        void toggle() {
            const auto character = Globals::getCharacter();

            if (character == nullptr) {
                return;
            }

            if (*enabled) {
                character->CharacterMovement->SetMovementMode(SDK::EMovementMode::MOVE_Falling, 0);
            } else {
                character->CharacterMovement->SetMovementMode(SDK::EMovementMode::MOVE_Walking, 0);
            }

            toggleInNextTick = true;
        }

        void tick() {
            if (Feats::Hotkey::hotkeyPressed(confToggleEnabled)) {
                toggle();
            }

            if (!*enabled) {
                return;
            }

            const auto character = Globals::getCharacter();

            if (character == nullptr) {
                return;
            }

            const auto controller = character->GetHottaPlayerController();

            if (controller == nullptr) {
                return;
            }

            const auto cameraManager = controller->PlayerCameraManager;

            SDK::FVector moveVector(0.0, 0.0, 0.0);

            if (GetAsyncKeyState('W') & 0x8000) {
                const auto forwardVector = cameraManager->GetActorForwardVector();
                moveVector += forwardVector;
            }

            if (GetAsyncKeyState('S') & 0x8000) {
                const auto forwardVector = cameraManager->GetActorForwardVector();
                moveVector -= forwardVector;
            }

            if (GetAsyncKeyState('A') & 0x8000) {
                const auto rightVector = cameraManager->GetActorRightVector();
                moveVector -= rightVector;
            }

            if (GetAsyncKeyState('D') & 0x8000) {
                const auto rightVector = cameraManager->GetActorRightVector();
                moveVector += rightVector;
            }

            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                const auto upVector = cameraManager->GetActorUpVector();
                moveVector += upVector;
            }

            if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) {
                const auto upVector = cameraManager->GetActorUpVector();
                moveVector -= upVector;
            }

            moveVector.Normalize();

            const auto velocity = character->CharacterMovement->MaxWalkSpeed / 100.0f;

            const auto movement = moveVector * velocity;

            // This is required to prevent the character from continuously sliding
            // after user input has stopped.
            character->CharacterMovement->Velocity = SDK::FVector(0.0, 0.0, 0.0);

            if (movement.IsZero()) {
                return;
            }

            const auto location = (SDK::FVector *)((byte *)character->CharacterMovement->UpdatedComponent + 0x1E0);
            *location += movement;
        }

        void menu() {
            if (ImGui::Checkbox("No Clip", &enabled)) {
                toggle();
            }
        }
    } // namespace NoClip
} // namespace Feats