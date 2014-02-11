/// ____________________________________________________________________ ///
///                                                                      ///
/// SoFiA 0.2 (WidgetSpreadsheet.cpp) - Source Finding Application       ///
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

#include "WidgetSpreadsheet.h"

// ----------- //
// CONSTRUCTOR //
// ----------- //

WidgetSpreadsheet::WidgetSpreadsheet(QWidget *parent)
{
    this->setParent(parent);
    
    // Create button icons:
    QIcon iconDialogClose = QIcon::fromTheme("dialog-close", QIcon(":/icons/22/dialog-close.png"));
    QIcon iconViewRefresh = QIcon::fromTheme("view-refresh", QIcon(":/icons/22/view-refresh.png"));
    
    // Create table widget:
    tableWidget = new QTableWidget(this);
    tableWidth  = 0;
    tableHeight = 0;
    tableWidget->setColumnCount(tableWidth);
    tableWidget->setRowCount(tableHeight);
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tableWidget->setStyleSheet("QTableWidget::item {padding:5px 10px;}");
    
    // Create control widget:
    widgetControls = new QWidget(this);
    
    widgetSort = new QWidget(widgetControls);
    buttonSort = new QComboBox(widgetSort);
    buttonSort->setEnabled(false);
    connect(buttonSort, SIGNAL(currentIndexChanged(int)), this, SLOT(sortTable(int)));
    layoutSort = new QFormLayout;
    layoutSort->addRow(tr("Sort by"), buttonSort);
    layoutSort->setContentsMargins(0, 0, 0, 0);
    layoutSort->setSpacing(5);
    widgetSort->setLayout(layoutSort);
    
    buttonReload = new QPushButton(tr("Reload"), widgetControls);
    buttonReload->setIcon(iconViewRefresh);
    buttonReload->setEnabled(false);
    connect(buttonReload, SIGNAL(clicked()), this, SLOT(reloadCatalog()));
    
    buttonClose = new QPushButton(tr("Close"), widgetControls);
    buttonClose->setIcon(iconDialogClose);
    connect(buttonClose, SIGNAL(clicked()), this, SLOT(close()));
    
    layoutControls = new QHBoxLayout;
    layoutControls->setContentsMargins(5, 5, 5, 5);
    layoutControls->setSpacing(10);
    layoutControls->addWidget(widgetSort);
    layoutControls->addStretch();
    layoutControls->addWidget(buttonReload);
    layoutControls->addWidget(buttonClose);
    widgetControls->setLayout(layoutControls);
    widgetControls->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    // Set up main layout:
    mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(tableWidget);
    mainLayout->addWidget(widgetControls);
    
    // Set up main window:
    this->setLayout(mainLayout);
    this->setWindowFlags(Qt::Window);
    this->setWindowTitle(tr("SoFiA - Source Catalogue"));
    this->resize(720, 480);
    
    return;
}



// --------------------- //
// LOAD SOURCE CATALOGUE //
//---------------------- //

int WidgetSpreadsheet::loadCatalog(QString &filename)
{
    if(filename.isEmpty()) return 1;
    
    currentFileName = filename;
    tableWidget->clear();              // Clear the existing table. This will automatically delete all QTableWidgetItem objects 
                                       // created before, so no memory will be leaked.
    tableWidth  = 0;
    tableHeight = 0;
    buttonSort->clear();               // Clear sort button as well.
    buttonSort->addItem(QString(""));
    buttonSort->setEnabled(false);
    buttonReload->setEnabled(false);
    
    QDomDocument catalogue(currentFileName);
    QFile file(currentFileName);
    
    if(!file.open(QIODevice::ReadOnly)) return 1;
    
    if(!catalogue.setContent(&file))
    {
        file.close();
        return 1;
    }
    
    file.close();
    
    // Get all tags named FIELD (contains header information):
    QDomNodeList headerTags = catalogue.elementsByTagName("FIELD");
    if(headerTags.isEmpty()) return 1;
    
    tableWidth = headerTags.size();
    tableWidget->setColumnCount(tableWidth);
    
    // Extract all header names:
    for(int i = 0; i < headerTags.size(); i++)
    {
        QDomNode field = headerTags.item(i);
        QDomNamedNodeMap fieldAttributes = field.attributes();
        QDomNode attribute = fieldAttributes.namedItem("name");
        QDomNode unit      = fieldAttributes.namedItem("unit");
        
        QString headerText = attribute.nodeValue().trimmed();
        
        buttonSort->addItem(headerText);
        
        if(!unit.isNull() and unit.nodeValue() != "-")
        {
            headerText.append("\n(");
            headerText.append(unit.nodeValue());
            headerText.append(")");
        }
        //else headerText.append("\n");
        
        QTableWidgetItem* headerItem = new QTableWidgetItem(headerText, QTableWidgetItem::Type);
        tableWidget->setHorizontalHeaderItem(i, headerItem);
    }
    
    buttonSort->setEnabled(true);
    buttonReload->setEnabled(true);
    
    // Get all tags named TR (data rows):
    QDomNodeList rowTags = catalogue.elementsByTagName("TR");
    if(rowTags.isEmpty()) return 1;
    
    tableHeight = rowTags.size();
    tableWidget->setRowCount(tableHeight);
    
    // Extract all fields (TD) within each row:
    for(int i = 0; i < rowTags.size(); i++)
    {
        QDomNode row = rowTags.item(i);
        QDomNodeList cellTags = row.childNodes();
        
        for(int j = 0; j < tableWidth; j++)
        {
            if(j < cellTags.size())
            {
                QDomNode cell = cellTags.item(j);
                
                if(!cell.isNull() and cell.hasChildNodes())
                {
                    QDomNode entry = cell.firstChild();
                    QString  text  = entry.nodeValue().trimmed();
                    double   value = text.toDouble();
                    QTableWidgetItem* cellItem = new QTableWidgetItem();
                    cellItem->setData(Qt::DisplayRole, value);
                    cellItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    cellItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                    tableWidget->setItem(i, j, cellItem);
                }
            }
        }
    }
    
    tableWidget->resizeColumnsToContents();
    tableWidget->resizeRowsToContents();
    
    return 0;
}



// -----------------  //
// SLOT TO SORT TABLE //
// -----------------  //

void WidgetSpreadsheet::sortTable(int column)
{
    column--;      // Ignore first, empty option in drop-down list (meaning: don't sort).
    
    if(column >= 0 and column < tableWidget->columnCount()) tableWidget->sortItems(column);
    
    return;
}



// -----------------------  //
// SLOT TO RELOAD CATALOGUE //
// -----------------------  //

void WidgetSpreadsheet::reloadCatalog()
{
    if(!currentFileName.isEmpty() and loadCatalog(currentFileName) != 0) std::cerr << "Failed to reload catalogue." << std::endl;
    
    return;
}



// ----------------- //
// CLEAN UP ON CLOSE //
// ----------------- //

void WidgetSpreadsheet::closeEvent(QCloseEvent *event)
{
    emit widgetClosed();
    event->accept();
}
