/*
 * Copyright 2010 (c) Sascha Mueller <mailbox@saschamueller.com>
 *
 * This file is part of qhashpw.
 *
 * qhashpw is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * qhashpw is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with qhashpw.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui/QTabBar>

#include "accountsetview.h"
#include "mytabwidget.h"

MyTabWidget::MyTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    tabBar = new QTabBar;
    tabBar->setVisible(false);
    setTabBar(tabBar);

    setDocumentMode(true);
    setMovable(true);
    setTabsClosable(true);
}

AccountSetView *MyTabWidget::currentSet() const
{
    return currentIndex() == -1 ? 0 : qobject_cast<AccountSetView*>(currentWidget());
}

void MyTabWidget::tabInserted(int)
{
    tabBar->setVisible(count() > 1);
}

void MyTabWidget::tabRemoved(int)
{
    tabBar->setVisible(count() > 1);
}
