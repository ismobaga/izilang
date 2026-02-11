# IziLang Web Playground

An interactive web-based playground for writing and running IziLang code directly in the browser.

## Features

- **Live Code Editor**: Write IziLang code with syntax highlighting
- **Instant Execution**: Run code and see output immediately
- **Example Library**: Pre-built examples to get started quickly
- **Code Sharing**: Share your code via URL
- **Keyboard Shortcuts**: Ctrl/Cmd+Enter to run code

## Quick Start

### Running Locally

```bash
cd playground/public
python3 -m http.server 8000
# Visit http://localhost:8000
```

Or use any static file server:

```bash
npx serve public
```

## WebAssembly Compilation

To enable actual code execution in the browser, compile IziLang to WebAssembly:

### Prerequisites

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)

### Build Steps

```bash
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# Build IziLang for WebAssembly
cd /path/to/izilang
mkdir build-wasm
cd build-wasm

emcc ../src/**/*.cpp \
    -I../src \
    -o izilang.js \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_runCode"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='IziLangModule' \
    -O3

# Copy to playground
cp izilang.js izilang.wasm ../playground/public/
```

### Integration

Update `index.html` to load the WASM module:

```html
<script src="izilang.js"></script>
<script>
  IziLangModule().then(Module => {
    window.Module = Module;
    console.log('IziLang WASM loaded!');
  });
</script>
```

## Deployment

### GitHub Pages

1. Enable GitHub Pages in repository settings
2. Set source to `playground/public` directory or `gh-pages` branch
3. Access at `https://username.github.io/izilang/`

### Netlify

```bash
# Install Netlify CLI
npm install -g netlify-cli

# Deploy
cd playground
netlify deploy --dir=public --prod
```

### Vercel

```bash
# Install Vercel CLI
npm install -g vercel

# Deploy
cd playground/public
vercel
```

## Project Structure

```
playground/
├── public/
│   ├── index.html       # Main HTML page
│   ├── playground.js    # JavaScript logic
│   ├── izilang.js       # WASM module (generated)
│   └── izilang.wasm     # WASM binary (generated)
├── src/
│   └── wasm-bindings.cpp  # C++ bindings for WASM (future)
└── README.md
```

## Development

### Current Status

- ✅ UI and editor interface
- ✅ Example code library
- ✅ Code sharing via URL
- ⬜ WebAssembly compilation
- ⬜ Actual code execution
- ⬜ Error highlighting
- ⬜ Syntax highlighting in editor

### Future Enhancements

- **Monaco Editor**: Replace textarea with full-featured Monaco editor
- **Syntax Highlighting**: Implement proper IziLang syntax highlighting
- **Error Display**: Show errors inline in the editor
- **Autocomplete**: Add code completion suggestions
- **Multiple Files**: Support for multi-file projects
- **Debugging**: Add step-by-step debugging
- **Performance**: Add execution timing and memory usage

## Monaco Editor Integration

For a more powerful editor experience:

```html
<!-- In index.html -->
<div id="editor" style="height: 100%; width: 100%;"></div>

<script src="https://unpkg.com/monaco-editor@latest/min/vs/loader.js"></script>
<script>
  require.config({ paths: { vs: 'https://unpkg.com/monaco-editor@latest/min/vs' } });
  
  require(['vs/editor/editor.main'], function() {
    // Register IziLang language
    monaco.languages.register({ id: 'izilang' });
    
    // Create editor
    var editor = monaco.editor.create(document.getElementById('editor'), {
      value: '// Your code here',
      language: 'izilang',
      theme: 'vs-dark'
    });
  });
</script>
```

## Contributing

Contributions are welcome! Areas that need work:

1. WebAssembly compilation and bindings
2. Monaco editor integration
3. Better syntax highlighting
4. Error reporting and diagnostics
5. Mobile responsiveness

## License

MIT

## Resources

- [Emscripten Documentation](https://emscripten.org/docs/)
- [Monaco Editor](https://microsoft.github.io/monaco-editor/)
- [WebAssembly](https://webassembly.org/)
