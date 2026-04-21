#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <vector>
#include <cstdlib>
#include <initializer_list>

#include "Database.h"
#include "Auth.h"
#include "Farm.h"
#include "Lot.h"
#include "Auction.h"

using namespace std;

// ========== ПРОТОТИПЫ ФУНКЦИЙ (forward declarations) ==========
void clearScreen();
void printHeader(const string& title);
void adminMenu();
void farmUserMenu(int farmId);
// ==============================================================

// Кроссплатформенная очистка экрана
#ifdef _WIN32
#define CLEAR_SCREEN "cls"
#else
#define CLEAR_SCREEN "clear"
#endif

void clearScreen() {
    // Подавляем предупреждение о неиспользуемом возвращаемом значении
    (void)system(CLEAR_SCREEN);
}

void printHeader(const string& title) {
    cout << "\n========== " << title << " ==========\n";
}

static bool farmExistsOrPrint(int farmId) {
    if (FarmManager::exists(farmId)) return true;
    cout << "Ошибка: фермы с ID " << farmId << " не существует." << endl;
    return false;
}

static string pickExistingPath(std::initializer_list<string> candidates) {
    for (const auto& p : candidates) {
        ifstream f(p);
        if (f.good()) return p;
    }
    return "";
}

int main() {
    // 1. Инициализация БД
    Database& db = Database::getInstance();
    const string dbPath = pickExistingPath({"data/auction.db", "../data/auction.db"});
    if (dbPath.empty()) {
        cerr << "Не удалось найти файл базы данных. Проверьте путь." << endl;
        return 1;
    }
    if (!db.open(dbPath)) {
        cerr << "Не удалось открыть БД" << endl;
        return 1;
    }

    // 2. Создание таблиц и заполнение данными из SQL-скрипта (если таблиц нет)
    const string scriptPath = pickExistingPath({"data/FurAuction_create.sql", "../data/FurAuction_create.sql"});
    if (scriptPath.empty()) {
        cerr << "Не удалось найти SQL-скрипт инициализации." << endl;
        return 1;
    }
    if (!db.initFromScript(scriptPath)) {
        cerr << "Ошибка инициализации БД из скрипта" << endl;
        return 1;
    }

    // 3. Цикл аутентификации
    while (true) {
        clearScreen();
        printHeader("Пушной аукцион");
        cout << "1. Войти\n0. Выход\nВыберите: ";
        int choice;
        cin >> choice;
        if (choice == 0) break;
        if (choice == 1) {
            string username, password;
            cout << "Логин: "; cin >> username;
            cout << "Пароль: "; cin >> password;
            User user = Auth::login(username, password);
            if (user.id != -1) {
                cout << "Добро пожаловать, " << user.username << "!" << endl;
                if (user.role == "admin") {
                    adminMenu();
                } else if (user.role == "farm_user") {
                    if (!farmExistsOrPrint(user.farm_id)) {
                        cout << "Обратитесь к администратору: у пользователя указан несуществующий farm_id." << endl;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cin.get();
                        continue;
                    }
                    farmUserMenu(user.farm_id);
                }
            } else {
                cout << "Неверный логин или пароль!" << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
            }
        }
    }

    db.close();
    return 0;
}

