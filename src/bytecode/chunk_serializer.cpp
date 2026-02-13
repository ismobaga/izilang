#include "chunk_serializer.hpp"
#include "vm_user_function.hpp"
#include "vm_native.hpp"
#include "vm_class.hpp"
#include "common/error.hpp"
#include <cstring>
#include <stdexcept>

namespace izi {

// Write helper methods
void ChunkSerializer::writeUint32(std::ofstream& out, uint32_t value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void ChunkSerializer::writeUint8(std::ofstream& out, uint8_t value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void ChunkSerializer::writeString(std::ofstream& out, const std::string& str) {
    writeUint32(out, static_cast<uint32_t>(str.size()));
    out.write(str.data(), str.size());
}

void ChunkSerializer::writeValue(std::ofstream& out, const Value& value) {
    if (std::holds_alternative<Nil>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::NIL));
    } else if (std::holds_alternative<bool>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::BOOL));
        writeUint8(out, std::get<bool>(value) ? 1 : 0);
    } else if (std::holds_alternative<double>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::NUMBER));
        double num = std::get<double>(value);
        out.write(reinterpret_cast<const char*>(&num), sizeof(num));
    } else if (std::holds_alternative<std::string>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::STRING));
        writeString(out, std::get<std::string>(value));
    } else if (std::holds_alternative<std::shared_ptr<Array>>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::ARRAY));
        const auto& arr = std::get<std::shared_ptr<Array>>(value);
        writeUint32(out, static_cast<uint32_t>(arr->elements.size()));
        for (const auto& elem : arr->elements) {
            writeValue(out, elem);
        }
    } else if (std::holds_alternative<std::shared_ptr<Map>>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::MAP));
        const auto& map = std::get<std::shared_ptr<Map>>(value);
        writeUint32(out, static_cast<uint32_t>(map->entries.size()));
        for (const auto& [key, val] : map->entries) {
            writeString(out, key);
            writeValue(out, val);
        }
    } else if (std::holds_alternative<std::shared_ptr<Set>>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::SET));
        const auto& set = std::get<std::shared_ptr<Set>>(value);
        writeUint32(out, static_cast<uint32_t>(set->values.size()));
        for (const auto& [key, val] : set->values) {
            writeString(out, key);
            writeValue(out, val);
        }
    } else if (std::holds_alternative<std::shared_ptr<VmCallable>>(value)) {
        auto callable = std::get<std::shared_ptr<VmCallable>>(value);
        
        // Check if it's a VmUserFunction
        if (auto userFunc = std::dynamic_pointer_cast<VmUserFunction>(callable)) {
            writeUint8(out, static_cast<uint8_t>(ValueType::FUNCTION));
            writeString(out, userFunc->name());
            
            // Write parameter count and names
            const auto& params = userFunc->params();
            writeUint32(out, static_cast<uint32_t>(params.size()));
            for (const auto& param : params) {
                writeString(out, param);
            }
            
            // Recursively serialize the function's chunk
            const Chunk& funcChunk = userFunc->getChunk();
            
            // Write code section
            writeUint32(out, static_cast<uint32_t>(funcChunk.code.size()));
            for (uint8_t byte : funcChunk.code) {
                writeUint8(out, byte);
            }
            
            // Write constants section
            writeUint32(out, static_cast<uint32_t>(funcChunk.constants.size()));
            for (const auto& constant : funcChunk.constants) {
                writeValue(out, constant);
            }
            
            // Write names section
            writeUint32(out, static_cast<uint32_t>(funcChunk.names.size()));
            for (const auto& name : funcChunk.names) {
                writeString(out, name);
            }
        } else {
            // Native function - store only name reference
            writeUint8(out, static_cast<uint8_t>(ValueType::NATIVE_FUNCTION));
            writeString(out, callable->name());
        }
    } else if (std::holds_alternative<std::shared_ptr<VmClass>>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::CLASS));
        const auto& vmClass = std::get<std::shared_ptr<VmClass>>(value);
        writeString(out, vmClass->className);
        
        // Write field names
        writeUint32(out, static_cast<uint32_t>(vmClass->fieldNames.size()));
        for (const auto& fieldName : vmClass->fieldNames) {
            writeString(out, fieldName);
        }
        
        // Write field defaults
        writeUint32(out, static_cast<uint32_t>(vmClass->fieldDefaults.size()));
        for (const auto& [fieldName, defaultValue] : vmClass->fieldDefaults) {
            writeString(out, fieldName);
            writeValue(out, defaultValue);
        }
        
        // Write methods
        writeUint32(out, static_cast<uint32_t>(vmClass->methods.size()));
        for (const auto& [methodName, methodCallable] : vmClass->methods) {
            writeString(out, methodName);
            writeValue(out, methodCallable);
        }
    } else if (std::holds_alternative<std::shared_ptr<Instance>>(value)) {
        // Instances cannot be serialized directly - they are runtime constructs
        throw std::runtime_error("Cannot serialize instance objects to bytecode");
    } else if (std::holds_alternative<std::shared_ptr<Error>>(value)) {
        writeUint8(out, static_cast<uint8_t>(ValueType::ERROR));
        const auto& err = std::get<std::shared_ptr<Error>>(value);
        writeString(out, err->message);
        writeString(out, err->type);
        // Note: We don't serialize cause chain or stack trace for simplicity
    } else {
        throw std::runtime_error("Unknown value type for serialization");
    }
}

