#include "disassembler.hpp"
#include "common/value.hpp"
#include <iomanip>
#include <sstream>

namespace izi {

void Disassembler::disassemble(const Chunk& chunk, const std::string& name, std::ostream& out) {
    out << "== " << name << " ==\n";
    for (size_t offset = 0; offset < chunk.code.size();) {
        offset = disassembleInstruction(chunk, offset, out);
    }
}

// Helper: format an offset as a 4-digit zero-padded decimal.
static std::string fmtOffset(size_t offset) {
    std::ostringstream ss;
    ss << std::setw(4) << std::setfill('0') << offset;
    return ss.str();
}

// Helper: format a line number column.
static std::string fmtLine(const Chunk& chunk, size_t offset) {
    int line = chunk.getLine(offset);
    if (line <= 0) return "   |";
    std::ostringstream ss;
    ss << std::setw(4) << line;
    return ss.str();
}

// Helper: print a simple (no-operand) instruction.
static size_t simpleInstruction(const std::string& name, size_t offset, std::ostream& out) {
    out << name << '\n';
    return offset + 1;
}

// Helper: print an instruction with a single byte operand (e.g. constant index).
static size_t byteInstruction(const std::string& name, const Chunk& chunk, size_t offset,
                               std::ostream& out) {
    uint8_t operand = chunk.code[offset + 1];
    out << std::left << std::setw(20) << name << " " << static_cast<int>(operand) << '\n';
    return offset + 2;
}

// Helper: print a CONSTANT instruction showing the constant value.
static size_t constantInstruction(const std::string& name, const Chunk& chunk, size_t offset,
                                  std::ostream& out) {
    uint8_t idx = chunk.code[offset + 1];
    out << std::left << std::setw(20) << name << " " << static_cast<int>(idx);
    if (idx < chunk.constants.size()) {
        out << " (";
        // Format the constant value inline
        const Value& v = chunk.constants[idx];
        if (std::holds_alternative<double>(v)) {
            out << std::get<double>(v);
        } else if (std::holds_alternative<bool>(v)) {
            out << (std::get<bool>(v) ? "true" : "false");
        } else if (std::holds_alternative<std::string>(v)) {
            out << '"' << std::get<std::string>(v) << '"';
        } else if (std::holds_alternative<Nil>(v)) {
            out << "nil";
        }
        out << ")";
    }
    out << '\n';
    return offset + 2;
}

// Helper: print a named instruction (GET/SET_GLOBAL, GET/SET_PROPERTY, etc.).
static size_t namedInstruction(const std::string& name, const Chunk& chunk, size_t offset,
                                std::ostream& out) {
    uint8_t idx = chunk.code[offset + 1];
    std::string varName = (idx < chunk.names.size()) ? chunk.names[idx] : "?";
    out << std::left << std::setw(20) << name << " '" << varName << "'\n";
    return offset + 2;
}

// Helper: print a jump instruction with the computed target.
static size_t jumpInstruction(const std::string& name, int sign, const Chunk& chunk, size_t offset,
                               std::ostream& out) {
    uint16_t jump = (static_cast<uint16_t>(chunk.code[offset + 1]) << 8) |
                    static_cast<uint16_t>(chunk.code[offset + 2]);
    size_t target = static_cast<size_t>(static_cast<int>(offset + 3) + sign * jump);
    out << std::left << std::setw(20) << name << " " << fmtOffset(offset + 3) << " -> "
        << fmtOffset(target) << '\n';
    return offset + 3;
}

size_t Disassembler::disassembleInstruction(const Chunk& chunk, size_t offset, std::ostream& out) {
    out << fmtOffset(offset) << " " << fmtLine(chunk, offset) << " ";

    if (offset >= chunk.code.size()) {
        out << "<end of chunk>\n";
        return offset + 1;
    }

    auto op = static_cast<OpCode>(chunk.code[offset]);
    switch (op) {
        case OpCode::CONSTANT:
            return constantInstruction("CONSTANT", chunk, offset, out);
        case OpCode::NIL:
            return simpleInstruction("NIL", offset, out);
        case OpCode::TRUE:
            return simpleInstruction("TRUE", offset, out);
        case OpCode::FALSE:
            return simpleInstruction("FALSE", offset, out);
        case OpCode::ADD:
            return simpleInstruction("ADD", offset, out);
        case OpCode::SUBTRACT:
            return simpleInstruction("SUBTRACT", offset, out);
        case OpCode::MULTIPLY:
            return simpleInstruction("MULTIPLY", offset, out);
        case OpCode::DIVIDE:
            return simpleInstruction("DIVIDE", offset, out);
        case OpCode::MODULO:
            return simpleInstruction("MODULO", offset, out);
        case OpCode::NEGATE:
            return simpleInstruction("NEGATE", offset, out);
        case OpCode::EQUAL:
            return simpleInstruction("EQUAL", offset, out);
        case OpCode::NOT_EQUAL:
            return simpleInstruction("NOT_EQUAL", offset, out);
        case OpCode::GREATER:
            return simpleInstruction("GREATER", offset, out);
        case OpCode::GREATER_EQUAL:
            return simpleInstruction("GREATER_EQUAL", offset, out);
        case OpCode::LESS:
            return simpleInstruction("LESS", offset, out);
        case OpCode::LESS_EQUAL:
            return simpleInstruction("LESS_EQUAL", offset, out);
        case OpCode::NOT:
            return simpleInstruction("NOT", offset, out);
        case OpCode::GET_GLOBAL:
            return namedInstruction("GET_GLOBAL", chunk, offset, out);
        case OpCode::SET_GLOBAL:
            return namedInstruction("SET_GLOBAL", chunk, offset, out);
        case OpCode::GET_LOCAL:
            return byteInstruction("GET_LOCAL", chunk, offset, out);
        case OpCode::SET_LOCAL:
            return byteInstruction("SET_LOCAL", chunk, offset, out);
        case OpCode::INDEX:
            return simpleInstruction("INDEX", offset, out);
        case OpCode::SET_INDEX:
            return simpleInstruction("SET_INDEX", offset, out);
        case OpCode::JUMP:
            return jumpInstruction("JUMP", 1, chunk, offset, out);
        case OpCode::JUMP_IF_FALSE:
            return jumpInstruction("JUMP_IF_FALSE", 1, chunk, offset, out);
        case OpCode::LOOP:
            return jumpInstruction("LOOP", -1, chunk, offset, out);
        case OpCode::CALL:
            return byteInstruction("CALL", chunk, offset, out);
        case OpCode::RETURN:
            return simpleInstruction("RETURN", offset, out);
        case OpCode::POP:
            return simpleInstruction("POP", offset, out);
        case OpCode::PRINT:
            return simpleInstruction("PRINT", offset, out);
        case OpCode::TRY: {
            // TRY opcode layout: catch-offset (2 bytes) + finally-offset (2 bytes) +
            // catch-var index (1 byte).  Display each field for debugging clarity.
            uint16_t catchOff = (static_cast<uint16_t>(chunk.code[offset + 1]) << 8) |
                                 static_cast<uint16_t>(chunk.code[offset + 2]);
            uint16_t finallyOff = (static_cast<uint16_t>(chunk.code[offset + 3]) << 8) |
                                   static_cast<uint16_t>(chunk.code[offset + 4]);
            uint8_t catchVar = chunk.code[offset + 5];
            std::string catchVarName =
                (catchVar < chunk.names.size()) ? chunk.names[catchVar] : "?";
            out << std::left << std::setw(20) << "TRY"
                << " catch->" << fmtOffset(offset + 6 + catchOff)
                << " finally->" << fmtOffset(offset + 6 + finallyOff)
                << " var='" << catchVarName << "'\n";
            return offset + 6;
        }
        case OpCode::THROW:
            return simpleInstruction("THROW", offset, out);
        case OpCode::END_TRY:
            return simpleInstruction("END_TRY", offset, out);
        case OpCode::GET_PROPERTY:
            return namedInstruction("GET_PROPERTY", chunk, offset, out);
        case OpCode::SET_PROPERTY:
            return namedInstruction("SET_PROPERTY", chunk, offset, out);
        case OpCode::GET_SUPER_METHOD:
            return namedInstruction("GET_SUPER_METHOD", chunk, offset, out);
        case OpCode::INHERIT:
            return simpleInstruction("INHERIT", offset, out);
        case OpCode::LOAD_MODULE:
            return namedInstruction("LOAD_MODULE", chunk, offset, out);
        default:
            out << "UNKNOWN(" << static_cast<int>(chunk.code[offset]) << ")\n";
            return offset + 1;
    }
}

}  // namespace izi
