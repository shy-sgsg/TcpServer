#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>

class FileReceiver; // 前向声明 FileReceiver 类

class TcpServer : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();

signals:
    void fileReceived(const QString& filePath); // ✅ 新增：文件接收成功信号

private slots:
    // 当有新的客户端连接时触发
    void handleNewConnection();
    void handleError();

private:
    QTcpServer *m_tcpServer;
};

#endif // TCPSERVER_H