// Read helper methods
uint32_t ChunkSerializer::readUint32(std::ifstream& in) {
    uint32_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    if (!in) {
        throw std::runtime_error("Failed to read uint32 from bytecode file");
    }
    return value;
}

uint8_t ChunkSerializer::readUint8(std::ifstream& in) {
    uint8_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    if (!in) {
        throw std::runtime_error("Failed to read uint8 from bytecode file");
    }
    return value;
}

std::string ChunkSerializer::readString(std::ifstream& in) {
    uint32_t length = readUint32(in);
    std::string str(length, '\0');
    in.read(&str[0], length);
    if (!in) {
        throw std::runtime_error("Failed to read string from bytecode file");
    }
    return str;
}

Value ChunkSerializer::readValue(std::ifstream& in) {
    uint8_t typeTag = readUint8(in);
    ValueType type = static_cast<ValueType>(typeTag);
    
    switch (type) {
        case ValueType::NIL:
            return Nil{};
            
        case ValueType::BOOL: {
            uint8_t boolVal = readUint8(in);
            return boolVal != 0;
        }
        
        case ValueType::NUMBER: {
            double num;
            in.read(reinterpret_cast<char*>(&num), sizeof(num));
            if (!in) {
                throw std::runtime_error("Failed to read number from bytecode file");
            }
            return num;
        }
        
        case ValueType::STRING:
            return readString(in);
            
        case ValueType::ARRAY: {
            uint32_t size = readUint32(in);
            auto arr = std::make_shared<Array>();
            arr->elements.reserve(size);
            for (uint32_t i = 0; i < size; ++i) {
                arr->elements.push_back(readValue(in));
            }
            return arr;
        }
        
        case ValueType::MAP: {
            uint32_t size = readUint32(in);
            auto map = std::make_shared<Map>();
            for (uint32_t i = 0; i < size; ++i) {
                std::string key = readString(in);
                Value val = readValue(in);
                map->entries[key] = val;
            }
            return map;
        }
        
        case ValueType::SET: {
            uint32_t size = readUint32(in);
            auto set = std::make_shared<Set>();
            for (uint32_t i = 0; i < size; ++i) {
                std::string key = readString(in);
                Value val = readValue(in);
                set->values[key] = val;
            }
            return set;
        }
        
        case ValueType::FUNCTION: {
            std::string funcName = readString(in);
            
            // Read parameters
            uint32_t paramCount = readUint32(in);
            std::vector<std::string> params;
            params.reserve(paramCount);
            for (uint32_t i = 0; i < paramCount; ++i) {
                params.push_back(readString(in));
            }
            
            // Read function's chunk
            Chunk funcChunk;
            
            // Read code section
            uint32_t codeSize = readUint32(in);
            funcChunk.code.reserve(codeSize);
            for (uint32_t i = 0; i < codeSize; ++i) {
                funcChunk.code.push_back(readUint8(in));
            }
            
            // Read constants section
            uint32_t constCount = readUint32(in);
            funcChunk.constants.reserve(constCount);
            for (uint32_t i = 0; i < constCount; ++i) {
                funcChunk.constants.push_back(readValue(in));
            }
            
            // Read names section
            uint32_t namesCount = readUint32(in);
            funcChunk.names.reserve(namesCount);
            for (uint32_t i = 0; i < namesCount; ++i) {
                funcChunk.names.push_back(readString(in));
            }
            
            auto funcChunkPtr = std::make_shared<Chunk>(std::move(funcChunk));
            return std::make_shared<VmUserFunction>(funcName, params, funcChunkPtr);
        }
        
        case ValueType::NATIVE_FUNCTION: {
            // For native functions, we can't restore them from name alone
            // They need to be registered during VM initialization
            std::string funcName = readString(in);
            throw std::runtime_error("Cannot deserialize native function: " + funcName + 
                                   ". Native functions must be registered at runtime.");
        }
        
        case ValueType::CLASS: {
            std::string className = readString(in);
            
            // Read field names
            uint32_t fieldCount = readUint32(in);
            std::vector<std::string> fieldNames;
            fieldNames.reserve(fieldCount);
            for (uint32_t i = 0; i < fieldCount; ++i) {
                fieldNames.push_back(readString(in));
            }
            
            // Read field defaults
            uint32_t defaultCount = readUint32(in);
            std::unordered_map<std::string, Value> fieldDefaults;
            for (uint32_t i = 0; i < defaultCount; ++i) {
                std::string fieldName = readString(in);
                Value defaultValue = readValue(in);
                fieldDefaults[fieldName] = defaultValue;
            }
            
            // Read methods
            uint32_t methodCount = readUint32(in);
            std::unordered_map<std::string, std::shared_ptr<VmCallable>> methods;
            for (uint32_t i = 0; i < methodCount; ++i) {
                std::string methodName = readString(in);
                Value methodValue = readValue(in);
                
                // Convert Value to VmCallable
                if (std::holds_alternative<std::shared_ptr<VmCallable>>(methodValue)) {
                    methods[methodName] = std::get<std::shared_ptr<VmCallable>>(methodValue);
                } else {
                    throw std::runtime_error("Class method is not a callable");
                }
            }
            
            return std::make_shared<VmClass>(className, fieldNames, fieldDefaults, methods);
        }
        
        case ValueType::ERROR: {
            std::string message = readString(in);
            std::string type = readString(in);
            return std::make_shared<Error>(message, type);
        }
        
        default:
            throw std::runtime_error("Unknown value type tag: " + std::to_string(typeTag));
    }
}

