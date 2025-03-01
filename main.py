#main.py
import sys
import os
import traceback
import time
from lexer import Lexer
from parser import Parser
from interpreter import Interpreter

def execute_code(code, lexer, parser, interpreter, debug=False):
    try:
        interpreter.set_debug_mode(debug)
        if not code or not code.strip():
            interpreter.log_error("Empty code provided", stack_info=True)
            return True
            
        tokens = lexer.tokenize(code)
        if debug:
            print("TOKENS:")
            for i, token in enumerate(tokens, 1):
                print(f"  {i}. {token}")
        interpreter.increment_saucerful("Tokenization successful")  # Check 2
            
        ast = parser.parse(code)
        if debug:
            print("\nAST:")
            print_ast(ast)
        interpreter.increment_saucerful("Parsing successful")  # Check 3
            
        interpreter.interpret(ast)
        interpreter.increment_saucerful("Interpretation successful")  # Check 4
        interpreter.increment_saucerful("Runtime stability confirmed")  # Check 5
        return True
        
    except SyntaxError as e:
        print(f"Syntax Error: {str(e)}")
        interpreter.log_error(f"Syntax error occurred: {str(e)}", stack_info=debug)
        return False
    except RuntimeError as e:
        print(f"Runtime Error: {str(e)}")
        interpreter.log_error(f"Runtime error occurred: {str(e)}", stack_info=debug)
        if debug:
            print("Run with --debug for stack trace.")
        return False
    except KeyboardInterrupt:
        print("\nExecution interrupted by user")
        interpreter.log_error("Execution interrupted by user", stack_info=False)
        return False
    except Exception as e:
        print(f"Unexpected Error: {str(e)}")
        interpreter.log_error(f"Unexpected error: {str(e)}", stack_info=debug)
        if debug:
            print(traceback.format_exc())
        return True

def print_ast(ast, indent=0):
    indent_str = "  " * indent
    if isinstance(ast, list):
        for i, stmt in enumerate(ast, 1):
            print(f"{indent_str}[Statement {i}]")
            print_ast(stmt, indent + 1)
    elif isinstance(ast, tuple):
        print(f"{indent_str}{ast[0]}:")
        for i in range(1, len(ast)):
            if isinstance(ast[i], (list, tuple)) and ast[i] and not (isinstance(ast[i], tuple) and ast[i][0] == 'literal'):
                print(f"{indent_str}  [Arg {i}]:")
                print_ast(ast[i], indent + 2)
            else:
                print(f"{indent_str}  [Arg {i}]: {ast[i]}")
    else:
        print(f"{indent_str}{ast}")

def print_help():
    print("OCL Interpreter - Enhanced Debugger")
    print("Usage: python main.py [options] [filename]")
    print("\nOptions:")
    print("  --help      Display this help message and exit")
    print("  --debug     Run in debug mode with detailed output and stack traces")
    print("  run editor  Launch the OCL Editor GUI")
    print("\nArguments:")
    print("  filename    Path to the OCL script file to execute")
    print("\nIf no filename is provided, enters interactive mode:")
    print("  - Type 'exit' to quit")
    print("  - Type 'help' for interactive commands")
    print("  - Press Enter twice to execute code")
    print("\nExamples:")
    print("  python main.py script.ocl          # Execute an OCL file")
    print("  python main.py --debug script.ocl  # Execute with debug output")
    print("  python main.py run editor          # Launch OCL Editor")
    print("  python main.py                     # Start interactive mode")
    print("\nSaucerful Rate: Starts at 0, aims for 4+, can exceed 4 with extra checks")

