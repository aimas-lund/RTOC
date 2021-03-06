#include "logger.h"

Logger::Logger(QObject* parent) : QObject(parent) {}

void Logger::writeLineToLog(QString msg) {
    if (msg.at(0) != '\n') {
        msg.prepend("\n");
    }
    writeToLog(msg);

    m_acqWaitTimer.setInterval(150);
    connect(&m_acqWaitTimer, &QTimer::timeout, this, &Logger::writeDot);
}
void Logger::writeToLog(QString msg) {
    m_log->moveCursor(QTextCursor::End);
    m_log->insertPlainText(msg);
    m_log->moveCursor(QTextCursor::End);
}
