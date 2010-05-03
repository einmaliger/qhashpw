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
        QMessageBox(QMessageBox::Critical, QObject::tr("File error"), QObject::tr("The input file could not be opened"), QMessageBox::Ok).exec();

    Account defAccount;

    QString s;

    bool noError = defAccount.readFrom(t);

    s += defAccount.errorMsg();

    QList<Account> all;
    while(noError)
    {
        Account a;
	if(t->error() == Tokenizer::EOF_ERROR) break;
        noError = a.readFrom(t);
        a.fillAccount(defAccount);
        s += a.errorMsg();
        all.append(a);
    }

    if(!noError) QMessageBox(QMessageBox::Information, QObject::tr("Load result"), s, QMessageBox::Ok).exec();

    MainWindow w(all, defAccount.note());
    w.show();

    return a.exec();
}
