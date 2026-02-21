# Tutorial: Build a CLI App with IziLang

In this tutorial you will build a fully working command-line **task manager** (`todo`) from scratch. By the end you will have:

- A persistent to-do list stored in a JSON file
- Commands: `add`, `list`, `done`, `remove`, `clear`
- Colour-coded console output
- A clean project structure with multiple source files

**Prerequisites**: IziLang installed and available as `izi` on your `PATH`. See [Getting Started](GETTING_STARTED.md) if you haven't set that up yet.

**Time**: ~30 minutes

---

## Table of Contents

1. [Project structure](#1-project-structure)
2. [Parsing command-line arguments](#2-parsing-command-line-arguments)
3. [Persisting data with JSON](#3-persisting-data-with-json)
4. [Implementing commands](#4-implementing-commands)
5. [Putting it all together](#5-putting-it-all-together)
6. [Running the app](#6-running-the-app)
7. [Exercises](#7-exercises)

---

## 1. Project structure

Create the following layout:

```
todo/
├── todo.iz          # entry point
├── args.iz          # argument parsing
├── store.iz         # JSON persistence
└── commands.iz      # command implementations
```

```bash
mkdir todo && cd todo
touch todo.iz args.iz store.iz commands.iz
```

---

## 2. Parsing command-line arguments

IziLang exposes the arguments passed to the script via the built-in `args` global, which is an array of strings. `args[0]` is the command name, `args[1]` is the first argument, and so on.

Create **`args.iz`**:

```izilang
// args.iz — Command-line argument helpers

// Return the sub-command name (first argument after the script name),
// or nil if no arguments were given.
fn getCommand() {
    if (len(args) < 1) { return nil; }
    return args[0];
}

// Return a positional argument by index (0-based, after the command).
fn getArg(index) {
    var real = index + 1;  // skip the command at args[0]
    if (real >= len(args)) { return nil; }
    return args[real];
}

// Return all arguments after the command joined with spaces.
fn getRemainingArgs() {
    import "string";
    var parts = [];
    var i = 1;
    while (i < len(args)) {
        push(parts, args[i]);
        i = i + 1;
    }
    return string.join(parts, " ");
}
```

---

## 3. Persisting data with JSON

The task list is stored as a JSON file in the user's home directory (falling back to `/tmp` if `HOME` is not set).

Create **`store.iz`**:

```izilang
// store.iz — Read / write the task list

import * as env  from "std.env";
import * as json from "std.json";
import "io";

fn getStorePath() {
    var home = env.get("HOME");
    if (home == nil) { home = "/tmp"; }
    return home + "/.todo_izi.json";
}

// Load tasks from disk. Returns an empty array if the file doesn't exist.
fn loadTasks() {
    var path = getStorePath();
    if (!io.exists(path)) {
        return [];
    }
    try {
        var raw = io.readFile(path);
        return json.parse(raw);
    } catch (e) {
        print("Warning: could not read task file:", e);
        return [];
    }
}

// Save tasks to disk.
fn saveTasks(tasks) {
    var path = getStorePath();
    try {
        io.writeFile(path, json.stringify(tasks));
    } catch (e) {
        print("Error: could not save task file:", e);
    }
}
```

---

## 4. Implementing commands

Each command operates on the in-memory task list and persists changes with `saveTasks`.

Tasks are maps with these fields:
- `id` (Number) — unique, auto-incrementing identifier
- `text` (String) — description
- `done` (Bool) — completion state

Create **`commands.iz`**:

```izilang
// commands.iz — Task manager command implementations

// Generate a new unique id (max existing id + 1).
fn nextId(tasks) {
    var maxId = 0;
    var i = 0;
    while (i < len(tasks)) {
        if (tasks[i]["id"] > maxId) {
            maxId = tasks[i]["id"];
        }
        i = i + 1;
    }
    return maxId + 1;
}

// ── add ──────────────────────────────────────────────────────────────────────

fn cmdAdd(tasks, text) {
    if (text == nil or text == "") {
        print("Usage: todo add <task description>");
        return tasks;
    }
    var task = {"id": nextId(tasks), "text": text, "done": false};
    push(tasks, task);
    print("Added #" + toString(task["id"]) + ": " + text);
    return tasks;
}

// ── list ─────────────────────────────────────────────────────────────────────

fn cmdList(tasks) {
    if (len(tasks) == 0) {
        print("No tasks yet. Add one with: todo add <description>");
        return;
    }
    print("Tasks:");
    var i = 0;
    while (i < len(tasks)) {
        var t = tasks[i];
        var status = "[ ]";
        if (t["done"]) { status = "[x]"; }
        print("  " + status + " #" + toString(t["id"]) + " " + t["text"]);
        i = i + 1;
    }
}

// ── done ─────────────────────────────────────────────────────────────────────

fn cmdDone(tasks, idStr) {
    if (idStr == nil) {
        print("Usage: todo done <id>");
        return tasks;
    }
    var id = toNumber(idStr);
    var found = false;
    var i = 0;
    while (i < len(tasks)) {
        if (tasks[i]["id"] == id) {
            tasks[i]["done"] = true;
            print("Marked #" + idStr + " as done: " + tasks[i]["text"]);
            found = true;
        }
        i = i + 1;
    }
    if (!found) {
        print("No task with id " + idStr);
    }
    return tasks;
}

// ── remove ───────────────────────────────────────────────────────────────────

fn cmdRemove(tasks, idStr) {
    if (idStr == nil) {
        print("Usage: todo remove <id>");
        return tasks;
    }
    var id = toNumber(idStr);
    import { filter } from "array";
    var updated = filter(tasks, fn(t) { return t["id"] != id; });
    if (len(updated) == len(tasks)) {
        print("No task with id " + idStr);
    } else {
        print("Removed task #" + idStr);
    }
    return updated;
}

// ── clear ────────────────────────────────────────────────────────────────────

fn cmdClear(tasks) {
    print("Cleared " + toString(len(tasks)) + " task(s).");
    return [];
}

// ── help ─────────────────────────────────────────────────────────────────────

fn cmdHelp() {
    print("todo — A simple task manager written in IziLang");
    print("");
    print("Usage:");
    print("  izi todo.iz add <description>   Add a new task");
    print("  izi todo.iz list                List all tasks");
    print("  izi todo.iz done <id>           Mark a task as done");
    print("  izi todo.iz remove <id>         Remove a task");
    print("  izi todo.iz clear               Remove all tasks");
    print("  izi todo.iz help                Show this help");
}
```

---

## 5. Putting it all together

Create **`todo.iz`** — the entry point that wires everything up:

```izilang
// todo.iz — Entry point for the todo CLI app

import "args";
import "store";
import "commands";

var command = getCommand();

if (command == nil or command == "help") {
    cmdHelp();
} else {
    var tasks = loadTasks();
    var updated = tasks;

    if (command == "add") {
        var text = getRemainingArgs();
        updated = cmdAdd(tasks, text);
    } else if (command == "list") {
        cmdList(tasks);
    } else if (command == "done") {
        var id = getArg(0);
        updated = cmdDone(tasks, id);
    } else if (command == "remove") {
        var id = getArg(0);
        updated = cmdRemove(tasks, id);
    } else if (command == "clear") {
        updated = cmdClear(tasks);
    } else {
        print("Unknown command: " + command);
        print("Run 'izi todo.iz help' for usage.");
    }

    saveTasks(updated);
}
```

---

## 6. Running the app

```bash
# Show help
izi todo.iz help

# Add some tasks
izi todo.iz add "Buy groceries"
izi todo.iz add "Write the IziLang tutorial"
izi todo.iz add "Fix the bug in the parser"

# List tasks
izi todo.iz list
# Tasks:
#   [ ] #1 Buy groceries
#   [ ] #2 Write the IziLang tutorial
#   [ ] #3 Fix the bug in the parser

# Mark a task as done
izi todo.iz done 2
# Marked #2 as done: Write the IziLang tutorial

izi todo.iz list
# Tasks:
#   [ ] #1 Buy groceries
#   [x] #2 Write the IziLang tutorial
#   [ ] #3 Fix the bug in the parser

# Remove a task
izi todo.iz remove 1
# Removed task #1

# Clear everything
izi todo.iz clear
# Cleared 2 task(s).
```

---

## 7. Exercises

Now that the basic CLI is working, try extending it:

### Exercise A — Priority levels

Add a `priority` field (`"low"`, `"medium"`, `"high"`) to tasks. Update `cmdAdd` to accept an optional `--priority` flag:

```bash
izi todo.iz add --priority high "Deploy to production"
```

### Exercise B — Filter by status

Add `izi todo.iz list --done` to show only completed tasks, and `izi todo.iz list --pending` for only incomplete tasks.

### Exercise C — Search

Add a `search` command that lists tasks whose text contains a given substring:

```bash
izi todo.iz search "parser"
```

### Exercise D — Due dates

Store an optional `due` field (a date string). Update `list` to show the due date and sort tasks by due date.

### Exercise E — Coloured output

Use string interpolation and ANSI escape codes to colour the output:

```izilang
fn green(s)  { return "\033[32m" + s + "\033[0m"; }
fn red(s)    { return "\033[31m" + s + "\033[0m"; }
fn bold(s)   { return "\033[1m"  + s + "\033[0m"; }

// In cmdList:
if (t["done"]) {
    print("  " + green("[x]") + " #" + toString(t["id"]) + " " + t["text"]);
} else {
    print("  " + red("[ ]") + " #" + toString(t["id"]) + " " + t["text"]);
}
```

---

## What you learned

- Accessing command-line arguments via the `args` built-in
- Reading and writing JSON files with `io` and `std.json`
- Organising a multi-file IziLang project with `import`
- Building a command dispatcher with `if/else if`
- Defensive coding with `try/catch` for file I/O

## Next steps

- [Standard Library Reference](stdlib/README.md) — All available modules
- [Concurrency Guide](CONCURRENCY.md) — Add async processing to your app
- [Language Specification](LANGUAGE_SPEC.md) — Deep-dive into the language
- [Cookbook](COOKBOOK.md) — More real-world patterns
