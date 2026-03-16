#include "mainwindow.hpp"

#include <spdlog/spdlog.h>
#include <format>
#include <QFileDialog>

#include "ui_mainwindow.h"
#include "utils/utils.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui_(new Ui::MainWindow){
    ui_->setupUi(this);

    ui_->statusbar->showMessage("未導入");

    connect(ui_->actionImportFromCSV, &QAction::triggered, this, &MainWindow::onImportFromCSV);
    connect(ui_->actionImportFromExcel, &QAction::triggered, this, &MainWindow::onImportFromXLSX);
    connect(ui_->actionExportToCSV, &QAction::triggered, this, &MainWindow::onExportToCSV);
    connect(ui_->actionExportToExcel, &QAction::triggered, this, &MainWindow::onExportToXLSX);
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

    ui_->statusbar->showMessage(QString("已導入檔案：%1").arg(getFileBasename(fileName.toStdString())));
}

void MainWindow::onImportFromXLSX() {
    spdlog::debug("onImportFromXLSX() clicked");
}

void MainWindow::onExportToCSV() {
    spdlog::debug("onExportToCSV() clicked");
    const QString fileName = QFileDialog::getSaveFileName(this, "輸出座位配置", "", "CSV 檔案 (*.csv)");
    if (fileName.isEmpty()) {
        spdlog::error("Unable to open the file. It probably means that the user canceled the operation.");
        return;
    }

    //  TODO: get grid from ui
    spdlog::info("Export to CSV done.");
    ui_->statusbar->showMessage(QString("已導出至檔案：%1").arg(getFileBasename(fileName.toStdString())));
}

void MainWindow::onExportToXLSX() {
    spdlog::debug("onExportToXLSX() clicked");
}