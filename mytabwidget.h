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

#ifndef MYTABWIDGET_H
#define MYTABWIDGET_H

#include <QtGui/QTabWidget>

class QTabBar;

// A tab widget that automatically hides its
// tab bar when less than two tabs are open
class MyTabWidget : public QTabWidget
{
public:
    MyTabWidget(QWidget *parent = 0);

protected:
    void tabInserted(int index);
    void tabRemoved(int index);
    QTabBar *tabBar;
};

#endif // MYTABWIDGET_H
