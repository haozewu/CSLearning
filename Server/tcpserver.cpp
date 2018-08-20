//1.首先我们新建一个TcpServer的类，继承于QObject，然后把QTcpServer include进来
//记得要在你的.pro文件里Qt += network，不然你是无法使用网络库的。
//2.我们的TcpServer是给多个类调用的，所以要以接口的方式编写，意思就是要其他类也能方便的调用它。
//3.一旦有新连接，他就会将新连接的socket用信号发射给调用他的类

#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QObject(parent)
{
    mServer = new QTcpServer(this);
    connect(mServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

}

void TcpServer::start(int port)
{
    if (!mServer->isListening()) {
        if (mServer->listen(QHostAddress::AnyIPv4, port)) {
            qDebug() << "服务端监听成功";
        } else {
            qDebug() << "服务端监听失败：" << mServer->errorString();
        }
    }
}

void TcpServer::stop()
{
    if (mServer->isListening()) {
        mServer->close();
    }
}

void TcpServer::newConnection()
{
    while (mServer->hasPendingConnections()) {
        // 获取新连接
        QTcpSocket *sock = mServer->nextPendingConnection();
        // 发射新连接信号让调用服务器的类知道
        emit newConnection(sock);
    }
}
