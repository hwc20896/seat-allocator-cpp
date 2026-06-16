#pragma once

#include <QMainWindow>
#include <QSoundEffect>

#include <regex>
#include <memory>

#include "algorithm/algorithm.hpp"
#include "utils/utils.hpp"

class QLabel;
class QTableWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
    Q_OBJECT
    public:
        using time_type = std::chrono::milliseconds;

        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override;

    private:
        //  Constructor helper functions
        void initializeSignals();
        void initializeEffects() const;

        Ui::MainWindow* ui_;
        QLabel* statusBarText_;
        std::unique_ptr<QSoundEffect> shuffleBeginSound_, shuffleEndSound_, wheelClickSound_;

        RegexColorMapper colorMapper_;
        GridShuffler shuffler_;
        int shuffleAnimationIteration_{0};
        Grid shuffleAnimationGrid_;

        ShuffleConfig config_;

        Grid currentGrid_;
        int currentIndex_{0};

        void refreshGrid(const Grid& grid, bool isOriginal = false);
        void refreshPageIndicator() const;

        void observeGrid();

        [[nodiscard]]
        static
        time_type getDelayForProgress(double progress, time_type minDelay, time_type maxDelay);

        //  Temporary in-app swap operations
        QTableWidgetItem* taggedItem_ = nullptr;
        std::unordered_map<int, Grid> manuallyModifiedGrids_;
        bool hasManualModifications_ = false;

        void resetTagState();
        void saveManualSwap(int index);
        void revertLastSwap();

        QTableWidgetItem* loadElement(const std::string& text, bool isOriginal) const;

    private slots:
        void onImportFromCSV();
        void onExportToCSV();
        void onImportFromXLSX();
        void onExportToXLSX();
        void onLoadConstraints();
        void onUnloadConstraints();

        void onImportColorPresets();
        void onDisableColorPresets();

        void onBeginShuffle();

        void onNextGridClicked();
        void onPreviousGridClicked();
        void onOriginalGridChecked(bool checked);

        void onItemDoubleClicked(QTableWidgetItem* item);
};