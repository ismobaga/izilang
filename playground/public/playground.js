// IziLang Playground JavaScript
// This file handles the playground UI and execution

// Example code snippets
const examples = {
    hello: `// Hello World
print("Hello, IziLang!");
print("Welcome to the playground!");`,

    variables: `// Variables and types
var name = "IziLang";
var version = 0.1;
var isAwesome = true;

print("Language: " + name);
print("Version: " + version);
print("Is awesome? " + isAwesome);`,

    functions: `// Functions
fn greet(name) {
    return "Hello, " + name + "!";
}

fn fibonacci(n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

print(greet("World"));
print("Fibonacci(10) = " + fibonacci(10));`,

    loops: `// Loops
var i = 0;
while (i < 5) {
    print("Iteration: " + i);
    i = i + 1;
}

var sum = 0;
var j = 1;
while (j <= 10) {
    sum = sum + j;
    j = j + 1;
}
print("Sum of 1-10: " + sum);`,

    arrays: `// Arrays and Maps
var numbers = [1, 2, 3, 4, 5];
print("Array: " + numbers);
print("Length: " + len(numbers));

numbers[2] = 10;
print("Modified: " + numbers);

var person = {"name": "Alice", "age": 30};
print("Person: " + person);
print("Name: " + person["name"]);`
};

// Get DOM elements
const editor = document.getElementById('editor');
const output = document.getElementById('output');
const runBtn = document.getElementById('run-btn');
const shareBtn = document.getElementById('share-btn');
const exampleSelect = document.getElementById('example-select');

// Handle example selection
exampleSelect.addEventListener('change', (e) => {
    const example = e.target.value;
    if (example && examples[example]) {
        editor.value = examples[example];
    }
});

// Handle run button click
runBtn.addEventListener('click', async () => {
    const code = editor.value;
    runCode(code);
});

// Handle share button click
shareBtn.addEventListener('click', () => {
    const code = editor.value;
    const encoded = btoa(encodeURIComponent(code));
    const url = window.location.origin + window.location.pathname + '?code=' + encoded;
    
    navigator.clipboard.writeText(url).then(() => {
        shareBtn.textContent = '✓ Copied!';
        setTimeout(() => {
            shareBtn.textContent = '🔗 Share';
        }, 2000);
    }).catch(() => {
        alert('Unable to copy to clipboard. URL: ' + url);
    });
});

// Load code from URL if present
window.addEventListener('load', () => {
    const params = new URLSearchParams(window.location.search);
    const encoded = params.get('code');
    if (encoded) {
        try {
            const code = decodeURIComponent(atob(encoded));
            editor.value = code;
        } catch (e) {
            console.error('Failed to decode URL code:', e);
        }
    }
});

// Run IziLang code
async function runCode(code) {
    output.innerHTML = '<span class="loading">Executing...</span>';
    
    try {
        // TODO: In a real implementation, this would compile IziLang to WASM
        // and execute it. For now, we'll simulate execution.
        
        // Check if WebAssembly module is loaded
        if (typeof Module === 'undefined' || !Module.runCode) {
            output.innerHTML = '<span class="error">Error: WebAssembly module not loaded.</span>\n\n' +
                '<span>To run IziLang code in the browser, you need to:</span>\n' +
                '1. Compile IziLang to WebAssembly using Emscripten\n' +
                '2. Load the WASM module in this page\n' +
                '3. Call the interpreter from JavaScript\n\n' +
                '<span class="success">For now, this is a UI demonstration.</span>\n' +
                'See the README for instructions on building the WASM version.';
            return;
        }
        
        // Execute code through WASM module
        const result = Module.runCode(code);
        
        if (result.error) {
            output.innerHTML = '<span class="error">Error: ' + escapeHtml(result.error) + '</span>';
        } else {
            output.innerHTML = '<span class="success">' + escapeHtml(result.output) + '</span>';
        }
        
    } catch (error) {
        output.innerHTML = '<span class="error">Runtime Error: ' + escapeHtml(error.message) + '</span>';
    }
}

// Escape HTML to prevent XSS
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Handle keyboard shortcuts
editor.addEventListener('keydown', (e) => {
    // Ctrl/Cmd + Enter to run
    if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
        e.preventDefault();
        runBtn.click();
    }
    
    // Tab key for indentation
    if (e.key === 'Tab') {
        e.preventDefault();
        const start = editor.selectionStart;
        const end = editor.selectionEnd;
        const value = editor.value;
        
        editor.value = value.substring(0, start) + '    ' + value.substring(end);
        editor.selectionStart = editor.selectionEnd = start + 4;
    }
});
