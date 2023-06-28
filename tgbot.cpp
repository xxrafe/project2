#include <stdio.h>
#include <tgbot/tgbot.h>
#include <sqlite3.h>
#include <fstream>
#include <algorithm>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind/bind.hpp>
#include <vector>
using namespace boost::placeholders;
using namespace TgBot;
using namespace std;


// Функции для работы с базами данных
static int createDB(const char* s)
{
    sqlite3* DB;
    int exit = 0;

    exit = sqlite3_open(s, &DB);

    sqlite3_close(DB);

    return 0;
}

std::string extractSearchItem(const std::string& input) {
    std::string searchItem;
 
    // Находим позиции открывающей и закрывающей скобок
    size_t openingBracketPos = input.find('(');
    size_t closingBracketPos = input.find(')');
 
    if (openingBracketPos != std::string::npos && closingBracketPos != std::string::npos) {
        // Извлекаем содержимое между скобками
        searchItem = input.substr(openingBracketPos + 1, closingBracketPos - openingBracketPos - 1);
    }
 
    return searchItem;
}

static int createTable(const char* s)
{
    sqlite3* DB;

    std::string sql = "CREATE TABLE IF NOT EXISTS INFO("
    "ID      INT  PRIMARY KEY, "
    "BALANCE INT  NOT NULL, "
    "ADDRESS TEXT NOT NULL );";

    try 
    {
        int exit = 0;
        exit = sqlite3_open(s, &DB);

        char* messaggeError;
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);

        if (exit != SQLITE_OK) {
            cerr << "Error Create Table" <<endl;
            sqlite3_free(messaggeError);
        }
        else
            std::cout << "Table created Successfully" << endl;
        sqlite3_close(DB);
    }
    catch (const exception & e)
    {
        cerr << e.what();
    }

    return 0;
}

static int insertData(int chatid, const char* s, std::string value)
{
    sqlite3* DB;
    char* messaggeError;

    int exit = sqlite3_open(s, &DB);
    // string sql("INSERT INTO INFO (" + data + ") VALUES('" + value + "');");
    string sql("INSERT INTO INFO (ID, BALANCE, ADDRESS) VALUES('" + std::to_string(chatid) + "', '10000', '" + value + "');");

    exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
    if (exit != SQLITE_OK) {
        cerr << "Error Insert" << endl;
        sqlite3_free(messaggeError);
    }
    else
        cout << "Records created Succesfully!" << endl;

    return 0;
}

bool checkUserExists(int userId) {
    sqlite3* db;
    int result = sqlite3_open("USERS.db", &db);
    
    if (result != SQLITE_OK) {
        std::cout << "Ошибка открытия базы данных: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    std::string query = "SELECT COUNT(*) FROM INFO WHERE ID = " + std::to_string(userId) + ";";
    sqlite3_stmt* statement;
    
    result = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, nullptr);
    
    if (result != SQLITE_OK) {
        std::cout << "Ошибка выполнения запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(statement);
        sqlite3_close(db);
        return false;
    }
    
    bool userExists = false;
    
    if (sqlite3_step(statement) == SQLITE_ROW) {
        int count = sqlite3_column_int(statement, 0);
        userExists = (count > 0);
    }
    
    sqlite3_finalize(statement);
    sqlite3_close(db);
    
    return userExists;
}


// Функция для выполнения запроса к базе данных
int executeQuery(const char* query, sqlite3* db, int(*callback)(void*, int, char**, char**), void* data) {
    char* errMsg = nullptr;
    int result = sqlite3_exec(db, query, callback, data, &errMsg);
 
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка выполнения запроса: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
 
    return result;
}

// Функция для изменения значения balance пользователя по ID
void changeBalanceByID(int userID, int balanceChange) {
    sqlite3* db;
    int result = sqlite3_open("USERS.db", &db);
 
    if (result != SQLITE_OK) {
        std::cerr << "Не удалось открыть базу данных" << std::endl;
        return;
    }
 
    std::string query = "UPDATE INFO SET BALANCE = BALANCE + " + std::to_string(balanceChange) + " WHERE ID = " + std::to_string(userID);
 
    result = executeQuery(query.c_str(), db, nullptr, nullptr);
 
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка выполнения запроса" << std::endl;
    } else {
        std::cout << "Баланс пользователя с ID " << userID << " успешно изменен" << std::endl;
    }
 
    sqlite3_close(db);
}

 
// Колбэк-функция для обработки результата запроса
int balanceCallback(void* data, int argc, char** argv, char** columnNames) {
    if (argc > 0) {
        // Получаем значение balance
        std::string balance = argv[0] ? argv[0] : "0";
 
        // Преобразуем значение в число и сохраняем его в переменной, переданной через data
        int* balancePtr = static_cast<int*>(data);
        *balancePtr = std::stoi(balance);
    }
 
    return 0;
}
 
