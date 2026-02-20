#pragma once

#include "chunk.hpp"
#include <string>
#include <fstream>

namespace izi {

/**
 * ChunkSerializer - Serialize and deserialize bytecode chunks to/from binary format
 *
 * Binary format (.izb file):
 * - Magic number: "IZB\0" (4 bytes)
 * - Version: uint32_t (4 bytes)
 * - Code section size: uint32_t (4 bytes)
 * - Code bytes: uint8_t[] (variable)
 * - Constants section size: uint32_t (4 bytes)
 * - Constants: serialized Value[] (variable)
 * - Names section size: uint32_t (4 bytes)
 * - Names: serialized string[] (variable)
 */
class ChunkSerializer {
   public:
    /**
     * Serialize a chunk to a binary file
     * @param chunk The chunk to serialize
     * @param filepath Path to output .izb file
     * @return true if successful, false otherwise
     */
    static bool serializeToFile(const Chunk& chunk, const std::string& filepath);

    /**
     * Deserialize a chunk from a binary file
     * @param filepath Path to .izb file
     * @return The deserialized chunk
     * @throws std::runtime_error if file cannot be read or format is invalid
     */
    static Chunk deserializeFromFile(const std::string& filepath);

   private:
    // Binary format version
    static constexpr uint32_t FORMAT_VERSION = 1;
    static constexpr char MAGIC[4] = {'I', 'Z', 'B', '\0'};

    // Value type tags for serialization
    enum class ValueType : uint8_t {
        NIL = 0,
        BOOL = 1,
        NUMBER = 2,
        STRING = 3,
        ARRAY = 4,
        MAP = 5,
        SET = 6,
        FUNCTION = 7,  // VmUserFunction
        NATIVE_FUNCTION = 8,  // VmNativeFunction
        CLASS = 9,  // VmClass
        INSTANCE = 10,  // Instance
        ERROR = 11  // Error
    };

    // Helper methods for writing
    static void writeUint32(std::ofstream& out, uint32_t value);
    static void writeUint8(std::ofstream& out, uint8_t value);
    static void writeString(std::ofstream& out, const std::string& str);
    static void writeValue(std::ofstream& out, const Value& value);

    // Helper methods for reading
    static uint32_t readUint32(std::ifstream& in);
    static uint8_t readUint8(std::ifstream& in);
    static std::string readString(std::ifstream& in);
    static Value readValue(std::ifstream& in);
};

}  // namespace izi
