#ifndef AUCTION_H
#define AUCTION_H

#include <string>
#include <vector>

struct AuctionResult {
    int id;
    int farmId;
    std::string furName;
    std::string furType;
    int soldUnits;
    double sellingPrice;
    std::string buyerCategory;
};

class AuctionManager {
public:
    // Returns created auction result id, or -1 on error
    static int addAuctionResult(const AuctionResult& result);
    
    static void reportFarmWithHighestPrice();
    static void reportByBuyerCategory();
    static void reportProfitByFarm();
    static void reportFarmsAboveAvgPrice();
    static void reportFarmMaxProfit();
    
    static bool validateAuctionResult(const AuctionResult& result);
    static void listFarmsLowProfit(double plannedPercent);
    static double getFarmProfit(int farmId);
};

#endif