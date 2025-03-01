OCL Core Keywords
let
Purpose: Declares a new variable within the current scope, allowing storage of data for later use.
Usage: Followed by a variable name, an optional type annotation using a colon, and an assignment using =.
Behavior: Creates a variable that can hold values like numbers, strings, or objects, enforcing type safety if annotated.
Example: let x: int = 5; declares an integer variable x with the value 5.
print
Purpose: Outputs a value or expression to the console for debugging or user interaction.
Usage: Takes a single expression after the keyword, which is converted to a string and displayed.
Behavior: Automatically appends a newline after the output, making it suitable for sequential logging.
Example: print "Hello"; prints "Hello" followed by a newline to the console.
if
Purpose: Initiates a conditional block that executes only if its condition evaluates to true.
Usage: Requires a condition followed by a colon and a brace-enclosed body {}.
Behavior: Can be extended with elif and else blocks for additional logic branching.
Example: if x > 0: { print "Positive"; } prints "Positive" if x is greater than 0.
elif
Purpose: Specifies an alternative condition in an if statement, checked if prior conditions fail.
Usage: Follows an if or another elif, with its own condition, colon, and brace-enclosed body.
Behavior: Executes its block only if its condition is true and all previous conditions were false.
Example: elif x < 0: { print "Negative"; } prints "Negative" if x is less than 0 and the prior if failed.
else
Purpose: Provides a default block in an if statement, executed when all prior conditions are false.
Usage: Follows an if or elif block, requiring only a colon and braces, no condition.
Behavior: Acts as a catch-all case, ensuring some code runs if no conditions are met.
Example: else: { print "Zero"; } prints "Zero" if neither if nor elif conditions are true.
while
Purpose: Creates a loop that repeats its body as long as its condition remains true.
Usage: Takes a condition followed by a colon and a brace-enclosed block.
Behavior: Can be controlled with break to exit or continue to skip iterations.
Example: while x > 0: { x = x - 1; } decrements x until it reaches 0.
define
Purpose: Defines a reusable function with a name and optional parameters for custom logic.
Usage: Followed by a function name, parentheses with parameters, a colon, and a brace-enclosed body.
Behavior: Stores the function for later invocation, allowing return values via return.
Example: define add(x, y): { return x + y; } defines a function add that returns the sum of two arguments.
return
Purpose: Exits a function and optionally provides a value back to the caller.
Usage: Used within a define block, optionally followed by an expression.
Behavior: Without an expression, it returns null; otherwise, it returns the evaluated result.
Example: return x + y; in a function returns the sum of x and y.
class
Purpose: Defines a new class with methods, enabling object-oriented programming.
Usage: Followed by a class name, a colon, and braces containing define statements for methods.
Behavior: Creates a blueprint for objects instantiated with ocl.classes.
Example: class Point: { define getX(): { return 0; } } defines a Point class with a getX method.
break
Purpose: Immediately terminates the nearest enclosing while loop.
Usage: Stands alone within a loop’s body, no arguments required.
Behavior: Transfers control out of the loop, skipping remaining iterations.
Example: if x == 0: { break; } exits a while loop when x equals 0.
continue
Purpose: Skips the rest of the current iteration in a while loop, jumping back to the condition.
Usage: Used standalone within a loop’s body, no additional syntax needed.
Behavior: Bypasses subsequent code in the iteration, resuming at the next check.
Example: if x < 0: { continue; } skips negative values in a loop.
true
Purpose: Represents the boolean value true for logical operations and conditions.
Usage: Can be assigned to variables or used directly in expressions.
Behavior: Evaluates as true in conditionals, contrasting with false.
Example: let b = true; assigns the boolean value true to b.
false
Purpose: Represents the boolean value false for logical operations and conditions.
Usage: Assignable to variables or usable in expressions like true.
Behavior: Evaluates as false in conditionals, opposite to true.
Example: if false: { print "Never"; } never executes its block.
null
Purpose: Indicates the absence of a value, used for initialization or clearing.
Usage: Assigned to variables or returned from functions when no value applies.
Behavior: Represents a null state, distinct from other data types.
Example: let x = null; sets x to have no value.
int
Purpose: Type annotation ensuring a variable holds only integer values.
Usage: Used with let after a colon to specify the variable’s type.
Behavior: Enforces type safety, rejecting non-integer assignments.
Example: let x: int = 5; declares x as an integer with value 5.
float
Purpose: Type annotation for variables storing floating-point (decimal) numbers.
Usage: Follows a variable name in let declarations with a colon.
Behavior: Ensures the variable only accepts numbers with decimal precision.
Example: let x: float = 5.5; sets x to a floating-point value.
bool
Purpose: Type annotation restricting a variable to boolean values (true or false).
Usage: Used in let statements with a colon for type specification.
Behavior: Prevents assignment of non-boolean values, maintaining logical integrity.
Example: let x: bool = true; declares x as a boolean set to true.
string
Purpose: Type annotation ensuring a variable contains text data.
Usage: Applied in let declarations with a colon before the value.
Behavior: Supports string-specific features like interpolation with {}.
Example: let x: string = "Hello"; assigns the string "Hello" to x.
ocl
Purpose: Namespace prefix for accessing OCL library functions and the OCL2DRA API.
Usage: Precedes function calls like ocl.get_input or ocl.get_ocl2dra.init.
Behavior: Provides a gateway to built-in utilities and rendering capabilities.
Example: ocl.get_ocl2dra.init(800, 600, "Title"); initializes a rendering window.
OCL General Functions
ocl.classes
Purpose: Creates an instance of a user-defined class for object-oriented use.
Usage: Takes a string argument naming the class, called as ocl.classes("ClassName").
Behavior: Returns a dictionary-like object with the class’s methods and properties.
Example: let obj = ocl.classes("Point"); instantiates a Point class object.
ocl.get_input
Purpose: Prompts the user for input and returns it as a string for interactive scripts.
Usage: Takes a string prompt as an argument, e.g., ocl.get_input("Prompt: ").
Behavior: Displays the prompt, waits for user input, and returns the entered text.
Example: let name = ocl.get_input("Enter name: "); stores user input in name.
ocl.get_set_input
Purpose: Prompts for input and stores it globally in input_value without returning it.
Usage: Called with a prompt string, like ocl.get_set_input("Enter: ").
Behavior: Sets the global input_value variable, accessible later in the script.
Example: ocl.get_set_input("Enter value: "); print input_value; prints the input.
OCL2DRA/OCL2DRI Functions (Accessed via ocl.get_ocl2dra.*)
ocl.get_ocl2dra.init
Purpose: Initializes a new window for 2D rendering, starting the OCL2DRI context.
Usage: Takes width (int), height (int), and title (string) as arguments.
Behavior: Returns a context pointer used for all subsequent OCL2DRI operations.
Example: let window = ocl.get_ocl2dra.init(800, 600, "Game"); opens an 800x600 window.
ocl.get_ocl2dra.set_background
Purpose: Sets the background color of the rendering window.
Usage: Requires a context pointer and RGB values (0-255) as integers.
Behavior: Updates the window’s background on the next render cycle.
Example: ocl.get_ocl2dra.set_background(window, 255, 0, 0); sets a red background.
ocl.get_ocl2dra.set_title
Purpose: Changes the title displayed in the window’s title bar.
Usage: Takes a context pointer and a string for the new title.
Behavior: Updates the window title immediately for user feedback.
Example: ocl.get_ocl2dra.set_title(window, "Running"); sets the title to "Running".
ocl.get_ocl2dra.set_size
Purpose: Resizes the rendering window to new dimensions.
Usage: Accepts a context pointer, width, and height as integers.
Behavior: Adjusts the window size dynamically during runtime.
Example: ocl.get_ocl2dra.set_size(window, 600, 400); resizes to 600x400 pixels.
ocl.get_ocl2dra.set_position
Purpose: Moves the window to a specified position on the screen.
Usage: Takes a context pointer and x, y coordinates as integers.
Behavior: Repositions the window instantly based on screen coordinates.
Example: ocl.get_ocl2dra.set_position(window, 100, 100); moves it to (100, 100).
ocl.get_ocl2dra.set_fullscreen
Purpose: Toggles the window between fullscreen and windowed modes.
Usage: Requires a context pointer and a boolean (1 for true, 0 for false).
Behavior: Switches display mode without closing the window.
Example: ocl.get_ocl2dra.set_fullscreen(window, 1); enables fullscreen.
ocl.get_ocl2dra.set_opacity
Purpose: Adjusts the window’s transparency level for visual effects.
Usage: Takes a context pointer and a float between 0.0 (transparent) and 1.0 (opaque).
Behavior: Changes the window’s opacity, affecting its visibility.
Example: ocl.get_ocl2dra.set_opacity(window, 0.5); sets 50% opacity.
ocl.get_ocl2dra.set_border
Purpose: Enables or disables the window’s border for a custom look.
Usage: Uses a context pointer and a boolean (1 for border, 0 for none).
Behavior: Alters the window’s appearance immediately.
Example: ocl.get_ocl2dra.set_border(window, 0); removes the border.
ocl.get_ocl2dra.set_min_size
Purpose: Sets the minimum size the window can be resized to.
Usage: Requires a context pointer, minimum width, and minimum height as integers.
Behavior: Prevents the window from shrinking below these dimensions.
Example: ocl.get_ocl2dra.set_min_size(window, 200, 150); sets a 200x150 minimum.
ocl.get_ocl2dra.set_max_size
Purpose: Caps the maximum size the window can expand to.
Usage: Takes a context pointer, maximum width, and maximum height as integers.
Behavior: Limits resizing beyond the specified dimensions.
Example: ocl.get_ocl2dra.set_max_size(window, 800, 600); caps at 800x600.
ocl.get_ocl2dra.set_always_on_top
Purpose: Forces the window to remain above other applications.
Usage: Uses a context pointer and a boolean (1 to enable, 0 to disable).
Behavior: Keeps the window in the foreground when enabled.
Example: ocl.get_ocl2dra.set_always_on_top(window, 1); sets it always on top.
ocl.get_ocl2dra.set_resizable
Purpose: Toggles whether the user can resize the window manually.
Usage: Requires a context pointer and a boolean (1 for resizable, 0 for fixed).
Behavior: Locks or unlocks the window’s resize capability.
Example: ocl.get_ocl2dra.set_resizable(window, 0); disables resizing.
ocl.get_ocl2dra.set_frame_rate
Purpose: Sets the target frames per second for smooth rendering.
Usage: Takes a context pointer and an integer FPS value.
Behavior: Controls the rendering speed, affecting animation fluidity.
Example: ocl.get_ocl2dra.set_frame_rate(window, 60); targets 60 FPS.
ocl.get_ocl2dra.update
Purpose: Refreshes the window display and processes user events.
Usage: Called with a context pointer as the only argument.
Behavior: Essential for rendering updates and handling input like closing the window.
Example: ocl.get_ocl2dra.update(window); updates the window state.
ocl.get_ocl2dra.is_running
Purpose: Checks if the window remains open and active.
Usage: Takes a context pointer and returns a boolean.
Behavior: Returns true if the window is running, false if closed.
Example: while ocl.get_ocl2dra.is_running(window): {} loops while the window is open.
ocl.get_ocl2dra.destroy
Purpose: Closes the window and frees associated resources.
Usage: Requires a context pointer as its sole argument.
Behavior: Cleans up the rendering context, ending the session.
Example: ocl.get_ocl2dra.destroy(window); shuts down the window.
ocl.get_ocl2dra.hide
Purpose: Temporarily hides the window without closing it.
Usage: Called with a context pointer, no additional arguments.
Behavior: Makes the window invisible until shown again.
Example: ocl.get_ocl2dra.hide(window); hides the window from view.
ocl.get_ocl2dra.show
Purpose: Makes a hidden window visible again.
Usage: Takes a context pointer as its only argument.
Behavior: Reverses the effect of hide, restoring visibility.
Example: ocl.get_ocl2dra.show(window); reveals a hidden window.
ocl.get_ocl2dra.set_icon
Purpose: Sets a custom icon for the window’s title bar.
Usage: Requires a context pointer and a file path string (e.g., to a BMP file).
Behavior: Updates the window’s icon for visual branding.
Example: ocl.get_ocl2dra.set_icon(window, "icon.bmp"); sets a custom icon.
ocl.get_ocl2dra.get_mouse_position
Purpose: Retrieves the current mouse cursor coordinates within the window.
Usage: Takes a context pointer and returns a tuple of (x, y) integers.
Behavior: Provides real-time mouse tracking for interaction.
Example: let pos = ocl.get_ocl2dra.get_mouse_position(window); gets the mouse position.
ocl.get_ocl2dra.get_mouse_button_state
Purpose: Checks if a specific mouse button is pressed.
Usage: Requires a context pointer and a button number (1=left, 2=middle, 3=right).
Behavior: Returns 1 if pressed, 0 if not, enabling mouse input handling.
Example: let click = ocl.get_ocl2dra.get_mouse_button_state(window, 1); checks the left button.
ocl.get_ocl2dra.get_delta_time
Purpose: Returns the time elapsed since the last frame in seconds.
Usage: Called with a context pointer, no additional arguments.
Behavior: Facilitates smooth animations by providing timing data.
Example: let dt = ocl.get_ocl2dra.get_delta_time(window); gets frame timing.
ocl.get_ocl2dra.get_key_state
Purpose: Checks if a specific keyboard key is currently pressed.
Usage: Takes a context pointer and a key name string (e.g., "h").
Behavior: Returns 1 if the key is pressed, 0 if not, for keyboard input.
Example: let key = ocl.get_ocl2dra.get_key_state(window, "h"); checks the "h" key.
Operators
=
Purpose: Assigns a value to a variable or attribute for storage.
Usage: Used in let declarations or standalone assignments with a variable on the left.
Behavior: Links the left-hand side to the evaluated right-hand expression.
Example: let x = 5; or x = 10; assigns 5 or 10 to x.
+=
Purpose: Adds a value to an existing variable and updates it in one step.
Usage: Applied to a defined variable followed by a value or expression.
Behavior: Combines addition and assignment, modifying the variable’s value.
Example: x += 2; increases x by 2.
-=
Purpose: Subtracts a value from an existing variable and updates it.
Usage: Used with a variable and a value or expression on the right.
Behavior: Performs subtraction and assignment together, altering the variable.
Example: x -= 3; decreases x by 3.
*=
Purpose: Multiplies an existing variable by a value and updates it.
Usage: Requires a variable and a multiplier after the operator.
Behavior: Combines multiplication and assignment into a single operation.
Example: x *= 2; doubles the value of x.
/=
Purpose: Divides an existing variable by a value and updates it.
Usage: Applied to a variable with a divisor on the right.
Behavior: Executes division and assignment, raising an error if dividing by zero.
Example: x /= 2; halves x.
+
Purpose: Adds two numbers or concatenates strings for arithmetic or text combination.
Usage: Used between two operands, like numbers or strings.
Behavior: Returns the sum for numbers or a joined string for text.
Example: let sum = 5 + 3; yields 8, or "a" + "b"; yields "ab".
-
Purpose: Subtracts the right operand from the left for arithmetic operations.
Usage: Placed between two numeric operands.
Behavior: Produces the difference between the values.
Example: let diff = 10 - 4; results in 6.
Purpose: Multiplies two numbers together for scaling or repetition.
Usage: Used between two numeric operands, integers or floats.
Behavior: Returns the product of the two values.
Example: let prod = 2 * 3; equals 6.
/
Purpose: Divides the left operand by the right, yielding a quotient.
Usage: Applied between two numbers, with a non-zero divisor.
Behavior: Returns an integer (floor division) or float, errors on zero division.
Example: let quot = 6 / 2; results in 3.
%
Purpose: Computes the remainder of division between two numbers (modulus).
Usage: Used with two numeric operands, divisor must be non-zero.
Behavior: Returns the leftover value after division.
Example: let rem = 7 % 3; returns 1.
==
Purpose: Checks if two values are equal for conditional logic.
Usage: Placed between two operands of any type.
Behavior: Returns true if they match, false otherwise, comparing values or identity.
Example: if x == 5: { print "Equal"; } prints if x is 5.
!=
Purpose: Tests if two values are not equal for differentiation.
Usage: Used between two operands to compare them.
Behavior: Returns true if they differ, false if identical.
Example: if x != 0: { print "Not zero"; } prints if x isn’t 0.
<
Purpose: Checks if the left value is less than the right for ordering.
Usage: Applied between two comparable operands, typically numbers.
Behavior: Returns true if the condition holds, false otherwise.
Example: if x < 10: { print "Small"; } prints if x is less than 10.
>
Purpose: Tests if the left value exceeds the right for comparison.
Usage: Used between two numeric operands.
Behavior: Returns true if greater, false if not.
Example: if x > 0: { print "Positive"; } prints if x is positive.
<=
Purpose: Verifies if the left value is less than or equal to the right.
Usage: Placed between two operands for inclusive comparison.
Behavior: Returns true if less or equal, false otherwise.
Example: if x <= 5: { print "Low"; } prints if x is 5 or less.
>=
Purpose: Checks if the left value is greater than or equal to the right.
Usage: Used between two operands for threshold checks.
Behavior: Returns true if greater or equal, false otherwise.
Example: if x >= 1: { print "At least one"; } prints if x is 1 or more.