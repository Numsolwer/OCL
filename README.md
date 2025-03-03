# OCL - Own Custom Language
# OCL2DRI - Own Custom Language 2D Rendering Interface

Welcome to **OCL2DRI**, a lightweight, custom scripting language and 2D rendering library designed for rapid prototyping and interactive applications. Built with Python and SDL3, OCL2DRI provides a simple yet powerful environment for developers to create 2D applications with ease. This project includes an interpreter, a rendering engine, and an enhanced editor for a complete development experience.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [OCL Language Keywords](#ocl-language-keywords)
  - [Core Keywords](#core-keywords)
  - [General Functions](#general-functions)
  - [OCL2DRI Functions](#ocl2dri-functions)
- [OCL Editor](#ocl-editor)
- [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

OCL2DRI combines a custom scripting language (OCL) with a 2D rendering library powered by SDL3. The language supports variables, control flow, functions, classes, and direct integration with rendering capabilities via the OCL2DRI API. The project is structured around:

- **Interpreter**: Executes OCL scripts (`interpreter.py`) with robust error handling and debugging.
- **Lexer and Parser**: Tokenizes and builds an AST from OCL code (`lexer.py`, `parser.py`).
- **OCL2DRI DLL**: A C-based rendering library interfaced via `ctypes` (`window.c`).
- **Editor**: An SDL-based GUI for writing, running, and debugging OCL scripts (`ocl_editor.c`).

The "Saucerful Rate" metric tracks execution success, starting at 0 and aiming for 4+ as a baseline, with no upper limit for additional stability checks.

---

## Features

- **Simple Syntax**: Easy-to-learn keywords and operators for quick scripting.
- **Type Safety**: Optional type annotations (`int`, `float`, `bool`, `string`) for robust code.
- **2D Rendering**: Direct access to window management, input handling, and rendering via OCL2DRI.
- **Interactive Mode**: Run scripts line-by-line or from files with real-time feedback.
- **Editor GUI**: Syntax highlighting, file management, and run/debug capabilities.
- **Extensible**: Add custom functions and integrate with external libraries.

---

## Installation

### Prerequisites
- Python 3.8+
- SDL3 (development libraries)
- GCC (for compiling the OCL2DRI DLL on Windows)
- SDL_ttf (for the editor)

### Steps
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/ocl2dri.git
   cd ocl2dri
Install Python Dependencies: No external Python packages are required beyond the standard library.
Set Up SDL3:
Download SDL3 from libsdl.org.
Place the bin, include, and lib folders in C:\SDL3-3.2.4\x86_64-w64-mingw32.
Compile the OCL2DRI DLL:
bash
Wrap
Copy
gcc -shared -o OCL2DRI/ocl2dri.dll window.c -IC:/SDL3-3.2.4/x86_64-w64-mingw32/include -LC:/SDL3-3.2.4/x86_64-w64-mingw32/lib -lSDL3
Build the Editor (optional):
Ensure SDL_ttf is installed.
Compile ocl_editor.c:
bash
Wrap
Copy
gcc -o ocl_editor.exe ocl_editor.c -IC:/SDL3-3.2.4/x86_64-w64-mingw32/include -LC:/SDL3-3.2.4/x86_64-w64-mingw32/lib -lSDL3 -lSDL3_ttf
Usage
Running Scripts
From a File:
bash
Wrap
Copy
python main.py script.ocl
With Debug Mode:
bash
Wrap
Copy
python main.py --debug script.ocl
Interactive Mode:
bash
Wrap
Copy
python main.py
Type exit to quit, help for commands, or press Enter twice to execute code.
Launching the Editor
bash
Wrap
Copy
python main.py run editor
OCL Language Keywords
Core Keywords
Keyword	Purpose	Example
let	Declares a variable with optional type annotation	let x: int = 5;
print	Outputs a value to the console with a newline	print "Hello";
if	Conditional execution based on a boolean expression	if x > 0: { print "Positive"; }
elif	Alternative condition in an if statement	elif x < 0: { print "Negative"; }
else	Default block in an if statement	else: { print "Zero"; }
while	Loops while a condition is true	while x > 0: { x = x - 1; }
define	Defines a function with parameters	define add(x, y): { return x + y; }
return	Exits a function, optionally returning a value	return x + y;
class	Defines a class with methods	class Point: { define getX(): { return 0; } }
break	Exits the nearest while loop	if x == 0: { break; }
continue	Skips to the next iteration of a while loop	if x < 0: { continue; }
true	Boolean true value	let b = true;
false	Boolean false value	if false: { print "Never"; }
null	Represents no value	let x = null;
int	Type annotation for integers	let x: int = 5;
float	Type annotation for floating-point numbers	let x: float = 5.5;
bool	Type annotation for booleans	let x: bool = true;
string	Type annotation for strings	let x: string = "Hello";
ocl	Namespace for OCL library functions	ocl.get_ocl2dra.init(800, 600, "Title");
Operators
Operator	Purpose	Example
=	Assignment	let x = 5;
+=	Add and assign	x += 2;
-=	Subtract and assign	x -= 3;
*=	Multiply and assign	x *= 2;
/=	Divide and assign	x /= 2;
+	Addition or string concatenation	5 + 3 or "a" + "b"
-	Subtraction	10 - 4;
*	Multiplication	2 * 3;
/	Division	6 / 2;
%	Modulus	7 % 3;
==	Equality	x == 5;
!=	Inequality	x != 0;
<	Less than	x < 10;
>	Greater than	x > 0;
<=	Less than or equal	x <= 5;
>=	Greater than or equal	x >= 1;
General Functions
Function	Purpose	Example
ocl.classes	Creates an instance of a defined class	let obj = ocl.classes("Point");
ocl.get_input	Prompts user for input and returns it	let name = ocl.get_input("Name: ");
ocl.get_set_input	Prompts user and stores input in input_value	ocl.get_set_input("Enter: ");
OCL2DRI Functions
These functions interface with the OCL2DRI rendering library, prefixed with ocl.get_ocl2dra.:

Function	Purpose	Example
init	Initializes a rendering window	let w = ocl.get_ocl2dra.init(800, 600, "App");
set_background	Sets window background color (RGB)	ocl.get_ocl2dra.set_background(w, 255, 0, 0);
set_title	Updates window title	ocl.get_ocl2dra.set_title(w, "Running");
set_size	Resizes window	ocl.get_ocl2dra.set_size(w, 600, 400);
set_position	Moves window on screen	ocl.get_ocl2dra.set_position(w, 100, 100);
set_fullscreen	Toggles fullscreen mode	ocl.get_ocl2dra.set_fullscreen(w, 1);
set_opacity	Adjusts window transparency (0.0-1.0)	ocl.get_ocl2dra.set_opacity(w, 0.5);
set_border	Enables/disables window border	ocl.get_ocl2dra.set_border(w, 0);
set_min_size	Sets minimum window size	ocl.get_ocl2dra.set_min_size(w, 200, 150);
set_max_size	Sets maximum window size	ocl.get_ocl2dra.set_max_size(w, 800, 600);
set_always_on_top	Keeps window above others	ocl.get_ocl2dra.set_always_on_top(w, 1);
set_resizable	Toggles window resizability	ocl.get_ocl2dra.set_resizable(w, 0);
set_frame_rate	Sets target FPS	ocl.get_ocl2dra.set_frame_rate(w, 60);
update	Updates window and processes events	ocl.get_ocl2dra.update(w);
is_running	Checks if window is active	while ocl.get_ocl2dra.is_running(w): {}
destroy	Closes window and frees resources	ocl.get_ocl2dra.destroy(w);
hide	Hides window	ocl.get_ocl2dra.hide(w);
show	Shows hidden window	ocl.get_ocl2dra.show(w);
set_icon	Sets window icon (BMP file)	ocl.get_ocl2dra.set_icon(w, "icon.bmp");
get_mouse_position	Returns mouse (x, y) coordinates	let pos = ocl.get_ocl2dra.get_mouse_position(w);
get_mouse_button_state	Checks mouse button state (1=left, 2=middle, 3=right)	let click = ocl.get_ocl2dra.get_mouse_button_state(w, 1);
get_delta_time	Gets time since last frame	let dt = ocl.get_ocl2dra.get_delta_time(w);
get_key_state	Checks if a key is pressed	let key = ocl.get_ocl2dra.get_key_state(w, "h");
OCL Editor
The OCL Editor is a graphical interface built with SDL2/SDL3 and SDL_ttf, enhancing the development workflow:

Features:
Syntax Highlighting: Colors for keywords, strings, comments, and numbers.
File Management: New, Open, Save, Save As, and Exit options.
Run/Debug: Execute OCL scripts or C code with GCC compilation.
Resizable Panels: Adjust layout for editor, file explorer, and console.
Fullscreen Support: Toggle with F11.
Launching:
bash
Wrap
Copy
python main.py run editor
Requires ocl_editor.exe compiled and placed in the specified directory.
Usage:
Use the "File", "Edit", and "View" menus for navigation.
Click "Run" or "Debug" to execute code, with output in the console panel.
Examples
Basic Window
ocl
Wrap
Copy
let window = ocl.get_ocl2dra.init(400, 300, "Hello OCL");
ocl.get_ocl2dra.set_background(window, 0, 128, 255);
while ocl.get_ocl2dra.is_running(window): {
    ocl.get_ocl2dra.update(window);
}
ocl.get_ocl2dra.destroy(window);
Feature Showcase
See OCL2DRI Feature Showcase in the provided documents for a full example with opacity fading, mouse/keyboard input, and window manipulation.

Contributing
Contributions are welcome! To contribute:

Fork the repository.
Create a feature branch (git checkout -b feature-name).
Commit changes (git commit -m "Add feature").
Push to your fork (git push origin feature-name).
Open a Pull Request.
Please ensure code follows the existing style and includes tests where applicable.

License
This project is licensed under the MIT License. See LICENSE for details.

Happy coding with OCL2DRI!

text
Wrap
Copy

### Improvements Made

1. **Comprehensive Structure**: Organized into clear sections for easy navigation.
2. **Keyword Tables**: Added detailed tables for core keywords, operators, general functions, and OCL2DRI functions, making them quick-reference-friendly.
3. **Editor Details**: Included a dedicated section for the OCL Editor with its features and usage.
4. **Installation Clarity**: Provided step-by-step instructions with prerequisites and compilation commands.
5. **Examples**: Referenced existing examples and provided a simple one inline.
6. **Professional Tone**: Written in a clear, concise, and welcoming manner suitable for developers.

This `README.md` should serve as a solid foundation for users and contributors alike. Let me know if you'd like further refinements!