// Функция для получения значения balance пользователя по ID
int getBalanceByID(int userID) {
    sqlite3* db;
    int result = sqlite3_open("USERS.db", &db); // Подставьте имя вашей базы данных
 
    if (result != SQLITE_OK) {
        std::cerr << "Не удалось открыть базу данных" << std::endl;
        return 0; // Или другое значение по умолчанию
    }
 
    std::string query = "SELECT balance FROM INFO WHERE ID = " + std::to_string(userID);
 
    int balance = 0;
    result = executeQuery(query.c_str(), db, balanceCallback, &balance);
 
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка выполнения запроса" << std::endl;
        balance = 0; // Или другое значение по умолчанию
    }
 
    sqlite3_close(db);
    return balance;
}
 

// Колбэк-функция для обработки результата запроса
int addressCallback(void* data, int argc, char** argv, char** columnNames) {
    if (argc > 0) {
        // Получаем значение ADDRESS (предполагается, что оно находится в первом столбце)
        std::string address = argv[0] ? argv[0] : "";
 
        // Сохраняем значение в переменной, переданной через data
        std::string* addressPtr = static_cast<std::string*>(data);
        *addressPtr = address;
    }
 
    return 0;
}
 
// Функция для получения значения ADDRESS по ID
std::string getAddressByID(int userID) {
    sqlite3* db;
    int result = sqlite3_open("USERS.db", &db); // Подставьте имя вашей базы данных
 
    if (result != SQLITE_OK) {
        std::cerr << "Не удалось открыть базу данных" << std::endl;
        return ""; // Или другое значение по умолчанию
    }
 
    std::string query = "SELECT ADDRESS FROM INFO WHERE ID = " + std::to_string(userID);
 
    std::string address = "";
    result = executeQuery(query.c_str(), db, addressCallback, &address);
 
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка выполнения запроса" << std::endl;
        address = ""; // Или другое значение по умолчанию
    }
 
    sqlite3_close(db);
    return address;
}
 

// Функция для сортировки массива строк по алфавиту
std::vector<std::string> sortStrings(const std::vector<std::string>& strings) {
    std::vector<std::string> sortedStrings = strings;
 
    // Используем std::sort с лямбда-функцией в качестве функции сравнения
    std::sort(sortedStrings.begin(), sortedStrings.end(), [](const std::string& a, const std::string& b) {
        return a < b;
    });
 
    return sortedStrings;
}

// Функция для сортировки товаров из txt файла
std::string SortProducts(std::string filepath, int case1){
    std::string result = "";
    std::string finalresult = "";
    vector <std::string> product;
    vector <int> productprice;
    fstream fin;
    fin.open(filepath);
    std::string s1, s2;
    while (!fin.eof())
    {
        fin >> s1;
        fin >> s2;
        std::cout << s1 << " " << s2 << endl;
        product.push_back(s1);
        productprice.push_back(atoi(s2.c_str()));
    }
    fin.close();
    if (case1==1)
    {
        for (int i = 0; i < productprice.size() - 1; i++) {
            for (int j = 0; j < productprice.size() - i - 1; j++) {
                if (productprice[j] > productprice[j + 1]) {
                    // меняем элементы местами
                    int temp1 = productprice[j];
                    std::string temp2 = product[j];
                    product[j] = product[j+1];
                    product[j+1] = temp2;
                    productprice[j] = productprice[j + 1];
                    productprice[j + 1] = temp1;
                }
            }
        }
        for (int i = 0; i < product.size()-1; i++){
            result = result + product[i] + " - " + std::to_string(productprice[i]) + " RUB\n";
        }
        finalresult = result;
    }
    if (case1==0)
    {
        std::vector <std::string> output;
        
        for (int i = 0; i < product.size()-1; i++){
            result = product[i] + " - " + std::to_string(productprice[i]) + " RUB\n";
            output.push_back(result);
            cout << result;
        }
        sortStrings(output);
        for (int i; i < output.size()-1; i++)
        {
            finalresult = finalresult + output[i];
        }
    }
    return finalresult;
}

