# IziLang: Language Manifesto

**Version**: 1.0  
**Date**: February 2026  
**Status**: Living Document

---

## The Name: Why "Izi"?

**IziLang** comes from how it sounds — ***easy***. The name is a promise: programming should be accessible, expressive, and straightforward. But "easy" doesn't mean "toy" — IziLang is designed for serious development with professional-grade features.

---

## One-Sentence Positioning

> **IziLang is a modern, expressive programming language that makes everyday programming tasks easy without sacrificing power, designed for developers who value clarity, productivity, and pragmatism over complexity.**

---

## Core Philosophy

### 1. Easy to Learn, Hard to Master

IziLang should be:
- **Approachable** for intermediate programmers (not complete beginners, not experts only)
- **Quick to learn** — productive within hours, not weeks
- **Deep enough** to support complex applications
- **Rewarding** as skills grow

**Concrete Example**:
```izi
// Hello World — instantly readable
fn greet(name) {
    print("Hello, " + name + "!");
}

// First-class functions — powerful but understandable
var makeCounter = fn() {
    var count = 0;
    return fn() { count = count + 1; return count; };
};
```

### 2. Clarity Over Cleverness

- **Explicit over implicit** (but not verbose)
- **Readable over writable** (optimize for reading, not typing)
- **Predictable over surprising** (principle of least astonishment)

**What This Means**:
- ❌ No operator overloading that changes meanings
- ❌ No implicit type coercion (`"5" + 3` is an error, not "53")
- ✅ Clear error messages with context
- ✅ One obvious way to do things

### 3. Pragmatic, Not Dogmatic

- **Multi-paradigm** by design (imperative + functional + OOP)
- **Dynamic typing** with optional gradual typing later
- **Batteries included** standard library
- **Practical solutions** over theoretical purity

**What This Means**:
- Use loops or map/filter — both are valid
- Mutability is allowed (but can be constrained)
- Exceptions for errors (not Result types everywhere)
- Real-world needs drive features, not academic ideals

### 4. Developer Experience First

- **Helpful error messages** (Rust-quality diagnostics)
- **Fast feedback loop** (quick compile/run cycles)
- **Good tooling** (formatter, LSP, debugger)
- **Comprehensive documentation** with examples

---

## What Problems Does IziLang Solve?

### 1. Scripting & Automation
**Better than**: Bash (more structured), Python (faster startup for compiled mode)

IziLang excels at:
- System administration scripts
- Build automation
- Data processing pipelines
- One-off utilities

**Example**: File processing with clear error handling
```izi
try {
    var content = readFile("config.json");
    var data = parseJSON(content);
    processConfig(data);
} catch (err) {
    print("Configuration error: " + err.message);
}
```

### 2. Backend Services
**Better than**: Node.js (clearer syntax), PHP (better structure)

IziLang is good for:
- REST API servers
- Microservices
- Background workers
- Internal tools

**Example**: Simple HTTP endpoint
```izi
import { Server } from "http";

var app = Server.new();
app.route("/api/users", fn(req) {
    return { users: getAllUsers() };
});
app.listen(8080);
```

### 3. Prototyping & Experimentation
**Better than**: Java (less boilerplate), C++ (memory safe)

Ideal for:
- Quick proof-of-concepts
- Algorithm experimentation
- Learning new concepts
- Teaching programming

### 4. Embedded Scripting
**Better than**: Lua (richer features), JavaScript (simpler embedding)

Good for:
- Game scripting
- Plugin systems
- Configuration DSLs
- Custom automation

---

## Who Is IziLang For?

### Primary Audience
**Intermediate programmers** who:
- Know at least one programming language
- Value productivity and clarity
- Build practical applications
- Want modern language features without a steep learning curve

### Use Cases
1. **Backend Developers** — REST APIs, microservices, CLIs
2. **DevOps Engineers** — Automation scripts, deployment tools
3. **Data Engineers** — ETL pipelines, data processing
4. **Students** — Learning programming concepts
5. **Hobbyists** — Side projects, experiments

### NOT For (Explicitly)
- ❌ Systems programming (use C, Rust, or Zig)
- ❌ High-frequency trading (use C++ or Rust)
- ❌ Operating system kernels (use C, Rust)
- ❌ Embedded microcontrollers (use C or Rust)
- ❌ Real-time systems with hard deadlines

---

## What "Easy" Means Concretely

### 1. Easy to Read
```izi
// ✅ Clear intent, no magic
fn factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// ❌ NOT this (too clever)
var fac = fn(n) => n < 2 ? 1 : n * fac(n-1);
```

