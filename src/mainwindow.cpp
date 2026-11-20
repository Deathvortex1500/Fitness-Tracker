#include "mainwindow.h"
#include "chartwidget.h"

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QTimer>
#include <cmath>

namespace {
QDoubleSpinBox *number(double maximum=10000, const QString &suffix={}) {
    auto *box = new QDoubleSpinBox; box->setRange(0, maximum); box->setDecimals(1);
    box->setSuffix(suffix); box->setMinimumHeight(38); return box;
}
QLabel *title(const QString &text, const QString &sub={}) {
    auto *label = new QLabel(QString("<h1>%1</h1><span style='color:#91a0ad'>%2</span>").arg(text,sub));
    label->setTextFormat(Qt::RichText); label->setWordWrap(true); return label;
}
QWidget *statCard(const QString &name, QLabel **value, QProgressBar **bar, const QString &accent) {
    auto *card = new QFrame; card->setObjectName("card");
    auto *layout = new QVBoxLayout(card); auto *caption = new QLabel(name);
    caption->setAlignment(Qt::AlignCenter); caption->setStyleSheet(QString("color:%1;font-weight:800;letter-spacing:1px").arg(accent)); *value = new QLabel("—");
    (*value)->setAlignment(Qt::AlignCenter); (*value)->setStyleSheet(QString("font-size:18px;font-weight:700;color:%1;background:#161b22;border-radius:10px;padding:10px").arg(accent)); *bar = new QProgressBar;
    (*bar)->setTextVisible(false); (*bar)->setRange(0,100);
    (*bar)->setStyleSheet("QProgressBar::chunk{background:#30363d;border-radius:4px}");
    layout->addWidget(caption); layout->addWidget(*value); layout->addWidget(*bar); return card;
}

class StickFigure final : public QWidget {
public:
    explicit StickFigure(QWidget *parent=nullptr):QWidget(parent){setMinimumSize(220,350);}
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);const qreal cx=width()/2.0;QColor color("#3f9188");
        p.setPen(Qt::NoPen);p.setBrush(color);p.drawEllipse(QPointF(cx,58),30,30);
        QPen pen(color,16,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);p.setPen(pen);p.drawLine(QPointF(cx,96),QPointF(cx,220));
        p.drawLine(QPointF(cx,126),QPointF(cx-72,184));p.drawLine(QPointF(cx,126),QPointF(cx+72,184));
        p.drawLine(QPointF(cx,216),QPointF(cx-55,320));p.drawLine(QPointF(cx,216),QPointF(cx+55,320));
    }
};
QString metricLabel(const QString &metric) {
    if (metric=="calories") return "Calories";
    if (metric=="water") return "Water";
    if (metric=="weight") return "Weight";
    return metric.left(1).toUpper()+metric.mid(1);
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Fitness · v0.86"); resize(1280, 820); setMinimumSize(320,480);
    qApp->setStyleSheet(R"(
        *{font-family:Inter,Segoe UI,Sans;font-size:14px} QMainWindow,QWidget{background:#0d1117;color:#d8dee9} QLabel{padding:4px 6px} QCheckBox,QRadioButton{padding:5px}
        #nav{background:#090d12;border-right:1px solid #252b33} #navButton{text-align:left;padding:0 18px;background:transparent;border:0;color:#8b949e;border-radius:8px}
        #navButton:hover{background:#151b23;color:#6daaa3} #navButton:checked{background:#192028;color:#4d9b92;font-weight:800;border-left:3px solid #3f9188}
        #card,QGroupBox{background:#131920;border:1px solid #29313a;border-radius:12px;padding:14px} QGroupBox{margin-top:12px;font-weight:700} QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 6px;color:#4d9b92}
        QPushButton{background:#1c242d;border:1px solid #303a45;border-radius:8px;padding:10px 16px;color:#c9d1d9} QPushButton:hover{background:#25303b;border-color:#3f9188} QPushButton#primary{background:#285d58;color:#e6fffc;font-weight:800;border:0}
        QLineEdit,QTextEdit,QSpinBox,QDoubleSpinBox,QDateEdit,QComboBox{background:#0b1015;border:1px solid #303a45;border-radius:7px;padding:7px;color:#d8dee9} QProgressBar{background:#1c232b;border:0;border-radius:5px;height:10px}
        QTableWidget,QListWidget{background:#10161c;border:1px solid #29313a;border-radius:8px;gridline-color:#29313a} QHeaderView::section{background:#182029;color:#5a9d96;border:0;padding:9px}
    )");
    auto *root = new QWidget; auto *outer = new QHBoxLayout(root); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
    navigation = new QFrame; navigation->setObjectName("nav"); navigation->setFixedWidth(210); auto *navLayout = new QVBoxLayout(navigation);
    navigationBrand = new QLabel("FITNESS<br><span style='font-size:10px;color:#668f8a'>v0.86</span>"); navigationBrand->setTextFormat(Qt::RichText); navigationBrand->setStyleSheet("font-size:19px;font-weight:900;color:#4d9b92;padding:18px 10px;letter-spacing:1px"); navLayout->addWidget(navigationBrand);
    pages = new QStackedWidget;
    navigationNames={"Overview","Log","Workouts","Pantry","Recipes","Analytics","Settings"};
    navigationIcons={"⌂","＋","◆","▦","▤","↗","⚙"};
    pages->addWidget(buildDashboard()); pages->addWidget(buildLogging()); pages->addWidget(buildWorkouts());
    pages->addWidget(buildPantry()); pages->addWidget(buildRecipes()); pages->addWidget(buildAnalytics()); pages->addWidget(buildSettings());
    auto *group = new QButtonGroup(this); group->setExclusive(true);
    for(int i=0;i<navigationNames.size();++i) { auto *b=new QPushButton(navigationIcons[i]+"   "+navigationNames[i]); b->setCheckable(true); b->setObjectName("navButton"); b->setMinimumHeight(48); group->addButton(b,i); navLayout->addWidget(b);navigationButtons<<b;if(i==0)b->setChecked(true); }
    navLayout->addStretch(); navigationPrivacy=new QLabel("Your data stays on this device."); navigationPrivacy->setWordWrap(true); navigationPrivacy->setStyleSheet("color:#657484;padding:12px"); navLayout->addWidget(navigationPrivacy);
    connect(group,&QButtonGroup::idClicked,this,[this](int id){pages->setCurrentIndex(id);if(id==0)refreshDashboard();if(id==1)refreshLogRecipes();if(id==2)refreshWorkouts();if(id==3)refreshPantry();if(id==4)refreshRecipes();if(id==5)refreshAnalytics();});
    auto *pageScroll=new QScrollArea;pageScroll->setWidgetResizable(true);pageScroll->setFrameShape(QFrame::NoFrame);pageScroll->setWidget(pages);outer->addWidget(navigation);outer->addWidget(pageScroll,1);
    setCentralWidget(store.hasProfile()?root:buildFirstSetup(root));
    refreshDashboard(); refreshPantry(); refreshRecipes(); refreshLogRecipes();
}

QWidget *MainWindow::dateNavigator(QLabel **label, QDate *date, const std::function<void()> &changed) {
    auto *wrap=new QWidget; auto *row=new QHBoxLayout(wrap); row->setContentsMargins(0,0,0,0); row->setSpacing(8);
    auto *back=new QPushButton("‹"); auto *next=new QPushButton("›"); back->setFixedWidth(38); next->setFixedWidth(38);
    *label=new QLabel(date->toString("ddd, MMM d")); (*label)->setAlignment(Qt::AlignCenter); (*label)->setMinimumWidth(125); (*label)->setStyleSheet("font-weight:800;color:#4d9b92");
    row->addWidget(back); row->addWidget(*label); row->addWidget(next);
    connect(back,&QPushButton::clicked,this,[date,label,changed]{*date=date->addDays(-1);(*label)->setText(date->toString("ddd, MMM d"));changed();});
    connect(next,&QPushButton::clicked,this,[date,label,changed]{*date=date->addDays(1);(*label)->setText(date->toString("ddd, MMM d"));changed();}); return wrap;
}

QWidget *MainWindow::buildDashboard() {
    auto *page=new QWidget; auto *layout=new QVBoxLayout(page); layout->setContentsMargins(32,25,32,25);
    auto *top=new QHBoxLayout; top->addWidget(title("Overview","Your progress at a glance")); top->addStretch(); top->addWidget(dateNavigator(&dashboardDate,&activeDashboardDate,[this]{refreshDashboard();})); layout->addLayout(top);
    dashboardContent=new QHBoxLayout; auto *body=new QFrame; body->setObjectName("card"); body->setMinimumWidth(260); auto *bodyLayout=new QVBoxLayout(body);
    profileHeading=new QLabel; profileHeading->setAlignment(Qt::AlignCenter); profileHeading->setStyleSheet("font-size:22px;font-weight:900;color:#587f9b"); bodyLayout->addWidget(profileHeading); auto *figure=new StickFigure; bodyLayout->addWidget(figure,1,Qt::AlignCenter);
    weightValue=new QLabel; weightValue->setAlignment(Qt::AlignCenter); weightValue->setStyleSheet("font-size:24px;font-weight:800;color:#4d9b92"); bodyLayout->addWidget(weightValue); dashboardContent->addWidget(body,2);
    auto *grid=new QGridLayout; grid->addWidget(statCard("CALORIES",&calorieValue,&calorieBar,"#8a7958"),0,0,1,2); grid->addWidget(statCard("PROTEIN",&proteinValue,&proteinBar,"#4d8a72"),1,0); grid->addWidget(statCard("CARBS",&carbValue,&carbBar,"#587f9b"),1,1); grid->addWidget(statCard("FAT",&fatValue,&fatBar,"#765f8a"),2,0); grid->addWidget(statCard("WATER",&waterValue,&waterBar,"#4d858c"),2,1); dashboardContent->addLayout(grid,3); layout->addLayout(dashboardContent,1);
    return page;
}

QWidget *MainWindow::buildLogging() {
    auto *page=new QWidget; auto *layout=new QVBoxLayout(page); layout->setContentsMargins(34,28,34,28); auto *top=new QHBoxLayout;top->addWidget(title("Log","Log one stat at a time"));top->addStretch();top->addWidget(dateNavigator(&logDateLabel,&activeLogDate,[]{}));layout->addLayout(top);
    logGrid=new QGridLayout;auto u=store.units();logCalories=number(42000);logProtein=number(1000);logCarbs=number(1000);logFat=number(1000);logWater=number(700);logWeight=number(1100);
    logCalorieUnit=new QLabel(u["energy"].toString());logProteinUnit=new QLabel(u["protein"].toString());logCarbUnit=new QLabel(u["carbs"].toString());logFatUnit=new QLabel(u["fat"].toString());logWaterUnit=new QLabel(u["water"].toString());logWeightUnit=new QLabel(u["weight"].toString());
    const QList<QPair<QString,QDoubleSpinBox*>> fields={{"Calories",logCalories},{"Protein",logProtein},{"Carbohydrates",logCarbs},{"Fat",logFat},{"Water",logWater},{"Weight",logWeight}};
    const QList<QLabel*> unitLabels={logCalorieUnit,logProteinUnit,logCarbUnit,logFatUnit,logWaterUnit,logWeightUnit};
    for(int i=0;i<fields.size();++i){auto *card=new QGroupBox(fields[i].first);auto *row=new QHBoxLayout(card);fields[i].second->setMaximumWidth(180);row->addWidget(fields[i].second);unitLabels[i]->setMinimumWidth(42);unitLabels[i]->setStyleSheet("color:#7d8b99;font-weight:700");row->addWidget(unitLabels[i]);auto *save=new QPushButton("Log");save->setObjectName("primary");row->addWidget(save);row->addStretch();logGrid->addWidget(card,i/2,i%2);connect(save,&QPushButton::clicked,this,[this,i,fields]{double v=fields[i].second->value();if(v<=0)return;auto u=store.units();QString key=i==0?"energy":i==1?"protein":i==2?"carbs":i==3?"fat":i==4?"water":"weight";QString unit=u[key].toString();if(unit=="kJ")v/=4.184;else if(unit=="lb")v/=2.20462;else if(unit=="fl oz")v/=33.814;else if(unit=="oz")v/=.035274;double x[6]={};x[i]=v;store.addDaily(activeLogDate,x[0],x[1],x[2],x[3],x[4],x[5]);auto *input=fields[i].second;input->setSpecialValueText("✓ Logged");input->setValue(0);input->setStyleSheet("color:#63a89f;border-color:#3f9188;font-weight:700");QTimer::singleShot(1100,input,[input]{input->setSpecialValueText({});input->setStyleSheet({});});refreshDashboard();});}
    layout->addLayout(logGrid);auto *recipeBox=new QGroupBox("Log a saved recipe");auto *recipeRow=new QHBoxLayout(recipeBox);logRecipeChoice=new QComboBox;logRecipeServings=new QDoubleSpinBox;logRecipeServings->setRange(.25,100);logRecipeServings->setSingleStep(.25);logRecipeServings->setValue(1);logRecipeServings->setSuffix(" serving(s)");auto *logRecipe=new QPushButton("Log recipe");logRecipe->setObjectName("primary");recipeRow->addWidget(logRecipeChoice,1);recipeRow->addWidget(logRecipeServings);recipeRow->addWidget(logRecipe);layout->addWidget(recipeBox);layout->addStretch();connect(logRecipe,&QPushButton::clicked,this,[this]{int id=logRecipeChoice->currentData().toInt();if(id<=0)return;store.consumeRecipe(id,activeLogDate,logRecipeServings->value());refreshDashboard();QMessageBox::information(this,"Recipe logged",QString("%1 serving(s) added to %2.").arg(logRecipeServings->value()).arg(activeLogDate.toString("ddd, MMM d")));});return page;
}

QWidget *MainWindow::buildWorkouts() {
    auto *page=new QWidget; auto *layout=new QVBoxLayout(page); layout->setContentsMargins(34,28,34,28); auto *top=new QHBoxLayout; top->addWidget(title("Workouts","Track strength and training volume"));top->addStretch();top->addWidget(dateNavigator(&workoutDateLabel,&activeWorkoutDate,[this]{refreshWorkouts();}));layout->addLayout(top);
    auto *entry=new QGroupBox("Log exercise");workoutFormLayout=new QGridLayout(entry);auto *form=workoutFormLayout;exerciseName=new QLineEdit;exerciseName->setPlaceholderText("e.g. Barbell squat");workoutWeight=number(1000," kg");workoutSets=new QSpinBox;workoutSets->setRange(1,100);workoutSets->setValue(3);workoutReps=new QSpinBox;workoutReps->setRange(1,1000);workoutReps->setValue(8);workoutNote=new QLineEdit;
    const QList<QPair<QString,QWidget*>> fields={{"Exercise",exerciseName},{"Weight",workoutWeight},{"Sets",workoutSets},{"Reps",workoutReps},{"Notes",workoutNote}};for(int i=0;i<fields.size();++i){form->addWidget(new QLabel(fields[i].first),0,i);form->addWidget(fields[i].second,1,i);}auto *add=new QPushButton("Add");add->setObjectName("primary");form->addWidget(add,1,5);layout->addWidget(entry);
    workoutTable=new QTableWidget(0,5);workoutTable->setHorizontalHeaderLabels({"Exercise","Weight","Sets","Reps","Volume"});workoutTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);workoutTable->setEditTriggers(QAbstractItemView::NoEditTriggers);layout->addWidget(workoutTable,1);
    connect(add,&QPushButton::clicked,this,[this]{if(exerciseName->text().trimmed().isEmpty())return;store.addWorkout(activeWorkoutDate,exerciseName->text(),workoutWeight->value(),workoutSets->value(),workoutReps->value(),workoutNote->text());exerciseName->clear();workoutNote->clear();refreshWorkouts();});return page;
}

QWidget *MainWindow::buildPantry() {
    auto *page=new QWidget;auto *layout=new QVBoxLayout(page);layout->setContentsMargins(34,28,34,28);
    auto *top=new QHBoxLayout;top->addWidget(title("Pantry","Create the ingredients used to calculate your recipes"));top->addStretch();auto *add=new QPushButton("+ Add ingredient");add->setObjectName("primary");top->addWidget(add);layout->addLayout(top);
    pantryList=new QListWidget;layout->addWidget(pantryList,1);
    connect(add,&QPushButton::clicked,this,[this]{QDialog dialog(this);dialog.setWindowTitle("Add pantry ingredient");dialog.resize(std::min(440,width()-24),std::min(560,height()-24));auto *layout=new QVBoxLayout(&dialog);auto *form=new QFormLayout;auto *name=new QLineEdit;auto *amount=number(100000);amount->setValue(100);auto *unit=new QComboBox;unit->addItems({"g","kg","mg","mL","L","tsp","tbsp","cup","fl oz","oz","lb","piece"});auto *calories=number(100000," kcal");auto *protein=number(10000," g");auto *carbs=number(10000," g");auto *fat=number(10000," g");auto *icon=new QComboBox;icon->setEditable(true);icon->addItems({"🥕","🥦","🍅","🥔","🧅","🧄","🥚","🥛","🧀","🥩","🍗","🐟","🍚","🍝","🌾","🫘","🥜","🍎","🍌","🫐","🧂","🌿"});form->addRow("Name",name);auto *reference=new QWidget;auto *referenceLayout=new QHBoxLayout(reference);referenceLayout->setContentsMargins(0,0,0,0);referenceLayout->addWidget(amount,1);referenceLayout->addWidget(unit);form->addRow("Nutrition applies to",reference);form->addRow("Calories",calories);form->addRow("Protein",protein);form->addRow("Carbohydrates",carbs);form->addRow("Fat",fat);form->addRow("Icon",icon);layout->addLayout(form);auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{if(name->text().trimmed().isEmpty()||amount->value()<=0){name->setFocus();return;}store.addPantryItem(name->text().trimmed(),calories->value(),protein->value(),carbs->value(),fat->value(),amount->value(),unit->currentText(),icon->currentText());dialog.accept();});if(dialog.exec()==QDialog::Accepted){refreshPantry();refreshRecipes();}});return page;
}

QWidget *MainWindow::buildRecipes() {
    auto *page=new QWidget;recipePageLayout=new QHBoxLayout(page);auto *layout=recipePageLayout;layout->setContentsMargins(34,28,34,28);auto *left=new QVBoxLayout;left->addWidget(title("Recipes","Nutrition is calculated automatically from your pantry"));auto *form=new QGroupBox("Create recipe");auto *f=new QFormLayout(form);recipeName=new QLineEdit;recipeServings=new QSpinBox;recipeServings->setRange(1,100);recipeServings->setValue(1);f->addRow("Name",recipeName);f->addRow("Total servings",recipeServings);
    f->addRow(new QLabel("<b>Ingredients</b>"));recipeIngredients=new QTableWidget(0,3);recipeIngredients->setHorizontalHeaderLabels({"Pantry ingredient","Amount","Unit"});recipeIngredients->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);recipeIngredients->setMinimumHeight(190);f->addRow(recipeIngredients);
    auto *ingredientButtons=new QWidget;auto *ingredientRow=new QHBoxLayout(ingredientButtons);ingredientRow->setContentsMargins(0,0,0,0);auto *addRow=new QPushButton("+ Ingredient");auto *removeRow=new QPushButton("− Remove selected");ingredientRow->addWidget(addRow);ingredientRow->addWidget(removeRow);ingredientRow->addStretch();f->addRow(ingredientButtons);
    recipeInstructions=new QTextEdit;recipeInstructions->setPlaceholderText("Describe the preparation and cooking steps…");recipeInstructions->setMinimumHeight(100);f->addRow("Instructions",recipeInstructions);recipeImagePreview=new QLabel("No image selected");recipeImagePreview->setMinimumHeight(90);recipeImagePreview->setAlignment(Qt::AlignCenter);recipeImagePreview->setStyleSheet("background:#0d151d;border:1px dashed #3b4a58;border-radius:8px;color:#788896");auto *pick=new QPushButton("Choose image");f->addRow(recipeImagePreview);f->addRow(pick);auto *save=new QPushButton("Save recipe");save->setObjectName("primary");f->addRow(save);auto *scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);scroll->setWidget(form);left->addWidget(scroll,1);layout->addLayout(left,3);
    auto *right=new QVBoxLayout;right->addWidget(new QLabel("<h2>Saved recipes</h2>"));recipeList=new QListWidget;right->addWidget(recipeList,1);layout->addLayout(right,2);
    auto addIngredientRow=[this]{loadedPantry=store.pantryItems();if(loadedPantry.isEmpty()){QMessageBox::information(this,"Pantry is empty","Add ingredients on the Pantry page first.");return;}int row=recipeIngredients->rowCount();recipeIngredients->insertRow(row);auto *choice=new QComboBox;for(const auto &item:loadedPantry)choice->addItem(item["icon"].toString()+"  "+item["name"].toString(),item["id"]);auto *amount=number(100000);amount->setValue(loadedPantry[0]["amount"].toDouble());auto *unit=new QLabel(loadedPantry[0]["unit"].toString());unit->setAlignment(Qt::AlignCenter);recipeIngredients->setCellWidget(row,0,choice);recipeIngredients->setCellWidget(row,1,amount);recipeIngredients->setCellWidget(row,2,unit);connect(choice,&QComboBox::currentIndexChanged,this,[this,choice,amount,unit](int index){auto pantry=store.pantryItems();int id=choice->itemData(index).toInt();for(const auto &item:pantry)if(item["id"].toInt()==id){amount->setValue(item["amount"].toDouble());unit->setText(item["unit"].toString());break;}});};
    connect(addRow,&QPushButton::clicked,this,addIngredientRow);connect(removeRow,&QPushButton::clicked,this,[this]{int row=recipeIngredients->currentRow();if(row>=0)recipeIngredients->removeRow(row);});connect(pick,&QPushButton::clicked,this,[this]{selectedImage=QFileDialog::getOpenFileName(this,"Choose recipe photo",{},"Images (*.png *.jpg *.jpeg *.webp)");if(!selectedImage.isEmpty()){QPixmap px(selectedImage);recipeImagePreview->setPixmap(px.scaled(180,90,Qt::KeepAspectRatio,Qt::SmoothTransformation));}});
    connect(save,&QPushButton::clicked,this,[this]{if(recipeName->text().trimmed().isEmpty()||recipeIngredients->rowCount()==0)return;auto pantry=store.pantryItems();QVector<QVariantMap> ingredients;double calories=0,protein=0,carbs=0,fat=0;for(int row=0;row<recipeIngredients->rowCount();++row){auto *choice=qobject_cast<QComboBox*>(recipeIngredients->cellWidget(row,0));auto *amount=qobject_cast<QDoubleSpinBox*>(recipeIngredients->cellWidget(row,1));int id=choice->currentData().toInt();for(const auto &p:pantry)if(p["id"].toInt()==id){double scale=amount->value()/std::max(.0001,p["amount"].toDouble());calories+=p["calories"].toDouble()*scale;protein+=p["protein"].toDouble()*scale;carbs+=p["carbs"].toDouble()*scale;fat+=p["fat"].toDouble()*scale;QVariantMap item;item["pantry_id"]=id;item["name"]=p["name"];item["amount"]=amount->value();item["unit"]=p["unit"];item["icon"]=p["icon"];ingredients<<item;break;}}store.addRecipe(recipeName->text().trimmed(),recipeServings->value(),calories,protein,carbs,fat,recipeInstructions->toPlainText(),selectedImage,ingredients);recipeName->clear();recipeInstructions->clear();recipeIngredients->setRowCount(0);recipeServings->setValue(1);selectedImage.clear();recipeImagePreview->setText("No image selected");recipeImagePreview->setPixmap({});refreshRecipes();});
    return page;
}

