#include "gui/AgentAssistantWidget.h"

#include "agent/AssistantRequest.h"
#include "agent/OpenAICompatibleAssistant.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QMetaObject>
#include <QPointer>
#include <QPushButton>
#include <QTextEdit>
#include <QThread>
#include <QVBoxLayout>

#include <utility>

namespace gts {

AgentAssistantWidget::AgentAssistantWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);

    auto* inputLabel = new QLabel("Natural language request", this);
    auto* providerLayout = new QHBoxLayout;
    auto* providerLabel = new QLabel("Assistant provider", this);
    providerCombo_ = new QComboBox(this);
    providerCombo_->setObjectName("assistantProviderCombo");
    providerCombo_->addItem("Local rule-based", "local");
    providerCombo_->addItem("OpenAI Compatible", "openai-compatible");
    providerStatusLabel_ = new QLabel("Local rule-based ready.", this);
    providerStatusLabel_->setObjectName("assistantProviderStatus");
    providerLayout->addWidget(providerLabel);
    providerLayout->addWidget(providerCombo_);
    providerLayout->addWidget(providerStatusLabel_);
    providerLayout->addStretch(1);

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

    root->addLayout(providerLayout);
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

bool AgentAssistantWidget::hasProviderSelectionForTesting() const
{
    return providerCombo_ != nullptr && providerCombo_->count() >= 2;
}

bool AgentAssistantWidget::hasProviderOptionForTesting(
    const QString& labelPart) const
{
    if (providerCombo_ == nullptr) {
        return false;
    }
    for (int index = 0; index < providerCombo_->count(); ++index) {
        if (providerCombo_->itemText(index).contains(labelPart,
                                                     Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool AgentAssistantWidget::hasProviderStatusForTesting() const
{
    return providerStatusLabel_ != nullptr;
}

void AgentAssistantWidget::setProviderForTesting(const QString& labelPart)
{
    if (providerCombo_ == nullptr) {
        return;
    }
    for (int index = 0; index < providerCombo_->count(); ++index) {
        if (providerCombo_->itemText(index).contains(labelPart,
                                                     Qt::CaseInsensitive)) {
            providerCombo_->setCurrentIndex(index);
            return;
        }
    }
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

QString AgentAssistantWidget::providerStatusForTesting() const
{
    if (providerStatusLabel_ == nullptr) {
        return {};
    }
    return providerStatusLabel_->text();
}

bool AgentAssistantWidget::isAnalyzingForTesting() const
{
    return analyzing_;
}

bool AgentAssistantWidget::waitForAnalysisForTesting(int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();
    while (analyzing_ && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QThread::msleep(5);
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    return !analyzing_;
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

    if (providerCombo_ != nullptr &&
        providerCombo_->currentData().toString() == "openai-compatible") {
        startProviderAnalysis(std::move(request));
        return;
    } else {
        lastResponse_ = assistant_.analyze(request);
    }
    applyAssistantResponse("Local rule-based analysis complete.");
}

void AgentAssistantWidget::startProviderAnalysis(AssistantRequest request)
{
    if (analyzing_) {
        return;
    }

    setAnalyzing(true, "OpenAI Compatible provider analyzing...");
    QPointer<AgentAssistantWidget> widget(this);
    auto* thread = QThread::create([widget, request = std::move(request)]() mutable {
        OpenAICompatibleAssistant llmAssistant;
        AssistantResponse response = llmAssistant.analyze(request);
        if (widget.isNull()) {
            return;
        }
        QMetaObject::invokeMethod(
            widget.data(),
            [widget, response = std::move(response)]() mutable {
                if (widget.isNull()) {
                    return;
                }
                widget->finishProviderAnalysis(std::move(response));
            },
            Qt::QueuedConnection);
    });

    analysisThread_ = thread;
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, thread]() {
        if (analysisThread_ == thread) {
            analysisThread_ = nullptr;
        }
    });
    thread->start();
}

void AgentAssistantWidget::finishProviderAnalysis(AssistantResponse response)
{
    lastResponse_ = std::move(response);
    applyAssistantResponse(
        lastResponse_.success
            ? "OpenAI Compatible provider analysis complete."
            : "OpenAI Compatible provider unavailable; local fallback shown.");
}

void AgentAssistantWidget::applyAssistantResponse(const QString& statusText)
{
    intentPreview_->setMarkdown(
        QString::fromStdString(lastResponse_.intentPreviewMarkdown));
    analysisPreview_->setMarkdown(
        QString::fromStdString(lastResponse_.analysisMarkdown));
    setAnalyzing(false, statusText);
}

void AgentAssistantWidget::setAnalyzing(bool analyzing, const QString& statusText)
{
    analyzing_ = analyzing;
    if (analyzeButton_ != nullptr) {
        analyzeButton_->setEnabled(!analyzing_);
    }
    if (providerCombo_ != nullptr) {
        providerCombo_->setEnabled(!analyzing_);
    }
    if (providerStatusLabel_ != nullptr) {
        providerStatusLabel_->setText(statusText);
    }
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
