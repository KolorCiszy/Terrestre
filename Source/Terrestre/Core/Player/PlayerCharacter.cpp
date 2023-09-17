// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Player/PlayerCharacter.h"
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
    UChunkUtilityLib::GetChunkManager()->RegisterPlayerCharacter(this);
}
void APlayerCharacter::UnRegisterCharacterToWorld()
{
    UChunkUtilityLib::GetChunkManager()->UnRegisterPlayerCharacter(this);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
    RegisterCharacterToWorld();
    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
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
        //UChunkUtilityLib::GetChunkManager(this);
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
    if (Controller != nullptr)
    {
        const FVector2D MoveValue = Value.Get<FVector2D>();
        const FRotator MovementRotation(0, Controller->GetControlRotation().Yaw, 0);

        // Forward/Backward direction
        if (MoveValue.Y != 0.f)
        {
            // Get forward vector
            const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);

            AddMovementInput(Direction, MoveValue.Y);//(GetCharacterMovement()->MaxWalkSpeed());
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
    if (GetCharacterMovement()->IsFlying())
    {
        AddMovementInput(FVector::UpVector, 200.0f, 0);
    }
}
void APlayerCharacter::Crouch()
{
    if(GetCharacterMovement()->IsFlying())
    {
        AddMovementInput(FVector::DownVector, 200.0f, 0);
    }
    if(CanCrouch())
    {
        Super::Crouch();
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