QWidget *MainWindow::buildAnalytics() {
    auto *page=new QWidget;auto *layout=new QVBoxLayout(page);layout->setContentsMargins(34,28,34,28);layout->addWidget(title("Analytics","All weekly trends at a glance"));auto *content=new QWidget;analyticsGrid=new QGridLayout(content);analyticsCharts.clear();for(int i=0;i<6;++i){auto *chart=new ChartWidget;chart->setMinimumSize(200,220);analyticsCharts<<chart;analyticsGrid->addWidget(chart,i/2,i%2);}auto *scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);scroll->setWidget(content);layout->addWidget(scroll,1);return page;
}

QWidget *MainWindow::buildSettings() {
    auto *page=new QWidget;auto *layout=new QVBoxLayout(page);layout->setContentsMargins(34,28,34,28);layout->addWidget(title("Settings","Profile, activity, and display units"));auto p=store.profile();
    auto *profile=new QWidget;auto *form=new QFormLayout(profile);auto *name=new QLineEdit(p["name"].toString());auto *dob=new QDateEdit(QDate::fromString(p["birth_date"].toString(),Qt::ISODate));dob->setCalendarPopup(true);dob->setDisplayFormat("ddd, MMM d");auto *height=number(300," cm");height->setValue(p["height_cm"].toDouble());auto *weight=number(500," kg");weight->setValue(p["initial_weight_kg"].toDouble());auto *workouts=new QSpinBox;workouts->setRange(0,14);workouts->setValue(p["workout_frequency"].toInt());auto *labor=new QSpinBox;labor->setRange(0,14);labor->setValue(p["labor_frequency"].toInt());form->addRow("Name",name);form->addRow("Date of birth",dob);form->addRow("Height",height);form->addRow("Initial weight",weight);form->addRow("Workouts / week",workouts);form->addRow("Labor-intensive days / 2 weeks",labor);auto *save=new QPushButton("Save");save->setObjectName("primary");form->addRow(save);layout->addWidget(profile);
    auto *unitsBox=new QGroupBox("Units");auto *unitsLayout=new QGridLayout(unitsBox);auto units=store.units();const QList<QStringList> choices={{"Energy","energy","kcal","kJ"},{"Weight / workout load","weight","kg","lb"},{"Height","height","cm","in"},{"Water","water","L","fl oz"},{"Protein","protein","g","oz"},{"Carbohydrates","carbs","g","oz"},{"Fat","fat","g","oz"}};for(int i=0;i<choices.size();++i){auto c=choices[i];auto *label=new QLabel(c[0]);auto *toggle=new QPushButton;toggle->setCheckable(true);toggle->setChecked(units[c[1]].toString()==c[3]);auto update=[toggle,c]{toggle->setText(toggle->isChecked()?c[3]:c[2]);};update();connect(toggle,&QPushButton::toggled,this,[this,toggle,c,update](bool){update();QString unit=toggle->isChecked()?c[3]:c[2];store.saveUnit(c[1],unit);if(c[1]=="energy")logCalorieUnit->setText(unit);else if(c[1]=="weight"){logWeightUnit->setText(unit);workoutWeight->setSuffix(" "+unit);}else if(c[1]=="water")logWaterUnit->setText(unit);else if(c[1]=="protein")logProteinUnit->setText(unit);else if(c[1]=="carbs")logCarbUnit->setText(unit);else if(c[1]=="fat")logFatUnit->setText(unit);refreshDashboard();});unitsLayout->addWidget(label,i/2,(i%2)*2);unitsLayout->addWidget(toggle,i/2,(i%2)*2+1);}layout->addWidget(unitsBox);
    auto *danger=new QGroupBox("Reset all data");auto *dangerLayout=new QHBoxLayout(danger);auto *warning=new QLabel("Permanently erase your profile, logs, workouts, recipes, goals, and unit settings.");warning->setWordWrap(true);auto *reset=new QPushButton("Reset all data");reset->setStyleSheet("color:#ff8585;border-color:#7a3f46");dangerLayout->addWidget(warning,1);dangerLayout->addWidget(reset);layout->addWidget(danger);layout->addStretch();
    connect(reset,&QPushButton::clicked,this,[this]{if(QMessageBox::warning(this,"Reset all data","This permanently deletes every saved item. Continue?",QMessageBox::Cancel|QMessageBox::Yes,QMessageBox::Cancel)!=QMessageBox::Yes)return;if(QMessageBox::critical(this,"Final confirmation","This cannot be undone. Reset everything now?",QMessageBox::Cancel|QMessageBox::Yes,QMessageBox::Cancel)!=QMessageBox::Yes)return;if(store.resetAllData()){QMessageBox::information(this,"Data reset","All data was erased. The app will close; reopen it to run setup again.");qApp->quit();}else QMessageBox::critical(this,"Reset failed",store.lastError());});connect(save,&QPushButton::clicked,this,[this,name,dob,height,weight,workouts,labor]{store.saveProfile(name->text(),dob->date(),height->value(),weight->value(),workouts->value(),labor->value());refreshDashboard();QMessageBox::information(this,"Saved","Profile and TDEE goals saved.");});return page;
}

