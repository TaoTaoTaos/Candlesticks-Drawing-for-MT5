#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QFont font("Microsoft YaHei");  // 使用中文字体
    a.setFont(font);
    w.resize(1700, 800);
    w.show();
    return a.exec();
}
