#include "datastore.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <algorithm>
#include <cmath>

namespace {
QVariantMap queryRow(QSqlQuery &query) {
    QVariantMap row;
    const auto record = query.record();
    for (int i = 0; i < record.count(); ++i)
        row.insert(record.fieldName(i), query.value(i));
    return row;
}
}

DataStore::DataStore() {
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(folder);
    db = QSqlDatabase::addDatabase("QSQLITE", "fitness-tracker");
    db.setDatabaseName(folder + "/fitness.db");
    if (!db.open()) error = db.lastError().text();
    else initialize();
}

DataStore::~DataStore() {
    db.close();
}

bool DataStore::isOpen() const { return db.isOpen(); }
QString DataStore::lastError() const { return error; }

void DataStore::initialize() {
    QSqlQuery q(db);
    const QStringList statements = {
        "CREATE TABLE IF NOT EXISTS daily (id INTEGER PRIMARY KEY, date TEXT NOT NULL, calories REAL DEFAULT 0, protein REAL DEFAULT 0, carbs REAL DEFAULT 0, fat REAL DEFAULT 0, water REAL DEFAULT 0, weight REAL DEFAULT 0, note TEXT)",
        "CREATE TABLE IF NOT EXISTS workouts (id INTEGER PRIMARY KEY, date TEXT NOT NULL, exercise TEXT NOT NULL, weight REAL, sets INTEGER, reps INTEGER, note TEXT)",
        "CREATE TABLE IF NOT EXISTS pantry (id INTEGER PRIMARY KEY, name TEXT NOT NULL, calories REAL DEFAULT 0, protein REAL DEFAULT 0, carbs REAL DEFAULT 0, fat REAL DEFAULT 0, amount REAL DEFAULT 1, unit TEXT, icon TEXT)",
        "CREATE TABLE IF NOT EXISTS recipes (id INTEGER PRIMARY KEY, name TEXT NOT NULL, calories REAL, protein REAL, carbs REAL, fat REAL, ingredients TEXT, image_path TEXT)",
        "CREATE TABLE IF NOT EXISTS recipe_ingredients (id INTEGER PRIMARY KEY, recipe_id INTEGER NOT NULL, name TEXT NOT NULL, amount REAL, unit TEXT, icon TEXT, FOREIGN KEY(recipe_id) REFERENCES recipes(id) ON DELETE CASCADE)",
        "CREATE TABLE IF NOT EXISTS goals (id INTEGER PRIMARY KEY CHECK(id=1), calories REAL, protein REAL, carbs REAL, fat REAL, water REAL)",
        "INSERT OR IGNORE INTO goals VALUES (1, 2200, 126, 250, 70, 2.5)",
        "CREATE TABLE IF NOT EXISTS profile (id INTEGER PRIMARY KEY CHECK(id=1), name TEXT, birth_date TEXT, height_cm REAL, initial_weight_kg REAL, workout_frequency INTEGER, labor_frequency INTEGER)",
        "CREATE TABLE IF NOT EXISTS units (stat TEXT PRIMARY KEY, unit TEXT)",
        "INSERT OR IGNORE INTO units VALUES ('weight','kg')",
        "INSERT OR IGNORE INTO units VALUES ('height','cm')",
        "INSERT OR IGNORE INTO units VALUES ('water','L')",
        "INSERT OR IGNORE INTO units VALUES ('energy','kcal')",
        "INSERT OR IGNORE INTO units VALUES ('protein','g')",
        "INSERT OR IGNORE INTO units VALUES ('carbs','g')",
        "INSERT OR IGNORE INTO units VALUES ('fat','g')"
    };
    for (const auto &statement : statements) {
        if (!q.exec(statement)) { error = q.lastError().text(); return; }
    }
    const auto addColumn = [&q](const QString &sql) {
        return q.exec(sql) || q.lastError().text().contains("duplicate column", Qt::CaseInsensitive);
    };
    if (!addColumn("ALTER TABLE recipes ADD COLUMN servings INTEGER DEFAULT 1") ||
        !addColumn("ALTER TABLE recipes ADD COLUMN instructions TEXT DEFAULT ''") ||
        !addColumn("ALTER TABLE recipe_ingredients ADD COLUMN pantry_id INTEGER"))
        error = q.lastError().text();
}

QVariantMap DataStore::profile() const {
    QSqlQuery q("SELECT * FROM profile WHERE id=1", db);
    return q.next() ? queryRow(q) : QVariantMap{};
}

bool DataStore::hasProfile() const { return !profile().isEmpty(); }

QVariantMap DataStore::units() const {
    QVariantMap result; QSqlQuery q("SELECT stat,unit FROM units", db);
    while (q.next()) result.insert(q.value(0).toString(), q.value(1));
    return result;
}

