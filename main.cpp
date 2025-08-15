#include <QCoreApplication>
#include "tcpserver.h"
#include "filereceiver.h"

#include <QApplication>
#include "tcpserver.h"
#include "mainwindow.h" // 新增头文件

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w; // 创建主窗口实例
    w.show();    // 显示主窗口

    TcpServer server;
    // 将 TcpServer 实例的信号连接到 MainWindow 的槽函数
    // 例如，当文件接收成功时，通知主窗口刷新列表或显示信息
    QObject::connect(&server, &TcpServer::fileReceived, &w, &MainWindow::onFileReceived);

    return a.exec();
}
