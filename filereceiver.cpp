#include "filereceiver.h"
#include <QDebug>
#include <QDateTime>
#include <QDir>

FileReceiver::FileReceiver(QTcpSocket* socket, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_file(nullptr)
    , m_fileSize(0)
    , m_receivedSize(0)
    , m_state(ReadingFileNameLength) // ✅ 修正：将初始状态设置为 ReadingFileNameLength
{
    // 将 socket 的信号连接到 FileReceiver 的槽函数
    connect(m_socket, &QTcpSocket::readyRead, this, &FileReceiver::readData);
    connect(m_socket, &QTcpSocket::disconnected, this, &FileReceiver::disconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &FileReceiver::socketError);
}

FileReceiver::~FileReceiver()
{
    if (m_file && m_file->isOpen()) {
        m_file->close();
    }
    delete m_file;
    m_socket->deleteLater();
    // qDebug() << "FileReceiver instance deleted.";
}

void FileReceiver::readData()
{
    while (m_socket->bytesAvailable() > 0) {
        switch (m_state) {
            case ReadingFileNameLength:
                // 确保有足够的数据来读取文件名长度
                if (m_socket->bytesAvailable() >= sizeof(qint32)) {
                    m_socket->read(reinterpret_cast<char*>(&m_fileNameLength), sizeof(qint32));

                    // 检查接收到的文件名长度是否有效
                    if (m_fileNameLength > 0 && m_fileNameLength < 1024) { // 限制一个合理的长度范围
                        // qDebug() << "接收到文件名长度：" << m_fileNameLength << "字节";
                        m_state = ReadingFileName;
                    } else {
                        qDebug() << "\033[31m接收到无效的文件名长度：" << m_fileNameLength << "，断开连接。\033[0m";
                        m_socket->disconnectFromHost();
                        return;
                    }
                }
                break;

            case ReadingFileName:
                if (m_socket->bytesAvailable() >= m_fileNameLength) {
                    QByteArray fileNameBytes = m_socket->read(m_fileNameLength);
                    m_fileName = QString::fromUtf8(fileNameBytes);
                    // qDebug() << "\033[36m接收到文件名：" << m_fileName << "\033[0m";
                    m_state = ReadingHeader;
                }
                break;

            case ReadingHeader:
                if (m_socket->bytesAvailable() >= 16) {
                    QByteArray header = m_socket->read(16);
                    m_fileSize = header.trimmed().toLongLong();
                    // qDebug() << "\033[36m接收到文件大小：" << m_fileSize << "字节\033[0m";

                    // 创建保存目录和文件
                    QString saveDir = "E:/AIR/小长ISAR/实时数据回传/receive";
                    QDir dir(saveDir);
                    if (!dir.exists()) {
                        dir.mkpath(saveDir);
                    }
                    QString filepath = QDir(saveDir).filePath(m_fileName);

                    m_file = new QFile(filepath);
                    if (!m_file->open(QIODevice::WriteOnly)) {
                        qDebug() << "\033[31m无法创建文件进行写入：" << filepath << "\033[0m";
                        m_socket->disconnectFromHost();
                        return;
                    }
                    m_state = ReadingData;
                }
                break;

            case ReadingData:
                QByteArray data = m_socket->readAll();
                m_receivedSize += m_file->write(data);

                if (m_receivedSize >= m_fileSize) {
                    m_file->close();
                    qDebug() << "\033[32m成功接收" << m_fileName << "，大小：" << m_fileSize << "字节\033[0m";

                    // 接收完成，发送成功应答给客户端
                    m_socket->write("SUCCESS");
                    m_socket->waitForBytesWritten();

                    m_isReceiveSuccessful = true; // 标记成功
                    m_socket->disconnectFromHost();
                    emit finished();
                    return;
                }
                break;
        }
    }
}

void FileReceiver::disconnected()
{
    // qDebug() << "连接已断开。";
    if (m_file && m_file->isOpen()) {
        m_file->close();
    }
    // 如果文件接收不完整，则删除文件并发送失败应答（如果连接还在）
    if (!m_isReceiveSuccessful && m_file && m_receivedSize < m_fileSize) {
        qDebug() << "文件接收不完整，已删除：" << m_file->fileName();
        m_file->remove();
        m_socket->write("FAILURE");
        m_socket->waitForBytesWritten();
    }
    emit finished();
}

void FileReceiver::socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    qDebug() << "Socket 发生错误: " << m_socket->errorString();
    // 如果文件接收不完整，则删除文件并发送失败应答（如果连接还在）
    if (m_file && m_file->isOpen() && m_receivedSize < m_fileSize) {
        m_file->close();
        qDebug() << "因错误导致文件接收中断，已删除：" << m_file->fileName();
        m_file->remove();
        m_socket->write("FAILURE");
        m_socket->waitForBytesWritten();
    }
    m_socket->disconnectFromHost();
    emit finished();
}
