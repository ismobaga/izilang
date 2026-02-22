// IziLang Playground – advanced UI
// Monaco Editor integration + IziLang syntax highlighting

// ── Example code snippets ────────────────────────────────────────────────────
const examples = {
    hello: `// Hello World
print("Hello, IziLang!");
print("Welcome to the playground!");`,

    variables: `// Variables and types
var name = "IziLang";
var version = 0.3;
var isAwesome = true;
var nothing = nil;

print("Language: " + name);
print("Version:  " + version);
print("Awesome?  " + isAwesome);
print("Nil:      " + nothing);`,

    functions: `// Functions
fn greet(name) {
    return "Hello, " + name + "!";
}

fn fibonacci(n) {
    if (n <= 1) { return n; }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

fn factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}

print(greet("World"));
print("fib(10)  = " + fibonacci(10));
print("fact(10) = " + factorial(10));`,

    loops: `// Loops
var i = 0;
while (i < 5) {
    print("while → " + i);
    i = i + 1;
}

var sum = 0;
var j = 1;
while (j <= 100) {
    sum = sum + j;
    j = j + 1;
}
print("Sum 1-100 = " + sum);`,

    arrays: `// Arrays and Maps
var nums = [1, 2, 3, 4, 5];
print("Array:    " + nums);
print("Length:   " + len(nums));

nums[2] = 99;
print("Modified: " + nums);

var person = {"name": "Alice", "age": 30, "active": true};
print("Person:   " + person);
print("Name:     " + person["name"]);
print("Age:      " + person["age"]);`,

    oop: `// Classes and OOP
class Animal {
    var species = "Unknown";

    fn init(name) {
        this.name = name;
    }

    fn speak() {
        print(this.name + " makes a sound");
    }

    fn describe() {
        print(this.name + " is a " + this.species);
    }
}

class Dog extends Animal {
    fn init(name) {
        super.init(name);
        this.species = "Canine";
    }

    fn speak() {
        print(this.name + " barks!");
    }
}

var a = Animal("Generic");
a.speak();
a.describe();

var d = Dog("Buddy");
d.speak();
d.describe();`,

    trycatch: `// Try / Catch / Finally
fn safeDivide(a, b) {
    if (b == 0) {
        throw "Division by zero!";
    }
    return a / b;
}

try {
    print("10 / 2 = " + safeDivide(10, 2));
    print("10 / 0 = " + safeDivide(10, 0));
} catch (e) {
    print("Caught error: " + e);
} finally {
    print("Done.");
}`,

    patternmatch: `// Pattern Matching
fn describe(val) {
    match val {
        0    -> print("zero"),
        1    -> print("one"),
        true -> print("boolean true"),
        _    -> print("something else: " + val)
    }
}

describe(0);
describe(1);
describe(true);
describe(42);
describe("hello");`,

    closures: `// Closures and higher-order functions
fn makeCounter(start) {
    var count = start;
    fn increment() {
        count = count + 1;
        return count;
    }
    return increment;
}

var c1 = makeCounter(0);
var c2 = makeCounter(10);

print(c1());  // 1
print(c1());  // 2
print(c2());  // 11
print(c1());  // 3
print(c2());  // 12`,

    async: `// Async / Await
async fn fetchData(url) {
    // Simulated async operation
    var result = await someAsyncCall(url);
    return result;
}

async fn main() {
    print("Starting async work...");
    var data = await fetchData("https://api.example.com/data");
    print("Got: " + data);
}

main();`,

    modules: `// Modules (import / export)
import math from "std/math";
import { readFile } from "std/fs";

var pi = math.PI;
print("π ≈ " + pi);

var result = math.sqrt(144);
print("√144 = " + result);`,

    fibonacci: `// Fibonacci – iterative vs recursive
fn fibRecursive(n) {
    if (n <= 1) { return n; }
    return fibRecursive(n - 1) + fibRecursive(n - 2);
}

fn fibIterative(n) {
    if (n <= 1) { return n; }
    var a = 0;
    var b = 1;
    var i = 2;
    while (i <= n) {
        var tmp = a + b;
        a = b;
        b = tmp;
        i = i + 1;
    }
    return b;
}

var k = 15;
print("fib(" + k + ") recursive = " + fibRecursive(k));
print("fib(" + k + ") iterative = " + fibIterative(k));`
};

