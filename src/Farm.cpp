#include "Farm.h"
#include "Database.h"
#include <sstream>

int FarmManager::addFarm(const Farm& farm) {
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "INSERT INTO Furfarms (adress, director_surname, phone) VALUES ("
        << "'" << farm.address << "', '" << farm.directorSurname << "', '" << farm.phone << "');";
    if (!db.execute(sql.str())) return -1;
    return static_cast<int>(sqlite3_last_insert_rowid(db.getHandle()));
}

bool FarmManager::updateFarm(const Farm& farm) {
    if (!FarmManager::exists(farm.id)) return false;
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "UPDATE Furfarms SET adress = '" << farm.address
        << "', director_surname = '" << farm.directorSurname
        << "', phone = '" << farm.phone
        << "' WHERE id = " << farm.id << ";";
    return db.execute(sql.str());
}

bool FarmManager::deleteFarm(int id) {
    if (!FarmManager::exists(id)) return false;
    Database& db = Database::getInstance();

    bool hasRefs = false;
    {
        std::stringstream q;
        q << "SELECT 1 FROM Exhibited_fur WHERE furfarm_number = " << id << " LIMIT 1;";
        auto cb = [&hasRefs](int argc, char** argv, char** col) -> int {
            hasRefs = true;
            return 0;
        };
        db.query(q.str(), cb);
    }
    {
        std::stringstream q;
        q << "SELECT 1 FROM Auction_Results WHERE furfarm_number = " << id << " LIMIT 1;";
        auto cb = [&hasRefs](int argc, char** argv, char** col) -> int {
            hasRefs = true;
            return 0;
        };
        db.query(q.str(), cb);
    }
    if (hasRefs) {
        return false;
    }

    std::stringstream sql;
    sql << "DELETE FROM Furfarms WHERE id = " << id << ";";
    return db.execute(sql.str());
}

Farm FarmManager::getFarmById(int id) {
    Farm farm;
    farm.id = -1;
    Database& db = Database::getInstance();
    std::stringstream sql;
    sql << "SELECT id, adress, director_surname, phone FROM Furfarms WHERE id = " << id << ";";
    
    auto callback = [&farm](int argc, char** argv, char** colName) -> int {
        if (argc >= 4) {
            farm.id = std::stoi(argv[0]);
            farm.address = argv[1];
            farm.directorSurname = argv[2];
            farm.phone = argv[3];
        }
        return 0;
    };
    db.query(sql.str(), callback);
    return farm;
}

std::vector<Farm> FarmManager::getAllFarms() {
    std::vector<Farm> farms;
    Database& db = Database::getInstance();
    std::string sql = "SELECT id, adress, director_surname, phone FROM Furfarms;";
    
    auto callback = [&farms](int argc, char** argv, char** colName) -> int {
        if (argc >= 4) {
            Farm f;
            f.id = std::stoi(argv[0]);
            f.address = argv[1];
            f.directorSurname = argv[2];
            f.phone = argv[3];
            farms.push_back(f);
        }
        return 0;
    };
    db.query(sql, callback);
    return farms;
}

Farm FarmManager::getFarmByNumber(int number) {
    return getFarmById(number);
}

bool FarmManager::exists(int id) {
    return getFarmById(id).id != -1;
}