// Функция для обработки команды "/start"
void handleStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Добро пожаловать!\nКоманды:\n/balance - проверить баланс\n/products - посмотреть список товаров\n/pay - пополнить баланс на 1000 RUB\nДля покупки товара введите <купить (Товар)>\nДля поиска товара введите <поиск (Товар)>\n\nДля доставки товаров введите город проживания:");
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/") or (StringTools::startsWith(message->text, "/"))) {
            return;
        }
        int chatid = message->chat->id;
        if (!checkUserExists(chatid)){
            std::string city = message->text;
            int chatid = message->chat->id;
            insertData(chatid, "USERS.db", city);
            bot.getApi().sendMessage(message->chat->id, "Город в котором вы проживаете: " + city);
        }
        return;
    });
    return;
}
 
// Функция для обработки команды "/balance"
void handleBalanceCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    // Здесь необходимо получить баланс пользователя из базы данных и отправить его в сообщении
    int chatid = message->chat->id;
    std::string balance = std::to_string(getBalanceByID(chatid));
    bot.getApi().sendMessage(message->chat->id, "Ваш баланс: " + balance);
}

// Функция для обработки команды "/pay"
void handlePayCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    int chatid = message->chat->id;
    changeBalanceByID(chatid, 1000);
    int balance = getBalanceByID(chatid);
    bot.getApi().sendMessage(message->chat->id, "Вы пополнили баланс на 1000 RUB\nВаш баланс: " + std::to_string(balance) + " RUB");
}
 
// Функция для обработки команды "/products"
void handleProductsCommand(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    std::string products = "";
    products = "Вот список товаров:\n" + SortProducts("products.txt", 1);
    bot.getApi().sendMessage(message->chat->id, products);
}
 
 
// Функция для обработки текстовых сообщений (поиск и покупка товаров)
void handleTextMessage(TgBot::Bot& bot, TgBot::Message::Ptr message)
{
    std::string text = message->text;
    std::string product = "";

    if (StringTools::startsWith(message->text, "поиск")){
        product = extractSearchItem(text);
        std::cout << product;
        fstream fin;
        fin.open("products.txt");
        std::string s1, s2;
        int ifFound = 0;
        while (!fin.eof() and product != s1)
        {
            fin >> s1;
            fin >> s2;
            if (product == s1){
                ifFound += 1;
            }
        }
        if (ifFound == 0){
            bot.getApi().sendMessage(message->chat->id, "Товар не найден!");
        } else {
            bot.getApi().sendMessage(message->chat->id, "Товар: " + s1 + "\nцена товара: " + s2);
        }
    }
    if (StringTools::startsWith(message->text, "купить")){
        product = extractSearchItem(text);
        std::cout << product;
        fstream fin;
        fin.open("products.txt");
        std::string s1, s2;
        int ifFound = 0;
        while (!fin.eof() and product != s1)
        {
            fin >> s1;
            fin >> s2;
            if (product == s1){
                ifFound += 1;
            }
        }
        if (ifFound == 0){
            bot.getApi().sendMessage(message->chat->id, "Товар не найден!");
        } else {
            changeBalanceByID(message->chat->id, -atoi(s2.c_str()));
            bot.getApi().sendMessage(message->chat->id, "Товар успешно куплен! Товар доставят по адресу г. " + getAddressByID(message->chat->id));
        }
    }
}

int main()
{
    // Инициализация базы данных
 
    // Токен бота
    TgBot::Bot bot("6134520934:AAFw27MZL3WP0y26extR9VDrjNqJFZC9L5o");
    
    //Работа с базами данных
    const char* dir = "USERS.db"; 
    sqlite3* DB;
    createDB(dir);
    createTable(dir);

    

    // Установка обработчиков команд
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) { handleStartCommand(bot, message); });
    bot.getEvents().onCommand("balance", [&bot](TgBot::Message::Ptr message) { handleBalanceCommand(bot, message); });
    bot.getEvents().onCommand("products", [&bot](TgBot::Message::Ptr message) { handleProductsCommand(bot, message); });
    bot.getEvents().onCommand("pay", [&bot](TgBot::Message::Ptr message) { handlePayCommand(bot, message); });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) { handleTextMessage(bot, message); });
 
    // Запуск бота
    try {
        bot.getApi().deleteWebhook();
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        std::cerr << "Telegram bot error: " << e.what() << std::endl;
    }
 
    return 0;
}