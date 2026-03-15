#pragma once

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow final : public QMainWindow {
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override;
    private:
        Ui::MainWindow* ui_;

    private slots:
        void onImportFromCSV();
        void onExportToCSV();
        void onImportFromXLSX();
        void onExportToXLSX();
};