def main():
    if len(sys.argv) >= 3 and sys.argv[1] == "run" and sys.argv[2] == "editor":
        editor_path = r"C:\Users\nayle\Documents\Projects\OCL\OCL2DRI\ocl_editor.exe"
        if not os.path.exists(editor_path):
            print(f"Error: 'ocl_editor.exe' not found in {editor_path}")
            sys.exit(1)
        print(f"Attempting to launch: {editor_path} run editor")
        result = os.system(f'"{editor_path}" run editor')
        if result != 0:
            print(f"Error: Failed to launch OCL Editor (exit code: {result})")
            # Try to get more details
            import subprocess
            try:
                process = subprocess.run([editor_path, "run", "editor"], capture_output=True, text=True)
                print(f"Stdout: {process.stdout}")
                print(f"Stderr: {process.stderr}")
            except Exception as e:
                print(f"Subprocess error: {str(e)}")
        sys.exit(result)  # Exit after launching editor
        
    if '--help' in sys.argv:
        print_help()
        sys.exit(0)

    lexer = Lexer()
    parser = Parser(lexer)
    
    debug_mode = '--debug' in sys.argv
    if debug_mode:
        sys.argv.remove('--debug')
        print("Debug mode enabled - Detailed error reporting and Saucerful progress active")

    is_interactive = len(sys.argv) < 2

    try:
        interpreter = Interpreter()
        interpreter.increment_saucerful("Interpreter initialized")  # Check 0
        print("Hello, World! Welcome User you're using OCL2DRI - Own Custom Language 2D Rendering library.")
        print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
    except Exception as e:
        print(f"Initialization Error: {str(e)}")
        sys.exit(1)

    if not is_interactive:
        filename = sys.argv[1]
        try:
            if not os.path.exists(filename):
                interpreter.log_error(f"File '{filename}' not found", stack_info=debug_mode)
                print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
                sys.exit(1)
                
            with open(filename, "r", encoding="utf-8-sig") as f:
                code = f.read()
            interpreter.increment_saucerful("File loaded successfully")  # Check 1
                
        except PermissionError:
            interpreter.log_error(f"No permission to read file '{filename}'", stack_info=debug_mode)
            print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
            sys.exit(1)
        except UnicodeDecodeError:
            try:
                with open(filename, "r", encoding="latin-1") as f:
                    code = f.read()
                interpreter.increment_saucerful("File loaded with fallback encoding")
            except Exception:
                interpreter.log_error(f"Unable to decode file '{filename}'. Check encoding.", stack_info=debug_mode)
                print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
                sys.exit(1)
        except Exception as e:
            interpreter.log_error(f"Error reading file '{filename}': {str(e)}", stack_info=debug_mode)
            if debug_mode:
                print(traceback.format_exc())
            print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
            sys.exit(1)

        success = execute_code(code, lexer, parser, interpreter, debug_mode)
        if not success:
            print("Execution failed. Use --debug for detailed diagnostics.")
            sys.exit(1)
        else:
            interpreter.increment_saucerful("Full script execution completed")  # Check 6
            print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
    else:
        print("OCL Interpreter - Interactive Mode")
        print("Type 'exit' to quit, 'help' for commands, or enter OCL code.")
        print("Press Enter twice to execute.")
        
        error_history = set()
        
        while True:
            try:
                code_lines = []
                prompt = "OCL>>> "
                
                while True:
                    try:
                        line = input(prompt)
                        prompt = "...    "
                    except EOFError:
                        print("\nExiting due to EOF")
                        return
                        
                    if not code_lines and line.strip().lower() == "exit":
                        print("Goodbye!")
                        print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
                        return
                    elif not code_lines and line.strip().lower() == "help":
                        print("\nAvailable commands:")
                        print("  exit     - Exit the interpreter")
                        print("  help     - Show this help message")
                        print("  clear    - Clear the current code buffer")
                        print("  debug    - Toggle debug mode (current:", "on" if debug_mode else "off", ")")
                        print("  errors   - Show last error summary")
                        print("\nEnter OCL code and press Enter twice to execute.\n")
                        break
                    elif not code_lines and line.strip().lower() == "clear":
                        code_lines = []
                        print("Code buffer cleared")
                        break
                    elif not code_lines and line.strip().lower() == "debug":
                        debug_mode = not debug_mode
                        print(f"Debug mode {'enabled' if debug_mode else 'disabled'}")
                        break
                    elif not code_lines and line.strip().lower() == "errors":
                        if not error_history:
                            print("No errors recorded yet.")
                        else:
                            print("\nError Summary:")
                            for i, err in enumerate(error_history, 1):
                                print(f"{i}. {err}")
                        break
                        
                    if line.strip() == "" and code_lines:
                        break
                        
                    code_lines.append(line)
                    
                code = "\n".join(code_lines)
                if code.strip():
                    print("\n--- Executing ---")
                    success = execute_code(code, lexer, parser, interpreter, debug_mode)
                    print("--- Done ---")
                    if success:
                        interpreter.increment_saucerful("Interactive execution completed")  # Check 6
                    print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
                    if not success and interpreter.last_error:
                        error_key = f"{interpreter.last_error['message']} at {interpreter.last_error.get('line', 'unknown')}"
                        error_history.add(error_key)
                    
            except KeyboardInterrupt:
                print("\nCommand interrupted. Type 'exit' to quit.")
                interpreter.log_error("Interactive command interrupted", stack_info=False)
                print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")
            except Exception as e:
                error_msg = f"Unexpected interactive error: {str(e)}"
                interpreter.log_error(error_msg, stack_info=debug_mode)
                error_history.add(error_msg)
                if debug_mode:
                    print(traceback.format_exc())
                print(f"Saucerful  {interpreter.get_saucerful_rate()}/∞ (4 is baseline)")

if __name__ == "__main__":
    main()