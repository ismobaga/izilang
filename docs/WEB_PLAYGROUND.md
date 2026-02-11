# IziLang Web Playground

## Overview

The IziLang playground provides an interactive environment for trying IziLang code in the browser.

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  Browser (UI)                       │
│                                                     │
│  ┌──────────────┐         ┌──────────────────┐    │
│  │    Editor    │────────▶│  JavaScript API  │    │
│  │  (textarea)  │         │  (playground.js) │    │
│  └──────────────┘         └──────────────────┘    │
│                                    │               │
│                                    ▼               │
│                          ┌──────────────────┐     │
│                          │  WASM Module     │     │
│                          │  (izilang.wasm)  │     │
│                          └──────────────────┘     │
│                                    │               │
│                                    ▼               │
│                          ┌──────────────────┐     │
│                          │  IziLang         │     │
│                          │  Interpreter     │     │
│                          └──────────────────┘     │
└─────────────────────────────────────────────────────┘
```

## Implementation Phases

### Phase 1: Static UI (Current) ✓
- Basic HTML/CSS interface
- Code editor (textarea)
- Example library
- URL sharing

### Phase 2: WASM Compilation
- Compile IziLang C++ code to WebAssembly
- Create JavaScript bindings
- Handle standard output capture
- Error handling and reporting

### Phase 3: Enhanced Editor
- Integrate Monaco Editor
- IziLang syntax highlighting
- Code completion
- Error markers

### Phase 4: Advanced Features
- Multi-file support
- File system API
- Import/export functionality
- Debugging tools

## WebAssembly API Design

### C++ Side

```cpp
// src/wasm-bindings.cpp
#include <emscripten/bind.h>
#include "interp/interpreter.h"

using namespace emscripten;

struct ExecutionResult {
    std::string output;
    std::string error;
    bool success;
};

ExecutionResult runCode(const std::string& code) {
    ExecutionResult result;
    
    try {
        // Redirect stdout to capture output
        std::ostringstream outputStream;
        auto oldBuf = std::cout.rdbuf(outputStream.rdbuf());
        
        // Run IziLang code
        izi::Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        
        izi::Parser parser(tokens);
        auto statements = parser.parse();
        
        izi::Interpreter interpreter;
        interpreter.interpret(statements);
        
        // Restore stdout
        std::cout.rdbuf(oldBuf);
        
        result.output = outputStream.str();
        result.success = true;
        
    } catch (const std::exception& e) {
        result.error = e.what();
        result.success = false;
    }
    
    return result;
}

EMSCRIPTEN_BINDINGS(izilang) {
    value_object<ExecutionResult>("ExecutionResult")
        .field("output", &ExecutionResult::output)
        .field("error", &ExecutionResult::error)
        .field("success", &ExecutionResult::success);
    
    function("runCode", &runCode);
}
```

### JavaScript Side

```javascript
// playground.js
async function runCode(code) {
    try {
        const result = Module.runCode(code);
        
        if (result.success) {
            output.innerHTML = escapeHtml(result.output);
        } else {
            output.innerHTML = '<span class="error">' + 
                escapeHtml(result.error) + '</span>';
        }
    } catch (error) {
        output.innerHTML = '<span class="error">Runtime error: ' + 
            escapeHtml(error.message) + '</span>';
    }
}
```

## Build Configuration

### Emscripten Flags

```bash
emcc src/**/*.cpp \
    -I src \
    -o playground/public/izilang.js \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='IziLangModule' \
    -s EXPORTED_FUNCTIONS='["_runCode"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s NO_EXIT_RUNTIME=1 \
    -s ENVIRONMENT='web' \
    -O3 \
    --bind
```

### Key Flags Explained

- `WASM=1`: Generate WebAssembly output
- `MODULARIZE=1`: Create a module that can be imported
- `ALLOW_MEMORY_GROWTH=1`: Allow dynamic memory allocation
- `NO_EXIT_RUNTIME=1`: Keep runtime alive after main()
- `--bind`: Enable embind for C++ bindings
- `-O3`: Maximum optimization

## Deployment Configuration

### GitHub Pages

Create `.github/workflows/deploy-playground.yml`:

```yaml
name: Deploy Playground

on:
  push:
    branches: [main]
    paths:
      - 'playground/**'

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Install Emscripten
        run: |
          git clone https://github.com/emscripten-core/emsdk.git
          cd emsdk
          ./emsdk install latest
          ./emsdk activate latest
      
      - name: Build WASM
        run: |
          source emsdk/emsdk_env.sh
          # Build commands here
      
      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./playground/public
```

## Testing

### Manual Testing Checklist

- [ ] Code editor loads correctly
- [ ] Examples can be selected and loaded
- [ ] Run button executes code (or shows appropriate message)
- [ ] Share button creates shareable URL
- [ ] Code can be loaded from URL
- [ ] Keyboard shortcut (Ctrl+Enter) works
- [ ] Tab key inserts spaces in editor
- [ ] Output panel displays results correctly
- [ ] Error messages are formatted properly

### Automated Testing

```javascript
// test-playground.js
describe('IziLang Playground', () => {
    it('should load example code', () => {
        // Test example selection
    });
    
    it('should execute code via WASM', () => {
        // Test code execution
    });
    
    it('should handle errors gracefully', () => {
        // Test error handling
    });
});
```

## Performance Considerations

1. **WASM Size**: Optimize compilation to reduce WASM file size
2. **Loading Time**: Use lazy loading for WASM module
3. **Execution Time**: Set timeouts to prevent infinite loops
4. **Memory**: Monitor memory usage and set limits

## Security Considerations

1. **Sandboxing**: WASM runs in browser sandbox
2. **Input Validation**: Sanitize user input
3. **Output Escaping**: Escape HTML in output
4. **Resource Limits**: Limit execution time and memory

## Future Roadmap

- [ ] WASM compilation and integration
- [ ] Monaco Editor integration
- [ ] Syntax highlighting
- [ ] Error markers in editor
- [ ] Code completion
- [ ] Multi-file support
- [ ] Debugging tools
- [ ] Mobile optimization
- [ ] Dark/light theme toggle
- [ ] Save/load from local storage
- [ ] GitHub Gist integration
