#include "gui/LogPanel.h"

#include <QTextEdit>
#include <QVBoxLayout>

namespace gts {

LogPanel::LogPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    textEdit_ = new QTextEdit(this);
    textEdit_->setReadOnly(true);
    textEdit_->setMinimumHeight(120);
    layout->addWidget(textEdit_);
}

void LogPanel::clear()
{
    textEdit_->clear();
}

void LogPanel::appendInfo(const QString& message)
{
    textEdit_->append("[INFO] " + message);
}

} // namespace gts
