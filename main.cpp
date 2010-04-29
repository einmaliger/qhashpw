#include <QtCore>
#include <QtGui/QApplication>
#include "mainwindow.h"
#include "tokenizer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QFile f("accounts.txt");
    f.open(QIODevice::ReadOnly);
    Tokenizer *t = new Tokenizer(&f);
    if(t->error() != Tokenizer::NO_ERROR)
        w.addInfo(QObject::tr("There was an error opening the file"));

    do {
        w.addInfo(t->currentTokenDesc());
    } while(t->next());

    return a.exec();
}
