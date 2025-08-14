#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>

class FileReceiver : public QObject
{
    Q_OBJECT

public:
    explicit FileReceiver(QTcpSocket* socket, QObject* parent = nullptr);
    ~FileReceiver();

signals:
    void finished();

private slots:
    void readData();
    void disconnected();
    void socketError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket* m_socket;
    QFile* m_file;
    qint64 m_fileSize;
    qint64 m_receivedSize;
    QString m_fileName; // 新增：用于存储接收到的文件名

    enum State {
        ReadingFileNameLength, // 接收文件名的长度
        ReadingFileName,       // 接收完整文件名
        ReadingHeader,         // 接收文件大小
        ReadingData            // 接收文件内容
    };
    State m_state;
    qint32 m_fileNameLength; // 新增：存储文件名长度
    bool m_isReceiveSuccessful = false;
};

#endif // FILERECEIVER_H