// ── IziLang language definition for Monaco ──────────────────────────────────
const IZILANG_KEYWORDS = [
    'fn', 'var', 'if', 'else', 'while', 'for', 'return', 'break', 'continue',
    'class', 'extends', 'super', 'this', 'import', 'export', 'from', 'as',
    'try', 'catch', 'finally', 'throw', 'match', 'async', 'await', 'macro',
    'default', 'print'
];

const IZILANG_BUILTINS = [
    'len', 'str', 'num', 'type', 'push', 'pop', 'keys', 'values',
    'toString', 'toNumber', 'input', 'assert', 'exit', 'sleep'
];

const IZILANG_LITERALS = ['true', 'false', 'nil'];

function registerIziLang(monaco) {
    monaco.languages.register({ id: 'izilang', extensions: ['.iz', '.izi'], aliases: ['IziLang', 'izi'] });

    monaco.languages.setMonarchTokensProvider('izilang', {
        keywords: IZILANG_KEYWORDS,
        builtins: IZILANG_BUILTINS,
        literals: IZILANG_LITERALS,

        tokenizer: {
            root: [
                // Line comment
                [/\/\/.*$/, 'comment'],
                // Block comment
                [/\/\*/, 'comment', '@block_comment'],
                // String
                [/"([^"\\]|\\.)*$/, 'string.invalid'],
                [/"/, 'string', '@string_dq'],
                // Number
                [/\d+(\.\d+)?/, 'number'],
                // Identifiers and keywords
                [/[a-zA-Z_]\w*/, {
                    cases: {
                        '@keywords': 'keyword',
                        '@builtins': 'type.identifier',
                        '@literals': 'constant',
                        '@default': 'identifier'
                    }
                }],
                // Operators
                [/[=!<>]=?|&&|\|\||[-+*/%&|^~]/, 'operator'],
                [/->/, 'operator'],
                // Delimiters
                [/[{}()[\]]/, 'delimiter'],
                [/[,;.:?]/, 'delimiter'],
                // Whitespace
                [/\s+/, 'white'],
            ],

            block_comment: [
                [/[^/*]+/, 'comment'],
                [/\*\//, 'comment', '@pop'],
                [/[/*]/, 'comment']
            ],

            string_dq: [
                [/[^\\"]+/, 'string'],
                [/\\./, 'string.escape'],
                [/"/, 'string', '@pop']
            ]
        }
    });

    monaco.languages.setLanguageConfiguration('izilang', {
        comments: { lineComment: '//', blockComment: ['/*', '*/'] },
        brackets: [
            ['{', '}'],
            ['[', ']'],
            ['(', ')']
        ],
        autoClosingPairs: [
            { open: '{', close: '}' },
            { open: '[', close: ']' },
            { open: '(', close: ')' },
            { open: '"', close: '"' }
        ],
        surroundingPairs: [
            { open: '{', close: '}' },
            { open: '[', close: ']' },
            { open: '(', close: ')' },
            { open: '"', close: '"' }
        ],
        indentationRules: {
            increaseIndentPattern: /^.*\{[^}]*$/,
            decreaseIndentPattern: /^\s*\}/
        },
        folding: {
            markers: { start: /^\s*\/\/#region\b/, end: /^\s*\/\/#endregion\b/ }
        },
        onEnterRules: [
            { beforeText: /^\s*\/\*\*(?!\/)/, afterText: /^\s*\*\/$/, action: { indentAction: 0, appendText: ' * ' } }
        ]
    });

    // Hover / completions
    monaco.languages.registerCompletionItemProvider('izilang', {
        provideCompletionItems(model, position) {
            const word = model.getWordUntilPosition(position);
            const range = {
                startLineNumber: position.lineNumber,
                endLineNumber: position.lineNumber,
                startColumn: word.startColumn,
                endColumn: word.endColumn
            };

            const kwSuggestions = IZILANG_KEYWORDS.map(kw => ({
                label: kw,
                kind: monaco.languages.CompletionItemKind.Keyword,
                insertText: kw,
                range
            }));

            const builtinSuggestions = IZILANG_BUILTINS.map(b => ({
                label: b,
                kind: monaco.languages.CompletionItemKind.Function,
                insertText: b + '($0)',
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                range
            }));

            const snippets = [
                { label: 'fn', insertText: 'fn ${1:name}(${2:params}) {\n\t$0\n}', detail: 'Function declaration' },
                { label: 'if', insertText: 'if (${1:condition}) {\n\t$0\n}', detail: 'If statement' },
                { label: 'ifelse', insertText: 'if (${1:condition}) {\n\t$2\n} else {\n\t$0\n}', detail: 'If-else statement' },
                { label: 'while', insertText: 'while (${1:condition}) {\n\t$0\n}', detail: 'While loop' },
                { label: 'class', insertText: 'class ${1:Name} {\n\tfn init(${2:params}) {\n\t\t$0\n\t}\n}', detail: 'Class declaration' },
                { label: 'try', insertText: 'try {\n\t$1\n} catch (${2:e}) {\n\t$0\n}', detail: 'Try-catch' }
            ].map(s => ({
                label: s.label,
                kind: monaco.languages.CompletionItemKind.Snippet,
                insertText: s.insertText,
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                detail: s.detail,
                range
            }));

            return { suggestions: [...kwSuggestions, ...builtinSuggestions, ...snippets] };
        }
    });

    // Hover information
    const hoverDocs = {
        print: 'print(value, ...) — Output values to the console',
        len: 'len(collection) — Return the length of an array or string',
        str: 'str(value) — Convert a value to its string representation',
        num: 'num(value) — Parse a string as a number',
        type: 'type(value) — Return the type of a value as a string',
        push: 'push(array, value) — Append a value to an array',
        pop: 'pop(array) — Remove and return the last element of an array',
        keys: 'keys(map) — Return an array of map keys',
        values: 'values(map) — Return an array of map values',
        assert: 'assert(condition, message?) — Throw if condition is falsy',
        exit: 'exit(code?) — Terminate the program with an optional exit code',
        sleep: 'sleep(ms) — Pause execution for the given number of milliseconds'
    };

    monaco.languages.registerHoverProvider('izilang', {
        provideHover(model, position) {
            const word = model.getWordAtPosition(position);
            if (!word) return null;
            const doc = hoverDocs[word.word];
            if (!doc) return null;
            return {
                contents: [
                    { value: '**`' + word.word + '`** — IziLang built-in' },
                    { value: doc }
                ]
            };
        }
    });
}

// ── DOM references ───────────────────────────────────────────────────────────
const outputEl      = document.getElementById('output');
const runBtn        = document.getElementById('run-btn');
const shareBtn      = document.getElementById('share-btn');
const clearBtn      = document.getElementById('clear-btn');
const exampleSelect = document.getElementById('example-select');
const settingsBtn   = document.getElementById('settings-btn');
const settingsPanel = document.getElementById('settings-panel');
const overlay       = document.getElementById('overlay');
const fontSlider    = document.getElementById('font-size-slider');
const fontLabel     = document.getElementById('font-size-label');
const wordWrapToggle = document.getElementById('word-wrap-toggle');
const minimapToggle  = document.getElementById('minimap-toggle');
const statusCursor   = document.getElementById('status-cursor');
const statusExecTime = document.getElementById('status-exec-time');
const statusMsg      = document.getElementById('status-msg');
const resizeHandle   = document.getElementById('resize-handle');
const editorPanel    = document.getElementById('editor-panel');
const outputPanel    = document.getElementById('output-panel');
const workspace      = document.getElementById('workspace');

// ── Monaco Editor initialization ─────────────────────────────────────────────
let monacoEditor = null;

// ── Fallback textarea (used when Monaco CDN is unavailable) ───────────────────
function initFallbackEditor(initialCode) {
    const container = document.getElementById('editor-container');
    container.style.display = 'flex';
    container.style.flexDirection = 'column';

    const ta = document.createElement('textarea');
    ta.id = 'editor-fallback';
    ta.spellcheck = false;
    ta.value = initialCode;
    ta.style.cssText = [
        'flex:1', 'font-family:"Consolas","Monaco","Courier New",monospace',
        'font-size:14px', 'padding:10px 14px', 'background:#1e1e1e',
        'color:#d4d4d4', 'border:none', 'resize:none', 'outline:none',
        'overflow:auto', 'white-space:pre', 'tab-size:4', 'line-height:1.55'
    ].join(';');

    container.appendChild(ta);

    // Tab key → 4 spaces
    ta.addEventListener('keydown', (e) => {
        if (e.key === 'Tab') {
            e.preventDefault();
            const s = ta.selectionStart, end = ta.selectionEnd;
            ta.value = ta.value.substring(0, s) + '    ' + ta.value.substring(end);
            ta.selectionStart = ta.selectionEnd = s + 4;
        }
        if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
            e.preventDefault();
            runBtn.click();
        }
    });

    ta.addEventListener('keyup', () => {
        const lines = ta.value.substring(0, ta.selectionStart).split('\n');
        statusCursor.textContent = `Ln ${lines.length}, Col ${lines[lines.length - 1].length + 1}`;
    });

    // Override getValue/setValue shims used by run/share/example handlers
    monacoEditor = {
        getValue: () => ta.value,
        setValue: (v) => { ta.value = v; ta.focus(); },
        updateOptions: (opts) => {
            if (opts.fontSize) ta.style.fontSize = opts.fontSize + 'px';
        },
        addCommand: () => {},
        onDidChangeCursorPosition: () => {},
        focus: () => ta.focus()
    };

    statusMsg.textContent = 'Editor ready (basic mode)';
}

const INITIAL_CODE = getCodeFromUrl() || `// Welcome to IziLang Playground!
// Write your IziLang code here and press ▶ Run (or Ctrl+Enter).

fn greet(name) {
    return "Hello, " + name + "!";
}

print(greet("IziLang"));
print("Version: 0.3.0");
`;

// If the Monaco loader script was blocked/unavailable, `require` won't exist.
// In that case, activate the fallback editor immediately.
if (typeof require === 'undefined') {
    initFallbackEditor(INITIAL_CODE);
} else {
    require.config({ paths: { vs: 'https://unpkg.com/monaco-editor@0.44.0/min/vs' } });

    // Timeout: if Monaco hasn't loaded within 8 s, activate the fallback editor
    const monacoLoadTimeout = setTimeout(() => {
        if (!monacoEditor) { initFallbackEditor(INITIAL_CODE); }
    }, 8000);

    require(['vs/editor/editor.main'], function (monaco) {
        clearTimeout(monacoLoadTimeout);
        registerIziLang(monaco);

    monacoEditor = monaco.editor.create(document.getElementById('editor-container'), {
        value: INITIAL_CODE,
        language: 'izilang',
        theme: 'vs-dark',
        fontSize: 14,
        lineHeight: 22,
        fontFamily: "'Cascadia Code', 'Consolas', 'Monaco', 'Courier New', monospace",
        fontLigatures: true,
        minimap: { enabled: false },
        wordWrap: 'on',
        scrollBeyondLastLine: false,
        automaticLayout: true,
        tabSize: 4,
        insertSpaces: true,
        renderLineHighlight: 'all',
        cursorBlinking: 'smooth',
        cursorSmoothCaretAnimation: 'on',
        smoothScrolling: true,
        bracketPairColorization: { enabled: true },
        guides: { bracketPairs: true },
        padding: { top: 10, bottom: 10 },
        suggest: { showKeywords: true, showSnippets: true }
    });

    // Cursor position in status bar
    monacoEditor.onDidChangeCursorPosition(e => {
        statusCursor.textContent = `Ln ${e.position.lineNumber}, Col ${e.position.column}`;
    });

    // Ctrl+Enter keyboard shortcut
    monacoEditor.addCommand(
        monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter,
        () => runBtn.click()
    );

    statusMsg.textContent = 'Editor ready';
    });
} // end else (Monaco available)

// ── Settings listeners (work for both Monaco and fallback) ───────────────────
fontSlider.addEventListener('input', () => {
    const sz = parseInt(fontSlider.value, 10);
    fontLabel.textContent = sz + 'px';
    if (monacoEditor) monacoEditor.updateOptions({ fontSize: sz });
});

wordWrapToggle.addEventListener('change', () => {
    if (monacoEditor) monacoEditor.updateOptions({ wordWrap: wordWrapToggle.checked ? 'on' : 'off' });
});

minimapToggle.addEventListener('change', () => {
    if (monacoEditor) monacoEditor.updateOptions({ minimap: { enabled: minimapToggle.checked } });
});

// ── Example selector (works for both Monaco and fallback) ───────────────────
exampleSelect.addEventListener('change', (e) => {
    const key = e.target.value;
    if (key && examples[key] && monacoEditor) {
        monacoEditor.setValue(examples[key]);
        monacoEditor.focus();
    }
});

// ── Run button ───────────────────────────────────────────────────────────────
runBtn.addEventListener('click', () => {
    const code = monacoEditor ? monacoEditor.getValue() : '';
    runCode(code);
});

// ── Share button ─────────────────────────────────────────────────────────────
shareBtn.addEventListener('click', () => {
    const code = monacoEditor ? monacoEditor.getValue() : '';
    const encoded = btoa(encodeURIComponent(code));
    const url = window.location.origin + window.location.pathname + '?code=' + encoded;

    navigator.clipboard.writeText(url).then(() => {
        shareBtn.textContent = '✓ Copied!';
        setTimeout(() => { shareBtn.textContent = '🔗 Share'; }, 2000);
    }).catch(() => {
        alert('Share URL:\n' + url);
    });
});

// ── Clear button ─────────────────────────────────────────────────────────────
clearBtn.addEventListener('click', () => {
    outputEl.innerHTML = '<span class="output-line muted">Output cleared.</span>';
    statusExecTime.textContent = '';
});

// ── Settings panel ────────────────────────────────────────────────────────────
settingsBtn.addEventListener('click', (e) => {
    e.stopPropagation();
    const isOpen = settingsPanel.classList.toggle('open');
    overlay.classList.toggle('visible', isOpen);
});

overlay.addEventListener('click', () => {
    settingsPanel.classList.remove('open');
    overlay.classList.remove('visible');
});

// ── Resizable split panel ────────────────────────────────────────────────────
(function initResize() {
    let dragging = false;
    let startX = 0;
    let startEditorWidth = 0;

    resizeHandle.addEventListener('mousedown', (e) => {
        dragging = true;
        startX = e.clientX;
        startEditorWidth = editorPanel.getBoundingClientRect().width;
        resizeHandle.classList.add('dragging');
        document.body.style.cursor = 'col-resize';
        document.body.style.userSelect = 'none';
    });

    document.addEventListener('mousemove', (e) => {
        if (!dragging) return;
        const delta = e.clientX - startX;
        const totalWidth = workspace.getBoundingClientRect().width;
        const handleWidth = resizeHandle.offsetWidth;
        const newEditorWidth = Math.min(
            Math.max(startEditorWidth + delta, 150),
            totalWidth - handleWidth - 150
        );
        editorPanel.style.flex = 'none';
        editorPanel.style.width = newEditorWidth + 'px';
        outputPanel.style.flex = '1';
    });

    document.addEventListener('mouseup', () => {
        if (!dragging) return;
        dragging = false;
        resizeHandle.classList.remove('dragging');
        document.body.style.cursor = '';
        document.body.style.userSelect = '';
    });
})();

// ── Code execution ───────────────────────────────────────────────────────────
async function runCode(code) {
    outputEl.innerHTML = '<span class="output-line info"><span class="spinner"></span>  Executing…</span>';
    runBtn.disabled = true;
    statusMsg.textContent = 'Running…';
    statusExecTime.textContent = '';

    const t0 = performance.now();

    try {
        if (typeof Module === 'undefined' || !Module.runCode) {
            // WASM module not loaded – show a friendly, informative message
            const elapsed = (performance.now() - t0).toFixed(1);
            statusExecTime.textContent = `${elapsed} ms`;

            outputEl.innerHTML =
                line('warn', '⚠  WebAssembly module not loaded') +
                line('muted', '') +
                line('info', 'To run IziLang code in the browser you need to:') +
                line('muted', '  1. Compile IziLang to WebAssembly using Emscripten') +
                line('muted', '  2. Copy izilang.js + izilang.wasm to playground/public/') +
                line('muted', '  3. Load the module in index.html (see README)') +
                line('muted', '') +
                line('success', '✓  This is a live UI demonstration of the playground.');
        } else {
            const result = Module.runCode(code);
            const elapsed = (performance.now() - t0).toFixed(1);
            statusExecTime.textContent = `${elapsed} ms`;

            if (result.error) {
                outputEl.innerHTML = line('error', '✗  ' + escapeHtml(result.error));
                statusMsg.textContent = 'Error';
            } else {
                const lines = escapeHtml(result.output).split('\n');
                outputEl.innerHTML = lines.map(l => line('success', l)).join('') ||
                    line('muted', '(no output)');
                statusMsg.textContent = `Done (${elapsed} ms)`;
            }
        }
    } catch (err) {
        const elapsed = (performance.now() - t0).toFixed(1);
        statusExecTime.textContent = `${elapsed} ms`;
        outputEl.innerHTML = line('error', '✗  Runtime error: ' + escapeHtml(err.message));
        statusMsg.textContent = 'Error';
    } finally {
        runBtn.disabled = false;
        runBtn.textContent = '▶ Run';
    }
}

function line(cls, text) {
    return `<span class="output-line ${cls}">${text}</span>\n`;
}

// ── Utility ───────────────────────────────────────────────────────────────────
function escapeHtml(text) {
    const d = document.createElement('div');
    d.textContent = text;
    return d.innerHTML;
}

function getCodeFromUrl() {
    const params = new URLSearchParams(window.location.search);
    const encoded = params.get('code');
    if (!encoded) return null;
    try { return decodeURIComponent(atob(encoded)); } catch { return null; }
}
