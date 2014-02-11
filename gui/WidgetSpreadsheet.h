/// ____________________________________________________________________ ///
///                                                                      ///
/// SoFiA 0.2 (WidgetSpreadsheet.h) - Source Finding Application         ///
/// Copyright (C) 2013-2014 Tobias Westmeier                             ///
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

#ifndef WIDGETSPREADSHEET_H
#define WIDGETSPREADSHEET_H

#include <iostream>

#include <QtCore/QString>
#include <QtCore/QFile>

#include <QtXml/QDomDocument>

#include <QtGui/QWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QCloseEvent>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>

class WidgetSpreadsheet : public QWidget
{
    Q_OBJECT
    
public:
    WidgetSpreadsheet(QWidget *parent = 0);
    int loadCatalog(QString &filename);
    
private:
    QString currentFileName;
    int tableWidth;
    int tableHeight;
    QTableWidget *tableWidget;
    QVBoxLayout  *mainLayout;
    
    QFormLayout *layoutSort;
    QWidget     *widgetSort;
    QComboBox   *buttonSort;
    QPushButton *buttonReload;
    QPushButton *buttonClose;
    QHBoxLayout *layoutControls;
    QWidget     *widgetControls;
    
private slots:
    void sortTable(int column);
    void reloadCatalog();
    
protected:
    virtual void closeEvent(QCloseEvent *event);
    
signals:
    void widgetClosed();
};

#endif