QWidget *MainWindow::buildFirstSetup(QWidget *mainApplication) {
    auto *page=new QWidget;auto *outer=new QVBoxLayout(page);outer->setContentsMargins(80,48,80,48);auto *card=new QFrame;card->setObjectName("card");card->setMaximumWidth(620);auto *layout=new QVBoxLayout(card);layout->setContentsMargins(44,36,44,36);layout->setSpacing(16);layout->addWidget(title("Welcome to Fitness","Complete setup to calculate your TDEE and daily goals"));auto *form=new QFormLayout;form->setVerticalSpacing(14);auto *name=new QLineEdit;name->setPlaceholderText("Your name");auto *dob=new QDateEdit(QDate::currentDate().addYears(-25));dob->setCalendarPopup(true);dob->setDisplayFormat("ddd, MMM d");dob->setMaximumDate(QDate::currentDate());
    auto *height=number(300," cm");height->setValue(170);auto *heightUnit=new QPushButton("cm");heightUnit->setCheckable(true);auto *heightRow=new QWidget;auto *heightLayout=new QHBoxLayout(heightRow);heightLayout->setContentsMargins(0,0,0,0);heightLayout->addWidget(height,1);heightLayout->addWidget(heightUnit);connect(heightUnit,&QPushButton::toggled,page,[height,heightUnit](bool feet){double cm=feet?height->value():height->value()*30.48;height->setRange(0,feet?9:300);height->setDecimals(feet?2:1);height->setValue(feet?cm/30.48:cm);height->setSuffix(feet?" ft":" cm");heightUnit->setText(feet?"ft":"cm");});
    auto *weight=number(500," kg");weight->setValue(70);auto *weightUnit=new QPushButton("kg");weightUnit->setCheckable(true);auto *weightRow=new QWidget;auto *weightLayout=new QHBoxLayout(weightRow);weightLayout->setContentsMargins(0,0,0,0);weightLayout->addWidget(weight,1);weightLayout->addWidget(weightUnit);connect(weightUnit,&QPushButton::toggled,page,[weight,weightUnit](bool pounds){double kg=pounds?weight->value():weight->value()/2.20462;weight->setRange(0,pounds?1100:500);weight->setValue(pounds?kg*2.20462:kg);weight->setSuffix(pounds?" lb":" kg");weightUnit->setText(pounds?"lb":"kg");});
    auto *workouts=new QSpinBox;workouts->setRange(0,14);workouts->setValue(3);auto *labor=new QSpinBox;labor->setRange(0,14);form->addRow("Name",name);form->addRow("Date of birth",dob);form->addRow("Height",heightRow);form->addRow("Initial weight",weightRow);form->addRow("Workouts / week",workouts);form->addRow("Labor-intensive days / 2 weeks",labor);layout->addLayout(form);auto *start=new QPushButton("Start tracking");start->setObjectName("primary");layout->addWidget(start);outer->addStretch();outer->addWidget(card,0,Qt::AlignCenter);outer->addStretch();
    connect(start,&QPushButton::clicked,page,[=,this]{if(name->text().trimmed().isEmpty()){name->setFocus();return;}double heightCm=heightUnit->isChecked()?height->value()*30.48:height->value();double weightKg=weightUnit->isChecked()?weight->value()/2.20462:weight->value();store.saveUnit("height",heightUnit->isChecked()?"ft":"cm");store.saveUnit("weight",weightUnit->isChecked()?"lb":"kg");store.saveProfile(name->text().trimmed(),dob->date(),heightCm,weightKg,workouts->value(),labor->value());auto *oldSettings=pages->widget(6);pages->removeWidget(oldSettings);oldSettings->deleteLater();pages->insertWidget(6,buildSettings());refreshDashboard();QTimer::singleShot(0,this,[this,mainApplication]{setCentralWidget(mainApplication);});});return page;
}

