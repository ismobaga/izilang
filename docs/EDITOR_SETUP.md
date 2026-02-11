# Editor Setup for IziLang

This guide will help you set up your editor for IziLang development with syntax highlighting, IDE support, and more.

## Visual Studio Code

### Installation

1. **Install the IziLang Extension** *(Coming Soon)*

   The official IziLang extension will be available on the VS Code marketplace. For now, you can build and install it manually.

### Manual Installation

#### Prerequisites
- Node.js 14+ and npm
- VS Code 1.60+

#### Build and Install

```bash
# Navigate to the VS Code extension directory
cd tools/vscode-extension

# Install dependencies
npm install

# Compile the extension
npm run compile

# Package the extension
npm run package

# Install the .vsix file
code --install-extension izilang-*.vsix
```

### Features

The IziLang VS Code extension provides:

- **Syntax Highlighting** - Color-coded syntax for IziLang files (.iz)
- **LSP Support** - Language server integration (when available)
- **Code Snippets** - Quick templates for common patterns
- **File Icons** - Custom icons for .iz files

### Configuration

Add these settings to your VS Code `settings.json`:

```json
{
  // Associate .iz files with IziLang
  "files.associations": {
    "*.iz": "izilang"
  },
  
  // IziLang-specific settings
  "izilang.trace.server": "off",
  "izilang.lsp.enabled": true,
  "izilang.lsp.path": "/path/to/izilang-lsp"
}
```

### Keyboard Shortcuts

Add these to your `keybindings.json`:

```json
[
  {
    "key": "ctrl+shift+r",
    "command": "izilang.run",
    "when": "editorLangId == izilang"
  },
  {
    "key": "ctrl+shift+c",
    "command": "izilang.check",
    "when": "editorLangId == izilang"
  }
]
```

### Recommended Extensions

For the best IziLang development experience:

- **Error Lens** - Inline error messages
- **Better Comments** - Enhanced comment styling
- **indent-rainbow** - Colored indentation levels

Note: VS Code now has built-in bracket pair colorization (since v1.60). Enable it in settings:
```json
{
  "editor.bracketPairColorization.enabled": true
}
```

---

## Vim/Neovim

### Syntax Highlighting

Create a Vim syntax file at `~/.vim/syntax/izilang.vim`:

```vim
" Vim syntax file for IziLang
" Language: IziLang
" Maintainer: IziLang Team

if exists("b:current_syntax")
  finish
endif

" Keywords
syn keyword iziKeyword fn var if else while for return break continue
syn keyword iziKeyword class import from match case try catch finally throw

" Built-in functions
syn keyword iziBuiltin print len push pop shift unshift type

" Constants
syn keyword iziConstant true false nil

" Operators
syn match iziOperator "\v\+|-|\*|/|%|\=|\=\=|\!\=|\<|\>|\<\=|\>\="
syn match iziOperator "\v\&\&|\|\||\!"

" Numbers
syn match iziNumber "\v<\d+>"
syn match iziNumber "\v<\d+\.\d+>"

" Strings
syn region iziString start='"' end='"' contains=iziInterpolation
syn region iziInterpolation start='\\{' end='}' contained

" Comments
syn match iziComment "//.*$"
syn region iziComment start="/\*" end="\*/"

" Highlighting
hi def link iziKeyword Keyword
hi def link iziBuiltin Function
hi def link iziConstant Constant
hi def link iziOperator Operator
hi def link iziNumber Number
hi def link iziString String
hi def link iziComment Comment
hi def link iziInterpolation Special

let b:current_syntax = "izilang"
```

Add file type detection to `~/.vim/ftdetect/izilang.vim`:

```vim
au BufRead,BufNewFile *.iz set filetype=izilang
```

### LSP Support (Neovim)

Configure the IziLang LSP server in your `init.lua`:

```lua
-- Using nvim-lspconfig
require'lspconfig'.izilang.setup{
  cmd = {"/path/to/izilang-lsp", "--stdio"},
  filetypes = {"izilang"},
  root_dir = require'lspconfig'.util.root_pattern(".git", "izi.toml"),
  settings = {}
}
```

### Running IziLang from Vim

Add these mappings to your `.vimrc` or `init.vim`:

```vim
" Run current file
nnoremap <leader>r :!izi run %<CR>

" Check current file
nnoremap <leader>c :!izi check %<CR>

" Build current file
nnoremap <leader>b :!izi build %<CR>
```

---

## Emacs

### Syntax Highlighting

Create an Emacs mode file at `~/.emacs.d/izilang-mode.el`:

