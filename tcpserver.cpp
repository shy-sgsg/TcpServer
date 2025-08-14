#include "tcpserver.h"
#include "filereceiver.h"
#include <QDebug>
#include <QTcpSocket>
#include <QFile>
#include <QHostAddress>
#include <QDateTime>
#include <QDir>

TcpServer::TcpServer(QObject *parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
{
    // 监听所有网络接口的 65432 端口
    if (!m_tcpServer->listen(QHostAddress::Any, 65432)) {
        qDebug() << "Server could not start listening.";
        qDebug() << "Error:" << m_tcpServer->errorString();
        return;
    }
    qDebug() << "转发模块正在监听 " << m_tcpServer->serverAddress().toString() << ":" << m_tcpServer->serverPort() << " ...";
    connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::handleNewConnection);
}

TcpServer::~TcpServer()
{
    if (m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
}

void TcpServer::handleNewConnection()
{
    // 获取新连接的 socket
    QTcpSocket* newSocket = m_tcpServer->nextPendingConnection();
    // qDebug() << "连接建立: " << newSocket->peerAddress().toString() << ":" << newSocket->peerPort();

    // 创建一个 FileReceiver 对象来处理这个新连接的文件接收
    FileReceiver* receiver = new FileReceiver(newSocket, this);
    // 连接信号，当接收器完成任务后，自动删除它
    connect(receiver, &FileReceiver::finished, receiver, &QObject::deleteLater);
}

void TcpServer::handleError()
{
    // 这个槽函数用于处理通用的错误，但 FileReceiver 会处理它自己的错误
    qDebug() << "服务器发生错误：" << m_tcpServer->errorString();
}
