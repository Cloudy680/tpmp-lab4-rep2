#ifndef LOT_H
#define LOT_H

#include <string>
#include <vector>

struct Lot {
    int id;
    int farmId;
    std::string furName;
    std::string furType;
    int numberUnits;
    double statedPrice;
    bool isSold;
};

class LotManager {
public:
    // Returns created lot id, or -1 on error
    static int addLot(const Lot& lot);
    static bool updateLot(const Lot& lot);
    static bool deleteLot(int id);
    static Lot getLotById(int id);
    static std::vector<Lot> getLotsByFarm(int farmId);
    static std::vector<Lot> getUnsoldLots();
    static bool markAsSold(int lotId, int soldUnits);
};
#endif