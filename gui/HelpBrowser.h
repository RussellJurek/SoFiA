/// ____________________________________________________________________ ///
///                                                                      ///
/// SoFiA 0.2 (HelpBrowser.h) - Source Finding Application               ///
/// Copyright (C) 2013 Tobias Westmeier                                  ///
/// ____________________________________________________________________ ///
///                                                                      ///
/// Address:  Tobias Westmeier                                           ///
///           ICRAR M468                                                 ///
///           The University of Western Australia                        ///
///           35 Stirling Highway                                        ///
///           Crawley WA 6009                                            ///
///           Australia                                                  ///
///                                                                      ///
/// E-mail:   tobias.westmeier@uwa.edu.au                                ///
/// ____________________________________________________________________ ///
///                                                                      ///
/// This program is free software: you can redistribute it and/or modify ///
/// it under the terms of the GNU General Public License as published by ///
/// the Free Software Foundation, either version 3 of the License, or    ///
/// (at your option) any later version.                                  ///
///                                                                      ///
/// This program is distributed in the hope that it will be useful,      ///
/// but WITHOUT ANY WARRANTY; without even the implied warranty of       ///
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         ///
/// GNU General Public License for more details.                         ///
///                                                                      ///
/// You should have received a copy of the GNU General Public License    ///
/// along with this program. If not, see http://www.gnu.org/licenses/.   ///
/// ____________________________________________________________________ ///
///                                                                      ///

#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include <QtCore/QString>

#include <QtGui/QWidget>
#include <QtGui/QTextBrowser>
#include <QtGui/QPushButton>

class HelpBrowser : public QWidget
{
    Q_OBJECT
    
public:
    HelpBrowser(const QString &path, const QString &page, QWidget *parent = 0);
    static void showPage(const QString &page);
    
private slots:
    void updateWindowTitle();
    
private:
    QTextBrowser *textBrowser;
    
    QPushButton  *buttonHome;
    QPushButton  *buttonBack;
    QPushButton  *buttonClose;
};

#endif