void MainWindow::refreshDashboard() {
    if(!calorieValue) return;auto d=store.day(activeDashboardDate),g=store.goals(),u=store.units(),p=store.profile();auto convert=[](double v,const QString &unit){if(unit=="kJ")return v*4.184;if(unit=="lb")return v*2.20462;if(unit=="fl oz")return v*33.814;if(unit=="oz")return v*.035274;return v;};auto update=[&](QLabel *label,QProgressBar *bar,double value,double goal,const QString &unit){int pct=goal>0?std::round(value/goal*100):0;label->setText(QString("%1 / %2 %3  ·  %4%").arg(convert(value,unit),0,'f',1).arg(convert(goal,unit),0,'f',1).arg(unit).arg(pct));bar->setValue(std::min(pct,100));};update(calorieValue,calorieBar,d["calories"].toDouble(),g["calories"].toDouble(),u["energy"].toString());update(proteinValue,proteinBar,d["protein"].toDouble(),g["protein"].toDouble(),u["protein"].toString());update(carbValue,carbBar,d["carbs"].toDouble(),g["carbs"].toDouble(),u["carbs"].toString());update(fatValue,fatBar,d["fat"].toDouble(),g["fat"].toDouble(),u["fat"].toString());update(waterValue,waterBar,d["water"].toDouble(),g["water"].toDouble(),u["water"].toString());QString wu=u["weight"].toString();weightValue->setText(d["weight"].toDouble()>0?QString::number(convert(d["weight"].toDouble(),wu),'f',1)+" "+wu:"No weight logged");if(profileHeading){QDate birth=QDate::fromString(p["birth_date"].toString(),Qt::ISODate);int age=birth.year()>0?birth.daysTo(QDate::currentDate())/365.2425:0;profileHeading->setText(QString("%1  ·  %2").arg(p["name"].toString()).arg(age));}
}

