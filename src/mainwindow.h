#pragma once

#include <QMainWindow>
#include <functional>
#include "datastore.h"

class QDateEdit;
class QDoubleSpinBox;
class QSpinBox;
class QLineEdit;
class QTextEdit;
class QTableWidget;
class QListWidget;
class QComboBox;
class QStackedWidget;
class QLabel;
class QProgressBar;
class QFrame;
class QPushButton;
class QGridLayout;
class QBoxLayout;
class QResizeEvent;
class ChartWidget;

// MainWindow builds the five-page interface and coordinates user actions with
// DataStore. Individual page builders keep the UI structure easy to navigate.
class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    DataStore store;
    QStackedWidget *pages{};
    QFrame *navigation{};
    QLabel *navigationBrand{}, *navigationPrivacy{};
    QVector<QPushButton*> navigationButtons;
    QStringList navigationNames, navigationIcons;
    QBoxLayout *dashboardContent{}, *recipePageLayout{};
    QGridLayout *logGrid{}, *workoutFormLayout{}, *analyticsGrid{};
    QDate activeDashboardDate{QDate::currentDate()}, activeLogDate{QDate::currentDate()};
    QWidget *dashboard{};
    QLabel *dashboardDate{}, *logDateLabel{}, *profileHeading{}, *weightValue{}, *calorieValue{};
    QLabel *proteinValue{}, *carbValue{}, *fatValue{}, *waterValue{}, *totalValue{};
    QLabel *logCalorieUnit{}, *logProteinUnit{}, *logCarbUnit{}, *logFatUnit{}, *logWaterUnit{}, *logWeightUnit{};
    QProgressBar *calorieBar{}, *proteinBar{}, *carbBar{}, *fatBar{}, *waterBar{}, *totalBar{};

    QDoubleSpinBox *logCalories{}, *logProtein{}, *logCarbs{}, *logFat{}, *logWater{}, *logWeight{};
    QComboBox *logRecipeChoice{};
    QDoubleSpinBox *logRecipeServings{};

    QDate activeWorkoutDate{QDate::currentDate()};
    QLabel *workoutDateLabel{};
    QLineEdit *exerciseName{}, *workoutNote{};
    QComboBox *workoutUnit{};
    QDoubleSpinBox *workoutWeight{};
    QSpinBox *workoutSets{}, *workoutReps{};
    QTableWidget *workoutTable{};

    QListWidget *pantryList{};
    QVector<QVariantMap> loadedPantry;

    QLineEdit *recipeName{};
    QSpinBox *recipeServings{};
    QTableWidget *recipeIngredients{};
    QTextEdit *recipeInstructions{};
    QLabel *recipeImagePreview{};
    QString selectedImage;
    QListWidget *recipeList{};
    QVector<QVariantMap> loadedRecipes;

    QVector<ChartWidget*> analyticsCharts;

    QWidget *buildDashboard();
    QWidget *buildLogging();
    QWidget *buildWorkouts();
    QWidget *buildPantry();
    QWidget *buildRecipes();
    QWidget *buildAnalytics();
    QWidget *buildSettings();
    QWidget *buildFirstSetup(QWidget *mainApplication);
    QWidget *dateNavigator(QLabel **label, QDate *date, const std::function<void()> &changed);
    void refreshDashboard();
    void refreshWorkouts();
    void refreshPantry();
    void refreshRecipes();
    void refreshLogRecipes();
    void refreshAnalytics();
protected:
    void resizeEvent(QResizeEvent *event) override;
};
