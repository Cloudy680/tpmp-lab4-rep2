#include "Auction.h"
#include "Database.h"
#include "Lot.h"
#include "Farm.h"
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>

bool AuctionManager::validateAuctionResult(const AuctionResult& result) {
    if (result.soldUnits <= 0) {
        std::cerr << "Ошибка: количество проданных единиц должно быть > 0" << std::endl;
        return false;
    }
    if (result.sellingPrice <= 0) {
        std::cerr << "Ошибка: продажная цена должна быть > 0" << std::endl;
        return false;
    }
    if (!(result.buyerCategory == "fur factory" ||
          result.buyerCategory == "studio" ||
          result.buyerCategory == "private individual")) {
        std::cerr << "Ошибка: некорректная категория покупателя" << std::endl;
        return false;
    }

    // Проверка, что количество проданных единиц не превышает заявленное
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "SELECT number_units FROM Exhibited_fur WHERE furfarm_number = " << result.farmId
        << " AND fur_name = '" << result.furName << "' AND fur_type = '" << result.furType << "';";
    
    int available = 0;
    auto callback = [&available](int argc, char** argv, char** colName) -> int {
        if (argc >= 1) available = std::stoi(argv[0]);
        return 0;
    };
    if (!db.query(sql.str(), callback)) return false;
    
    if (available == 0) {
        std::cerr << "Ошибка: такой лот не найден!" << std::endl;
        return false;
    }
    if (result.soldUnits > available) {
        std::cerr << "Ошибка: количество проданных единиц (" << result.soldUnits
                  << ") превышает заявленное (" << available << ")" << std::endl;
        return false;
    }
    return true;
}

int AuctionManager::addAuctionResult(const AuctionResult& result) {
    if (!validateAuctionResult(result)) return -1;
    
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "INSERT INTO Auction_Results (furfarm_number, fur_name, fur_type, number_of_units_sold, selling_price, buyer_category) VALUES ("
        << result.farmId << ", '" << result.furName << "', '" << result.furType << "', "
        << result.soldUnits << ", " << result.sellingPrice << ", '" << result.buyerCategory << "');";
    
    if (!db.execute(sql.str())) return -1;
    
    // Обновляем количество оставшихся единиц в Exhibited_fur (уменьшаем)
    std::stringstream updateSql;
    updateSql << "UPDATE Exhibited_fur SET number_units = number_units - " << result.soldUnits
              << " WHERE furfarm_number = " << result.farmId
              << " AND fur_name = '" << result.furName << "' AND fur_type = '" << result.furType << "';";
    db.execute(updateSql.str());
    
    return static_cast<int>(sqlite3_last_insert_rowid(db.getHandle()));
}

// Отчёт 1: ферма с самой высокой ценой меха (максимальная selling_price)
void AuctionManager::reportFarmWithHighestPrice() {
    Database& db = Database::getInstance();
    std::string sql = R"(
        SELECT f.id, f.adress, f.director_surname, MAX(ar.selling_price) as max_price
        FROM Auction_Results ar
        JOIN Furfarms f ON ar.furfarm_number = f.id
        GROUP BY f.id
        ORDER BY max_price DESC
        LIMIT 1;
    )";
    
    auto callback = [](int argc, char** argv, char** colName) -> int {
        if (argc >= 4) {
            std::cout << "Ферма с самой высокой ценой меха:\n";
            std::cout << "ID: " << argv[0] << "\nАдрес: " << argv[1]
                      << "\nДиректор: " << argv[2] << "\nМаксимальная цена: " << argv[3] << std::endl;
        }
        return 0;
    };
    db.query(sql, callback);
}

// Отчёт 2: по каждой категории покупателей – общее количество и сумма
void AuctionManager::reportByBuyerCategory() {
    Database& db = Database::getInstance();
    std::string sql = R"(
        SELECT buyer_category, SUM(number_of_units_sold) as total_units, SUM(number_of_units_sold * selling_price) as total_sum
        FROM Auction_Results
        GROUP BY buyer_category;
    )";
    
    auto callback = [](int argc, char** argv, char** colName) -> int {
        if (argc >= 3) {
            std::cout << "Категория: " << argv[0]
                      << ", Кол-во единиц: " << argv[1]
                      << ", Общая сумма: " << argv[2] << std::endl;
        }
        return 0;
    };
    db.query(sql, callback);
}