void MainWindow::refreshWorkouts(){if(!workoutTable)return;auto rows=store.workouts(activeWorkoutDate);workoutTable->setRowCount(rows.size());for(int r=0;r<rows.size();++r){auto x=rows[r];QStringList values={x["exercise"].toString(),QString::number(x["weight"].toDouble(),'f',1)+" kg",x["sets"].toString(),x["reps"].toString(),QString::number(x["weight"].toDouble()*x["sets"].toInt()*x["reps"].toInt(),'f',0)+" kg"};for(int c=0;c<values.size();++c)workoutTable->setItem(r,c,new QTableWidgetItem(values[c]));}}

void MainWindow::refreshPantry(){if(!pantryList)return;loadedPantry=store.pantryItems();pantryList->clear();for(const auto &item:loadedPantry){auto *entry=new QListWidgetItem(QString("%1  %2\n%3 kcal · P %4g · C %5g · F %6g per %7 %8").arg(item["icon"].toString(),item["name"].toString()).arg(item["calories"].toDouble(),0,'f',1).arg(item["protein"].toDouble(),0,'f',1).arg(item["carbs"].toDouble(),0,'f',1).arg(item["fat"].toDouble(),0,'f',1).arg(item["amount"].toDouble(),0,'g',8).arg(item["unit"].toString()),pantryList);entry->setSizeHint(QSize(0,68));}}

