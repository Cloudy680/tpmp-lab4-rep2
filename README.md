# Fur Auction (Lab 4 / Task 6.2)

## RU

Консольное приложение (вариант **«Пушной аукцион»**) с хранением данных в **SQLite**.

### Функциональность
- **Аутентификация/авторизация** из таблицы `Users` (роли `admin`, `farm_user`)
- **CRUD**:
  - `Furfarms` (фермы)
  - `Exhibited_fur` (лоты пушнины)
  - `Auction_Results` (результаты торгов)
- **Отчёты** (SELECT-запросы) по варианту
- Валидация фиксации результатов (нельзя внести «нереальные» данные)

### Требования
- CMake ≥ 3.10
- Компилятор C++ (clang/gcc)
- SQLite3 dev (заголовки/библиотека)

### Сборка и запуск

```bash
cmake -S . -B build
cmake --build build -j
./bin/auction_app
```

При первом запуске БД инициализируется из `data/FurAuction_create.sql`.

### Тесты

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### Учётные записи (сид в SQL)
- admin: `admin` / `admin123`
- farm user: `farm1` / `farm1pass`
- farm user: `farm2` / `farm2pass`

## EN

Console app (variant **“Fur auction”**) with **SQLite** storage.

Build/run:

```bash
cmake -S . -B build
cmake --build build -j
./bin/auction_app
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

