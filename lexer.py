#lexer.py
import re

class Lexer:
    def __init__(self):
        self.tokens = []
        self.token_patterns = [
            ('comment', r'#[^\n]*'),
            ('whitespace', r'[ \t]+'),
            ('newline', r'\n'),
            ('let', r'\blet\b'),
            ('print', r'\bprint\b'),
            ('if', r'\bif\b'),
            ('elif', r'\belif\b'),
            ('else', r'\belse\b'),
            ('while', r'\bwhile\b'),
            ('define', r'\bdefine\b'),
            ('return', r'\breturn\b'),
            ('class', r'\bclass\b'),
            ('break', r'\bbreak\b'),
            ('continue', r'\bcontinue\b'),
            ('true', r'\btrue\b'),
            ('false', r'\bfalse\b'),
            ('null', r'\bnull\b'),
            ('int', r'\bint\b'),
            ('float', r'\bfloat\b'),
            ('bool', r'\bbool\b'),
            ('string', r'\bstring\b'),
            ('ocl', r'\bocl\b'),
            ('string_literal', r'"[^"]*"'),
            ('number', r'\b\d+(\.\d+)?\b'),
            ('identifier', r'[a-zA-Z_][a-zA-Z0-9_.]*'),
            ('aug_assignment', r'\+=|-=|\*=|/='),
            ('operator', r'==|!=|<=|>=|[+\-*/%<>]'),
            ('assignment', r'='),
            ('semicolon', r';'),
            ('colon', r':'),
            ('left_paren', r'\('),
            ('right_paren', r'\)'),
            ('left_brace', r'\{'),
            ('right_brace', r'\}'),
            ('comma', r','),
            ('dot', r'\.'),
            ('left_bracket', r'\['),
            ('right_bracket', r'\]'),
        ]

    def tokenize(self, code):
        self.tokens = []
        pos = 0
        line = 1
        line_start = 0
        try:
            while pos < len(code):
                match = None
                for token_type, pattern in self.token_patterns:
                    flags = re.IGNORECASE if token_type in (
                        'let', 'print', 'if', 'elif', 'else', 'while', 'define', 'return',
                        'class', 'break', 'continue', 'true', 'false', 'null',
                        'int', 'float', 'bool', 'string', 'ocl'
                    ) else 0
                    regex = re.compile(pattern, flags)
                    match = regex.match(code, pos)
                    if match:
                        text = match.group(0)
                        if token_type == 'newline':
                            line += 1
                            line_start = pos + 1
                        elif token_type not in ('whitespace', 'comment'):
                            column = pos - line_start + 1
                            self.tokens.append((token_type, text, line, column))
                        pos = match.end()
                        break
                if not match:
                    column = pos - line_start + 1
                    raise SyntaxError(f"Invalid token at line {line}, column {column}: '{code[pos:pos+10]}'")
            return self.tokens
        except re.error as e:
            raise SyntaxError(f"Lexer regex error: {str(e)}")
        except Exception as e:
            raise SyntaxError(f"Lexer error: {str(e)}")