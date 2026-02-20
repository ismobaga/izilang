#include "type.hpp"

namespace izi {

std::string TypeAnnotation::toString() const {
    switch (kind) {
        case Kind::Number:
            return "Number";
        case Kind::String:
            return "String";
        case Kind::Bool:
            return "Bool";
        case Kind::Nil:
            return "Nil";
        case Kind::Any:
            return "Any";
        case Kind::Void:
            return "Void";
        case Kind::Array:
            if (elementType) {
                return "Array<" + elementType->toString() + ">";
            }
            return "Array";
        case Kind::Map:
            if (keyType && valueType) {
                return "Map<" + keyType->toString() + ", " + valueType->toString() + ">";
            }
            return "Map";
        case Kind::Function: {
            std::string result = "Function(";
            for (size_t i = 0; i < paramTypes.size(); ++i) {
                if (i > 0) result += ", ";
                result += paramTypes[i]->toString();
            }
            result += ")";
            if (valueType) {
                result += " -> " + valueType->toString();
            }
            return result;
        }
    }
    return "Unknown";
}

bool TypeAnnotation::isCompatibleWith(const TypeAnnotation& other) const {
    // Any is compatible with everything
    if (kind == Kind::Any || other.kind == Kind::Any) {
        return true;
    }

    // Same base type
    if (kind != other.kind) {
        return false;
    }

    // For Array, check element types
    if (kind == Kind::Array) {
        if (elementType && other.elementType) {
            return elementType->isCompatibleWith(*other.elementType);
        }
        return true;  // Untyped arrays are compatible
    }

    // For Map, check key and value types
    if (kind == Kind::Map) {
        bool keyCompatible = !keyType || !other.keyType || keyType->isCompatibleWith(*other.keyType);
        bool valueCompatible = !valueType || !other.valueType || valueType->isCompatibleWith(*other.valueType);
        return keyCompatible && valueCompatible;
    }

    // For Function, check parameter and return types
    if (kind == Kind::Function) {
        if (paramTypes.size() != other.paramTypes.size()) {
            return false;
        }
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            if (!paramTypes[i]->isCompatibleWith(*other.paramTypes[i])) {
                return false;
            }
        }
        if (valueType && other.valueType) {
            return valueType->isCompatibleWith(*other.valueType);
        }
        return true;
    }

    // For simple types, already checked equality
    return true;
}

}  // namespace izi
