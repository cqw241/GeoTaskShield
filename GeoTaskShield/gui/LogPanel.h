#pragma once

#include <QWidget>

class QTextEdit;

namespace gts {

class LogPanel : public QWidget {
public:
    explicit LogPanel(QWidget* parent = nullptr);

    void clear();
    void appendInfo(const QString& message);

private:
    QTextEdit* textEdit_{};
};

} // namespace gts
