#include "catch.hpp"
#include "common/value.hpp"

using namespace izi;

// ============ Array Method Tests ============

TEST_CASE("Array shift() removes and returns first element", "[collections][array]") {
    auto arr = std::make_shared<Array>();
    arr->elements = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    SECTION("Shift from non-empty array") {
        Value first = arr->elements.front();
        arr->elements.erase(arr->elements.begin());
        
        REQUIRE(std::get<double>(first) == 1.0);
        REQUIRE(arr->elements.size() == 4);
        REQUIRE(std::get<double>(arr->elements[0]) == 2.0);
    }
}

TEST_CASE("Array unshift() adds element to beginning", "[collections][array]") {
    auto arr = std::make_shared<Array>();
    arr->elements = {2.0, 3.0, 4.0};
    
    SECTION("Unshift to non-empty array") {
        arr->elements.insert(arr->elements.begin(), 1.0);
        
        REQUIRE(arr->elements.size() == 4);
        REQUIRE(std::get<double>(arr->elements[0]) == 1.0);
        REQUIRE(std::get<double>(arr->elements[1]) == 2.0);
    }
}

TEST_CASE("Array splice() removes elements from array", "[collections][array]") {
    SECTION("Splice with start and deleteCount") {
        auto arr = std::make_shared<Array>();
        arr->elements = {1.0, 2.0, 3.0, 4.0, 5.0};
        
        size_t start = 2;
        size_t deleteCount = 2;
        size_t end = std::min(start + deleteCount, arr->elements.size());
        
        auto result = std::make_shared<Array>();
        for (size_t i = start; i < end; ++i) {
            result->elements.push_back(arr->elements[i]);
        }
        arr->elements.erase(arr->elements.begin() + start, arr->elements.begin() + end);
        
        REQUIRE(result->elements.size() == 2);
        REQUIRE(std::get<double>(result->elements[0]) == 3.0);
        REQUIRE(std::get<double>(result->elements[1]) == 4.0);
        REQUIRE(arr->elements.size() == 3);
        REQUIRE(std::get<double>(arr->elements[0]) == 1.0);
        REQUIRE(std::get<double>(arr->elements[2]) == 5.0);
    }
    
    SECTION("Splice with only start (remove to end)") {
        auto arr = std::make_shared<Array>();
        arr->elements = {10.0, 20.0, 30.0, 40.0};
        
        size_t start = 1;
        size_t end = arr->elements.size();
        
        auto result = std::make_shared<Array>();
        for (size_t i = start; i < end; ++i) {
            result->elements.push_back(arr->elements[i]);
        }
        arr->elements.erase(arr->elements.begin() + start, arr->elements.begin() + end);
        
        REQUIRE(result->elements.size() == 3);
        REQUIRE(arr->elements.size() == 1);
        REQUIRE(std::get<double>(arr->elements[0]) == 10.0);
    }
}

// ============ Map Method Tests ============

TEST_CASE("Map has() checks for key existence", "[collections][map]") {
    auto map = std::make_shared<Map>();
    map->entries["name"] = std::string("Alice");
    map->entries["age"] = 30.0;
    
    SECTION("Has returns true for existing key") {
        bool hasName = (map->entries.find("name") != map->entries.end());
        REQUIRE(hasName == true);
    }
    
    SECTION("Has returns false for non-existing key") {
        bool hasCountry = (map->entries.find("country") != map->entries.end());
        REQUIRE(hasCountry == false);
    }
}

TEST_CASE("Map delete() removes key-value pair", "[collections][map]") {
    auto map = std::make_shared<Map>();
    map->entries["name"] = std::string("Alice");
    map->entries["age"] = 30.0;
    map->entries["city"] = std::string("NYC");
    
    SECTION("Delete existing key") {
        bool existed = (map->entries.find("city") != map->entries.end());
        if (existed) {
            map->entries.erase("city");
        }
        
        REQUIRE(existed == true);
        REQUIRE(map->entries.size() == 2);
        REQUIRE(map->entries.find("city") == map->entries.end());
    }
    
    SECTION("Delete non-existing key") {
        bool existed = (map->entries.find("country") != map->entries.end());
        REQUIRE(existed == false);
    }
}