// Отчёт 3: прибыль по каждой ферме (прибыль = (selling_price - stated_price) * sold_units)
void AuctionManager::reportProfitByFarm() {
    Database& db = Database::getInstance();
    std::string sql = R"(
        SELECT f.id, f.adress, SUM((ar.selling_price - ef.stated_price) * ar.number_of_units_sold) as profit
        FROM Auction_Results ar
        JOIN Exhibited_fur ef ON ar.furfarm_number = ef.furfarm_number 
            AND ar.fur_name = ef.fur_name AND ar.fur_type = ef.fur_type
        JOIN Furfarms f ON ar.furfarm_number = f.id
        GROUP BY f.id;
    )";
    
    auto callback = [](int argc, char** argv, char** colName) -> int {
        if (argc >= 3) {
            std::cout << "Ферма ID " << argv[0] << " (" << argv[1] << "): Прибыль = " << argv[2] << std::endl;
        }
        return 0;
    };
    db.query(sql, callback);
}

// Отчёт 4: фермы, продавшие шкурки по цене выше средней аукционной цены
void AuctionManager::reportFarmsAboveAvgPrice() {
    Database& db = Database::getInstance();
    std::string sql = R"(
        SELECT DISTINCT f.id, f.adress, ar.selling_price
        FROM Auction_Results ar
        JOIN Furfarms f ON ar.furfarm_number = f.id
        WHERE ar.selling_price > (SELECT AVG(selling_price) FROM Auction_Results);
    )";
    
    auto callback = [](int argc, char** argv, char** colName) -> int {
        if (argc >= 3) {
            std::cout << "Ферма ID " << argv[0] << ", Адрес: " << argv[1] << ", Цена продажи: " << argv[2] << std::endl;
        }
        return 0;
    };
    db.query(sql, callback);
}

// Отчёт 5: ферма с максимальной прибылью
void AuctionManager::reportFarmMaxProfit() {
    Database& db = Database::getInstance();
    std::string sql = R"(
        SELECT f.id, f.adress, f.director_surname, SUM((ar.selling_price - ef.stated_price) * ar.number_of_units_sold) as profit
        FROM Auction_Results ar
        JOIN Exhibited_fur ef ON ar.furfarm_number = ef.furfarm_number 
            AND ar.fur_name = ef.fur_name AND ar.fur_type = ef.fur_type
        JOIN Furfarms f ON ar.furfarm_number = f.id
        GROUP BY f.id
        ORDER BY profit DESC
        LIMIT 1;
    )";
    
    auto callback = [](int argc, char** argv, char** colName) -> int {
        if (argc >= 4) {
            std::cout << "Ферма с максимальной прибылью:\n";
            std::cout << "ID: " << argv[0] << "\nАдрес: " << argv[1]
                      << "\nДиректор: " << argv[2] << "\nПрибыль: " << argv[3] << std::endl;
        }
        return 0;
    };
    db.query(sql, callback);
}

// Функция для пункта 5: список ферм с прибылью меньше планового процента (прибыль в процентах от выручки? Уточнение: плановый процент прибыли – например, 20% от выручки)
void AuctionManager::listFarmsLowProfit(double plannedPercent) {
    Database& db = Database::getInstance();
    // Вычисляем для каждой фермы выручку и прибыль, затем прибыль в процентах = (прибыль / выручка)*100
    std::string sql = R"(
        SELECT f.id, f.adress, 
               SUM(ar.number_of_units_sold * ar.selling_price) as revenue,
               SUM((ar.selling_price - ef.stated_price) * ar.number_of_units_sold) as profit
        FROM Auction_Results ar
        JOIN Exhibited_fur ef ON ar.furfarm_number = ef.furfarm_number 
            AND ar.fur_name = ef.fur_name AND ar.fur_type = ef.fur_type
        JOIN Furfarms f ON ar.furfarm_number = f.id
        GROUP BY f.id;
    )";
    
    auto callback = [plannedPercent](int argc, char** argv, char** colName) -> int {
        if (argc >= 4) {
            double revenue = std::stod(argv[2]);
            double profit = std::stod(argv[3]);
            if (revenue > 0) {
                double profitPercent = (profit / revenue) * 100.0;
                if (profitPercent < plannedPercent) {
                    std::cout << "Ферма ID " << argv[0] << " (" << argv[1] << "): прибыль = " << profit
                              << " (" << profitPercent << "%) < " << plannedPercent << "%" << std::endl;
                }
            }
        }
        return 0;
    };
    db.query(sql, callback);
}

double AuctionManager::getFarmProfit(int farmId) {
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "SELECT SUM((ar.selling_price - ef.stated_price) * ar.number_of_units_sold) as profit "
        << "FROM Auction_Results ar "
        << "JOIN Exhibited_fur ef ON ar.furfarm_number = ef.furfarm_number "
        << "    AND ar.fur_name = ef.fur_name AND ar.fur_type = ef.fur_type "
        << "WHERE ar.furfarm_number = " << farmId << ";";
    
    double profit = 0.0;
    auto callback = [&profit](int argc, char** argv, char** colName) -> int {
        if (argc >= 1 && argv[0]) profit = std::stod(argv[0]);
        return 0;
    };
    db.query(sql.str(), callback);
    return profit;
}