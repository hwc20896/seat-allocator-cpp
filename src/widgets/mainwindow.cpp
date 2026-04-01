#include "mainwindow.hpp"

#include <QColor>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QTimer>

#include <chrono>
#include <format>
#include <memory>
#include <numbers>
#include <regex>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "ui_mainwindow.h"
#include "utils/utils.hpp"
#include "utils/constants.hpp"

using nlohmann::json;
using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent),
    ui_(new Ui::MainWindow),
    shuffleBeginSound_(std::make_unique<QSoundEffect>()),
    shuffleEndSound_(std::make_unique<QSoundEffect>()),
    wheelClickSound_(std::make_unique<QSoundEffect>())
{
    ui_->setupUi(this);

    this->setMinimumSize(900, 550);
    this->resize(1000, 700);

    this->setWindowTitle("Seat Allocator");

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
    connect(ui_->actionpreviousGrid, &QAction::triggered, this, &MainWindow::onPreviousGridClicked);
    connect(ui_->actionnextGrid, &QAction::triggered, this, &MainWindow::onNextGridClicked);
    connect(ui_->toOriginal, &QPushButton::toggled, this, &MainWindow::onOriginalGridChecked);
    connect(ui_->actionLoadConstraint, &QAction::triggered, this, &MainWindow::onLoadConstraints);
    connect(ui_->actionUnloadConstraint, &QAction::triggered, this, &MainWindow::onUnloadConstraints);

    shuffleBeginSound_->setVolume(.6f);
    shuffleBeginSound_->setSource({Constants::SHUFFLE_START_SOUND});
    shuffleEndSound_->setVolume(.6f);
    shuffleEndSound_->setSource({Constants::SHUFFLE_END_SOUND});
    wheelClickSound_->setVolume(.6f);
    wheelClickSound_->setSource({Constants::SHUFFLE_WHEEL_CLICK_SOUND});
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
        currentGrid_ = grid;
        this->refreshGrid(shuffler_.getGrid());
    }
    catch (const std::invalid_argument& e) {
        spdlog::error("Error: {}", e.what());
        QMessageBox::critical(this, "無效的檔案", "此檔案含有重複元素，無法導入。請重試。");
        return;
    }

    ui_->toOriginal->setEnabled(false);

    statusBarText_->setText(QString("已導入檔案：%1").arg(getFileBasename(fileName.toStdString())));
    ui_->pageIndicator->setText("原始列表");
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
        currentGrid_ = grid;
        this->refreshGrid(shuffler_.getGrid());
    }
    catch (const std::invalid_argument& e) {
        spdlog::error("Error: {}", e.what());
        QMessageBox::critical(this, "無效的檔案", "此檔案含有重複元素，無法導入。請重試。");
        return;
    }

    ui_->toOriginal->setEnabled(false);

    statusBarText_->setText(QString("已導入檔案：%1").arg(getFileBasename(fileName.toStdString())));
    ui_-> pageIndicator->setText("原始列表");
}

void MainWindow::onExportToCSV() {
    spdlog::debug("onExportToCSV() clicked");

    if (currentGrid_.empty()) {
        spdlog::error("Nothing to export. Perhaps you forgot to import anything?");
        QMessageBox::critical(this, "檔案未導入", "請先導入檔案。");
        return;
    }

    if (currentGrid_ == shuffler_.getOriginalGrid()){
        spdlog::warn("This is the original grid. This is not recommended to export it.");
        const auto reply = QMessageBox::warning(
            this,
            "確認導出",
            "這是原始名單，確定要導出嗎？\n\n建議先執行洗牌操作後再導出。",
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel
        );

        if (reply != QMessageBox::Ok)
            return;
    }

    const QString fileName = QFileDialog::getSaveFileName(this, "輸出座位配置", "", "CSV 檔案 (*.csv)");
    if (fileName.isEmpty()) {
        spdlog::error("Unable to get file name. It probably means that the user canceled the operation.");
        return;
    }

    writeCSV(fileName.toStdString(), currentGrid_);

    spdlog::info("Export to CSV done. File: {}", fileName.toStdString());
    statusBarText_->setText(QString("已導出至檔案：%1").arg(getFileBasename(fileName.toStdString())));
}

