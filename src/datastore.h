#pragma once

#include <QDate>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QVector>

// DataStore owns the local SQLite database. Keeping persistence behind this
// class makes it possible to swap in a cloud/web API in a future version.
class DataStore {
public:
    explicit DataStore();
    ~DataStore();

    bool isOpen() const;
    QString lastError() const;
    QVariantMap day(const QDate &date) const;
    QVariantMap goals() const;
    QVariantMap profile() const;
    QVariantMap units() const;
    bool hasProfile() const;
    void saveProfile(const QString &name, const QDate &birthDate, double heightCm,
                     double initialWeightKg, int workoutFrequency, int laborFrequency);
    void saveUnit(const QString &stat, const QString &unit);
    void calculateGoals();
    void saveGoals(double calories, double protein, double carbs, double fat, double water);
    void addDaily(const QDate &date, double calories, double protein, double carbs,
                  double fat, double water, double weight, const QString &note = {});

    void addWorkout(const QDate &date, const QString &exercise, double weightKg,
                    const QString &weightUnit, int sets, int reps, const QString &note);
    QVector<QVariantMap> workouts(const QDate &date) const;

    int addPantryItem(const QString &name, double calories, double protein, double carbs,
                      double fat, double cost, double amount, const QString &unit, const QString &icon);
    QVector<QVariantMap> pantryItems() const;

    int addRecipe(const QString &name, int servings, double calories, double protein,
                  double carbs, double fat, double cost, const QString &instructions,
                  const QString &imagePath, const QVector<QVariantMap> &ingredients);
    QVector<QVariantMap> recipes() const;
    QVector<QVariantMap> recipeIngredients(int recipeId) const;
    void consumeRecipe(int recipeId, const QDate &date, double servings);
    bool resetAllData();

    QVector<QVariantMap> dailyRange(const QDate &from, const QDate &to) const;

private:
    QSqlDatabase db;
    QString error;
    void initialize();
};
