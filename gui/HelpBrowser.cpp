/// ____________________________________________________________________ ///
///                                                                      ///
/// SoFiA 0.3 (HelpBrowser.cpp) - Source Finding Application             ///
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

#include "HelpBrowser.h"

HelpBrowser::HelpBrowser(const QString &path, const QString &page, QWidget *parent)
{
    this->setParent(parent);
    
    this->setAttribute(Qt::WA_DeleteOnClose);    // Will automatically delete widget on close()!
    //this->setAttribute(Qt::WA_GroupLeader);    // This property is deprecated in Qt 4.8!
    
    QIcon iconSoFiA;
    iconSoFiA.addFile(QString(":/icons/32/SoFiA.png"), QSize(32, 32));
    
    QIcon iconGoPrevious  = QIcon::fromTheme("go-previous", QIcon(":/icons/22/go-previous.png"));
    QIcon iconGoHome      = QIcon::fromTheme("go-home", QIcon(":/icons/22/go-home.png"));
    QIcon iconDialogClose = QIcon::fromTheme("dialog-close", QIcon(":/icons/22/dialog-close.png"));
    
    textBrowser = new QTextBrowser(this);
    
    buttonHome  = new QPushButton(tr("Home"), this);
    buttonHome->setShortcut(Qt::Key_Home);
    buttonHome->setIcon(iconGoHome);
    
    buttonBack  = new QPushButton(tr("Back"), this);
    buttonBack->setIcon(iconGoPrevious);
    
    buttonClose = new QPushButton(tr("Close"), this);
    buttonClose->setShortcut(Qt::Key_Escape);
    buttonClose->setIcon(iconDialogClose);
    
    QHBoxLayout *layoutButtons = new QHBoxLayout;
    layoutButtons->setContentsMargins(0, 0, 0, 0);
    layoutButtons->setSpacing(10);
    layoutButtons->addWidget(buttonBack);
    layoutButtons->addWidget(buttonHome);
    layoutButtons->addStretch();
    layoutButtons->addWidget(buttonClose);
    
    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setContentsMargins(5, 5, 5, 5);
    layoutMain->setSpacing(5);
    layoutMain->addWidget(textBrowser);
    layoutMain->addLayout(layoutButtons);
    
    this->setLayout(layoutMain);
    
    connect(buttonHome, SIGNAL(clicked()), textBrowser, SLOT(home()));
    connect(buttonBack, SIGNAL(clicked()), textBrowser, SLOT(backward()));
    connect(buttonClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(textBrowser, SIGNAL(sourceChanged(const QUrl &)), this, SLOT(updateWindowTitle()));
    
    textBrowser->setSearchPaths(QStringList() << path);
    textBrowser->setSource(page);
    textBrowser->setOpenExternalLinks(true);
    
    this->setWindowIcon(iconSoFiA);
    
    return;
}

void HelpBrowser::updateWindowTitle()
{
    this->setWindowTitle(tr("Help: %1").arg(textBrowser->documentTitle()));
    
    //buttonBack->setEnabled(textBrowser->isBackwardAvailable());
    //std::cout << textBrowser->backwardHistoryCount() << std::endl;
    
    return;
}

void HelpBrowser::showPage(const QString &page)
{
    // This member function has been declared static, i.e. it can be called without first 
    // creating an instance of class HelpBrowser! An object of class HelpBrowser will 
    // actually be created here. Because we set the Qt::WA_DeleteOnClose in the constructor, 
    // it will not need to be deleted explicitly, as this will happen automatically when 
    // the close() slot gets called.
    
    //QString path = QApplication::applicationDirPath() + QString("/doc");
    QString path = QString(getenv("SOFIA_PIPELINE_PATH"));
    path.replace("sofia_pipeline.py", "gui/doc");
    
    HelpBrowser *browser = new HelpBrowser(path, page);    // Set up in constructor to be deleted on close()!
    
    browser->resize(600, 640);
    browser->show();
    
    return;
}
