// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

#pragma warning(disable : 4263)  /* warning from Crouch and UnCrouch function not being virtual, 
								 /*	because if marked virtual, it didn't want to bound using enhanced input */
#pragma warning(disable : 4264)  /* consequence of previous warning - Crouch and UnCrouch 
								 /*	does not override any base class function */

class UCameraComponent;
class UEnhancedInputComponent;
class UEnchancedInputLocalPlayerSubsystem;
class UInputAction;
class UInputMappingContext;
class UPlayerInputConfigData;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerLocationChangedDelegate, FVector, currentLocation);

UCLASS()
class TERRESTRE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(BlueprintAssignable)
	FPlayerLocationChangedDelegate OnPlayerLocationChanged;

	void RegisterCharacterToWorld();

	void UnRegisterCharacterToWorld();
	


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void EndPlay(EEndPlayReason::Type reason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultInputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UPlayerInputConfigData> InputActions;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	/* Name of players character*/
	

	/* In unreal units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visibility")
	int32 VisibilityTraceDistance;

	/* Hide/show visibility debug trace line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visibility", DisplayName = "Show visibility line trace?")
	bool bShowVisibilityTrace;

	UFUNCTION(BlueprintCallable, Category = "Player Visibility")
	void VisibilityLineTrace(bool& bHitSuccesful, FHitResult& hitResult);

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	virtual void Jump() override;

	void Crouch();

	void UnCrouch();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	FVector LastTickLocation;
	
};