void MainWindow::onExportToXLSX() {
    spdlog::debug("onExportToXLSX() clicked");

    if (currentGrid_.empty()) {
        spdlog::error("Nothing to export. Perhaps you forgot to import anything?");
        QMessageBox::critical(this, "檔案未導入", "請先導入檔案。");
        return;
    }

    if (currentGrid_ == shuffler_.getOriginalGrid()){
        spdlog::warn("This is the original grid. This is not recommended to export it.");
        const auto reply = QMessageBox::warning(
            this,
            "確認導出",
            "這是原始名單，確定要導出嗎？\n\n建議先執行洗牌操作後再導出。",
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel
        );

        if (reply != QMessageBox::Ok)
            return;
    }

    const QString fileName = QFileDialog::getSaveFileName(this, "輸出座位配置", "", "Excel 檔案 (*.xlsx)");
    if (fileName.isEmpty()) {
        spdlog::error("Unable to get file name. It probably means that the user canceled the operation.");
        return;
    }

    writeXLSX(fileName.toStdString(), currentGrid_);

    spdlog::info("Export to EXCEL done. File: {}", fileName.toStdString());
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

void MainWindow::refreshGrid(const Grid& grid, const bool isOriginal) const {
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

            if (isOriginal) {
                item->setBackground(QColor(207, 248, 248));
            }

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
    ui_->actionbeginShuffle->setEnabled(false);

    shuffleAnimationIteration_ = 0;

    spdlog::debug("After reset: shuffleAnimationIteration_ = {}", shuffleAnimationIteration_);

    shuffleAnimationGrid_ = shuffler_.getOriginalGrid();

    constexpr auto shuffleCount = 40;
    constexpr auto minDelay = 50ms;   // Fastest (middle)
    constexpr auto maxDelay = 300ms;  // Slowest (start/end)

    auto* timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [this, timer, minDelay, maxDelay] {
        try {
            if (shuffleAnimationIteration_ < shuffleCount) {
                shuffleGrid(shuffleAnimationGrid_);
                this->refreshGrid(shuffleAnimationGrid_);
                wheelClickSound_->play();
                ++shuffleAnimationIteration_;

                double progress = static_cast<double>(shuffleAnimationIteration_) / shuffleCount;
                const auto nextDelay = getDelayForProgress(progress, minDelay, maxDelay);

                // Restart timer with new delay
                timer->stop();
                timer->setInterval(nextDelay);
                timer->start();

                spdlog::debug("Completed iteration no. {}", shuffleAnimationIteration_);
            } else {
                spdlog::debug("Reached shuffle limit, finalizing...");

                shuffler_.shuffle();
                const auto& finalGrid = shuffler_.getGrid();
                currentGrid_ = finalGrid;
                this->refreshGrid(finalGrid);
                shuffleEndSound_->play();

                timer->stop();
                timer->deleteLater();
                ui_->beginShuffle->setEnabled(true);
                ui_->actionbeginShuffle->setEnabled(true);
                spdlog::info("Shuffle completed: generated and displayed new shuffled grid.");

                currentIndex_ = static_cast<int>(shuffler_.getSize());
                this->refreshPageIndicator();

                ui_->toOriginal->setEnabled(true);
            }
        }
        catch (const std::exception& e) {
            spdlog::error("Error: {}", e.what());
            QMessageBox::critical(this, "洗牌錯誤",
                QString("洗牌過程中發生錯誤：%1").arg(e.what()));
            timer->stop();
            timer->deleteLater();
            ui_->beginShuffle->setEnabled(true);
            ui_->actionbeginShuffle->setEnabled(true);
        }
    });

    shuffleBeginSound_->play();
    timer->start(maxDelay);
}

void MainWindow::onPreviousGridClicked() {
    spdlog::debug("onPreviousGridClicked() clicked");

    if (currentIndex_ == 1) return;

    --currentIndex_;
    this->refreshGrid(shuffler_.getGrid(currentIndex_-1));
    this->refreshPageIndicator();
}

void MainWindow::onNextGridClicked() {
    spdlog::debug("onNextGridClicked() clicked");

    if (currentIndex_ == static_cast<int>(shuffler_.getSize())) return;

    ++currentIndex_;
    this->refreshGrid(shuffler_.getGrid(currentIndex_-1));
    this->refreshPageIndicator();
}


void MainWindow::refreshPageIndicator() const {
    ui_->pageIndicator->setText(QString("第 %1 次打亂").arg(currentIndex_));

    const auto totalPageCount = static_cast<int>(shuffler_.getSize());
    ui_->previousGrid->setEnabled(currentIndex_ != 1);
    ui_->actionpreviousGrid->setEnabled(currentIndex_ != 1);
    ui_->nextGrid->setEnabled(currentIndex_ != totalPageCount);
    ui_->actionnextGrid->setEnabled(currentIndex_ != totalPageCount);
}

void MainWindow::onOriginalGridChecked(const bool checked) {
    spdlog::debug("onOriginalGridChecked() toggled with state = {}", checked);

    const auto& targetGrid = checked ? shuffler_.getOriginalGrid() : shuffler_.getGrid();
    this->refreshGrid(targetGrid, checked);

    ui_->pageIndicator->setText(checked ? "原始列表" : QString("第 %1 次打亂").arg(currentIndex_));
}

MainWindow::time_type MainWindow::getDelayForProgress(const double progress, const time_type minDelay, const time_type maxDelay) {
    const auto normalized = std::sin(progress * std::numbers::pi);
    const auto delay = maxDelay - normalized * (maxDelay - minDelay);
    spdlog::debug("Progress: {}, Delay: {:.3f}", progress, delay.count());
    return std::chrono::duration_cast<time_type>(delay);
}

void MainWindow::onLoadConstraints() {
    spdlog::debug("onLoadConstraints() clicked");

    const auto filePath = QFileDialog::getOpenFileName(this, "載入座位", "", "JSON 檔案 (*.json)");
    if (filePath.isEmpty()) {
        spdlog::error("Unable to get file name. It probably means that the file doesn't exist or the user canceled the operation.");
        return;
    }

    const auto json = getFileContent(filePath.toStdString());

    if (json.empty()) {
        spdlog::error("File is empty.");
        return;
    }

    const auto config = ShuffleConfig::from_json(json::parse(json));

    spdlog::debug("Got constraints: ");
    spdlog::debug("- allow_fixed_points = {}", config.allow_fixed_points);
    spdlog::debug("- allow_original_neighbors = {}", config.allow_original_neighbors);
    spdlog::debug("- diagonals_are_neighbors = {}", config.diagonals_are_neighbors);
    spdlog::debug("- constraints.size() = {}", config.constraints.size());
    spdlog::debug("That's all that I can do rn.");

    this->config_ = config;

    this->statusBarText_->setText(QString("算法約束載入完成：%1。").arg(filePath));
    ui_->actionUnloadConstraint->setEnabled(true);
}

void MainWindow::onUnloadConstraints() {
    spdlog::debug("onUnloadConstraints() clicked");

    this->config_ = ShuffleConfig{};

    this->statusBarText_->setText("已解除所有約束。");
    ui_->actionUnloadConstraint->setEnabled(false);
}