void DataStore::saveUnit(const QString &stat, const QString &unit) {
    QSqlQuery q(db); q.prepare("INSERT OR REPLACE INTO units(stat,unit) VALUES(?,?)");
    q.addBindValue(stat); q.addBindValue(unit); q.exec();
}

void DataStore::saveProfile(const QString &name, const QDate &birthDate, double heightCm,
                            double initialWeightKg, int workoutFrequency, int laborFrequency) {
    QSqlQuery q(db);
    q.prepare("INSERT OR REPLACE INTO profile(id,name,birth_date,height_cm,initial_weight_kg,workout_frequency,labor_frequency) VALUES(1,?,?,?,?,?,?)");
    q.addBindValue(name); q.addBindValue(birthDate.toString(Qt::ISODate)); q.addBindValue(heightCm);
    q.addBindValue(initialWeightKg); q.addBindValue(workoutFrequency); q.addBindValue(laborFrequency);
    q.exec(); calculateGoals();
}

void DataStore::calculateGoals() {
    const auto p=profile(); if(p.isEmpty()) return;
    const int age=QDate::fromString(p["birth_date"].toString(),Qt::ISODate).daysTo(QDate::currentDate())/365.2425;
    const double weight=p["initial_weight_kg"].toDouble(), height=p["height_cm"].toDouble();
    // Mifflin-St Jeor estimate with a neutral constant, multiplied by a
    // standard activity factor. Labor frequency is stored per two weeks.
    const double weeklyActivity=p["workout_frequency"].toInt()+p["labor_frequency"].toInt()/2.0;
    const double factor=weeklyActivity<1?1.2:weeklyActivity<3?1.375:weeklyActivity<5?1.55:weeklyActivity<7?1.725:1.9;
    const double bmr=10*weight+6.25*height-5*age-78;
    const double calories=std::max(1200.0,bmr*factor);
    saveGoals(std::round(calories),std::round(weight*1.6),std::round(calories*.46/4),
              std::round(calories*.27/9),std::max(2.0,weight*.033));
}

QVariantMap DataStore::day(const QDate &date) const {
    QSqlQuery q(db);
    q.prepare("SELECT COALESCE(SUM(calories),0) calories, COALESCE(SUM(protein),0) protein, COALESCE(SUM(carbs),0) carbs, COALESCE(SUM(fat),0) fat, COALESCE(SUM(water),0) water, COALESCE((SELECT weight FROM daily d2 WHERE d2.date=? AND weight>0 ORDER BY id DESC LIMIT 1),0) weight FROM daily WHERE date=?");
    q.addBindValue(date.toString(Qt::ISODate)); q.addBindValue(date.toString(Qt::ISODate));
    return q.exec() && q.next() ? queryRow(q) : QVariantMap{};
}

QVariantMap DataStore::goals() const {
    QSqlQuery q("SELECT * FROM goals WHERE id=1", db);
    return q.next() ? queryRow(q) : QVariantMap{};
}

void DataStore::saveGoals(double calories, double protein, double carbs, double fat, double water) {
    QSqlQuery q(db);
    q.prepare("UPDATE goals SET calories=?, protein=?, carbs=?, fat=?, water=? WHERE id=1");
    for (double value : {calories, protein, carbs, fat, water}) q.addBindValue(value);
    q.exec();
}

void DataStore::addDaily(const QDate &date, double calories, double protein, double carbs,
                         double fat, double water, double weight, const QString &note) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO daily(date,calories,protein,carbs,fat,water,weight,note) VALUES(?,?,?,?,?,?,?,?)");
    q.addBindValue(date.toString(Qt::ISODate));
    for (double value : {calories, protein, carbs, fat, water, weight}) q.addBindValue(value);
    q.addBindValue(note); q.exec();
}

void DataStore::addWorkout(const QDate &date, const QString &exercise, double weight,
                           int sets, int reps, const QString &note) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO workouts(date,exercise,weight,sets,reps,note) VALUES(?,?,?,?,?,?)");
    q.addBindValue(date.toString(Qt::ISODate)); q.addBindValue(exercise); q.addBindValue(weight);
    q.addBindValue(sets); q.addBindValue(reps); q.addBindValue(note); q.exec();
}

QVector<QVariantMap> DataStore::workouts(const QDate &date) const {
    QVector<QVariantMap> rows; QSqlQuery q(db);
    q.prepare("SELECT * FROM workouts WHERE date=? ORDER BY id DESC");
    q.addBindValue(date.toString(Qt::ISODate)); q.exec();
    while (q.next()) rows << queryRow(q);
    return rows;
}

int DataStore::addPantryItem(const QString &name, double calories, double protein, double carbs,
                             double fat, double amount, const QString &unit, const QString &icon) {
    QSqlQuery q(db);q.prepare("INSERT INTO pantry(name,calories,protein,carbs,fat,amount,unit,icon) VALUES(?,?,?,?,?,?,?,?)");
    q.addBindValue(name);for(double value:{calories,protein,carbs,fat,amount})q.addBindValue(value);q.addBindValue(unit);q.addBindValue(icon);
    return q.exec()?q.lastInsertId().toInt():-1;
}

