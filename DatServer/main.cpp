#include "serverwindow.h"
#include <QApplication>
#include "serverthread.h"
#include <stdio.h>
#include <QDateTime>

#define QMSG_FLUSH 1
#ifdef Q_OS_WIN
#include <Windows.h>
#include <Dbghelp.h>

void print_stack(void)
{
    unsigned int   i;
    void         * stack[100];
    unsigned short frames;
	SYMBOL_INFO  * symbol;
	HANDLE         process;

    process = GetCurrentProcess();

    SymInitialize(process, NULL, TRUE);

    frames = CaptureStackBackTrace(0, 100, stack, NULL);
    symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO)+256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (i = 0; i < frames; i++)
    {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

        qInfo("%i: %s ", frames - i - 1, symbol->Name);
    }

	free(symbol);
}
#else
#include <execinfo.h>
void print_stack(void) {
    void    * array[10];
    size_t  size;
    char    ** strings;
    size_t  i;

    size = backtrace(array, 10);
    strings = backtrace_symbols (array, size);
    if (NULL == strings)
    {
        perror("backtrace_synbols");
        exit(EXIT_FAILURE);
    }

    printf ("Obtained %zd stack frames.\n", size);

    for (i = 0; i < size; i++)
        printf ("%s\n", strings[i]);

    free (strings);
    strings = NULL;
}
#endif
static FILE * fp = NULL;
static int fp_idx = 0;
static int fp_write = 0;
#define LOG_FILE_SIZE 102400000

//Debug out format <$level>[$dnum,$module] [$time] [$func] $msg
//Release output format <$level>[$time] $msg
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (fp==NULL) {
        fp = fopen("server_log0.txt", "wt");
        if (fp==NULL)
            exit(-1);
    }
    if (fp_write > LOG_FILE_SIZE) {
        fclose(fp);
        fp_idx = (fp_idx + 1) % 2;
        fp_write = 0;
        char fname[100];
        qsnprintf(fname, 100, "server_log%d.txt", fp_idx);
        fp = fopen(fname, "wt");
        if (fp==NULL)
            exit(-1);
    }

    QTime datetime;
    datetime = QTime::currentTime();
    QString str_dt = datetime.toString("hh:mm:ss.zzz");
    if (msg == "*#*#DumpMessage#*#*") {
        fflush(fp);
        return;
    }
    if (context.function == NULL) {
            unsigned thread_id = quintptr(QThread::currentThreadId());
            switch (type) {
            case QtDebugMsg:
                fp_write += fprintf(fp, "<D>[%s] [%d] %s\n", qPrintable(str_dt), thread_id, qPrintable(msg));
    #if QMSG_FLUSH
                fflush(fp);
    #endif
                break;
            case QtInfoMsg:
                fp_write += fprintf(fp, "<I>[%s] [%d] %s\n", qPrintable(str_dt), thread_id, qPrintable(msg));
    #if QMSG_FLUSH
                fflush(fp);
    #endif
                break;
            case QtWarningMsg:
                fp_write += fprintf(fp, "<W>[%s] [%d] %s\n", qPrintable(str_dt), thread_id, qPrintable(msg));
                fflush(fp);
                break;
            case QtCriticalMsg:
                fp_write += fprintf(fp, "<E>[%s] [%d] %s\n", qPrintable(str_dt), thread_id, qPrintable(msg));
                fflush(fp);
                break;
            case QtFatalMsg:
                fp_write += fprintf(fp, "<F>[%s] [%d] %s\n", qPrintable(str_dt), thread_id, qPrintable(msg));
                fclose(fp);
                exit(-1);
            }
            return;
        }
    char func[100];
    char file[100];
    int size, kuo=0;
    const char * pend, * pmid, * pbegin;
    size = (int) strlen(context.function);
    for (pend = context.function +size; pend!=context.function; pend--) {
        if (*pend==')')
            kuo++;
        if (*pend=='(') {
            kuo--;
            if (kuo<=0)
                break;
        }
    }
    if (pend==context.function)
        pend = context.function +size;

    for (pmid = pend; pmid!=context.function && *pmid!=':'; pmid--);
    if (*pmid==':')
        pmid++;
    size= pend- pmid;
    memcpy(func, pmid, size);
    func[size]=0;
    while (*(pmid-1)==':' && pmid!=context.function)
        pmid--;
    for (pbegin=pmid; *pbegin!=' ' && pbegin !=context.function; pbegin--);
    size = pmid -pbegin;
    memcpy(file, pbegin, size);
    file[size]=0;

    switch (type) {
    case QtDebugMsg:
        fp_write += fprintf(fp, "<D>[%d,%s] [%s] [%s] %s\n", context.line, file, qPrintable(str_dt), func, qPrintable(msg));
        break;
    case QtInfoMsg:
        fp_write += fprintf(fp, "<I>[%d,%s] [%s] [%s] %s\n", context.line, file, qPrintable(str_dt), func, qPrintable(msg));
        break;
    case QtWarningMsg:
        fp_write += fprintf(fp, "<W>[%d,%s] [%s] [%s] %s\n", context.line, file, qPrintable(str_dt), func, qPrintable(msg));
        break;
    case QtCriticalMsg:
        fp_write += fprintf(fp, "<E>[%d,%s] [%s] [%s] %s\n", context.line, file, qPrintable(str_dt), func, qPrintable(msg));
        fflush(fp);
        break;
    case QtFatalMsg:
        fp_write += fprintf(fp, "<F>[%d,%s] [%s] [%s] %s\n", context.line, file, qPrintable(str_dt), func, qPrintable(msg));
        fclose(fp);
        exit(-1);
    }
}
#ifdef Q_OS_WIN
#include <Windows.h>
#include <DbgHelp.h>
void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)
{
    HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = pep;
        mdei.ClientPointers = FALSE;
        MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);
        BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (pep != 0) ? &mdei : 0, 0, 0);
        CloseHandle(hFile);
    }
}

LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
    CreateMiniDump(pExceptionInfo, L"core.dmp");
    qWarning("crash happen");
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerWindow w;
    ServerThread st;
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
#endif
    qInstallMessageHandler(myMessageOutput);
    w.show();
    st.start();

    return a.exec();
}
