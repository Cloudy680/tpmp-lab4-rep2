-- Таблица ферм
CREATE TABLE IF NOT EXISTS Furfarms (
    id INTEGER PRIMARY KEY,
    adress TEXT NOT NULL,
    director_surname TEXT NOT NULL,
    phone TEXT NOT NULL
);

-- Таблица пользователей (аутентификация/авторизация)
CREATE TABLE IF NOT EXISTS Users (
    id INTEGER PRIMARY KEY,
    username TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    role TEXT NOT NULL CHECK(role IN ('admin', 'farm_user')),
    farm_id INTEGER,
    FOREIGN KEY (farm_id) REFERENCES Furfarms(id) ON DELETE SET NULL
);

-- Таблица выставленной пушнины
CREATE TABLE IF NOT EXISTS Exhibited_fur (
    id INTEGER PRIMARY KEY,
    furfarm_number INTEGER NOT NULL,
    fur_name TEXT NOT NULL,
    fur_type TEXT NOT NULL,
    number_units INTEGER NOT NULL CHECK(number_units > 0),
    stated_price REAL NOT NULL,
    FOREIGN KEY (furfarm_number) REFERENCES Furfarms(id) ON DELETE RESTRICT
);

-- Таблица результатов аукциона
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

-- Вставка тестовых данных (фермы)
INSERT OR IGNORE INTO Furfarms (id, adress, director_surname, phone) VALUES
(1, 'г. Москва, ул. Лесная 5', 'Иванов И.И.', '+7-495-123-4567'),
(2, 'г. Иркутск, ул. Тайга 12', 'Сидоров С.С.', '+7-395-222-3344');

-- Вставка пользователей (логины для входа)
INSERT OR IGNORE INTO Users (id, username, password, role, farm_id) VALUES
(1, 'admin', 'admin123', 'admin', NULL),
(2, 'farm1', 'farm1pass', 'farm_user', 1),
(3, 'farm2', 'farm2pass', 'farm_user', 2);

-- Вставка лотов пушнины
INSERT OR IGNORE INTO Exhibited_fur (id, furfarm_number, fur_name, fur_type, number_units, stated_price) VALUES
(1, 1, 'Соболь', 'Высший', 100, 5000.00),
(2, 1, 'Норка', 'Первый', 200, 2000.00),
(3, 2, 'Песец', 'Высший', 50, 3000.00);

-- Вставка результатов аукциона
INSERT OR IGNORE INTO Auction_Results (id, furfarm_number, fur_name, fur_type, number_of_units_sold, selling_price, buyer_category) VALUES
(1, 1, 'Соболь', 'Высший', 80, 5500.00, 'fur factory'),
(2, 1, 'Соболь', 'Высший', 20, 4800.00, 'private individual'),
(3, 1, 'Норка', 'Первый', 150, 2100.00, 'studio'),
(4, 2, 'Песец', 'Высший', 40, 3200.00, 'fur factory');