#include "serverwindow.h"
#include <QApplication>
#include "serverthread.h"
#include <stdio.h>

static FILE * fp = NULL;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (fp==NULL) {
        fp = fopen("server_log.txt", "wt");
        if (fp==NULL)
            exit(-1);
    }
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(fp, "D: (%s:%u, %s)\n", context.function, context.line, localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(fp, "I: (%s:%u, %s)\n", context.function, context.line, localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(fp, "W: (%s:%u, %s)\n", context.function, context.line, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(fp, "C: (%s:%u, %s)\n", context.function, context.line, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(fp, "F: (%s:%u, %s)\n", context.function, context.line, localMsg.constData());
        exit(-1);
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerWindow w;
    ServerThread st;

    qInstallMessageHandler(myMessageOutput);
    w.show();
    st.start();

    return a.exec();
}
