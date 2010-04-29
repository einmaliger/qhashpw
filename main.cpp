#include <QtCore>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include "account.h"
#include "mainwindow.h"
#include "tokenizer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile f("accounts.txt");
    f.open(QIODevice::ReadOnly);
    Tokenizer *t = new Tokenizer(&f);
    if(t->error() != Tokenizer::NO_ERROR)
        QMessageBox(QMessageBox::Critical, QObject::tr("File error"), QObject::tr("The input file could not be opened"), QMessageBox::Ok);

    Account defAccount;

    QString s;

    bool tryOneMore = defAccount.readFrom(t);

    s += defAccount.errorMsg();

    QList<Account> all;
    while(tryOneMore)
    {
        Account a;
        tryOneMore = a.readFrom(t);
        a.fillAccount(defAccount);
        s += a.errorMsg();
        all.append(a);
    }


    QMessageBox(QMessageBox::Information, QObject::tr("Load result"), s, QMessageBox::Ok);

    MainWindow w(all);
    w.show();

    return a.exec();
}