void MainWindow::refreshRecipes(){
    if(!recipeList)return; loadedRecipes=store.recipes(); recipeList->clear();
    for(int i=0;i<loadedRecipes.size();++i){const auto r=loadedRecipes[i];const int servings=std::max(1,r["servings"].toInt());
        auto *item=new QListWidgetItem(recipeList);item->setSizeHint(QSize(0,78));auto *card=new QWidget;auto *row=new QHBoxLayout(card);row->setContentsMargins(10,6,10,6);
        auto *summary=new QLabel(QString("<b style='font-size:16px'>%1</b><br><span style='color:#91a0ad'>%2 servings · %3 kcal/serving · P %4g · C %5g · F %6g</span>").arg(r["name"].toString().toHtmlEscaped()).arg(servings).arg(r["calories"].toDouble()/servings,0,'f',0).arg(r["protein"].toDouble()/servings,0,'f',0).arg(r["carbs"].toDouble()/servings,0,'f',0).arg(r["fat"].toDouble()/servings,0,'f',0));
        auto *view=new QToolButton;view->setText("👁");view->setToolTip("View recipe and ingredients");view->setFixedSize(42,42);row->addWidget(summary,1);row->addWidget(view);recipeList->setItemWidget(item,card);
        connect(view,&QToolButton::clicked,this,[this,i]{recipeList->setCurrentRow(i);const auto r=loadedRecipes[i];QDialog dialog(this);dialog.setWindowTitle(r["name"].toString());dialog.resize(std::min(560,width()-24),std::min(540,height()-24));auto *layout=new QVBoxLayout(&dialog);layout->addWidget(new QLabel(QString("<h2>%1</h2><span style='color:#91a0ad'>%2 total servings</span>").arg(r["name"].toString().toHtmlEscaped()).arg(std::max(1,r["servings"].toInt()))));if(!r["image_path"].toString().isEmpty()){auto *image=new QLabel;image->setAlignment(Qt::AlignCenter);QPixmap px(r["image_path"].toString());image->setPixmap(px.scaled(480,180,Qt::KeepAspectRatio,Qt::SmoothTransformation));layout->addWidget(image);}layout->addWidget(new QLabel("<b>Ingredients</b>"));auto ingredients=store.recipeIngredients(r["id"].toInt());auto *table=new QTableWidget(ingredients.size(),4);table->setHorizontalHeaderLabels({"","Ingredient","Amount","Unit"});table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);table->setEditTriggers(QAbstractItemView::NoEditTriggers);for(int row=0;row<ingredients.size();++row){table->setItem(row,0,new QTableWidgetItem(ingredients[row]["icon"].toString()));table->setItem(row,1,new QTableWidgetItem(ingredients[row]["name"].toString()));table->setItem(row,2,new QTableWidgetItem(QString::number(ingredients[row]["amount"].toDouble(),'g',8)));table->setItem(row,3,new QTableWidgetItem(ingredients[row]["unit"].toString()));}layout->addWidget(table,1);layout->addWidget(new QLabel("<b>Instructions</b>"));auto *instructions=new QLabel(r["instructions"].toString().toHtmlEscaped().replace("\n","<br>"));instructions->setWordWrap(true);instructions->setTextFormat(Qt::RichText);layout->addWidget(instructions);auto *buttons=new QDialogButtonBox(QDialogButtonBox::Close);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);layout->addWidget(buttons);dialog.exec();});
    }
    refreshLogRecipes();
}

