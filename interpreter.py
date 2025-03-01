# interpreter.py
import re
import traceback
import sys
import ctypes
import os
import time

class ReturnException(Exception):
    def __init__(self, value):
        self.value = value

class Interpreter:
    def __init__(self):
        self.variables = {'input_value': ''}
        self.functions = {}
        self.classes = {}
        self.type_map = {'int': int, 'float': float, 'bool': bool, 'string': str}
        self.debug_mode = False
        self.saucerful_rate = 0  # Start at 0, no upper limit
        self.last_error = None
        self.error_logged = False

        # Check 1: DLL and SDL setup
        dll_base_path = os.path.dirname(__file__)
        dll_path = os.path.join(dll_base_path, 'OCL2DRI', 'ocl2dri.dll')
        sdl_dir = r"C:\SDL3-3.2.4\x86_64-w64-mingw32\bin"
        sdl_dll_path = os.path.join(sdl_dir, 'SDL3.dll')

        if os.path.exists(sdl_dll_path):
            os.environ['PATH'] = sdl_dir + os.pathsep + os.environ.get('PATH', '')
            self.increment_saucerful("SDL3.dll path configured")
        else:
            self.log_error(f"SDL3.dll not found at {sdl_dll_path}. Ensure it’s in PATH or beside ocl2dri.dll.", stack_info=False)

        if not os.path.exists(dll_path):
            raise FileNotFoundError(f"DLL not found at {dll_path}. Build it with: gcc -shared -o {dll_path} window.c -IC:/SDL3-3.2.4/x86_64-w64-mingw32/include -LC:/SDL3-3.2.4/x86_64-w64-mingw32/lib -lSDL3")
        else:
            self.increment_saucerful("OCL2DRI DLL found")

        try:
            self.ocl2dri_lib = ctypes.CDLL(dll_path)
            self.increment_saucerful("OCL2DRI DLL loaded successfully")  # Check 2
        except OSError as e:
            raise RuntimeError(f"Failed to load DLL: {e}. Ensure it’s built correctly and dependencies (e.g., SDL3.dll) are available.")

        if self.ocl2dri_lib:
            expected_functions = [
                ('ocl2dri_init', [ctypes.c_int, ctypes.c_int, ctypes.c_char_p], ctypes.c_void_p),
                ('ocl2dri_set_background', [ctypes.c_void_p, ctypes.c_uint8, ctypes.c_uint8, ctypes.c_uint8], None),
                ('ocl2dri_set_title', [ctypes.c_void_p, ctypes.c_char_p], None),
                ('ocl2dri_set_size', [ctypes.c_void_p, ctypes.c_int, ctypes.c_int], None),
                ('ocl2dri_set_position', [ctypes.c_void_p, ctypes.c_int, ctypes.c_int], None),
                ('ocl2dri_set_fullscreen', [ctypes.c_void_p, ctypes.c_int], None),
                ('ocl2dri_set_opacity', [ctypes.c_void_p, ctypes.c_float], None),
                ('ocl2dri_set_border', [ctypes.c_void_p, ctypes.c_int], None),
                ('ocl2dri_set_min_size', [ctypes.c_void_p, ctypes.c_int, ctypes.c_int], None),
                ('ocl2dri_set_max_size', [ctypes.c_void_p, ctypes.c_int, ctypes.c_int], None),
                ('ocl2dri_set_always_on_top', [ctypes.c_void_p, ctypes.c_int], None),
                ('ocl2dri_set_resizable', [ctypes.c_void_p, ctypes.c_int], None),
                ('ocl2dri_set_frame_rate', [ctypes.c_void_p, ctypes.c_int], None),
                ('ocl2dri_update', [ctypes.c_void_p], None),
                ('ocl2dri_is_running', [ctypes.c_void_p], ctypes.c_int),
                ('ocl2dri_destroy', [ctypes.c_void_p], None),
                ('ocl2dri_get_key_state', [ctypes.c_void_p, ctypes.c_char_p], ctypes.c_int),
                ('ocl2dri_hide', [ctypes.c_void_p], None),
                ('ocl2dri_show', [ctypes.c_void_p], None),
                ('ocl2dri_set_icon', [ctypes.c_void_p, ctypes.c_char_p], None),
                ('ocl2dri_get_mouse_position', [ctypes.c_void_p, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)], None),
                ('ocl2dri_get_mouse_button_state', [ctypes.c_void_p, ctypes.c_int], ctypes.c_int),
                ('ocl2dri_get_delta_time', [ctypes.c_void_p], ctypes.c_float),
            ]

            missing_functions = []
            for func_name, argtypes, restype in expected_functions:
                if hasattr(self.ocl2dri_lib, func_name):
                    func = getattr(self.ocl2dri_lib, func_name)
                    func.argtypes = argtypes
                    if restype:
                        func.restype = restype
                else:
                    missing_functions.append(func_name)

            if missing_functions:
                self.log_error(f"Missing functions in ocl2dri.dll: {', '.join(missing_functions)}. Rebuild with the latest window.c.", stack_info=False)
            else:
                self.increment_saucerful("All OCL2DRI functions verified")  # Check 3
    
    def increment_saucerful(self, reason):
        """Increase Saucerful rate by 1 with no upper limit."""
        self.saucerful_rate += 1
        if self.debug_mode:
            print(f"Saucerful increased to {self.saucerful_rate}/∞: {reason}")
    
    def log_error(self, message, stack_info=False):
        """Log an error without modifying rate."""
        if not self.error_logged:
            error_details = {
                'message': message,
                'timestamp': time.time(),
                'stack': traceback.format_exc() if stack_info else None,
                'line': self.current_line if hasattr(self, 'current_line') else None
            }
            self.last_error = error_details
            print(f"Error: {message}")
            if stack_info and self.debug_mode:
                print(error_details['stack'])
            self.error_logged = True

    def get_saucerful_rate(self):
        """Return the current Saucerful rate."""
        return self.saucerful_rate

    def set_debug_mode(self, mode=True):
        self.debug_mode = mode
        return self.debug_mode

    def interpret(self, ast, in_function=False):
        self.error_logged = False
        try:
            last_result = None
            for statement in ast:
                if isinstance(statement, tuple) and len(statement) > 2 and isinstance(statement[2], int):
                    self.current_line = statement[2]
                result = self.execute(statement)
                if result in ['break', 'continue']:
                    return result
                last_result = result
            return last_result if in_function else None
        except ReturnException as e:
            if in_function:
                raise e
            else:
                return e.value
        except Exception as e:
            self.log_error(f"Interpretation error: {str(e)}", stack_info=self.debug_mode)
            return None
        finally:
            if hasattr(self, 'current_line'):
                del self.current_line

    def execute(self, statement):
        if not statement:
            return None
        try:
            stmt_type = statement[0] if isinstance(statement, (tuple, list)) else None
            if not stmt_type:
                return None

            if stmt_type == 'declare':
                _, _, var_name, type_annot, expr = statement
                value = self.evaluate(expr)
                if type_annot and type_annot in self.type_map:
                    expected_type = self.type_map[type_annot]
                    if value is not None and not isinstance(value, expected_type):
                        raise TypeError(f"Variable '{var_name}' annotated as {type_annot}, got {type(value).__name__}")
                self.variables[var_name] = value

            elif stmt_type == 'assign':
                _, left, expr = statement
                value = self.evaluate(expr)
                if left[0] != 'identifier':
                    raise ValueError("Assignment target must be an identifier")
                name = left[1]
                if '.' in name:
                    parts = name.split('.')
                    obj_name = parts[0]
                    attr_path = parts[1:]
                    obj = self.variables.get(obj_name)
                    if obj is None or not isinstance(obj, dict):
                        raise ValueError(f"Cannot assign to attribute on non-object '{obj_name}'")
                    current = obj
                    for part in attr_path[:-1]:
                        current = current.setdefault(part, {})
                    current[attr_path[-1]] = value
                else:
                    self.variables[name] = value

            elif stmt_type == 'aug_assign':
                _, left, op, expr = statement
                expr_val = self.evaluate(expr)
                if left[0] != 'identifier':
                    raise ValueError("Augmented assignment target must be an identifier")
                name = left[1]
                if '.' in name:
                    parts = name.split('.')
                    obj_name = parts[0]
                    attr_path = parts[1:]
                    obj = self.variables.get(obj_name)
                    if obj is None or not isinstance(obj, dict):
                        raise ValueError(f"Cannot assign to attribute on non-object '{obj_name}'")
                    current = obj
                    for part in attr_path[:-1]:
                        current = current.get(part, {})
                    current_val = current.get(attr_path[-1], 0)
                    current[attr_path[-1]] = self.apply_op(current_val, op, expr_val)
                else:
                    if name not in self.variables:
                        raise ValueError(f"Variable '{name}' not defined for augmented assignment")
                    current_val = self.variables[name]
                    self.variables[name] = self.apply_op(current_val, op, expr_val)

            elif stmt_type == 'print':
                _, expr = statement
                value = self.evaluate(expr)
                sys.stdout.write(str(value) if value is not None else "null")
                sys.stdout.write("\n")
                sys.stdout.flush()

            elif stmt_type == 'if':
                _, condition, body, elif_blocks, else_block = statement
                condition_val = self.evaluate(condition)
                if not isinstance(condition_val, bool):
                    raise TypeError("If condition must evaluate to a boolean")
                if condition_val:
                    return self.interpret(body)
                for elif_cond, elif_body in elif_blocks:
                    elif_cond_val = self.evaluate(elif_cond)
                    if not isinstance(elif_cond_val, bool):
                        raise TypeError("Elif condition must evaluate to a boolean")
                    if elif_cond_val:
                        return self.interpret(elif_body)
                if else_block:
                    return self.interpret(else_block)

            elif stmt_type == 'while':
                _, condition, body = statement
                while True:
                    condition_val = self.evaluate(condition)
                    if not isinstance(condition_val, bool):
                        raise TypeError("While condition must evaluate to a boolean")
                    if not condition_val:
                        break
                    result = self.interpret(body)
                    if result == 'break':
                        break
                    elif result == 'continue':
                        continue

            elif stmt_type == 'define':
                _, func_name, params, body = statement
                self.functions[func_name] = (params, body)

            elif stmt_type == 'class':
                _, class_name, methods = statement
                self.classes[class_name] = {method[1]: (method[2], method[3]) for method in methods}

            elif stmt_type == 'return':
                _, expr = statement
                value = self.evaluate(expr) if expr else None
                raise ReturnException(value)

            elif stmt_type == 'break':
                return 'break'

            elif stmt_type == 'continue':
                return 'continue'

            elif stmt_type == 'call':
                return self.evaluate(statement)

            else:
                raise ValueError(f"Unknown statement type: {stmt_type}")

        except ReturnException:
            raise
        except Exception as e:
            self.log_error(f"Error executing statement {str(statement)[:50]}...: {str(e)}")
            if self.debug_mode:
                print(traceback.format_exc())
            return None  # Continue despite error

    def evaluate(self, expr):
        try:
            if expr is None:
                return None
            if not isinstance(expr, (tuple, list)):
                return expr

            expr_type = expr[0]
            if expr_type == 'literal':
                if isinstance(expr[1], str):
                    return self.interpolate_string(expr[1])
                return expr[1]

            elif expr_type == 'identifier':
                name = expr[1]
                if '.' in name:
                    return self.resolve(name)
                if name not in self.variables:
                    raise NameError(f"Variable '{name}' is not defined")
                return self.variables[name]

            elif expr_type == 'binary':
                _, op, left, right = expr
                left_val = self.evaluate(left)
                right_val = self.evaluate(right)
                return self.apply_op(left_val, op, right_val)

            elif expr_type == 'index':
                _, base_expr, index_expr = expr
                base = self.evaluate(base_expr)
                index = self.evaluate(index_expr)
                if not isinstance(base, tuple):
                    raise ValueError(f"Cannot index non-tuple value: {base}")
                if not isinstance(index, int):
                    raise ValueError(f"Index must be an integer, got: {index}")
                if 0 <= index < len(base):
                    return base[index]
                raise ValueError(f"Index {index} out of range for tuple of length {len(base)}")

            elif expr_type == 'call_method':
                _, object_expr, method_name, args = expr
                obj = self.evaluate(object_expr)
                if not (isinstance(obj, dict) and '__class__' in obj):
                    raise ValueError("Attempt to call method on non-object")
                class_name = obj['__class__']
                if class_name not in self.classes or method_name not in self.classes[class_name]:
                    raise ValueError(f"Method '{method_name}' not found in class '{class_name}'")
                params, body = self.classes[class_name][method_name]
                evaluated_args = [self.evaluate(arg) for arg in args]
                if params and params[0][0] == 'self':
                    expected_args = len(params) - 1
                else:
                    expected_args = len(params)
                if expected_args != len(evaluated_args):
                    raise ValueError(f"Method '{method_name}' expects {expected_args} arguments, got {len(evaluated_args)}")
                local_vars = {'self': obj} if params and params[0][0] == 'self' else {}
                param_start = 1 if params and params[0][0] == 'self' else 0
                for (param, _), arg in zip(params[param_start:], evaluated_args):
                    local_vars[param] = arg
                old_vars = self.variables.copy()
                self.variables = {**self.variables, **local_vars}
                try:
                    result = self.interpret(body, in_function=True)
                    self.variables = old_vars
                    return result if result is not None else None
                except ReturnException as e:
                    self.variables = old_vars
                    return e.value

            elif expr_type == 'call':
                _, func_name, args = expr
                evaluated_args = [self.evaluate(arg) for arg in args]
                if '.' in func_name:
                    obj_name, method_name = func_name.rsplit('.', 1)
                    obj = self.variables.get(obj_name)
                    if obj and isinstance(obj, dict) and '__class__' in obj:
                        return self.evaluate(('call_method', ('identifier', obj_name), method_name, args))
                if func_name == 'ocl.classes':
                    if len(args) != 1 or not isinstance(evaluated_args[0], str):
                        raise ValueError("ocl.classes expects one string argument")
                    class_name = evaluated_args[0]
                    if class_name not in self.classes:
                        raise ValueError(f"Class '{class_name}' not defined")
                    return {'__class__': class_name}
                elif func_name == 'ocl.get_input':
                    if len(args) != 1 or not isinstance(evaluated_args[0], str):
                        raise ValueError("ocl.get_input expects one string argument")
                    sys.stdout.write(evaluated_args[0])
                    sys.stdout.flush()
                    try:
                        user_input = input()
                    except KeyboardInterrupt:
                        raise KeyboardInterrupt("Input interrupted by user")
                    sys.stdout.write("\n")
                    sys.stdout.flush()
                    return user_input
                elif func_name == 'ocl.get_set_input':
                    if len(args) != 1 or not isinstance(evaluated_args[0], str):
                        raise ValueError("ocl.get_set_input expects one string argument")
                    sys.stdout.write(evaluated_args[0])
                    sys.stdout.flush()
                    try:
                        user_input = input()
                    except KeyboardInterrupt:
                        raise KeyboardInterrupt("Input interrupted by user")
                    sys.stdout.write("\n")
                    sys.stdout.flush()
                    self.variables['input_value'] = user_input
                    return None
                elif func_name == 'ocl.get_ocl2dra.init':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 3 or not all(isinstance(arg, (int, str)) for arg in evaluated_args[:2] + evaluated_args[2:]):
                        raise ValueError("ocl.get_ocl2dra.init expects (width: int, height: int, title: string)")
                    width, height, title = evaluated_args
                    ctx_ptr = self.ocl2dri_lib.ocl2dri_init(width, height, title.encode('utf-8'))
                    if not ctx_ptr:
                        self.log_error("Failed to initialize OCL2DRI context")
                        return None
                    self.ocl2dri_lib.ocl2dri_update(ctx_ptr)
                    return ctx_ptr
                elif func_name == 'ocl.get_ocl2dra.set_background':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 4 or not all(isinstance(arg, (int, float)) for arg in evaluated_args[1:]):
                        raise ValueError("ocl.get_ocl2dra.set_background expects (context: pointer, r: int/float, g: int/float, b: int/float)")
                    ctx, r, g, b = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_background(ctx, int(r), int(g), int(b))
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_title':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], str):
                        raise ValueError("ocl.get_ocl2dra.set_title expects (context: pointer, title: string)")
                    ctx, title = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_title(ctx, title.encode('utf-8'))
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_size':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 3 or not all(isinstance(arg, (int, float)) for arg in evaluated_args[1:]):
                        raise ValueError("ocl.get_ocl2dra.set_size expects (context: pointer, width: int/float, height: int/float)")
                    ctx, width, height = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_size(ctx, int(width), int(height))
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_position':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 3 or not all(isinstance(arg, (int, float)) for arg in evaluated_args[1:]):
                        raise ValueError("ocl.get_ocl2dra.set_position expects (context: pointer, x: int/float, y: int/float)")
                    ctx, x, y = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_position(ctx, int(x), int(y))
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_fullscreen':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], (int, bool)):
                        raise ValueError("ocl.get_ocl2dra.set_fullscreen expects (context: pointer, fullscreen: bool/int)")
                    ctx, fullscreen = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_fullscreen(ctx, 1 if fullscreen else 0)
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_opacity':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], (int, float)):
                        raise ValueError("ocl.get_ocl2dra.set_opacity expects (context: pointer, opacity: float 0.0-1.0)")
                    ctx, opacity = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_opacity(ctx, float(opacity))
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_border':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], (int, bool)):
                        raise ValueError("ocl.get_ocl2dra.set_border expects (context: pointer, bordered: bool/int)")
                    ctx, bordered = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_border(ctx, 1 if bordered else 0)
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_min_size':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 3 or not all(isinstance(arg, (int, float)) for arg in evaluated_args[1:]):
                        raise ValueError("ocl.get_ocl2dra.set_min_size expects (context: pointer, min_width: int/float, min_height: int/float)")
                    ctx, min_width, min_height = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_min_size(ctx, int(min_width), int(min_height))
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_max_size':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 3 or not all(isinstance(arg, (int, float)) for arg in evaluated_args[1:]):
                        raise ValueError("ocl.get_ocl2dra.set_max_size expects (context: pointer, max_width: int/float, max_height: int/float)")
                    ctx, max_width, max_height = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_max_size(ctx, int(max_width), int(max_height))
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_always_on_top':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], (int, bool)):
                        raise ValueError("ocl.get_ocl2dra.set_always_on_top expects (context: pointer, on_top: bool/int)")
                    ctx, on_top = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_always_on_top(ctx, 1 if on_top else 0)
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_resizable':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], (int, bool)):
                        raise ValueError("ocl.get_ocl2dra.set_resizable expects (context: pointer, resizable: bool/int)")
                    ctx, resizable = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_resizable(ctx, 1 if resizable else 0)
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_frame_rate':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], (int, float)):
                        raise ValueError("ocl.get_ocl2dra.set_frame_rate expects (context: pointer, fps: int/float)")
                    ctx, fps = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_frame_rate(ctx, int(fps))
                    return None
                elif func_name == 'ocl.get_ocl2dra.hide':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 1:
                        raise ValueError("ocl.get_ocl2dra.hide expects (context: pointer)")
                    ctx = evaluated_args[0]
                    self.ocl2dri_lib.ocl2dri_hide(ctx)
                    return None
                elif func_name == 'ocl.get_ocl2dra.show':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 1:
                        raise ValueError("ocl.get_ocl2dra.show expects (context: pointer)")
                    ctx = evaluated_args[0]
                    self.ocl2dri_lib.ocl2dri_show(ctx)
                    return None
                elif func_name == 'ocl.get_ocl2dra.set_icon':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], str):
                        raise ValueError("ocl.get_ocl2dra.set_icon expects (context: pointer, icon_path: string)")
                    ctx, icon_path = evaluated_args
                    self.ocl2dri_lib.ocl2dri_set_icon(ctx, icon_path.encode('utf-8'))
                    return None
                elif func_name == 'ocl.get_ocl2dra.get_mouse_position':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 1:
                        raise ValueError("ocl.get_ocl2dra.get_mouse_position expects (context: pointer)")
                    ctx = evaluated_args[0]
                    x = ctypes.c_float()
                    y = ctypes.c_float()
                    self.ocl2dri_lib.ocl2dri_get_mouse_position(ctx, ctypes.byref(x), ctypes.byref(y))
                    return (int(x.value), int(y.value))
                elif func_name == 'ocl.get_ocl2dra.get_mouse_button_state':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], (int, float)):
                        raise ValueError("ocl.get_ocl2dra.get_mouse_button_state expects (context: pointer, button: int/float)")
                    ctx, button = evaluated_args
                    return self.ocl2dri_lib.ocl2dri_get_mouse_button_state(ctx, int(button))
                elif func_name == 'ocl.get_ocl2dra.get_delta_time':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 1:
                        raise ValueError("ocl.get_ocl2dra.get_delta_time expects (context: pointer)")
                    ctx = evaluated_args[0]
                    return self.ocl2dri_lib.ocl2dri_get_delta_time(ctx)
                elif func_name == 'ocl.get_ocl2dra.update':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 1:
                        raise ValueError("ocl.get_ocl2dra.update expects (context: pointer)")
                    ctx = evaluated_args[0]
                    self.ocl2dri_lib.ocl2dri_update(ctx)
                    return None
                elif func_name == 'ocl.get_ocl2dra.is_running':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 1:
                        raise ValueError("ocl.get_ocl2dra.is_running expects (context: pointer)")
                    ctx = evaluated_args[0]
                    return bool(self.ocl2dri_lib.ocl2dri_is_running(ctx))
                elif func_name == 'ocl.get_ocl2dra.destroy':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 1:
                        raise ValueError("ocl.get_ocl2dra.destroy expects (context: pointer)")
                    ctx = evaluated_args[0]
                    self.ocl2dri_lib.ocl2dri_destroy(ctx)
                    return None
                elif func_name == 'ocl.get_ocl2dra.get_key_state':
                    if not self.ocl2dri_lib:
                        raise ValueError("OCL2DRI library not loaded")
                    if len(args) != 2 or not isinstance(evaluated_args[1], str):
                        raise ValueError("ocl.get_ocl2dra.get_key_state expects (context: pointer, key: string)")
                    ctx, key = evaluated_args
                    return self.ocl2dri_lib.ocl2dri_get_key_state(ctx, key.encode('utf-8'))
                elif func_name in self.functions:
                    params, body = self.functions[func_name]
                    if len(params) != len(evaluated_args):
                        raise ValueError(f"Function '{func_name}' expects {len(params)} arguments, got {len(evaluated_args)}")
                    local_vars = {param[0]: arg for (param, _), arg in zip(params, evaluated_args)}
                    old_vars = self.variables.copy()
                    self.variables = {**self.variables, **local_vars}
                    try:
                        result = self.interpret(body, in_function=True)
                        self.variables = old_vars
                        return result if result is not None else None
                    except ReturnException as e:
                        self.variables = old_vars
                        return e.value
                else:
                    raise ValueError(f"Undefined function: '{func_name}'")
            else:
                raise ValueError(f"Unknown expression type: {expr_type}")
        except Exception as e:
            self.log_error(f"Error evaluating expression {str(expr)[:50]}...: {str(e)}")
            if self.debug_mode:
                print(traceback.format_exc())
            return None  # Continue despite error

    def interpolate_string(self, string):
        try:
            def replace_var(match):
                var_name = match.group(1)
                if var_name in self.variables:
                    value = self.variables[var_name]
                    return str(value) if value is not None else "null"
                else:
                    raise NameError(f"Undefined variable in string interpolation: '{var_name}'")
            return re.sub(r'\{([a-zA-Z_][a-zA-Z0-9_]*)\}', replace_var, string)
        except Exception as e:
            self.log_error(f"String interpolation error: {str(e)}")
            return string  # Return original string on error

    def resolve(self, name):
        try:
            parts = name.split('.')
            value = self.variables.get(parts[0])
            if value is None:
                raise ValueError(f"Undefined variable: '{parts[0]}'")
            for part in parts[1:]:
                if isinstance(value, dict):
                    if part not in value:
                        raise ValueError(f"Attribute '{part}' not found on object")
                    value = value.get(part)
                elif isinstance(value, tuple) and part.isdigit():
                    idx = int(part)
                    if 0 <= idx < len(value):
                        value = value[idx]
                    else:
                        raise ValueError(f"Index '{idx}' out of range for tuple")
                else:
                    raise ValueError(f"Cannot access attribute '{part}' on non-object or non-tuple")
                if value is None and part != '__class__':
                    raise ValueError(f"Attribute '{part}' has null value")
            return value
        except Exception as e:
            self.log_error(f"Error resolving '{name}': {str(e)}")
            return None  # Return None on error

    def apply_op(self, left, op, right):
        try:
            if op in ('==', '!='):
                if op == '==':
                    return left is right
                elif op == '!=':
                    return left is not right
            if left is None or right is None:
                raise ValueError(f"Cannot perform operation '{op}' with null value")
            if op == '+':
                if isinstance(left, int) and isinstance(right, int):
                    return left + right
                elif isinstance(left, (int, float)) and isinstance(right, (int, float)):
                    return left + right
                return left + right
            elif op == '-':
                if isinstance(left, int) and isinstance(right, int):
                    return left - right
                elif isinstance(left, (int, float)) and isinstance(right, (int, float)):
                    return left - right
                return left - right
            elif op == '*':
                if isinstance(left, int) and isinstance(right, int):
                    return left * right
                elif isinstance(left, (int, float)) and isinstance(right, (int, float)):
                    return left * right
                return left * right
            elif op == '/':
                if right == 0:
                    raise ZeroDivisionError("Division by zero")
                if isinstance(left, int) and isinstance(right, int):
                    return left // right
                elif isinstance(left, (int, float)) and isinstance(right, (int, float)):
                    return left / right
                return left / right
            elif op == '%':
                if right == 0:
                    raise ZeroDivisionError("Modulus by zero")
                if isinstance(left, (int, float)) and isinstance(right, (int, float)):
                    return left % right
                return left % right
            elif op == '<':
                return left < right
            elif op == '>':
                return left > right
            elif op == '<=':
                return left <= right
            elif op == '>=':
                return left >= right
            else:
                raise ValueError(f"Unsupported operator: '{op}'")
        except TypeError:
            self.log_error(f"Type error in operation '{op}' with values {left} ({type(left).__name__}) and {right} ({type(right).__name__})")
            return None
        except Exception as e:
            self.log_error(f"Operator '{op}' error with {left} and {right}: {str(e)}")
            return None