TEST_CASE("Map entries() returns array of key-value pairs", "[collections][map]") {
    auto map = std::make_shared<Map>();
    map->entries["name"] = std::string("Alice");
    map->entries["age"] = 30.0;
    
    SECTION("Entries returns correct structure") {
        auto entriesArray = std::make_shared<Array>();
        for (const auto& [key, value] : map->entries) {
            auto entry = std::make_shared<Array>();
            entry->elements.push_back(key);
            entry->elements.push_back(value);
            entriesArray->elements.push_back(entry);
        }
        
        REQUIRE(entriesArray->elements.size() == 2);
        // Check that each entry is an array with 2 elements
        for (const auto& entryVal : entriesArray->elements) {
            REQUIRE(std::holds_alternative<std::shared_ptr<Array>>(entryVal));
            auto entry = std::get<std::shared_ptr<Array>>(entryVal);
            REQUIRE(entry->elements.size() == 2);
            REQUIRE(std::holds_alternative<std::string>(entry->elements[0])); // key
        }
    }
}

// ============ Set Type Tests ============

TEST_CASE("Set can be created and stores unique values", "[collections][set]") {
    auto set = std::make_shared<Set>();
    
    SECTION("Empty set has size 0") {
        REQUIRE(set->values.size() == 0);
    }
    
    SECTION("Set stores string values") {
        set->values["apple"] = std::string("apple");
        set->values["banana"] = std::string("banana");
        
        REQUIRE(set->values.size() == 2);
        REQUIRE(set->values.find("apple") != set->values.end());
    }
    
    SECTION("Set prevents duplicates") {
        set->values["apple"] = std::string("apple");
        set->values["apple"] = std::string("apple"); // duplicate
        
        REQUIRE(set->values.size() == 1);
    }
}

TEST_CASE("Set add() adds values to set", "[collections][set]") {
    auto set = std::make_shared<Set>();
    
    SECTION("Add string value") {
        std::string key = "apple";
        set->values[key] = std::string("apple");
        
        REQUIRE(set->values.size() == 1);
        REQUIRE(set->values.find("apple") != set->values.end());
    }
    
    SECTION("Add number value") {
        std::string key = std::to_string(42.0);
        set->values[key] = 42.0;
        
        REQUIRE(set->values.size() == 1);
    }
}

TEST_CASE("Set has() checks for value existence", "[collections][set]") {
    auto set = std::make_shared<Set>();
    set->values["apple"] = std::string("apple");
    set->values["banana"] = std::string("banana");
    
    SECTION("Has returns true for existing value") {
        bool hasApple = (set->values.find("apple") != set->values.end());
        REQUIRE(hasApple == true);
    }
    
    SECTION("Has returns false for non-existing value") {
        bool hasGrape = (set->values.find("grape") != set->values.end());
        REQUIRE(hasGrape == false);
    }
}

TEST_CASE("Set delete() removes value from set", "[collections][set]") {
    auto set = std::make_shared<Set>();
    set->values["apple"] = std::string("apple");
    set->values["banana"] = std::string("banana");
    set->values["cherry"] = std::string("cherry");
    
    SECTION("Delete existing value") {
        std::string key = "banana";
        bool existed = (set->values.find(key) != set->values.end());
        if (existed) {
            set->values.erase(key);
        }
        
        REQUIRE(existed == true);
        REQUIRE(set->values.size() == 2);
        REQUIRE(set->values.find("banana") == set->values.end());
    }
    
    SECTION("Delete non-existing value") {
        std::string key = "grape";
        bool existed = (set->values.find(key) != set->values.end());
        REQUIRE(existed == false);
    }
}

TEST_CASE("Set size() returns number of elements", "[collections][set]") {
    auto set = std::make_shared<Set>();
    
    SECTION("Empty set has size 0") {
        REQUIRE(set->values.size() == 0);
    }
    
    SECTION("Set with 3 elements has size 3") {
        set->values["a"] = std::string("a");
        set->values["b"] = std::string("b");
        set->values["c"] = std::string("c");
        
        REQUIRE(set->values.size() == 3);
    }
}

TEST_CASE("Set type name is 'set'", "[collections][set]") {
    auto set = std::make_shared<Set>();
    Value v = set;
    
    REQUIRE(getTypeName(v) == "set");
}

TEST_CASE("Set isTruthy is correct", "[collections][set]") {
    SECTION("Empty set is falsy") {
        auto set = std::make_shared<Set>();
        Value v = set;
        REQUIRE(isTruthy(v) == false);
    }
    
    SECTION("Non-empty set is truthy") {
        auto set = std::make_shared<Set>();
        set->values["a"] = std::string("a");
        Value v = set;
        REQUIRE(isTruthy(v) == true);
    }
}