```elisp
;;; izilang-mode.el --- Major mode for IziLang

(defvar izilang-mode-syntax-table
  (let ((table (make-syntax-table)))
    ;; Comments
    (modify-syntax-entry ?/ ". 124b" table)
    (modify-syntax-entry ?* ". 23" table)
    (modify-syntax-entry ?\n "> b" table)
    ;; Strings
    (modify-syntax-entry ?\" "\"" table)
    table))

(defvar izilang-keywords
  '("fn" "var" "if" "else" "while" "for" "return" 
    "break" "continue" "class" "import" "from"
    "match" "case" "try" "catch" "finally" "throw"))

(defvar izilang-font-lock-keywords
  `((,(regexp-opt izilang-keywords 'words) . font-lock-keyword-face)
    ("\\<\\(true\\|false\\|nil\\)\\>" . font-lock-constant-face)
    ("\\<[0-9]+\\(\\.[0-9]+\\)?\\>" . font-lock-constant-face)))

(define-derived-mode izilang-mode prog-mode "IziLang"
  "Major mode for editing IziLang code."
  :syntax-table izilang-mode-syntax-table
  (setq-local comment-start "// ")
  (setq-local comment-end "")
  (setq-local font-lock-defaults '(izilang-font-lock-keywords)))

(add-to-list 'auto-mode-alist '("\\.iz\\'" . izilang-mode))

(provide 'izilang-mode)
```

Add to your `.emacs` or `init.el`:

```elisp
(add-to-list 'load-path "~/.emacs.d")
(require 'izilang-mode)
```

### LSP Support

Using `lsp-mode`:

```elisp
(use-package lsp-mode
  :hook (izilang-mode . lsp)
  :config
  (lsp-register-client
   (make-lsp-client
    :new-connection (lsp-stdio-connection '("/path/to/izilang-lsp" "--stdio"))
    :major-modes '(izilang-mode)
    :server-id 'izilang-lsp)))
```

---

## Sublime Text

### Syntax Highlighting

Create a syntax file at `~/.config/sublime-text/Packages/User/IziLang.sublime-syntax`:

```yaml
%YAML 1.2
---
name: IziLang
file_extensions: [iz]
scope: source.izilang

contexts:
  main:
    - include: comments
    - include: strings
    - include: keywords
    - include: numbers

  keywords:
    - match: \b(fn|var|if|else|while|for|return|break|continue|class|import|match|case|try|catch|finally|throw)\b
      scope: keyword.control.izilang
    - match: \b(true|false|nil)\b
      scope: constant.language.izilang

  comments:
    - match: //
      scope: punctuation.definition.comment.izilang
      push:
        - meta_scope: comment.line.izilang
        - match: $
          pop: true

  strings:
    - match: '"'
      scope: punctuation.definition.string.begin.izilang
      push:
        - meta_scope: string.quoted.double.izilang
        - match: '"'
          scope: punctuation.definition.string.end.izilang
          pop: true

  numbers:
    - match: \b\d+(\.\d+)?\b
      scope: constant.numeric.izilang
```

---

## Language Server Protocol (LSP)

### Building the LSP Server

```bash
cd tools/lsp
premake5 gmake2
make config=release
```

The LSP server binary will be at `tools/lsp/bin/Release/izilang-lsp`.

### Features (MVP)

The IziLang LSP server provides:

- **Diagnostics** - Real-time syntax and semantic errors
- **Go to Definition** - Jump to variable/function definitions
- **Hover Information** - Show symbol information on hover
- **Document Symbols** - Code outline/structure
- **Autocompletion** - Context-aware code completion

### Configuration

Most LSP clients will auto-detect the server if it's in your PATH:

```bash
# Add LSP server to PATH
export PATH="$PATH:/path/to/izilang/tools/lsp/bin/Release"
```

Or configure the path explicitly in your editor.

---

## Other Editors

### IntelliJ IDEA / CLion

1. Install the **TextMate Bundle** plugin
2. Import the IziLang syntax file
3. Configure file associations for `.iz` files

### Atom

1. Create a package in `~/.atom/packages/language-izilang`
2. Add grammar and syntax definitions
3. Use `atom-ide-ui` for LSP support

### Kate / KWrite

1. Create a syntax file in `~/.local/share/katepart5/syntax/`
2. Use the XML-based Kate syntax format

---

## Tips

### File Associations

Make sure your editor associates `.iz` files with IziLang:

- **Extension**: `.iz`
- **MIME Type**: `text/x-izilang` (proposed)
- **Language ID**: `izilang`

### Formatting

Use `izi fmt` (when available) to automatically format code:

```bash
# Format and print to stdout
izi fmt script.iz

# Format and write to file
izi fmt --write script.iz
```

### Linting

Use `izi check` or `izi build` for quick lint-like checking:

```bash
izi check *.iz
```

---

## Troubleshooting

### LSP Server Not Working

1. Check if the LSP server is built:
   ```bash
   which izilang-lsp
   ```

2. Test the server manually:
   ```bash
   izilang-lsp --version
   ```

3. Check editor logs for LSP errors

### Syntax Highlighting Not Working

1. Verify file extension is `.iz`
2. Reload/restart your editor
3. Check syntax file installation path
4. Clear editor cache

### Editor Configuration Issues

1. Check configuration file syntax
2. Verify paths are absolute and correct
3. Restart the editor after configuration changes

---

## Contributing

Help improve IziLang editor support:

- Report bugs or missing features
- Submit syntax highlighting improvements
- Create editor plugins for new editors
- Improve LSP server functionality

---

## See Also

- [Getting Started Guide](GETTING_STARTED.md)
- [CLI Reference](CLI_REFERENCE.md)
- [LSP Server Documentation](LSP_SERVER.md)

---

**IziLang** - A modern, expressive programming language