void MainWindow::refreshLogRecipes(){if(!logRecipeChoice)return;int selected=logRecipeChoice->currentData().toInt();logRecipeChoice->clear();for(const auto &recipe:store.recipes())logRecipeChoice->addItem(recipe["name"].toString(),recipe["id"]);int index=logRecipeChoice->findData(selected);if(index>=0)logRecipeChoice->setCurrentIndex(index);logRecipeChoice->setEnabled(logRecipeChoice->count()>0);}

void MainWindow::refreshAnalytics(){if(analyticsCharts.size()!=6)return;const int days=7;QDate today=QDate::currentDate(),start=today.addDays(-days+1),previousStart=start.addDays(-days);auto currentRows=store.dailyRange(start,today),previousRows=store.dailyRange(previousStart,start.addDays(-1));const QStringList keys={"calories","protein","carbs","fat","water","weight"};for(int n=0;n<keys.size();++n){QString key=keys[n];auto series=[&](const QVector<QVariantMap>&rows,QDate from){QVector<double> out(days,0);for(const auto&r:rows){int i=from.daysTo(QDate::fromString(r["date"].toString(),Qt::ISODate));if(i>=0&&i<days)out[i]=r[key].toDouble();}return out;};QString unit=key=="calories"?"kcal":key=="water"?"L":key=="weight"?"kg":"g";analyticsCharts[n]->setSeries(series(currentRows,start),series(previousRows,previousStart),metricLabel(key)+" · 7 days",unit);}}

