#include "user_function.hpp"
#include "interpreter.hpp"

namespace izi {

// Helper: create a bound callable (function + captured args) as a Task
class BoundCall : public Callable {
   public:
    BoundCall(std::shared_ptr<Callable> fn, std::vector<Value> args)
        : fn_(std::move(fn)), args_(std::move(args)) {}

    int arity() const override { return 0; }
    std::string name() const override { return fn_->name(); }

    Value call(Interpreter& interp, const std::vector<Value>& /*unused*/) override {
        return fn_->call(interp, args_);
    }

   private:
    std::shared_ptr<Callable> fn_;
    std::vector<Value> args_;
};

Value UserFunction::call(Interpreter& interp, const std::vector<Value>& arguments) {
    // If async, wrap in a pending Task instead of running immediately.
    // We create a copy of this UserFunction with isAsync_=false so that when the
    // Task is later executed (via await), it runs the body directly without
    // wrapping again (which would cause infinite recursion / re-wrapping).
    if (isAsync_) {
        // We need a shared_ptr to this; create a copy wrapped as shared_ptr
        // We capture arguments in a BoundCall so the task can be awaited later
        auto self = std::make_shared<UserFunction>(*this);
        self->isAsync_ = false;  // Run body directly when the task executes
        auto task = std::make_shared<Task>();
        task->callable = std::make_shared<BoundCall>(std::move(self), arguments);
        task->state = Task::State::Pending;
        return task;
    }

    // Check call depth to prevent stack overflow
    if (interp.callDepth >= MAX_CALL_DEPTH) {
        throw std::runtime_error("Stack overflow: Maximum call depth of " + std::to_string(MAX_CALL_DEPTH) +
                                 " exceeded.");
    }

    // Increment call depth
    interp.callDepth++;

    // Get params and body from either decl or funcExpr
    const std::vector<std::string>* params = nullptr;
    const std::vector<StmtPtr>* body = nullptr;
    std::string funcName;
    int funcLine = 0;

    if (decl) {
        params = &decl->params;
        body = &decl->body;
        funcName = decl->name.empty() ? "<anonymous>" : decl->name;
        funcLine = decl->line;
    } else if (funcExpr) {
        params = &funcExpr->params;
        body = &funcExpr->body;
        funcName = "<anonymous>";
    } else {
        interp.callDepth--;  // Restore call depth before throwing
        throw std::runtime_error("Invalid UserFunction: no declaration or expression");
    }

    // Notify debug hook of function entry
    interp.notifyFunctionEnter(funcName, funcLine);

    auto localEnv = interp.arena_.create(closure);

    for (size_t i = 0; i < params->size(); ++i) {
        const std::string& paramName = (*params)[i];
        Value argVal = (i < arguments.size()) ? arguments[i] : Nil{};

        localEnv->define(paramName, argVal);
    }

    try {
        interp.executeBlock(*body, localEnv);
    } catch (const ReturnSignal& returnValue) {
        interp.callDepth--;  // Restore call depth on return
        interp.notifyFunctionExit();
        return returnValue.value;
    } catch (RuntimeError& e) {
        interp.callDepth--;  // Restore call depth on exception
        interp.notifyFunctionExit();
        e.addFrame(funcName, funcLine);
        throw;
    } catch (...) {
        interp.callDepth--;  // Restore call depth on exception
        interp.notifyFunctionExit();
        throw;
    }

    interp.callDepth--;  // Restore call depth on normal exit
    interp.notifyFunctionExit();
    return Nil{};
}
}  // namespace izi