// ===================== МЕНЮ АДМИНИСТРАТОРА =====================
void adminMenu() {
    int choice;
    do {
        clearScreen();
        printHeader("Меню администратора");
        cout << "1. Отчёт: ферма с самой высокой ценой меха\n";
        cout << "2. Отчёт: по категориям покупателей\n";
        cout << "3. Отчёт: прибыль по фермам\n";
        cout << "4. Отчёт: фермы выше средней цены\n";
        cout << "5. Отчёт: ферма с максимальной прибылью\n";
        cout << "6. Список ферм с прибылью < планового %\n";
        cout << "7. Прибыль указанной фермы\n";
        cout << "8. Управление фермами (CRUD)\n";
        cout << "9. Управление лотами (CRUD)\n";
        cout << "10. Фиксация результатов торгов\n";
        cout << "0. Выйти из аккаунта\nВыберите: ";
        cin >> choice;

        switch (choice) {
            case 1: AuctionManager::reportFarmWithHighestPrice(); break;
            case 2: AuctionManager::reportByBuyerCategory(); break;
            case 3: AuctionManager::reportProfitByFarm(); break;
            case 4: AuctionManager::reportFarmsAboveAvgPrice(); break;
            case 5: AuctionManager::reportFarmMaxProfit(); break;
            case 6: {
                double percent;
                cout << "Введите плановый процент прибыли (например, 20): ";
                cin >> percent;
                AuctionManager::listFarmsLowProfit(percent);
                break;
            }
            case 7: {
                int fid;
                cout << "Введите ID фермы: ";
                cin >> fid;
                if (!farmExistsOrPrint(fid)) break;
                double profit = AuctionManager::getFarmProfit(fid);
                cout << "Прибыль фермы: " << profit << endl;
                break;
            }
            case 8: {
                int sub;
                cout << "1. Добавить ферму\n2. Изменить ферму\n3. Удалить ферму\n4. Список всех ферм\nВыберите: ";
                cin >> sub;
                if (sub == 1) {
                    Farm f;
                    cout << "Адрес: "; cin.ignore(); getline(cin, f.address);
                    cout << "Директор: "; getline(cin, f.directorSurname);
                    cout << "Телефон: "; getline(cin, f.phone);
                    int createdId = FarmManager::addFarm(f);
                    if (createdId != -1) cout << "Добавлено. ID = " << createdId << "\n";
                } else if (sub == 2) {
                    Farm f;
                    cout << "ID фермы для изменения: "; cin >> f.id;
                    if (!farmExistsOrPrint(f.id)) break;
                    cout << "Новый адрес: "; cin.ignore(); getline(cin, f.address);
                    cout << "Новый директор: "; getline(cin, f.directorSurname);
                    cout << "Новый телефон: "; getline(cin, f.phone);
                    if (FarmManager::updateFarm(f)) cout << "Обновлено\n";
                } else if (sub == 3) {
                    int id;
                    cout << "ID фермы для удаления: "; cin >> id;
                    if (!farmExistsOrPrint(id)) break;
                    if (FarmManager::deleteFarm(id)) cout << "Удалено\n";
                } else if (sub == 4) {
                    auto farms = FarmManager::getAllFarms();
                    for (auto& f : farms) {
                        cout << "ID: " << f.id << ", Адрес: " << f.address << ", Директор: " << f.directorSurname << endl;
                    }
                }
                break;
            }
            case 9: {
                int sub;
                cout << "1. Добавить лот\n2. Изменить лот\n3. Удалить лот\n4. Список лотов по ферме\nВыберите: ";
                cin >> sub;
                if (sub == 1) {
                    Lot l;
                    cout << "ID фермы: "; cin >> l.farmId;
                    if (!farmExistsOrPrint(l.farmId)) break;
                    cout << "Название меха: "; cin.ignore(); getline(cin, l.furName);
                    cout << "Тип/сорт: "; getline(cin, l.furType);
                    cout << "Количество единиц: "; cin >> l.numberUnits;
                    cout << "Заявленная цена: "; cin >> l.statedPrice;
                    int createdId = LotManager::addLot(l);
                    if (createdId != -1) cout << "Добавлено. ID лота = " << createdId << "\n";
                } else if (sub == 2) {
                    Lot l;
                    cout << "ID лота для изменения: "; cin >> l.id;
                    if (LotManager::getLotById(l.id).id == -1) { cout << "Ошибка: такого лота нет.\n"; break; }
                    cout << "Новое название меха: "; cin.ignore(); getline(cin, l.furName);
                    cout << "Новый тип: "; getline(cin, l.furType);
                    cout << "Новое количество: "; cin >> l.numberUnits;
                    cout << "Новая заявленная цена: "; cin >> l.statedPrice;
                    l.farmId = 0; // не меняем ферму
                    if (LotManager::updateLot(l)) cout << "Обновлено\n";
                } else if (sub == 3) {
                    int id;
                    cout << "ID лота для удаления: "; cin >> id;
                    if (LotManager::getLotById(id).id == -1) { cout << "Ошибка: такого лота нет.\n"; break; }
                    if (LotManager::deleteLot(id)) cout << "Удалено\n";
                } else if (sub == 4) {
                    int fid;
                    cout << "ID фермы: "; cin >> fid;
                    if (!farmExistsOrPrint(fid)) break;
                    auto lots = LotManager::getLotsByFarm(fid);
                    for (auto& l : lots) {
                        cout << "ID: " << l.id << ", Мех: " << l.furName << ", Тип: " << l.furType
                             << ", Кол-во: " << l.numberUnits << ", Цена: " << l.statedPrice << endl;
                    }
                }
                break;
            }
            case 10: {
                AuctionResult ar;
                cout << "ID фермы: "; cin >> ar.farmId;
                if (!farmExistsOrPrint(ar.farmId)) break;
                cout << "Название меха: "; cin.ignore(); getline(cin, ar.furName);
                cout << "Тип меха: "; getline(cin, ar.furType);
                cout << "Количество проданных единиц: "; cin >> ar.soldUnits;
                cout << "Продажная цена: "; cin >> ar.sellingPrice;
                cout << "Категория покупателя (fur factory/studio/private individual): "; cin.ignore(); getline(cin, ar.buyerCategory);
                int createdId = AuctionManager::addAuctionResult(ar);
                if (createdId != -1) cout << "Результат зафиксирован. ID = " << createdId << "\n";
                else cout << "Ошибка фиксации\n";
                break;
            }
            case 0: return;
            default: cout << "Неверный выбор\n";
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    } while (choice != 0);
}

// ===================== МЕНЮ ЗВЕРОФЕРМЫ =====================
void farmUserMenu(int farmId) {
    int choice;
    do {
        clearScreen();
        printHeader("Меню зверофермы (ID " + to_string(farmId) + ")");
        cout << "1. Мои лоты (выставленная пушнина)\n";
        cout << "2. Моя прибыль\n";
        cout << "0. Выйти\nВыберите: ";
        cin >> choice;

        if (choice == 1) {
            auto lots = LotManager::getLotsByFarm(farmId);
            if (lots.empty()) cout << "Нет выставленных лотов.\n";
            for (auto& l : lots) {
                cout << "Мех: " << l.furName << ", Сорт: " << l.furType
                     << ", Количество: " << l.numberUnits << ", Заявленная цена: " << l.statedPrice << endl;
            }
        } else if (choice == 2) {
            double profit = AuctionManager::getFarmProfit(farmId);
            cout << "Прибыль вашей фермы: " << profit << endl;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    } while (choice != 0);
}