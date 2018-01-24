#include "simdsp.h"
#include <QApplication>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);

    QCoreApplication::setOrganizationName("SimDSP");
    QCoreApplication::setOrganizationDomain("github.com/lmcapacho/SimDSP");
    QCoreApplication::setApplicationName("SimDSP");

    QFile file(":/resources/stylesheets/simdsp-design.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());

    QApplication a(argc, argv);

    SimDSP w;
    w.showMaximized();

    a.setStyleSheet(styleSheet);

    return a.exec();
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            abort();
    }
}