### 2. Easy to Understand Errors
```
Runtime Error at line 12, column 18:
  12 | var result = name + age;
     |                  ^
Cannot add string and number.
Operands must be two numbers or two strings.

Hint: Use string concatenation: name + string(age)
```

### 3. Easy to Learn Syntax
- **Familiar keywords**: `fn`, `var`, `if`, `while`, `for`, `return`
- **Minimal punctuation**: No `$`, `@`, `::`, `->`
- **Consistent patterns**: One way to declare functions, variables
- **No hidden behavior**: What you see is what you get

### 4. Easy to Use Tools
```bash
izi run script.iz        # Execute script
izi build app.iz         # Compile to executable
izi test                 # Run tests
izi fmt src/             # Format code
izi check src/           # Static analysis
```

### 5. Easy to Debug
- Stack traces with source locations
- REPL for interactive testing
- Debugger with breakpoints (future)
- Print debugging "just works"

---

## Language Principles

### Principle 1: Predictability
**Code should behave as expected based on common sense.**

- Variables are mutable by default (explicit immutability later)
- Functions are first-class values
- Arrays are 0-indexed
- `nil` represents absence
- No global variable surprises

### Principle 2: Safety
**Prevent common mistakes at compile/runtime.**

- Memory safe (no manual malloc/free)
- No null pointer dereference (explicit nil checks)
- Strong type checks at runtime
- Exception handling required for I/O
- No silent failures

### Principle 3: Performance
**Fast enough for real work, not fastest possible.**

- Target: Match Python performance (v0.3+)
- Startup time < 50ms for small programs
- Memory efficient (no wasteful copies)
- JIT compilation possible later (v1.0+)
- Profile-guided optimization

**Performance Philosophy**: 
- Correctness > Performance initially
- Optimize common paths, not edge cases
- Measure before optimizing
- "Fast enough" is good enough

### Principle 4: Composability
**Small, understandable pieces that combine well.**

- Functions compose naturally
- Modules are explicit
- No action-at-a-distance
- Clear data flow
- Minimal global state

### Principle 5: Discoverability
**Features should be easy to find and understand.**

- Consistent naming (camelCase for functions, lowercase for keywords)
- Standard library is organized (std.*)
- Good defaults (sensible behavior without config)
- Documentation is comprehensive
- Examples for everything

---

## Non-Goals (What IziLang Will NOT Do)

### 1. Systems Programming
- **No** manual memory management
- **No** bare-metal access
- **No** inline assembly
- **No** zero-cost abstractions guarantee

### 2. Pure Functional Programming
- **No** enforced immutability
- **No** lazy evaluation by default
- **No** complex type systems (HKT, GADTs)
- **No** category theory abstractions

### 3. Maximum Performance
- **No** JIT in v1.0 (maybe later)
- **No** SIMD intrinsics
- **No** manual vectorization
- **No** competing with C/C++/Rust speed

### 4. Cutting-Edge Features
- **No** dependent types
- **No** effect systems
- **No** proof assistants
- **No** experimental PL research

### 5. Enterprise Java-Style Complexity
- **No** XML configuration
- **No** annotation processors
- **No** heavyweight frameworks
- **No** design pattern overload

### 6. "Do Everything"
- **No** native GUI toolkit
- **No** game engine
- **No** computer vision library
- **No** machine learning framework

**Rationale**: Focus on core language quality. Let ecosystem provide specialized tools.

---

## Design Constraints

### Must Have (Non-Negotiable)
1. **Memory safety** — No segfaults, use-after-free, or data races
2. **Clear errors** — Helpful messages with source locations
3. **Fast iteration** — Quick feedback from run/test commands
4. **Cross-platform** — Linux, macOS, Windows support
5. **Embeddable** — Can be used as scripting engine

### Should Have (High Priority)
1. **Good performance** — Comparable to Python/Ruby
2. **Rich stdlib** — Batteries-included standard library
3. **Module system** — Clear code organization
4. **Pattern matching** — Expressive control flow
5. **Closures** — First-class functions with captures

### Could Have (Nice to Have)
1. **Static typing** — Optional type annotations (gradual typing)
2. **Concurrency** — Async/await or threads
3. **REPL** — Interactive shell
4. **Debugger** — Breakpoints and stepping
5. **Package manager** — Dependency management

---

## Success Criteria

### For Developers
- **"I was productive in 2 hours"** — Quick learning curve
- **"The errors actually help"** — Useful diagnostics
- **"It just works"** — Minimal friction
- **"I can read others' code easily"** — Consistent style

