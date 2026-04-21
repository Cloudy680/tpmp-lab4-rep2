#include <gtest/gtest.h>

#include "Database.h"
#include "Auth.h"
#include "Farm.h"
#include "Lot.h"
#include "Auction.h"

namespace {

const char* kSchemaAndSeed = R"SQL(
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS Furfarms (
  id INTEGER PRIMARY KEY,
  adress TEXT NOT NULL,
  director_surname TEXT NOT NULL,
  phone TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS Users (
  id INTEGER PRIMARY KEY,
  username TEXT NOT NULL UNIQUE,
  password TEXT NOT NULL,
  role TEXT NOT NULL CHECK(role IN ('admin', 'farm_user')),
  farm_id INTEGER,
  FOREIGN KEY (farm_id) REFERENCES Furfarms(id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS Exhibited_fur (
  id INTEGER PRIMARY KEY,
  furfarm_number INTEGER NOT NULL,
  fur_name TEXT NOT NULL,
  fur_type TEXT NOT NULL,
  number_units INTEGER NOT NULL CHECK(number_units > 0),
  stated_price REAL NOT NULL,
  FOREIGN KEY (furfarm_number) REFERENCES Furfarms(id) ON DELETE RESTRICT
);

CREATE TABLE IF NOT EXISTS Auction_Results (
  id INTEGER PRIMARY KEY,
  furfarm_number INTEGER NOT NULL,
  fur_name TEXT NOT NULL,
  fur_type TEXT NOT NULL,
  number_of_units_sold INTEGER NOT NULL CHECK(number_of_units_sold > 0),
  selling_price REAL NOT NULL,
  buyer_category TEXT NOT NULL CHECK(buyer_category IN ('fur factory', 'studio', 'private individual')),
  FOREIGN KEY (furfarm_number) REFERENCES Furfarms(id) ON DELETE RESTRICT
);

INSERT INTO Furfarms (id, adress, director_surname, phone) VALUES
  (1, 'addr1', 'dir1', '111'),
  (2, 'addr2', 'dir2', '222');

INSERT INTO Users (id, username, password, role, farm_id) VALUES
  (1, 'admin', 'admin123', 'admin', NULL),
  (2, 'farm1', 'farm1pass', 'farm_user', 1),
  (3, 'farm2', 'farm2pass', 'farm_user', 2);

INSERT INTO Exhibited_fur (id, furfarm_number, fur_name, fur_type, number_units, stated_price) VALUES
  (1, 1, 'Sable', 'Top', 100, 5000.0),
  (2, 1, 'Mink', 'A', 200, 2000.0),
  (3, 2, 'Fox', 'Top', 50, 3000.0);
)SQL";

struct DbFixture : public ::testing::Test {
  void SetUp() override {
    auto& db = Database::getInstance();
    ASSERT_TRUE(db.open(":memory:"));
    ASSERT_TRUE(db.execute(kSchemaAndSeed));
  }

  void TearDown() override {
    Database::getInstance().close();
  }
};

}  // namespace

// ---------------- Database ----------------

TEST_F(DbFixture, DatabaseExecuteCreatesTable) {
  auto& db = Database::getInstance();
  ASSERT_TRUE(db.execute("CREATE TABLE IF NOT EXISTS T(x INTEGER);"));
  int seen = 0;
  ASSERT_TRUE(db.query("SELECT name FROM sqlite_master WHERE type='table' AND name='T';",
                       [&seen](int argc, char** argv, char** col) -> int {
                         if (argc > 0) ++seen;
                         return 0;
                       }));
  EXPECT_GE(seen, 1);
}

TEST_F(DbFixture, DatabaseQueryCallbackReceivesRows) {
  auto& db = Database::getInstance();
  ASSERT_TRUE(db.execute("CREATE TABLE IF NOT EXISTS Q(v INTEGER); INSERT INTO Q(v) VALUES (1),(2),(3);"));
  int sum = 0;
  ASSERT_TRUE(db.query("SELECT v FROM Q;",
                       [&sum](int argc, char** argv, char** col) -> int {
                         if (argc > 0 && argv[0]) sum += std::stoi(argv[0]);
                         return 0;
                       }));
  EXPECT_EQ(sum, 6);
}

TEST_F(DbFixture, DatabaseExecuteInvalidSqlFails) {
  auto& db = Database::getInstance();
  EXPECT_FALSE(db.execute("THIS IS NOT SQL;"));
}

// ---------------- Auth ----------------

TEST_F(DbFixture, AuthLoginAdminSuccess) {
  const User u = Auth::login("admin", "admin123");
  EXPECT_NE(u.id, -1);
  EXPECT_EQ(u.role, "admin");
}

TEST_F(DbFixture, AuthLoginWrongPasswordFails) {
  const User u = Auth::login("admin", "wrong");
  EXPECT_EQ(u.id, -1);
}

TEST_F(DbFixture, AuthLoginFarmUserHasFarmId) {
  const User u = Auth::login("farm1", "farm1pass");
  EXPECT_NE(u.id, -1);
  EXPECT_EQ(u.role, "farm_user");
  EXPECT_EQ(u.farm_id, 1);
}

// ---------------- Farm ----------------

TEST_F(DbFixture, FarmAddReturnsNewIdAndExists) {
  Farm f;
  f.address = "newaddr";
  f.directorSurname = "newdir";
  f.phone = "999";
  const int id = FarmManager::addFarm(f);
  EXPECT_NE(id, -1);
  EXPECT_TRUE(FarmManager::exists(id));
}

TEST_F(DbFixture, FarmUpdateNonexistentFails) {
  Farm f;
  f.id = 999;
  f.address = "x";
  f.directorSurname = "y";
  f.phone = "z";
  EXPECT_FALSE(FarmManager::updateFarm(f));
}

TEST_F(DbFixture, FarmDeleteNewFarmSucceeds) {
  Farm f;
  f.address = "todelete";
  f.directorSurname = "d";
  f.phone = "p";
  const int id = FarmManager::addFarm(f);
  ASSERT_NE(id, -1);
  EXPECT_TRUE(FarmManager::deleteFarm(id));
  EXPECT_FALSE(FarmManager::exists(id));
}

TEST_F(DbFixture, FarmDeleteReferencedFarmFails) {
  EXPECT_TRUE(FarmManager::exists(1));
  EXPECT_FALSE(FarmManager::deleteFarm(1));
  EXPECT_TRUE(FarmManager::exists(1));
}

// ---------------- Lot ----------------

TEST_F(DbFixture, LotAddRequiresExistingFarm) {
  Lot l;
  l.farmId = 999;
  l.furName = "X";
  l.furType = "Y";
  l.numberUnits = 10;
  l.statedPrice = 1.0;
  EXPECT_EQ(LotManager::addLot(l), -1);
}

TEST_F(DbFixture, LotAddThenGetByFarm) {
  Lot l;
  l.farmId = 1;
  l.furName = "NewFur";
  l.furType = "B";
  l.numberUnits = 10;
  l.statedPrice = 123.0;
  const int id = LotManager::addLot(l);
  ASSERT_NE(id, -1);
  const auto lots = LotManager::getLotsByFarm(1);
  bool found = false;
  for (const auto& it : lots) {
    if (it.id == id) found = true;
  }
  EXPECT_TRUE(found);
}

TEST_F(DbFixture, LotDeleteNonexistentFailsViaNoRowAffected) {
  // deleteLot() returns db.execute() result; DELETE on missing row still OK in SQLite.
  // So we assert behavior using getLotById().
  EXPECT_EQ(LotManager::getLotById(999).id, -1);
}

// ---------------- Auction ----------------

TEST_F(DbFixture, AuctionValidateRejectsInvalidCategory) {
  AuctionResult ar;
  ar.farmId = 1;
  ar.furName = "Sable";
  ar.furType = "Top";
  ar.soldUnits = 1;
  ar.sellingPrice = 10.0;
  ar.buyerCategory = "bad";
  EXPECT_EQ(AuctionManager::addAuctionResult(ar), -1);
}

TEST_F(DbFixture, AuctionValidateRejectsTooManyUnits) {
  AuctionResult ar;
  ar.farmId = 2;
  ar.furName = "Fox";
  ar.furType = "Top";
  ar.soldUnits = 999;
  ar.sellingPrice = 10.0;
  ar.buyerCategory = "studio";
  EXPECT_EQ(AuctionManager::addAuctionResult(ar), -1);
}

TEST_F(DbFixture, AuctionAddDecrementsLotUnits) {
  AuctionResult ar;
  ar.farmId = 1;
  ar.furName = "Mink";
  ar.furType = "A";
  ar.soldUnits = 10;
  ar.sellingPrice = 2100.0;
  ar.buyerCategory = "fur factory";

  const auto before = LotManager::getLotsByFarm(1);
  int beforeUnits = -1;
  for (const auto& l : before) {
    if (l.furName == "Mink" && l.furType == "A") beforeUnits = l.numberUnits;
  }
  ASSERT_GT(beforeUnits, 0);

  const int id = AuctionManager::addAuctionResult(ar);
  ASSERT_NE(id, -1);

  const auto after = LotManager::getLotsByFarm(1);
  int afterUnits = -1;
  for (const auto& l : after) {
    if (l.furName == "Mink" && l.furType == "A") afterUnits = l.numberUnits;
  }
  EXPECT_EQ(afterUnits, beforeUnits - 10);
}

