# scheme

Интерпретатор Scheme на C++: токенизация → парсинг выражений → вычисление.

## Features

- tokenizer для скобок, чисел, символов и специальных токенов
- Parser S-expression (AST/объекты)
- Evaluator с окружением (scope)

### Supported forms (пример)
- `quote`
- `if`
- `define`
- `lambda`
- `set!`
- `etc.`

### Builtins (пример)
- арифметика: `+ - * /`
- списки: `cons car cdr list`
- предикаты: `null? pair? number? symbol? boolean?`
- сравнения: `< > <= >= =`

## Build
Сборка:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j```

