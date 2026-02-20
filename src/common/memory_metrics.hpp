#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <iostream>

namespace izi {

// Simple memory metrics tracker for debug mode
class MemoryMetrics {
   public:
    static MemoryMetrics& getInstance() {
        static MemoryMetrics instance;
        return instance;
    }

    void recordAllocation(const std::string& type, size_t size) {
        totalAllocations_++;
        totalBytesAllocated_ += size;
        allocationsByType_[type]++;
        bytesByType_[type] += size;
        currentBytes_ += size;
        if (currentBytes_ > peakBytes_) {
            peakBytes_ = currentBytes_;
        }
    }

    void recordDeallocation(const std::string& type, size_t size) {
        totalDeallocations_++;
        currentBytes_ -= size;
    }

    void reset() {
        totalAllocations_ = 0;
        totalDeallocations_ = 0;
        totalBytesAllocated_ = 0;
        currentBytes_ = 0;
        peakBytes_ = 0;
        allocationsByType_.clear();
        bytesByType_.clear();
    }

    void printReport() const {
        std::cout << "\n╔════════════════════════════════════════╗\n";
        std::cout << "║     Memory Usage Report (Debug)       ║\n";
        std::cout << "╚════════════════════════════════════════╝\n\n";

        std::cout << "Overall Statistics:\n";
        std::cout << "  Total allocations:   " << totalAllocations_ << "\n";
        std::cout << "  Total deallocations: " << totalDeallocations_ << "\n";
        std::cout << "  Active allocations:  " << (totalAllocations_ - totalDeallocations_) << "\n";
        std::cout << "  Total bytes allocated: " << formatBytes(totalBytesAllocated_) << "\n";
        std::cout << "  Current memory usage:  " << formatBytes(currentBytes_) << "\n";
        std::cout << "  Peak memory usage:     " << formatBytes(peakBytes_) << "\n\n";

        if (!allocationsByType_.empty()) {
            std::cout << "Allocations by Type:\n";
            for (const auto& [type, count] : allocationsByType_) {
                size_t bytes = bytesByType_.at(type);
                std::cout << "  " << type << ": " << count << " allocations, " << formatBytes(bytes) << "\n";
            }
            std::cout << "\n";
        }
    }

    size_t getTotalAllocations() const { return totalAllocations_; }
    size_t getCurrentBytes() const { return currentBytes_; }
    size_t getPeakBytes() const { return peakBytes_; }

   private:
    MemoryMetrics() = default;

    std::string formatBytes(size_t bytes) const {
        if (bytes < 1024) {
            return std::to_string(bytes) + " bytes";
        } else if (bytes < 1024 * 1024) {
            return std::to_string(bytes / 1024) + " KB";
        } else {
            return std::to_string(bytes / (1024 * 1024)) + " MB";
        }
    }

    size_t totalAllocations_ = 0;
    size_t totalDeallocations_ = 0;
    size_t totalBytesAllocated_ = 0;
    size_t currentBytes_ = 0;
    size_t peakBytes_ = 0;
    std::unordered_map<std::string, size_t> allocationsByType_;
    std::unordered_map<std::string, size_t> bytesByType_;
};

// RAII helper for tracking memory allocations
class MemoryAllocationTracker {
   public:
    MemoryAllocationTracker(const std::string& type, size_t size, bool enabled)
        : type_(type), size_(size), enabled_(enabled) {
        if (enabled_) {
            MemoryMetrics::getInstance().recordAllocation(type_, size_);
        }
    }

    ~MemoryAllocationTracker() {
        if (enabled_) {
            MemoryMetrics::getInstance().recordDeallocation(type_, size_);
        }
    }

   private:
    std::string type_;
    size_t size_;
    bool enabled_;
};

}  // namespace izi
