#pragma once

#include <QMainWindow>
#include <QSoundEffect>

#include <regex>
#include <memory>

#include "algorithm/algorithm.hpp"
#include "utils/utils.hpp"

class QLabel;

namespace Ui {
    class MainWindow;
}

class MainWindow final : public QMainWindow {
    Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override;

    private:
        Ui::MainWindow* ui_;
        QLabel* statusBarText_;
        std::unique_ptr<QSoundEffect> shuffleAnimationSound_;

        RegexColorMapper colorMapper_;
        GridShuffler shuffler_;
        int shuffleAnimationIteration_{0};
        Grid shuffleAnimationGrid_;
        Grid currentGrid_;

        void refreshGrid(const Grid& grid);

    private slots:
        void onImportFromCSV();
        void onExportToCSV();
        void onImportFromXLSX();
        void onExportToXLSX();
        void onImportColorPresets();
        void onDisableColorPresets();
        void onBeginShuffle();
};