void MainWindow::resizeEvent(QResizeEvent *event){
    QMainWindow::resizeEvent(event);const bool phone=event->size().width()<700;const bool compact=event->size().width()<980;
    if(navigation){navigation->setFixedWidth(phone?62:210);navigationBrand->setVisible(!phone);navigationPrivacy->setVisible(!phone);for(int i=0;i<navigationButtons.size();++i){navigationButtons[i]->setText(phone?navigationIcons[i]:navigationIcons[i]+"   "+navigationNames[i]);navigationButtons[i]->setToolTip(navigationNames[i]);}}
    if(dashboardContent)dashboardContent->setDirection(compact?QBoxLayout::TopToBottom:QBoxLayout::LeftToRight);
    if(recipePageLayout)recipePageLayout->setDirection(compact?QBoxLayout::TopToBottom:QBoxLayout::LeftToRight);
    if(logGrid){QVector<QWidget*> cards;for(int i=0;i<logGrid->count();++i)if(logGrid->itemAt(i)->widget())cards<<logGrid->itemAt(i)->widget();for(int i=0;i<cards.size();++i)logGrid->addWidget(cards[i],phone?i:i/2,phone?0:i%2);}
    if(analyticsGrid){for(int i=0;i<analyticsCharts.size();++i)analyticsGrid->addWidget(analyticsCharts[i],phone?i:i/2,phone?0:i%2);}
    if(pages)for(int i=0;i<pages->count();++i)if(pages->widget(i)->layout())pages->widget(i)->layout()->setContentsMargins(phone?12:34,phone?12:28,phone?12:34,phone?12:28);
}
