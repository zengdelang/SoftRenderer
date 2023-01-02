#pragma once

#include "CoreMinimal.h"
#include "MoveDirection.generated.h"

/**
 * This is an 4 direction movement enum.
 *
 * MoveDirection provides a way of switching between moving states. You must assign these states to actions, such as moving the GameObject by an up vector when in the Up state.
 * Having states like these are easier to identify than always having to include a large amount of vectors and calculations.Instead, you define what you want the state to do in only one part, and switch to the appropriate state when it is needed.
 */
UENUM(BlueprintType)
enum class EMoveDirection : uint8
{
	/**
	 * This is the Left state of MoveDirection.  Assign functionality for moving to the left.
	 *
	 * Use the Left state for an easily identifiable way of moving a GameObject to the left (-1 , 0 , 0). This is a state without any predefined functionality. Before using this state, you should define what your GameObject will do in code.
	 */
	MoveDirection_Left UMETA(DisplayName = "Left"),

	/**
	 * This is the Up state of MoveDirection.  Assign functionality for moving in an upward direction.
	 *
	 * Use the Up state for an easily identifiable way of moving a GameObject upwards (0 , 1 , 0). This is a state without any predefined functionality. Before using this state, you should define what your GameObject will do in code.
     */
	MoveDirection_Up UMETA(DisplayName = "Up"),

	/**
	 * This is the Right state of MoveDirection. Assign functionality for moving to the right.
	 *
	 * Use the Right state for an easily identifiable way of moving a GameObject to the right (1 , 0 , 0). This is a state without any predefined functionality. Before using this state, you should define what your GameObject will do in code.
   	 */
	MoveDirection_Right UMETA(DisplayName = "Right"),

    /**
	 * The Down State of MoveDirection. Assign functionality for moving in a downward direction.
	 *
	 * Use the Down state for an easily identifiable way of moving a GameObject downwards (0 , -1 , 0). This is a state without any predefined functionality. Before using this state, you should define what your GameObject will do in code.
     */
	MoveDirection_Down UMETA(DisplayName = "Down"),

	/**
	 * This is the None state. Assign functionality that stops movement.
	 *
	 * Use the None state for an easily identifiable way of stopping, resetting or initialising a GameObject's movement. This is a state without any predefined functionality. Before using this state, you should define what your GameObject will do in code.
	 */
	MoveDirection_None UMETA(DisplayName = "None"),
	
};