QVector<QVariantMap> DataStore::pantryItems() const {
    QVector<QVariantMap> rows;QSqlQuery q("SELECT * FROM pantry ORDER BY name COLLATE NOCASE",db);while(q.next())rows<<queryRow(q);return rows;
}

int DataStore::addRecipe(const QString &name, int servings, double calories, double protein,
                         double carbs, double fat, const QString &instructions,
                         const QString &imagePath, const QVector<QVariantMap> &ingredients) {
    QSqlQuery q(db);
    db.transaction();
    q.prepare("INSERT INTO recipes(name,calories,protein,carbs,fat,ingredients,image_path,servings,instructions) VALUES(?,?,?,?,?,?,?,?,?)");
    q.addBindValue(name);
    for (double value : {calories, protein, carbs, fat}) q.addBindValue(value);
    q.addBindValue(QString()); q.addBindValue(imagePath); q.addBindValue(servings); q.addBindValue(instructions);
    if (!q.exec()) { db.rollback(); return -1; }
    const int recipeId=q.lastInsertId().toInt();
    for (const auto &ingredient : ingredients) {
        QSqlQuery item(db); item.prepare("INSERT INTO recipe_ingredients(recipe_id,name,amount,unit,icon,pantry_id) VALUES(?,?,?,?,?,?)");
        item.addBindValue(recipeId); item.addBindValue(ingredient["name"]); item.addBindValue(ingredient["amount"]);
        item.addBindValue(ingredient["unit"]); item.addBindValue(ingredient["icon"]); item.addBindValue(ingredient["pantry_id"]);
        if (!item.exec()) { db.rollback(); return -1; }
    }
    return db.commit() ? recipeId : -1;
}

QVector<QVariantMap> DataStore::recipes() const {
    QVector<QVariantMap> rows; QSqlQuery q("SELECT * FROM recipes ORDER BY name", db);
    while (q.next()) rows << queryRow(q);
    return rows;
}

QVector<QVariantMap> DataStore::recipeIngredients(int recipeId) const {
    QVector<QVariantMap> rows; QSqlQuery q(db);
    q.prepare("SELECT COALESCE(p.name,ri.name) name,ri.amount,COALESCE(p.unit,ri.unit) unit,COALESCE(p.icon,ri.icon) icon FROM recipe_ingredients ri LEFT JOIN pantry p ON p.id=ri.pantry_id WHERE ri.recipe_id=? ORDER BY ri.id");
    q.addBindValue(recipeId); q.exec(); while(q.next()) rows << queryRow(q); return rows;
}

void DataStore::consumeRecipe(int recipeId, const QDate &date, double servings) {
    QSqlQuery q(db); q.prepare("SELECT * FROM recipes WHERE id=?"); q.addBindValue(recipeId);
    if (!q.exec() || !q.next()) return;
    const double totalServings=std::max(1, q.value("servings").toInt());
    addDaily(date, q.value("calories").toDouble()/totalServings*servings, q.value("protein").toDouble()/totalServings*servings,
             q.value("carbs").toDouble()/totalServings*servings, q.value("fat").toDouble()/totalServings*servings, 0, 0,
             QString("%1 × %2 serving(s)").arg(q.value("name").toString()).arg(servings));
}

bool DataStore::resetAllData() {
    if (!db.transaction()) return false;
    QSqlQuery q(db);
    for (const QString &table : {"daily","workouts","recipe_ingredients","recipes","pantry","profile","units","goals"})
        if (!q.exec("DELETE FROM "+table)) { db.rollback(); error=q.lastError().text(); return false; }
    if (!q.exec("INSERT INTO goals VALUES (1,2200,126,250,70,2.5)")) { db.rollback(); return false; }
    const QStringList defaults={"('weight','kg')","('height','cm')","('water','L')","('energy','kcal')","('protein','g')","('carbs','g')","('fat','g')"};
    for (const auto &values:defaults) if(!q.exec("INSERT INTO units VALUES "+values)){db.rollback();return false;}
    return db.commit();
}

QVector<QVariantMap> DataStore::dailyRange(const QDate &from, const QDate &to) const {
    QVector<QVariantMap> rows; QSqlQuery q(db);
    q.prepare("SELECT date, SUM(calories) calories, SUM(protein) protein, SUM(carbs) carbs, SUM(fat) fat, SUM(water) water, MAX(weight) weight FROM daily WHERE date BETWEEN ? AND ? GROUP BY date ORDER BY date");
    q.addBindValue(from.toString(Qt::ISODate)); q.addBindValue(to.toString(Qt::ISODate)); q.exec();
    while (q.next()) rows << queryRow(q);
    return rows;
}
