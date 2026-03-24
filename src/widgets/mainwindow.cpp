#include "mainwindow.hpp"

#include <QColor>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QTimer>

#include <chrono>
#include <format>
#include <memory>
#include <regex>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "ui_mainwindow.h"
#include "utils/utils.hpp"
#include "utils/constants.hpp"

using nlohmann::json;
using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui_(new Ui::MainWindow), shuffleAnimationSound_(std::make_unique<QSoundEffect>()){
    ui_->setupUi(this);

    this->resize(1000, 800);

    ui_->actionDisableColor->setEnabled(false);

    ui_->beginShuffle->setDefaultAction(ui_->actionbeginShuffle);
    ui_->previousGrid->setDefaultAction(ui_->actionpreviousGrid);
    ui_->nextGrid->setDefaultAction(ui_->actionnextGrid);

    statusBarText_ = new QLabel("未導入", this);
    statusBarText_->setStyleSheet(R"(border: none; background-color: transparent;)");
    ui_->statusbar->addWidget(statusBarText_);

    ui_->gridDisplayer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_->gridDisplayer->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui_->actionImportFromCSV, &QAction::triggered, this, &MainWindow::onImportFromCSV);
    connect(ui_->actionImportFromExcel, &QAction::triggered, this, &MainWindow::onImportFromXLSX);
    connect(ui_->actionExportToCSV, &QAction::triggered, this, &MainWindow::onExportToCSV);
    connect(ui_->actionExportToExcel, &QAction::triggered, this, &MainWindow::onExportToXLSX);
    connect(ui_->actionEnableColor, &QAction::triggered, this, &MainWindow::onImportColorPresets);
    connect(ui_->actionDisableColor, &QAction::triggered, this, &MainWindow::onDisableColorPresets);
    connect(ui_->actionbeginShuffle, &QAction::triggered, this, &MainWindow::onBeginShuffle);

    shuffleAnimationSound_->setSource({Constants::SHUFFLE_ANIMATION_SOUND});
}

MainWindow::~MainWindow(){
    delete ui_;
}

void MainWindow::onImportFromCSV() {
    spdlog::debug("onImportFromCSV() clicked");
    const QString fileName = QFileDialog::getOpenFileName(this, "導入座位配置", "", "CSV 檔案 (*.csv)");
    if (fileName.isEmpty()) {
        spdlog::error("Unable to open the file. It probably means that the file doesn't exist or the user canceled the operation.");
        return;
    }

    const auto grid = readCSV(fileName.toStdString());
    spdlog::debug(std::format("Got grid:\n{}", grid));
    spdlog::info("Import from CSV done.");

    try {
        shuffler_.setGrid(grid);
        this->refreshGrid(shuffler_.getGrid());
    }
    catch (const std::invalid_argument& e) {
        spdlog::error("Error: {}", e.what());
        QMessageBox::critical(this, "無效的檔案", "此檔案含有重複元素，無法導入。請重試。");
        return;
    }

    statusBarText_->setText(QString("已導入檔案：%1").arg(getFileBasename(fileName.toStdString())));
}

void MainWindow::onImportFromXLSX() {
    spdlog::debug("onImportFromXLSX() clicked");
    const QString fileName = QFileDialog::getOpenFileName(this, "導入座位配置", "", "Excel 檔案 (*.xlsx)");
    if (fileName.isEmpty()) {
        spdlog::error("Unable to open the file. It probably means that the file doesn't exist or the user canceled the operation.");
        return;
    }

    const auto grid = readXLSX(fileName.toStdString());
    spdlog::debug(std::format("Got grid:\n{}", grid));
    spdlog::info("Import from EXCEL done.");

    try {
        shuffler_.setGrid(grid);
        this->refreshGrid(shuffler_.getGrid());
    }
    catch (const std::invalid_argument& e) {
        spdlog::error("Error: {}", e.what());
        QMessageBox::critical(this, "無效的檔案", "此檔案含有重複元素，無法導入。請重試。");
        return;
    }

    statusBarText_->setText(QString("已導入檔案：%1").arg(getFileBasename(fileName.toStdString())));
}

void MainWindow::onExportToCSV() {
    spdlog::debug("onExportToCSV() clicked");
    const QString fileName = QFileDialog::getSaveFileName(this, "輸出座位配置", "", "CSV 檔案 (*.csv)");
    if (fileName.isEmpty()) {
        spdlog::error("Unable to get file name. It probably means that the user canceled the operation.");
        return;
    }

    //  TODO: get grid from ui

    //  TODO: export grid to csv

    spdlog::info("Export to CSV done.");
    statusBarText_->setText(QString("已導出至檔案：%1").arg(getFileBasename(fileName.toStdString())));
}

