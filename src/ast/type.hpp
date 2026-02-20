#pragma once

#include <memory>
#include <string>
#include <vector>

namespace izi {

// Type annotation for gradual typing (v0.3)
// Supports: Number, String, Bool, Nil, Array, Map, Function, Any
struct TypeAnnotation {
    enum class Kind {
        Number,
        String,
        Bool,
        Nil,
        Array,
        Map,
        Function,
        Any,  // Default for unannotated code
        Void  // For functions with no return value
    };

    Kind kind;
    std::unique_ptr<TypeAnnotation> elementType;  // For Array<T>
    std::unique_ptr<TypeAnnotation> keyType;  // For Map<K, V>
    std::unique_ptr<TypeAnnotation> valueType;  // For Map<K, V> or Function return
    std::vector<std::unique_ptr<TypeAnnotation>> paramTypes;  // For Function types

    explicit TypeAnnotation(Kind k) : kind(k), elementType(nullptr), keyType(nullptr), valueType(nullptr) {}

    // For simple types (Number, String, Bool, Nil, Any, Void)
    static std::unique_ptr<TypeAnnotation> simple(Kind k) { return std::make_unique<TypeAnnotation>(k); }

    // For Array<T>
    static std::unique_ptr<TypeAnnotation> array(std::unique_ptr<TypeAnnotation> elem) {
        auto type = std::make_unique<TypeAnnotation>(Kind::Array);
        type->elementType = std::move(elem);
        return type;
    }

    // For Map<K, V>
    static std::unique_ptr<TypeAnnotation> map(std::unique_ptr<TypeAnnotation> key,
                                               std::unique_ptr<TypeAnnotation> value) {
        auto type = std::make_unique<TypeAnnotation>(Kind::Map);
        type->keyType = std::move(key);
        type->valueType = std::move(value);
        return type;
    }

    // For Function(T1, T2, ...) -> R
    static std::unique_ptr<TypeAnnotation> function(std::vector<std::unique_ptr<TypeAnnotation>> params,
                                                    std::unique_ptr<TypeAnnotation> returnType) {
        auto type = std::make_unique<TypeAnnotation>(Kind::Function);
        type->paramTypes = std::move(params);
        type->valueType = std::move(returnType);
        return type;
    }

    // Helper to convert type to string for error messages
    std::string toString() const;

    // Type compatibility check (for type checking)
    bool isCompatibleWith(const TypeAnnotation& other) const;
};

using TypePtr = std::unique_ptr<TypeAnnotation>;

}  // namespace izi