### For the Language
- **100+ packages** in registry (by v1.0)
- **10+ production deployments** (by v1.0)
- **1000+ GitHub stars** (by v1.0)
- **Active community** with discussions and contributions

### Technical Metrics
- **Startup time** < 50ms for small programs
- **Performance** within 2x of Python 3.11
- **Test coverage** > 90% for core language
- **Documentation** coverage 100% (all features documented)

---

## Evolution Strategy

### Phase 1: Foundation (v0.1 - v0.3)
**Goal**: Prove the language works

- Core features working
- Good documentation
- Stable syntax
- Test suite
- Multi-platform builds

### Phase 2: Adoption (v0.4 - v0.9)
**Goal**: Attract early adopters

- Tooling (LSP, formatter, debugger)
- Package ecosystem
- Example projects
- Community building
- Performance improvements

### Phase 3: Stability (v1.0)
**Goal**: Production readiness

- Backward compatibility guarantee
- Semantic versioning
- Long-term support
- Enterprise adoption
- Training materials

### Phase 4: Maturity (v1.x)
**Goal**: Best-in-class developer experience

- Advanced optimizations
- Rich ecosystem
- Conference presence
- Books and courses
- Established best practices

---

## Comparison with Other Languages

### vs Python
- ✅ **Faster** (compiled mode)
- ✅ **Better errors** (source location, context)
- ✅ **Simpler** (fewer ways to do things)
- ❌ **Smaller ecosystem** (for now)
- ❌ **Less mature** (fewer libraries)

### vs JavaScript
- ✅ **No type coercion** (explicit conversions)
- ✅ **Cleaner syntax** (no `var`/`let`/`const` confusion)
- ✅ **Better module system** (ES6-style from start)
- ❌ **No browser runtime** (not for web frontend)
- ❌ **Smaller job market** (newer language)

### vs Ruby
- ✅ **Simpler syntax** (less magic)
- ✅ **Explicit over implicit**
- ✅ **Modern features** (pattern matching from start)
- ❌ **Less object-oriented** (OOP is optional)
- ❌ **Smaller community**

### vs Go
- ✅ **Gradual typing** (optional types)
- ✅ **More expressive** (pattern matching, closures)
- ✅ **Richer syntax** (more language features)
- ❌ **Not compiled to native** (interpreted/bytecode)
- ❌ **Slower** (no Go-level performance)

### vs Lua
- ✅ **Richer standard library**
- ✅ **Better error messages**
- ✅ **Module system** (explicit imports)
- ❌ **Larger runtime** (not as embeddable)
- ❌ **Newer** (less proven for embedding)

---

## Guiding Questions for Feature Proposals

Before adding any feature, ask:

1. **Does this make IziLang easier?**
   - Does it reduce boilerplate?
   - Does it make code more readable?
   - Can beginners understand it?

2. **Does it preserve predictability?**
   - Is the behavior obvious?
   - Are there hidden side effects?
   - Does it follow existing patterns?

3. **Is it teachable in < 5 minutes?**
   - Can it be explained simply?
   - Does it require deep theory?
   - Are there good examples?

4. **Is it required for v1.0?**
   - Can real programs be written without it?
   - Can it be added later compatibly?
   - Is it a nice-to-have or must-have?

5. **Can it be deferred without harm?**
   - Does it block other features?
   - Can it be added in v1.x or v2.0?
   - Is there a workaround?

**If the answer to 1-3 is "no" or 4-5 is "yes", defer the feature.**

---

## Cultural Values

### Code Style
- **Clarity** — Use descriptive names
- **Simplicity** — Avoid over-abstraction
- **Consistency** — Follow conventions
- **Comments** — Explain "why", not "what"

### Community
- **Welcoming** — Help newcomers
- **Respectful** — Assume good intent
- **Collaborative** — Review code kindly
- **Inclusive** — All backgrounds welcome

### Development
- **Quality** — Test thoroughly
- **Documentation** — Write as you code
- **Iteration** — Ship early, improve often
- **Feedback** — Listen to users

---

## Conclusion

IziLang is not trying to be the **fastest**, the **most powerful**, or the **most innovative** language. It's trying to be the **most practical** and **most pleasant** for everyday programming tasks.

**Success** means developers choose IziLang not because they have to, but because they **want to** — because it makes their work easier, their code clearer, and their day better.

The name says it all: **Izi** (easy). That's the promise, the goal, and the measure of success.

---

**Document Status**: Living Document  
**Last Updated**: February 2026  
**Maintained By**: IziLang Core Team  
**Feedback**: GitHub Discussions, Issues with `manifesto` label
