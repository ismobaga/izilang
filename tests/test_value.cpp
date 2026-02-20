#include "catch.hpp"
#include "common/value.hpp"

using namespace izi;

TEST_CASE("Value types can be created and checked", "[value]") {
    SECTION("Nil value") {
        Value v = Nil{};
        REQUIRE(std::holds_alternative<Nil>(v));
        REQUIRE(getTypeName(v) == "nil");
    }

    SECTION("Boolean value") {
        Value v = true;
        REQUIRE(std::holds_alternative<bool>(v));
        REQUIRE(std::get<bool>(v) == true);
        REQUIRE(getTypeName(v) == "boolean");
    }

    SECTION("Number value") {
        Value v = 42.5;
        REQUIRE(std::holds_alternative<double>(v));
        REQUIRE(std::get<double>(v) == 42.5);
        REQUIRE(getTypeName(v) == "number");
    }

    SECTION("String value") {
        Value v = std::string("hello");
        REQUIRE(std::holds_alternative<std::string>(v));
        REQUIRE(std::get<std::string>(v) == "hello");
        REQUIRE(getTypeName(v) == "string");
    }
}

TEST_CASE("isTruthy function works correctly", "[value]") {
    SECTION("Nil is falsy") {
        Value v = Nil{};
        REQUIRE(isTruthy(v) == false);
    }

    SECTION("Boolean values") {
        Value vTrue = true;
        Value vFalse = false;
        REQUIRE(isTruthy(vTrue) == true);
        REQUIRE(isTruthy(vFalse) == false);
    }

    SECTION("Number values") {
        Value vZero = 0.0;
        Value vNonZero = 42.0;
        REQUIRE(isTruthy(vZero) == false);
        REQUIRE(isTruthy(vNonZero) == true);
    }

    SECTION("String values") {
        Value vEmpty = std::string("");
        Value vNonEmpty = std::string("hello");
        REQUIRE(isTruthy(vEmpty) == false);
        REQUIRE(isTruthy(vNonEmpty) == true);
    }
}

TEST_CASE("asNumber function works correctly", "[value]") {
    SECTION("Extracts number from number value") {
        Value v = 123.456;
        REQUIRE(asNumber(v) == 123.456);
    }

    SECTION("Throws exception for non-number value") {
        Value v = std::string("not a number");
        REQUIRE_THROWS(asNumber(v));
    }
}

TEST_CASE("Array values work correctly", "[value]") {
    SECTION("Create and access array") {
        auto arr = std::make_shared<Array>();
        arr->elements.push_back(1.0);
        arr->elements.push_back(2.0);
        arr->elements.push_back(3.0);

        Value v = arr;
        REQUIRE(std::holds_alternative<std::shared_ptr<Array>>(v));
        REQUIRE(getTypeName(v) == "array");

        auto extractedArr = std::get<std::shared_ptr<Array>>(v);
        REQUIRE(extractedArr->elements.size() == 3);
        REQUIRE(std::get<double>(extractedArr->elements[0]) == 1.0);
        REQUIRE(std::get<double>(extractedArr->elements[1]) == 2.0);
        REQUIRE(std::get<double>(extractedArr->elements[2]) == 3.0);
    }

    SECTION("Empty array is falsy") {
        auto arr = std::make_shared<Array>();
        Value v = arr;
        REQUIRE(isTruthy(v) == false);
    }

    SECTION("Non-empty array is truthy") {
        auto arr = std::make_shared<Array>();
        arr->elements.push_back(1.0);
        Value v = arr;
        REQUIRE(isTruthy(v) == true);
    }
}

TEST_CASE("Map values work correctly", "[value]") {
    SECTION("Create and access map") {
        auto map = std::make_shared<Map>();
        map->entries["key1"] = 10.0;
        map->entries["key2"] = std::string("value");

        Value v = map;
        REQUIRE(std::holds_alternative<std::shared_ptr<Map>>(v));
        REQUIRE(getTypeName(v) == "map");

        auto extractedMap = std::get<std::shared_ptr<Map>>(v);
        REQUIRE(extractedMap->entries.size() == 2);
        REQUIRE(std::get<double>(extractedMap->entries["key1"]) == 10.0);
        REQUIRE(std::get<std::string>(extractedMap->entries["key2"]) == "value");
    }

    SECTION("Empty map is falsy") {
        auto map = std::make_shared<Map>();
        Value v = map;
        REQUIRE(isTruthy(v) == false);
    }

    SECTION("Non-empty map is truthy") {
        auto map = std::make_shared<Map>();
        map->entries["key"] = 1.0;
        Value v = map;
        REQUIRE(isTruthy(v) == true);
    }
}
