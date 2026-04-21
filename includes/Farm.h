#ifndef FARM_H
#define FARM_H

#include <string>
#include <vector>

struct Farm {
    int id;
    std::string address;
    std::string directorSurname;
    std::string phone;
};

class FarmManager {
public:
    // Returns created farm id, or -1 on error
    static int addFarm(const Farm& farm);
    static bool updateFarm(const Farm& farm);
    static bool deleteFarm(int id);
    static Farm getFarmById(int id);
    static std::vector<Farm> getAllFarms();
    static Farm getFarmByNumber(int number);
    static bool exists(int id);
};

#endif