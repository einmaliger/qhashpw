#include <QtCore>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include "accountset.h"
#include "mainwindow.h"
#include "tokenizer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile f("accounts.txt");
    f.open(QIODevice::ReadOnly);
    Tokenizer *t = new Tokenizer(&f);
    if(t->error() != Tokenizer::NO_ERROR)
        QMessageBox(QMessageBox::Critical, QObject::tr("File error"), QObject::tr("The input file could not be opened"), QMessageBox::Ok).exec();

    AccountSet accounts;

    if(!accounts.readFrom(t)) QMessageBox(QMessageBox::Information, QObject::tr("Load result"), accounts.errorMsg(), QMessageBox::Ok).exec();

    MainWindow w(&accounts);
    w.show();

    return a.exec();
}