// Main serialization methods
bool ChunkSerializer::serializeToFile(const Chunk& chunk, const std::string& filepath) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out) {
        return false;
    }
    
    try {
        // Write magic number
        out.write(MAGIC, 4);
        
        // Write version
        writeUint32(out, FORMAT_VERSION);
        
        // Write code section
        writeUint32(out, static_cast<uint32_t>(chunk.code.size()));
        for (uint8_t byte : chunk.code) {
            writeUint8(out, byte);
        }
        
        // Write constants section
        writeUint32(out, static_cast<uint32_t>(chunk.constants.size()));
        for (const auto& constant : chunk.constants) {
            writeValue(out, constant);
        }
        
        // Write names section
        writeUint32(out, static_cast<uint32_t>(chunk.names.size()));
        for (const auto& name : chunk.names) {
            writeString(out, name);
        }
        
        return out.good();
    } catch (const std::exception& e) {
        return false;
    }
}

Chunk ChunkSerializer::deserializeFromFile(const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Could not open bytecode file: " + filepath);
    }
    
    // Read and verify magic number
    char magic[4];
    in.read(magic, 4);
    if (!in || std::memcmp(magic, MAGIC, 4) != 0) {
        throw std::runtime_error("Invalid bytecode file: magic number mismatch");
    }
    
    // Read and verify version
    uint32_t version = readUint32(in);
    if (version != FORMAT_VERSION) {
        throw std::runtime_error("Incompatible bytecode version: " + std::to_string(version));
    }
    
    Chunk chunk;
    
    // Read code section
    uint32_t codeSize = readUint32(in);
    chunk.code.reserve(codeSize);
    for (uint32_t i = 0; i < codeSize; ++i) {
        chunk.code.push_back(readUint8(in));
    }
    
    // Read constants section
    uint32_t constCount = readUint32(in);
    chunk.constants.reserve(constCount);
    for (uint32_t i = 0; i < constCount; ++i) {
        chunk.constants.push_back(readValue(in));
    }
    
    // Read names section
    uint32_t namesCount = readUint32(in);
    chunk.names.reserve(namesCount);
    for (uint32_t i = 0; i < namesCount; ++i) {
        chunk.names.push_back(readString(in));
    }
    
    return chunk;
}

} // namespace izi
