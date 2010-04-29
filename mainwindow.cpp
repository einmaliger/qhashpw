#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    aLabel = new QTextEdit();
    aLabel->setReadOnly(true);
    setCentralWidget(aLabel);
}

MainWindow::~MainWindow()
{
}

void MainWindow::addInfo(const QString &info)
{
    aLabel->append(info);
}
