// Copyright Epic Games, Inc. All Rights Reserved.

#include "StarlightPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include <Kismet/KismetSystemLibrary.h>
#include "StarlightAudioCaptureComponent.h"
#include "StarlightGameInstance.h"

//////////////////////////////////////////////////////////////////////////
// AStarlightDemoPlayer

AStarlightPlayer::AStarlightPlayer()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create AudioCaptureComponent
	AudioCapture = CreateDefaultSubobject<UStarlightAudioCaptureComponent>(TEXT("AudioCaptureComponent"));
	AudioCapture->SetupAttachment(RootComponent);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AStarlightPlayer::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set player id on game instance
	UStarlightGameInstance* GameInstance = Cast<UStarlightGameInstance>(this->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->PlayerId = Id;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AStarlightPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStarlightPlayer::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AStarlightPlayer::Look);

		EnhancedInputComponent->BindAction(StartConversationAction, ETriggerEvent::Triggered, this, &AStarlightPlayer::StartConversation);
		EnhancedInputComponent->BindAction(EndConversationAction, ETriggerEvent::Triggered, this, &AStarlightPlayer::EndConversation);

		EnhancedInputComponent->BindAction(StartRecordingAction, ETriggerEvent::Triggered, this, &AStarlightPlayer::StartRecording);
		EnhancedInputComponent->BindAction(StopRecordingAction, ETriggerEvent::Triggered, this, &AStarlightPlayer::StopRecording);
	}

}

void AStarlightPlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AStarlightPlayer::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AStarlightPlayer::StartConversation() {
	// Find the actor within range that the player is looking at
	// using UKismetSystemLibrary::SphereOverlapActors()
	TArray<AActor*> OverlappingActors;
	UKismetSystemLibrary::SphereOverlapActors(this, GetActorLocation(), 300.0f, TArray<TEnumAsByte<EObjectTypeQuery>>(), AStarlightCharacter::StaticClass(), TArray<AActor*>(), OverlappingActors);
	
	// Drag debug sphere to see the range
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 300.0f, 12, FColor::Red, false, 5.0f, 0, 3.0f);

	if (OverlappingActors.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("No overlapping actors found"))
		return;
	}

	UStarlightConversation* Conversation = NewObject<UStarlightConversation>();
	ActiveConversation = Conversation;
	ActiveConversation->Participants.Add(this);
	FString Participants = "[";

	for (AActor* Actor : OverlappingActors) {
		AStarlightCharacter* Character = Cast<AStarlightCharacter>(Actor);
		if (Character) {
			Character->ActiveConversation = Conversation;
			Conversation->Participants.Add(Character);
			// if character isn't the last in the array, add a comma
			if (Character != OverlappingActors.Last()) {
				Participants += FString::Printf(TEXT("\"%s\","), *Character->Id);
			}
			else {
				Participants += FString::Printf(TEXT("\"%s\""), *Character->Id);
			}	
		}
	}

	Participants += "]";

	// Send websocket event to server
	UStarlightGameInstance* GameInstance = Cast<UStarlightGameInstance>(this->GetGameInstance());
	if (!GameInstance || !GameInstance->WebSocket->IsConnected()) {
		return;
	}

	FString Packet = FString::Printf(TEXT("{\"type\":\"ConversationStart\",\"data\":{\"playerId\":\"%s\",\"characters\":%s }}"), *this->Id, *Participants);
	GameInstance->WebSocket->Send(Packet);
}

void AStarlightPlayer::EndConversation() {
	// log
	UE_LOG(LogTemp, Warning, TEXT("End Conversation"));

	if (ActiveConversation) {
		ActiveConversation = nullptr;
	}

	// TODO: Send websocket event to server
}

void AStarlightPlayer::StartRecording() {
	AudioCapture->StartRecording();
}

void AStarlightPlayer::StopRecording() {
	AudioCapture->StopRecording();
}




