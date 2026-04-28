#include "gui/AgentAssistantWidget.h"

#include "agent/AssistantRequest.h"

#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include <utility>

namespace gts {

AgentAssistantWidget::AgentAssistantWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);

    auto* inputLabel = new QLabel("Natural language request", this);
    promptEdit_ = new QTextEdit(this);
    promptEdit_->setMinimumHeight(90);
    promptEdit_->setPlaceholderText(
        "Compare privacy mechanisms and explain completion, privacy utility, "
        "privacy loss, and fairness.");

    analyzeButton_ = new QPushButton("Analyze", this);
    analyzeButton_->setObjectName("assistantAnalyzeButton");

    auto* previewGrid = new QGridLayout;
    intentPreview_ = new QTextEdit(this);
    intentPreview_->setReadOnly(true);
    intentPreview_->setMinimumHeight(180);
    intentPreview_->setObjectName("assistantIntentPreview");
    analysisPreview_ = new QTextEdit(this);
    analysisPreview_->setReadOnly(true);
    analysisPreview_->setMinimumHeight(300);
    analysisPreview_->setObjectName("assistantMarkdownPreview");

    previewGrid->addWidget(new QLabel("Parsed intent", this), 0, 0);
    previewGrid->addWidget(new QLabel("Markdown analysis", this), 0, 1);
    previewGrid->addWidget(intentPreview_, 1, 0);
    previewGrid->addWidget(analysisPreview_, 1, 1);
    previewGrid->setColumnStretch(0, 1);
    previewGrid->setColumnStretch(1, 2);

    exportMarkdownButton_ = new QPushButton("Export Markdown", this);
    exportMarkdownButton_->setObjectName("assistantExportMarkdownButton");

    root->addWidget(inputLabel);
    root->addWidget(promptEdit_);
    root->addWidget(analyzeButton_, 0, Qt::AlignLeft);
    root->addLayout(previewGrid, 1);
    root->addWidget(exportMarkdownButton_, 0, Qt::AlignRight);

    connect(analyzeButton_, &QPushButton::clicked, this, [this]() {
        analyzePrompt();
    });
    connect(exportMarkdownButton_, &QPushButton::clicked, this, [this]() {
        exportMarkdown();
    });
}

void AgentAssistantWidget::setBatchResultsProvider(
    std::function<std::vector<BatchResultRecord>()> provider)
{
    batchResultsProvider_ = std::move(provider);
}

bool AgentAssistantWidget::hasAssistantControlsForTesting() const
{
    return promptEdit_ != nullptr && analyzeButton_ != nullptr &&
           intentPreview_ != nullptr && analysisPreview_ != nullptr &&
           exportMarkdownButton_ != nullptr;
}

void AgentAssistantWidget::setPromptForTesting(const QString& prompt)
{
    promptEdit_->setPlainText(prompt);
}

void AgentAssistantWidget::analyzeForTesting()
{
    analyzePrompt();
}

QString AgentAssistantWidget::intentPreviewForTesting() const
{
    return intentPreview_->toPlainText();
}

QString AgentAssistantWidget::analysisMarkdownForTesting() const
{
    return analysisPreview_->toMarkdown();
}

bool AgentAssistantWidget::exportMarkdownForTesting(const QString& filePath) const
{
    return writeMarkdownToFile(filePath);
}

void AgentAssistantWidget::analyzePrompt()
{
    AssistantRequest request;
    request.prompt = promptEdit_->toPlainText().toStdString();
    request.sourceLabel = "current Batch Results filter";
    if (batchResultsProvider_) {
        request.batchResults = batchResultsProvider_();
    }

    lastResponse_ = assistant_.analyze(request);
    intentPreview_->setMarkdown(
        QString::fromStdString(lastResponse_.intentPreviewMarkdown));
    analysisPreview_->setMarkdown(
        QString::fromStdString(lastResponse_.analysisMarkdown));
}

void AgentAssistantWidget::exportMarkdown()
{
    if (lastResponse_.analysisMarkdown.empty()) {
        QMessageBox::information(this, "No analysis",
                                 "Analyze a prompt before exporting Markdown.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Assistant Markdown", "agent-assistant-analysis.md",
        "Markdown files (*.md);;All files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    if (!filePath.endsWith(".md", Qt::CaseInsensitive)) {
        filePath += ".md";
    }

    if (!writeMarkdownToFile(filePath)) {
        QMessageBox::warning(this, "Export failed",
                             "Could not write the assistant Markdown report.");
        return;
    }
    QMessageBox::information(this, "Export complete",
                             "Assistant Markdown exported.");
}

bool AgentAssistantWidget::writeMarkdownToFile(const QString& filePath) const
{
    if (filePath.isEmpty() || lastResponse_.analysisMarkdown.empty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QByteArray content =
        QString::fromStdString(lastResponse_.analysisMarkdown).toUtf8();
    return file.write(content) == content.size();
}

} // namespace gts