void MainWindow::onExportToXLSX() {
    spdlog::debug("onExportToXLSX() clicked");
    const QString fileName = QFileDialog::getSaveFileName(this, "輸出座位配置", "", "CSV 檔案 (*.csv)");
    if (fileName.isEmpty()) {
        spdlog::error("Unable to get file name. It probably means that the user canceled the operation.");
        return;
    }

    //  TODO: get grid from ui

    //  TODO: export grid to xlsx

    spdlog::info("Export to CSV done.");
    statusBarText_->setText(QString("已導出至檔案：%1").arg(getFileBasename(fileName.toStdString())));
}

void MainWindow::onImportColorPresets() {
    spdlog::debug("onImportColorPresets() clicked");

    const auto fileName = QFileDialog::getOpenFileName(this, "導入顏色設定", "", "JSON 檔案 (*.json)");
    if (fileName.isEmpty()) {
        spdlog::error("Unable to open the file. It probably means that the user canceled the operation.");
        return;
    }

    const auto jsonColor = getFileContent(fileName.toStdString());
    spdlog::debug("Got color json: {}", jsonColor);

    json json;
    try {
        json = json::parse(jsonColor);
    } catch (const json::parse_error& e) {
        spdlog::error("Error parsing JSON: {}", e.what());
        return;
    }

    colorMapper_.reset();

    for (const auto& [regex, color] : json.items()) {
        spdlog::debug("Regex: {}; Color: {}", regex, color.get<std::string>());
        colorMapper_.addPair(regex, color.get<std::string>());
    }

    this->refreshGrid(shuffler_.getGrid());

    ui_->actionDisableColor->setEnabled(true);
}

void MainWindow::onDisableColorPresets() {
    spdlog::debug("onDisableColorPresets() clicked");

    colorMapper_.reset();
    this->refreshGrid(shuffler_.getGrid());

    ui_->actionDisableColor->setEnabled(false);
}

void MainWindow::refreshGrid(const Grid& grid) {
    if (grid.empty()) {
        spdlog::error("Grid is empty.");
        return;
    }

    const auto rowCount = grid.size();
    const auto colCount = grid[0].size();

    ui_->gridDisplayer->setRowCount(rowCount);
    ui_->gridDisplayer->setColumnCount(colCount);

    for (const auto row : std::views::iota(0ULL, rowCount)) {
        for (const auto col : std::views::iota(0ULL, colCount)) {
            const auto text = grid[row][col];
            auto item = std::make_unique<QTableWidgetItem>(QString::fromStdString(text));
            item->setTextAlignment(Qt::AlignCenter);
            item->setFont(QFont("Microsoft JhengHei", 16));

            const auto color = colorMapper_.getColor(text);
            item->setForeground(QColor(color.c_str()));

            ui_->gridDisplayer->setItem(row, col, item.release());
        }
    }
}

void MainWindow::onBeginShuffle() {
    spdlog::debug("onBeginShuffle() clicked");

    if (shuffler_.getOriginalGrid().empty()) {
        spdlog::warn("Original grid empty, it'll lead to undesired result; did you forgot to import the grid?");
        return;
    }

    spdlog::debug("Before reset: shuffleAnimationIteration_ = {}", shuffleAnimationIteration_);

    ui_->beginShuffle->setEnabled(false);

    shuffleAnimationIteration_ = 0;

    spdlog::debug("After reset: shuffleAnimationIteration_ = {}", shuffleAnimationIteration_);

    shuffleAnimationGrid_ = shuffler_.getOriginalGrid();

    constexpr auto shuffleCount = 25;
    constexpr auto cooldownMS = 80ms;

    auto* timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [this, timer, shuffleCount]() {
        try {
            spdlog::debug("=== Timer tick: iteration={}, limit={} ===", shuffleAnimationIteration_, shuffleCount);

            if (shuffleAnimationIteration_ < shuffleCount) {
                shuffleGrid(shuffleAnimationGrid_);
                this->refreshGrid(shuffleAnimationGrid_);
                ++shuffleAnimationIteration_;
                spdlog::debug("Completed iteration no. {}", shuffleAnimationIteration_);
            } else {
                spdlog::debug("Reached shuffle limit, finalizing...");

                shuffler_.shuffle();
                const auto& finalGrid = shuffler_.getGrid();
                this->refreshGrid(finalGrid);
                shuffleAnimationSound_->play();

                timer->stop();
                timer->deleteLater();
                ui_->beginShuffle->setEnabled(true);
                spdlog::info("Shuffle completed: generated and displayed new shuffled grid.");
            }
        }
        catch (const std::exception& e) {
            spdlog::error("Error: {}", e.what());
            QMessageBox::critical(this, "洗牌錯誤",
                QString("洗牌過程中發生錯誤：%1").arg(e.what()));
            timer->stop();
            timer->deleteLater();
            ui_->beginShuffle->setEnabled(true);
        }
    });

    spdlog::debug("Timer started with interval {}ms", cooldownMS.count());
    timer->start(cooldownMS);
}
