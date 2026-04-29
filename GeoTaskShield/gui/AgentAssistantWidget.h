#pragma once

#include "agent/AssistantResponse.h"
#include "agent/RuleBasedAssistant.h"
#include "experiment/BatchResultRecord.h"

#include <QString>
#include <QWidget>

#include <functional>
#include <vector>

class QPushButton;
class QComboBox;
class QLabel;
class QTextEdit;
class QThread;

namespace gts {

class AgentAssistantWidget : public QWidget {
    Q_OBJECT

public:
    explicit AgentAssistantWidget(QWidget* parent = nullptr);

    void setBatchResultsProvider(
        std::function<std::vector<BatchResultRecord>()> provider);

    bool hasAssistantControlsForTesting() const;
    bool hasProviderSelectionForTesting() const;
    bool hasProviderOptionForTesting(const QString& labelPart) const;
    bool hasProviderStatusForTesting() const;
    void setProviderForTesting(const QString& labelPart);
    void setPromptForTesting(const QString& prompt);
    void analyzeForTesting();
    QString intentPreviewForTesting() const;
    QString analysisMarkdownForTesting() const;
    QString providerStatusForTesting() const;
    bool isAnalyzingForTesting() const;
    bool waitForAnalysisForTesting(int timeoutMs);
    bool exportMarkdownForTesting(const QString& filePath) const;

private:
    void analyzePrompt();
    void startProviderAnalysis(AssistantRequest request);
    void finishProviderAnalysis(AssistantResponse response);
    void applyAssistantResponse(const QString& statusText);
    void setAnalyzing(bool analyzing, const QString& statusText);
    void exportMarkdown();
    bool writeMarkdownToFile(const QString& filePath) const;

    std::function<std::vector<BatchResultRecord>()> batchResultsProvider_;
    RuleBasedAssistant assistant_;
    AssistantResponse lastResponse_;

    QComboBox* providerCombo_{};
    QLabel* providerStatusLabel_{};
    QTextEdit* promptEdit_{};
    QPushButton* analyzeButton_{};
    QTextEdit* intentPreview_{};
    QTextEdit* analysisPreview_{};
    QPushButton* exportMarkdownButton_{};
    QThread* analysisThread_{};
    bool analyzing_{};
};

} // namespace gts
