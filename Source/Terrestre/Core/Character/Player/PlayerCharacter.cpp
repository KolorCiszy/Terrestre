// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Character/Player/PlayerCharacter.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerInputConfigData.h"





// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = .1f;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
   
    
}

void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (Camera)
    {
        Camera->AttachToComponent(GetMesh(), FAttachmentTransformRules{ EAttachmentRule::SnapToTarget, true }, FName("headSocket"));
        Camera->Mobility = EComponentMobility::Movable;
        Camera->SetRelativeRotation(FRotator(0, 90, 90));
        Camera->SetRelativeLocation(FVector(0, 20, 175 ));
        Camera->bUsePawnControlRotation = true;
    }
}

void APlayerCharacter::RegisterCharacterToWorld()
{
    if (auto manager = UChunkUtilityLib::GetChunkManager())
    {
        manager->RegisterPlayerCharacter(this);
    }
}
void APlayerCharacter::UnRegisterCharacterToWorld()
{
    if(auto manager = UChunkUtilityLib::GetChunkManager())
    {
        manager->UnRegisterPlayerCharacter(this);
    }
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
    RegisterCharacterToWorld();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(DefaultInputMapping, 0);
	}
}
// Called every frame

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if(LastTickLocation != GetActorLocation())
    {
        OnPlayerLocationChanged.Broadcast(GetActorLocation());
    }
    LastTickLocation = GetActorLocation();
    
}
void APlayerCharacter::VisibilityLineTrace(bool& bHitSuccesful, FHitResult& hitResult)
{
    FHitResult result{};
    FVector start = Camera->GetComponentLocation();
    
    FRotator cameraRotation = Camera->GetComponentRotation();
    FVector cameraForwardVector = UKismetMathLibrary::GetForwardVector(cameraRotation);
    FVector end = start + (cameraForwardVector * VisibilityTraceDistance);
    FCollisionQueryParams params;
    params.AddIgnoredActor(this);
    if(bShowVisibilityTrace)
    {
#ifdef DEBUG
        const FName TraceTag("MyTraceTag");
        GetWorld()->DebugDrawTraceTag = TraceTag;
        params.TraceTag = TraceTag;
#endif
    }
    bHitSuccesful = GetWorld()->LineTraceSingleByChannel(result, start, end, ECollisionChannel::ECC_Visibility, params);
    hitResult = result;
}
void APlayerCharacter::Move(const FInputActionValue& Value)
{
    if (Controller == nullptr)
    {
        return;
    }
    FRotator MovementRotation{ 0,0,0 };
    const FVector2D MoveValue = Value.Get<FVector2D>();
    if (GetCharacterMovement()->IsSwimming())
    {
        if (bSwimmingOnWaterSurface)
        {
           
            if (FMath::IsWithin(GetControlRotation().Pitch, 270, 360))
            {
                MovementRotation.Pitch = GetControlRotation().Pitch;
            }
          
            MovementRotation.Yaw = GetControlRotation().Yaw;

            if (MoveValue.Y > 0.f)
            {
                // Get forward vector
                const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);

                AddMovementInput(Direction, MoveValue.Y);
            }

        }
        else
        {
            MovementRotation.Pitch = Controller->GetControlRotation().Pitch;
            MovementRotation.Yaw = Controller->GetControlRotation().Yaw;

            if (MoveValue.Y > 0.f)
            {
                // Get forward vector
                const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);

                AddMovementInput(Direction, MoveValue.Y);
            }
        }
    }

    else
    {
        MovementRotation.Yaw = Controller->GetControlRotation().Yaw;
    
        // Forward/Backward direction
        if (MoveValue.Y != 0.f)
        {
            // Get forward vector
            const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);

            AddMovementInput(Direction, MoveValue.Y);
        }

        // Right/Left direction
        if (MoveValue.X != 0.f)
        {
            // Get right vector
            const FVector Direction = MovementRotation.RotateVector(FVector::RightVector);

            AddMovementInput(Direction, MoveValue.X);
        }
    }
        
    
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    if (Controller != nullptr)
    {
        const FVector2D LookValue = Value.Get<FVector2D>();

        if (LookValue.X != 0.f)
        {
            AddControllerYawInput(LookValue.X);
        }

        if (LookValue.Y != 0.f)
        {
            AddControllerPitchInput(-LookValue.Y);
        }
    }
}
void APlayerCharacter::Jump()
{
    Super::Jump();
    switch (GetCharacterMovement()->MovementMode)
    {
        case MOVE_Flying: AddMovementInput(FVector::UpVector, 200.0f, 0);
            break;
        case MOVE_Swimming: //AddMovementInput(FVector::UpVector, 100.0f, 0);
            break;
    }
   
}
void APlayerCharacter::Crouch()
{
    switch (GetCharacterMovement()->MovementMode)
    {
    case MOVE_Flying: AddMovementInput(FVector::DownVector, 200.0f, 0);
        break;
    case MOVE_Swimming: //AddMovementInput(FVector::UpVector, 100.0f, 0);
        break;
    case MOVE_Walking:  
        if (CanCrouch())
        {
            Super::Crouch();
        }
        break;
    }
   
}
void APlayerCharacter::UnCrouch()
{
    Super::UnCrouch();
}
// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputActions->InputMove, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
        EnhancedInputComponent->BindAction(InputActions->InputJump, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);

		EnhancedInputComponent->BindAction(InputActions->InputLook, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
        
        EnhancedInputComponent->BindAction(InputActions->InputCrouch, ETriggerEvent::Triggered, this, &APlayerCharacter::Crouch);
        EnhancedInputComponent->BindAction(InputActions->InputCrouch, ETriggerEvent::Completed, this, &APlayerCharacter::UnCrouch);
        
	}
}
void APlayerCharacter::EndPlay(EEndPlayReason::Type reason)
{
    Super::EndPlay(reason);
    UnRegisterCharacterToWorld();
}

