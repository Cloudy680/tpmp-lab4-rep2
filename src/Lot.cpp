#include "Lot.h"
#include "Database.h"
#include "Farm.h"
#include <sstream>

int LotManager::addLot(const Lot& lot) {
    if (!FarmManager::exists(lot.farmId)) return -1;
    if (lot.numberUnits <= 0) return -1;
    if (lot.statedPrice <= 0) return -1;

    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "INSERT INTO Exhibited_fur (furfarm_number, fur_name, fur_type, number_units, stated_price) VALUES ("
        << lot.farmId << ", '" << lot.furName << "', '" << lot.furType << "', "
        << lot.numberUnits << ", " << lot.statedPrice << ");";
    if (!db.execute(sql.str())) return -1;
    return static_cast<int>(sqlite3_last_insert_rowid(db.getHandle()));
}

bool LotManager::updateLot(const Lot& lot) {
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "UPDATE Exhibited_fur SET fur_name = '" << lot.furName
        << "', fur_type = '" << lot.furType
        << "', number_units = " << lot.numberUnits
        << ", stated_price = " << lot.statedPrice
        << " WHERE id = " << lot.id << ";";
    return db.execute(sql.str());
}

bool LotManager::deleteLot(int id) {
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "DELETE FROM Exhibited_fur WHERE id = " << id << ";";
    return db.execute(sql.str());
}

Lot LotManager::getLotById(int id) {
    Lot lot;
    lot.id = -1;
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "SELECT id, furfarm_number, fur_name, fur_type, number_units, stated_price FROM Exhibited_fur WHERE id = " << id << ";";
    
    auto callback = [&lot](int argc, char** argv, char** colName) -> int {
        if (argc >= 6) {
            lot.id = std::stoi(argv[0]);
            lot.farmId = std::stoi(argv[1]);
            lot.furName = argv[2];
            lot.furType = argv[3];
            lot.numberUnits = std::stoi(argv[4]);
            lot.statedPrice = std::stod(argv[5]);
        }
        return 0;
    };
    db.query(sql.str(), callback);
    return lot;
}

std::vector<Lot> LotManager::getLotsByFarm(int farmId) {
    std::vector<Lot> lots;
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "SELECT id, furfarm_number, fur_name, fur_type, number_units, stated_price FROM Exhibited_fur WHERE furfarm_number = " << farmId << ";";
    
    auto callback = [&lots](int argc, char** argv, char** colName) -> int {
        if (argc >= 6) {
            Lot l;
            l.id = std::stoi(argv[0]);
            l.farmId = std::stoi(argv[1]);
            l.furName = argv[2];
            l.furType = argv[3];
            l.numberUnits = std::stoi(argv[4]);
            l.statedPrice = std::stod(argv[5]);
            lots.push_back(l);
        }
        return 0;
    };
    db.query(sql.str(), callback);
    return lots;
}