#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <tgbot/tgbot.h>
#include <sqlite3.h>
#include "tgbot.cpp"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind/bind.hpp>
#include <vector>

TEST_CASE("Test createDB") {
    // Тестирование функции createDB
    CHECK(createDB("test.db") == 0);
}

TEST_CASE("Test createDB") {
    // Тестирование функции createDB
    CHECK(createDB("test.db") == 1);
}

TEST_CASE("Test extractSearchItem") {
    // Тестирование функции extractSearchItem
    std::string input = "search(text)";
    std::string expected = "text";
    CHECK(extractSearchItem(input) == expected);
}

TEST_CASE("Test extractSearchItem") {
    // Тестирование функции extractSearchItem
    std::string input = "search ddwfoutputfefef";
    std::string expected = "output";
    CHECK(extractSearchItem(input) != expected);
}


TEST_CASE("Test changeBalanceByID") {
    // Тестирование функции changeBalanceByID
    int userID = 1;
    int balanceChange = 500;

    changeBalanceByID(userID, balanceChange);
    int newBalance = getBalanceByID(userID);
    CHECK(newBalance == 1500);
}

TEST_CASE("Test changeBalanceByID") {
    // Тестирование функции changeBalanceByID
    int userID = 1;
    int balanceChange = 800;

    changeBalanceByID(userID, balanceChange);
    int newBalance = getBalanceByID(userID);
    CHECK(newBalance != 1800);
}
