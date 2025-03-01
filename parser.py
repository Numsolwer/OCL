#parser.py
from lexer import Lexer

class Parser:
    def __init__(self, lexer):
        self.lexer = lexer
        self.tokens = []
        self.current_token = None
        self.current_pos = 0

    def parse(self, code):
        try:
            self.tokens = self.lexer.tokenize(code)
            self.current_pos = 0
            self.advance()
            return self.program()
        except SyntaxError as e:
            raise SyntaxError(f"Parsing failed: {str(e)}")

    def advance(self):
        if self.current_pos < len(self.tokens):
            self.current_token = self.tokens[self.current_pos]
            self.current_pos += 1
        else:
            self.current_token = None

    def eat(self, token_type):
        if self.current_token and self.current_token[0] == token_type:
            self.advance()
        else:
            expected = token_type
            got = self.current_token[1] if self.current_token else "EOF"
            line = self.current_token[2] if self.current_token else "unknown"
            column = self.current_token[3] if self.current_token else "unknown"
            context = self.get_context()
            raise SyntaxError(f"Line {line}, column {column}: Expected '{expected}', got '{got}'. {context}")

    def get_context(self):
        if not self.current_token:
            return ""
        token_type = self.current_token[0].lower()
        if token_type == 'if':
            return "Did you forget a colon after the 'if' condition?"
        elif token_type == 'while':
            return "Did you forget a colon after the 'while' condition?"
        elif token_type == 'define':
            return "Did you forget parentheses or a colon in the function definition?"
        elif token_type == 'class':
            return "Did you forget a colon or brace in the class definition?"
        return ""

    def program(self):
        statements = []
        while self.current_token is not None:
            try:
                stmt = self.statement()
                statements.append(stmt)
                if self.current_token and self.current_token[0] == 'semicolon':
                    self.eat('semicolon')
            except SyntaxError as e:
                print(f"Syntax Error: {str(e)}")
                self.recover()
        return statements

    def recover(self):
        while self.current_token and self.current_token[0] not in ('semicolon', 'right_brace'):
            self.advance()
        if self.current_token and self.current_token[0] == 'semicolon':
            self.eat('semicolon')

    def statement(self):
        token_type = self.current_token[0].lower() if self.current_token else None
        if token_type == 'let':
            return self.declaration()
        elif token_type == 'print':
            return self.print_statement()
        elif token_type == 'if':
            return self.if_statement()
        elif token_type == 'while':
            return self.while_statement()
        elif token_type == 'define':
            return self.function_definition()
        elif token_type == 'class':
            return self.class_definition()
        elif token_type == 'ocl':
            return self.ocl_statement()
        elif token_type == 'return':
            self.advance()
            expr = self.expression() if self.current_token and self.current_token[0] != 'semicolon' else None
            return ('return', expr)
        elif token_type == 'break':
            self.advance()
            return ('break',)
        elif token_type == 'continue':
            self.advance()
            return ('continue',)
        elif token_type == 'identifier':
            return self.assignment_or_call_statement()
        else:
            line = self.current_token[2] if self.current_token else "unknown"
            column = self.current_token[3] if self.current_token else "unknown"
            raise SyntaxError(f"Line {line}, column {column}: Unexpected token: {self.current_token[1] if self.current_token else 'EOF'}")

    def class_definition(self):
        self.advance()
        if self.current_token[0] != 'identifier':
            raise SyntaxError(f"Line {self.current_token[2]}, column {self.current_token[3]}: Expected class name")
        class_name = self.current_token[1]
        self.advance()
        self.eat('colon')
        self.eat('left_brace')
        methods = []
        while self.current_token and self.current_token[0] != 'right_brace':
            if self.current_token[0].lower() == 'define':
                methods.append(self.function_definition())
                if self.current_token and self.current_token[0] == 'semicolon':
                    self.eat('semicolon')
            else:
                line = self.current_token[2]
                column = self.current_token[3]
                raise SyntaxError(f"Line {line}, column {column}: Expected method definition in class")
        self.eat('right_brace')
        return ('class', class_name, methods)

    def ocl_statement(self):
        self.eat('ocl')
        self.eat('dot')
        if self.current_token[1].lower() == 'classes':
            self.advance()
            self.eat('left_paren')
            if self.current_token[0] != 'string_literal':
                line = self.current_token[2]
                column = self.current_token[3]
                raise SyntaxError(f"Line {line}, column {column}: Expected class name as string literal")
            class_name = self.current_token[1][1:-1]
            self.advance()
            self.eat('right_paren')
            return ('call', 'ocl.classes', [('literal', class_name)])
        elif self.current_token[1].lower() in (
            'get_input', 'get_set_input', 'get_ocl2dra.init', 'get_ocl2dra.set_background',
            'get_ocl2dra.set_title', 'get_ocl2dra.set_size', 'get_ocl2dra.set_position',
            'get_ocl2dra.set_fullscreen', 'get_ocl2dra.set_opacity', 'get_ocl2dra.set_border',
            'get_ocl2dra.set_min_size', 'get_ocl2dra.set_max_size', 'get_ocl2dra.set_always_on_top',
            'get_ocl2dra.set_resizable', 'get_ocl2dra.set_frame_rate', 'get_ocl2dra.update',
            'get_ocl2dra.is_running', 'get_ocl2dra.destroy', 'get_ocl2dra.hide', 'get_ocl2dra.show',
            'get_ocl2dra.set_icon', 'get_ocl2dra.get_mouse_position', 'get_ocl2dra.get_mouse_button_state',
            'get_ocl2dra.get_delta_time', 'get_ocl2dra.get_key_state'
        ):
            ocl_func = 'ocl.' + self.current_token[1]
            self.advance()
            self.eat('left_paren')
            args = []
            if self.current_token and self.current_token[0] != 'right_paren':
                while True:
                    args.append(self.expression())
                    if self.current_token[0] == 'right_paren':
                        break
                    self.eat('comma')
            self.eat('right_paren')
            return ('call', ocl_func, args)
        else:
            line = self.current_token[2]
            column = self.current_token[3]
            raise SyntaxError(f"Line {line}, column {column}: Unknown ocl command")

    def function_definition(self):
        self.advance()
        if self.current_token[0] != 'identifier':
            raise SyntaxError(f"Line {self.current_token[2]}, column {self.current_token[3]}: Expected function name")
        func_name = self.current_token[1]
        self.advance()
        self.eat('left_paren')
        params = []
        if self.current_token and self.current_token[0] != 'right_paren':
            while True:
                if self.current_token[0] != 'identifier':
                    raise SyntaxError(f"Line {self.current_token[2]}, column {self.current_token[3]}: Expected parameter name")
                param_name = self.current_token[1]
                self.advance()
                param_type = None
                if self.current_token and self.current_token[0] == 'colon':
                    self.eat('colon')
                    param_type = self.current_token[1].lower()
                    self.advance()
                params.append((param_name, param_type))
                if self.current_token[0] == 'right_paren':
                    break
                self.eat('comma')
        self.eat('right_paren')
        self.eat('colon')
        self.eat('left_brace')
        body = self.block()
        return ('define', func_name, params, body)

    def declaration(self):
        self.advance()
        if self.current_token[0] != 'identifier':
            raise SyntaxError(f"Line {self.current_token[2]}, column {self.current_token[3]}: Expected variable name")
        var_name = self.current_token[1]
        self.advance()
        type_annot = None
        if self.current_token and self.current_token[0] == 'colon':
            self.eat('colon')
            type_annot = self.current_token[1].lower()
            self.advance()
        self.eat('assignment')
        expr = self.expression()
        return ('declare', 'let', var_name, type_annot, expr)

    def assignment_or_call_statement(self):
        expr = self.expression()
        if self.current_token and self.current_token[0] in ['assignment', 'aug_assignment']:
            if self.current_token[0] == 'assignment':
                self.eat('assignment')
                value = self.expression()
                return ('assign', expr, value)
            elif self.current_token[0] == 'aug_assignment':
                op = self.current_token[1][0]
                self.eat('aug_assignment')
                value = self.expression()
                return ('aug_assign', expr, op, value)
        return expr

    def print_statement(self):
        self.advance()
        expr = self.expression()
        return ('print', expr)

    def if_statement(self):
        self.advance()
        condition = self.expression()
        self.eat('colon')
        self.eat('left_brace')
        body = self.block()
        elif_blocks = []
        while self.current_token and self.current_token[0].lower() == 'elif':
            self.advance()
            elif_cond = self.expression()
            self.eat('colon')
            self.eat('left_brace')
            elif_body = self.block()
            elif_blocks.append((elif_cond, elif_body))
        else_block = []
        if self.current_token and self.current_token[0].lower() == 'else':
            self.advance()
            self.eat('colon')
            self.eat('left_brace')
            else_block = self.block()
        return ('if', condition, body, elif_blocks, else_block)

    def while_statement(self):
        self.advance()
        condition = self.expression()
        self.eat('colon')
        self.eat('left_brace')
        body = self.block()
        return ('while', condition, body)

    def block(self):
        statements = []
        while self.current_token and self.current_token[0] != 'right_brace':
            stmt = self.statement()
            statements.append(stmt)
            while self.current_token and self.current_token[0] == 'semicolon':
                self.eat('semicolon')
        self.eat('right_brace')
        return statements

    def expression(self):
        return self.comparison()

    def comparison(self):
        left = self.arithmetic()
        while self.current_token and self.current_token[0] == 'operator' and self.current_token[1] in ('==', '!=', '<', '>', '<=', '>='):
            op = self.current_token[1]
            self.advance()
            right = self.arithmetic()
            left = ('binary', op, left, right)
        return left

    def arithmetic(self):
        left = self.term()
        while self.current_token and self.current_token[0] == 'operator' and self.current_token[1] in ('+', '-'):
            op = self.current_token[1]
            self.advance()
            right = self.term()
            left = ('binary', op, left, right)
        return left

    def term(self):
        left = self.factor()
        while self.current_token and self.current_token[0] == 'operator' and self.current_token[1] in ('*', '/', '%'):
            op = self.current_token[1]
            self.advance()
            right = self.factor()
            left = ('binary', op, left, right)
        return left

    def factor(self):
        if self.current_token[0] == 'left_paren':
            self.eat('left_paren')
            expr = self.expression()
            self.eat('right_paren')
            return expr
        return self.primary()

    def primary(self):
        if self.current_token[0] == 'number':
            value = float(self.current_token[1]) if '.' in self.current_token[1] else int(self.current_token[1])
            self.advance()
            return ('literal', value)
        elif self.current_token[0] == 'string_literal':
            value = self.current_token[1][1:-1]
            self.advance()
            return ('literal', value)
        elif self.current_token[0].lower() in ('true', 'false'):
            value = True if self.current_token[0].lower() == 'true' else False
            self.advance()
            return ('literal', value)
        elif self.current_token[0].lower() == 'null':
            self.advance()
            return ('literal', None)
        elif self.current_token[0] == 'ocl':
            return self.ocl_statement()
        elif self.current_token[0] == 'identifier':
            name = self.current_token[1]
            self.advance()
            if self.current_token and self.current_token[0] == 'dot':
                self.eat('dot')
                if self.current_token[0] != 'identifier':
                    line = self.current_token[2]
                    column = self.current_token[3]
                    raise SyntaxError(f"Line {line}, column {column}: Expected identifier after dot")
                method_name = self.current_token[1]
                self.advance()
                if self.current_token and self.current_token[0] == 'left_paren':
                    self.eat('left_paren')
                    args = []
                    if self.current_token and self.current_token[0] != 'right_paren':
                        while True:
                            args.append(self.expression())
                            if self.current_token[0] == 'right_paren':
                                break
                            self.eat('comma')
                    self.eat('right_paren')
                    return ('call_method', ('identifier', name), method_name, args)
                return ('attribute', ('identifier', name), method_name)
            elif self.current_token and self.current_token[0] == 'left_bracket':
                self.eat('left_bracket')
                index = self.expression()
                self.eat('right_bracket')
                return ('index', ('identifier', name), index)
            elif self.current_token and self.current_token[0] == 'left_paren':
                self.eat('left_paren')
                args = []
                if self.current_token and self.current_token[0] != 'right_paren':
                    while True:
                        args.append(self.expression())
                        if self.current_token[0] == 'right_paren':
                            break
                        self.eat('comma')
                self.eat('right_paren')
                return ('call', name, args)
            return ('identifier', name)
        elif self.current_token[0] == 'left_paren':
            self.eat('left_paren')
            expr = self.expression()
            self.eat('right_paren')
            return expr
        else:
            line = self.current_token[2] if self.current_token else "unknown"
            column = self.current_token[3] if self.current_token else "unknown"
            raise SyntaxError(f"Line {line}, column {column}: Unexpected token in primary: {self.current_token[1] if self.current_token else 'EOF'}")