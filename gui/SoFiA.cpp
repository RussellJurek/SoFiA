/// ____________________________________________________________________ ///
///                                                                      ///
/// SoFiA 0.2 (SoFiA.cpp) - Source Finding Application                   ///
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
#include "SoFiA.h"



// -----------
// Constructor
// -----------

SoFiA::SoFiA()
{
    pipelineProcess = new QProcess(this);
    connect(pipelineProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(pipelineProcessReadStd()));
    connect(pipelineProcess, SIGNAL(readyReadStandardError()), this, SLOT(pipelineProcessReadErr()));
    connect(pipelineProcess, SIGNAL(started()), this, SLOT(pipelineProcessStarted()));
    connect(pipelineProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(pipelineProcessFinished(int, QProcess::ExitStatus)));
    connect(pipelineProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(pipelineProcessError(QProcess::ProcessError)));
    
    this->createInterface();
    this->setDefaults();
    this->setAcceptDrops(true);        // Enable drag and drop
    
    // Create catalogue viewer window, but don't show it yet:
    spreadsheet = new WidgetSpreadsheet(this);
    spreadsheet->hide();
    
    return;
}



// ----------
// Destructor
// ----------

SoFiA::~SoFiA()
{
    if(QFile::exists(SOFIA_TEMP_FILE))
    {
        // Temporary parameter file present, needs to be deleted
        if(QFile::remove(SOFIA_TEMP_FILE) == false)
        {
            std::cerr << "Warning: Failed to remove temporary parameter file on exit." << std::endl;
        }
    }
    
    return;
}



// ----------------------------
// Function to display messages
// ----------------------------

int SoFiA::showMessage(int severity, QString &messageText, QString &statusText)
{
    if(severity < 0 or severity > 2) severity = 0;
    
    if(!statusText.isEmpty())
    {
        QString statusTitle[3] = {tr("Information"), tr("Warning"), tr("Error")};
        this->statusBar()->showMessage(QString("%1: %2").arg(statusTitle[severity]).arg(statusText));
    }
    
    if(!messageText.isEmpty())
    {
        QString titleText[3] = {tr("SoFiA - Information"), tr("SoFiA - Warning"), tr("SoFiA - Error")};
        QMessageBox messageBox;
        messageBox.setWindowTitle(titleText[severity]);
        messageBox.setText(messageText);
        if      (severity == 2) messageBox.setIcon(QMessageBox::Critical);
        else if (severity == 1) messageBox.setIcon(QMessageBox::Warning);
        else                    messageBox.setIcon(QMessageBox::Information);
        messageBox.exec();
    }
    
    return 0;
}



// ---------------------------------
// Function to select file from disc
// ---------------------------------

int SoFiA::selectFile(QLineEdit *target)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("SoFiA - Select File"), QDir::currentPath());
    
    if(fileName.isEmpty() or target == 0) return 1;
    
    target->setText(fileName);
    
    updateFields();
    
    return 0;
}



// ------------------------
// Function to set defaults
// ------------------------

int SoFiA::setDefaults()
{
    QString fileName = SOFIA_DEFAULT_SETTINGS;
    
    if(loadFile(fileName))
    {
        QString messageText = tr("<p>Failed to load default parameters.</p><p>Please close the programme and check your installation. SoFiA will not function properly without the default parameters.</p>");
        QString statusText = tr("Failed to load default parameters.");
        showMessage(2, messageText, statusText);
        
        return 1;
    }
    
    currentFileName.clear();
    
    QString messageText = QString("");
    QString statusText = tr("Parameters reset to default.");
    showMessage(0, messageText, statusText);
    
    this->setWindowTitle(tr("SoFiA"));
    
    return 0;
}



// ----------------------------
// Function to update variables
// ----------------------------

int SoFiA::updateVariables()
{
    // For each type of input field/button a separate set of commands is needed, because 
    // the access methods all differ (text(), value(), isChecked(), etc.)
    
    QList<QLineEdit*> widgetLineEdit = tabs->findChildren<QLineEdit*>();
    
    foreach(QLineEdit *w, widgetLineEdit)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            parameters.insert(w->objectName(), w->text());
        }
    }
    
    QList<QTextEdit*> widgetTextEdit = tabs->findChildren<QTextEdit*>();
    
    foreach(QTextEdit *w, widgetTextEdit)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            parameters.insert(w->objectName(), w->toPlainText());
        }
    }
    
    QList<QComboBox*> widgetComboBox = tabs->findChildren<QComboBox*>();
    
    foreach(QComboBox *w, widgetComboBox)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            parameters.insert(w->objectName(), w->currentText());
        }
    }
    
    QList<QSpinBox*> widgetSpinBox = tabs->findChildren<QSpinBox*>();
    
    foreach(QSpinBox *w, widgetSpinBox)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            QString value;
            value.setNum(w->value());
            
            parameters.insert(w->objectName(), value);
        }
    }
    
    QList<QAbstractButton*> widgetAbstractButton = tabs->findChildren<QAbstractButton*>();
    
    foreach(QAbstractButton *w, widgetAbstractButton)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            QString value = "false";
            if(w->isChecked()) value = "true";
            
            parameters.insert(w->objectName(), value);
        }
    }
    
    QList<QGroupBox*> widgetGroupBox = tabs->findChildren<QGroupBox*>();
    
    foreach(QGroupBox *w, widgetGroupBox)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            QString value = "false";
            if(w->isChecked()) value = "true";
            
            parameters.insert(w->objectName(), value);
        }
    }
    
    // Treat list of output parameters separately and check respective check boxes:
    QString listOutputPar;
    
    if(tabOutputButtonParameterAll->isChecked())
    {
        listOutputPar = QString("\'*\'");
    }
    else
    {
        if(tabOutputButtonParameterID->isChecked())     listOutputPar.append(QString("\'ID\',"));
        if(tabOutputButtonParameterX->isChecked())      listOutputPar.append(QString("\'Xm\',"));
        if(tabOutputButtonParameterY->isChecked())      listOutputPar.append(QString("\'Ym\',"));
        if(tabOutputButtonParameterZ->isChecked())      listOutputPar.append(QString("\'Zm\',"));
        if(tabOutputButtonParameterW50->isChecked())    listOutputPar.append(QString("\'W50\',"));
        if(tabOutputButtonParameterW20->isChecked())    listOutputPar.append(QString("\'W20\',"));
        if(tabOutputButtonParameterFpeak->isChecked())  listOutputPar.append(QString("\'Fmax\',"));
        if(tabOutputButtonParameterFint->isChecked())   listOutputPar.append(QString("\'Ftot\',"));
        if(tabOutputButtonParameterEllMaj->isChecked()) listOutputPar.append(QString("\'ELL_MAJ\',"));
        if(tabOutputButtonParameterEllMin->isChecked()) listOutputPar.append(QString("\'ELL_MIN\',"));
        if(tabOutputButtonParameterEllPA->isChecked())  listOutputPar.append(QString("\'ELL_PA\',"));
        if(tabOutputButtonParameterRel->isChecked())    listOutputPar.append(QString("\'Rel\',"));
        if(tabOutputButtonParameterRms->isChecked())    listOutputPar.append(QString("\'RMS_CUBE\',"));
        
        if(!listOutputPar.isEmpty())
        {
            listOutputPar.truncate(listOutputPar.size() - 1);
        }
        else
        {
            listOutputPar = QString("\'*\'");
        }
    }
    
    listOutputPar.prepend(QString("["));
    listOutputPar.append(QString("]"));
    
    parameters.insert(QString("writeCat.parameters"), listOutputPar);
    
    return 0;
}



//-----------------------
// Function to set fields
// ----------------------

int SoFiA::setFields()
{
    // For each type of input field/button a separate set of commands is needed, because 
    // the access methods all differ (text(), value(), isChecked(), etc.)
    
    QList<QLineEdit*> widgetLineEdit = tabs->findChildren<QLineEdit*>();
    
    foreach(QLineEdit *w, widgetLineEdit)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            w->setText(parameters.value(w->objectName()));
        }
    }
    
    QList<QTextEdit*> widgetTextEdit = tabs->findChildren<QTextEdit*>();
    
    foreach(QTextEdit *w, widgetTextEdit)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            w->setPlainText(parameters.value(w->objectName()));
        }
    }
    
    QList<QComboBox*> widgetComboBox = tabs->findChildren<QComboBox*>();
    
    foreach(QComboBox *w, widgetComboBox)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            int index = w->findText(parameters.value(w->objectName()));
            if(index >= 0) w->setCurrentIndex(index);
        }
    }
    
    QList<QSpinBox*> widgetSpinBox = tabs->findChildren<QSpinBox*>();
    
    foreach(QSpinBox *w, widgetSpinBox)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            QString value = parameters.value(w->objectName());
            
            w->setValue(value.toInt());
        }
    }
    
    QList<QAbstractButton*> widgetAbstractButton = tabs->findChildren<QAbstractButton*>();
    
    foreach(QAbstractButton *w, widgetAbstractButton)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            QString value = parameters.value(w->objectName());
            
            if(value == "True" or value == "true" or value == "TRUE" or value == "T" or value == "t" or value == "1" or value == "Yes" or value == "yes" or value == "YES" or value == "Y" or value == "y") w->setChecked(true);
            else w->setChecked(false);
        }
    }
    
    QList<QGroupBox*> widgetGroupBox = tabs->findChildren<QGroupBox*>();
    
    foreach(QGroupBox *w, widgetGroupBox)
    {
        if(parameters.contains(w->objectName()))      // Only existing parameters will get updated!
        {
            QString value = parameters.value(w->objectName());
            
            if(value == "True" or value == "true" or value == "TRUE" or value == "T" or value == "t" or value == "1" or value == "Yes" or value == "yes" or value == "YES" or value == "Y" or value == "y") w->setChecked(true);
            else w->setChecked(false);
        }
    }
    
    // Treat list of output parameters separately and set respective check boxes:
    QString listOutputPar = parameters.value(QString("writeCat.parameters"));
    
    if(listOutputPar.contains(QString("\'*\'")))
    {
        tabOutputButtonParameterAll->setChecked(true);
    }
    else
    {
        tabOutputButtonParameterAll->setChecked(false);
        tabOutputButtonParameterID->setChecked(false);
        tabOutputButtonParameterX->setChecked(false);
        tabOutputButtonParameterY->setChecked(false);
        tabOutputButtonParameterZ->setChecked(false);
        tabOutputButtonParameterLon->setChecked(false);
        tabOutputButtonParameterLat->setChecked(false);
        tabOutputButtonParameterFreq->setChecked(false);
        tabOutputButtonParameterVrad->setChecked(false);
        tabOutputButtonParameterW50->setChecked(false);
        tabOutputButtonParameterW20->setChecked(false);
        tabOutputButtonParameterFpeak->setChecked(false);
        tabOutputButtonParameterFint->setChecked(false);
        tabOutputButtonParameterRel->setChecked(false);
        tabOutputButtonParameterFlags->setChecked(false);
        tabOutputButtonParameterEllMaj->setChecked(false);
        tabOutputButtonParameterEllMin->setChecked(false);
        tabOutputButtonParameterEllPA->setChecked(false);
        tabOutputButtonParameterRms->setChecked(false);
        tabOutputButtonParameterBFPar->setChecked(false);
        tabOutputButtonParameterBFPhys->setChecked(false);
        tabOutputButtonParameterBFInfo->setChecked(false);
        
        if(listOutputPar.contains(QString("\'ID\'")))       tabOutputButtonParameterID->setChecked(true);
        if(listOutputPar.contains(QString("\'Xm\'")))       tabOutputButtonParameterX->setChecked(true);
        if(listOutputPar.contains(QString("\'Ym\'")))       tabOutputButtonParameterY->setChecked(true);
        if(listOutputPar.contains(QString("\'Zm\'")))       tabOutputButtonParameterZ->setChecked(true);
        if(listOutputPar.contains(QString("\'W50\'")))      tabOutputButtonParameterW50->setChecked(true);
        if(listOutputPar.contains(QString("\'W20\'")))      tabOutputButtonParameterW20->setChecked(true);
        if(listOutputPar.contains(QString("\'Fmax\'")))     tabOutputButtonParameterFpeak->setChecked(true);
        if(listOutputPar.contains(QString("\'Ftot\'")))     tabOutputButtonParameterFint->setChecked(true);
        if(listOutputPar.contains(QString("\'ELL_MAJ\'")))  tabOutputButtonParameterEllMaj->setChecked(true);
        if(listOutputPar.contains(QString("\'ELL_MIN\'")))  tabOutputButtonParameterEllMin->setChecked(true);
        if(listOutputPar.contains(QString("\'ELL_PA\'")))   tabOutputButtonParameterEllPA->setChecked(true);
        if(listOutputPar.contains(QString("\'Rel\'")))      tabOutputButtonParameterRel->setChecked(true);
        if(listOutputPar.contains(QString("\'RMS_CUBE\'"))) tabOutputButtonParameterRms->setChecked(true);
    }
    
    return 0;
}



// ---------------------
// Slot to load settings
// ---------------------

void SoFiA::loadSettings()
{
    QString newFileName = QFileDialog::getOpenFileName(this, tr("SoFiA - Load Parameters"), QDir::currentPath());
    
    if(!newFileName.isEmpty())
    {
        setDefaults();       // Load default settings first.
        
        if(loadFile(newFileName))
        {
            QString messageText = tr("<p>Failed to read input file %1.</p>").arg(newFileName.section('/', -1));
            QString statusText = tr("Failed to read input file %1.").arg(newFileName.section('/', -1));
            showMessage(2, messageText, statusText);
        }
    }
    
    return;
}



// ---------------------
// Function to load file
// ---------------------

int SoFiA::loadFile(QString &fileName)
{
    if(!fileName.isEmpty())
    {
        /*if(!currentFileName.isEmpty())
        {
            QMessageBox messageBox(this);
            messageBox.setWindowTitle(tr("SoFiA - Load Parameters"));
            messageBox.setText(tr("<p>Opening a new file will override all current parameter settings. Unsaved changes will be lost.</p><p>Do you wish to open a new file?</p>"));
            messageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
            messageBox.setDefaultButton(QMessageBox::Ok);
            messageBox.setIcon(QMessageBox::Warning);
            int choice = messageBox.exec();
            
            if(choice != QMessageBox::Ok)
            {
                return 0;
            }
        }*/
        
        QFile inFile(fileName);
        
        if(!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return 1;        // Error message should be generated by calling function.
        }
        
        QTextStream inStream(&inFile);
        QString keyname;
        QString value;
        
        int counter = 0;
        
        while(!inStream.atEnd())
        {
            QString line = inStream.readLine().trimmed();
            
            if((!line.isEmpty()) and (!line.startsWith("#")))
            {
                keyname = line.section(QChar('='), 0, 0).trimmed();
                value   = line.section(QChar('='), 1).trimmed();
                
                // Surprisingly, the following actually works:
                //QWidget *widget = tabs->findChild<QWidget*>(keyname);
                
                if((fileName == SOFIA_DEFAULT_SETTINGS or parameters.contains(keyname)) and keyname.size() != 0) //and widget != 0
                {
                    //parameters.insert(widget->objectName(), value);
                    parameters.insert(keyname, value);
                    counter++;
                }
            }
        }
        
        inFile.close();
        
        if(counter == 0)
        {
            QString messageText = tr("<p>No valid parameters found in input file %1.</p>").arg(fileName.section('/', -1));
            QString statusText = tr("No valid parameters found in input file %1.").arg(fileName.section('/', -1));
            showMessage(2, messageText, statusText);
        }
        else
        {
            currentFileName = fileName;
            
            setFields();
            updateFields();
            
            QString messageText = QString("");
            QString statusText = tr("Parameters loaded from %1.").arg(currentFileName.section('/', -1));
            showMessage(0, messageText, statusText);
            
            this->setWindowTitle(tr("SoFiA - %1").arg(currentFileName.section('/', -1)));
        }
    }
    
    return 0;
}



// ---------------------
// Slot to save settings
// ---------------------

void SoFiA::saveSettings()
{
    if(currentFileName.isEmpty())
    {
        saveSettingsAs();
    }
    else
    {
        QFile outFile(currentFileName);
        
        if(!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QString messageText = tr("<p>Failed to write to output file %1.</p>").arg(currentFileName.section('/', -1));
            QString statusText = tr("Failed to write to output file %1.").arg(currentFileName.section('/', -1));
            showMessage(2, messageText, statusText);
            
            currentFileName.clear();
            this->setWindowTitle(tr("SoFiA"));
            
            return;
        }
        
        updateVariables();   // This will update all parameters before saving.
        
        QTextStream outStream(&outFile);
        
        /*QList<QWidget*> widget = tabs->findChildren<QWidget*>();
        
        foreach(QWidget *w, widget)
        {
            if(parameter.contains(w))
            {
                outStream << w->objectName() << "\t=\t" << parameter.value(w) << endl;
            }
        }*/
        
        for(QMap<QString, QString>::iterator iter = parameters.begin(); iter != parameters.end(); iter++)
        {
            outStream << iter.key() << "\t=\t" << iter.value() << endl;
        }
        
        outFile.close();
        
        QString messageText = QString("");
        QString statusText = tr("Parameters saved to %1.").arg(currentFileName.section('/', -1));
        showMessage(0, messageText, statusText);
        
        this->setWindowTitle(tr("SoFiA - %1").arg(currentFileName.section('/', -1)));
    }
    
    return;
}



// ---------------------------
// Slot to save settings as...
// ---------------------------

void SoFiA::saveSettingsAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("SoFia - Save Parameters"), QDir::currentPath());
    
    if(!fileName.isEmpty())
    {
        currentFileName = fileName;
        saveSettings();
    }
    
    return;
}



// ------------------------------------
// Slot to save pipeline messages as...
// ------------------------------------

void SoFiA::saveLogAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("SoFia - Save Pipeline Messages"), QDir::currentPath());
    
    if(fileName.isEmpty()) return;
    
    QFile file(fileName);
    
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << outputText->toPlainText();
        
        file.close();
        
        QString messageText = QString("");
        QString statusText  = tr("Pipeline messages written to %1.").arg(fileName.section('/', -1));
        showMessage(0, messageText, statusText);
    }
    else
    {
        QString messageText = tr("Failed to write to file %1. Pipeline messages not saved.").arg(fileName.section('/', -1));
        QString statusText  = tr("Failed to write pipeline messages to %1.").arg(fileName.section('/', -1));
        showMessage(2, messageText, statusText);
    }
    
    return;
}



// -------------------------------
// Slot to clear pipeline messages
// -------------------------------

void SoFiA::clearLog()
{
    /*QMessageBox messageBox(this);
    messageBox.setWindowTitle(tr("SoFiA - Clear Pipeline Messages"));
    messageBox.setText(tr("<p>This action will clear the pipeline message interface and discard all output messages generated by previous pipeline runs.</p><p>Do you wish to clear all pipeline messages?</p>"));
    messageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.setIcon(QMessageBox::Warning);
    int choice = messageBox.exec();*/
    
    //if(choice == QMessageBox::Ok)
    if(true)
    {
        outputText->clear();
        outputProgress->setValue(0);
        
        QString messageText = tr("");
        QString statusText  = tr("Pipeline messages cleared.");
        showMessage(0, messageText, statusText);
        
        updateActions();
    }
    
    return;
}



// -------------------------
// Slot to update the fields
// -------------------------

void SoFiA::updateFields()
{
    // Activate or de-activate fields and buttons
    
    tabOutFilterFieldW50Min->setEnabled(tabOutFilterButtonW50->isChecked());
    tabOutFilterFieldW50Max->setEnabled(tabOutFilterButtonW50->isChecked());
    tabOutFilterFieldW20Min->setEnabled(tabOutFilterButtonW20->isChecked());
    tabOutFilterFieldW20Max->setEnabled(tabOutFilterButtonW20->isChecked());
    tabOutFilterFieldFpeakMin->setEnabled(tabOutFilterButtonFpeak->isChecked());
    tabOutFilterFieldFpeakMax->setEnabled(tabOutFilterButtonFpeak->isChecked());
    tabOutFilterFieldFintMin->setEnabled(tabOutFilterButtonFint->isChecked());
    tabOutFilterFieldFintMax->setEnabled(tabOutFilterButtonFint->isChecked());
    
    double n = tabParametrisationFieldRelMin->text().toDouble();
    if(n < 0.0) tabParametrisationFieldRelMin->setText("0.0");
    else if(n > 1.0) tabParametrisationFieldRelMin->setText("1.0");
    
    n = tabSourceFindingFieldThreshold->text().toDouble();
    if(n < 0.0) tabSourceFindingFieldThreshold->setText("0.0");
    
    n = tabSourceFindingFieldThreshold2->text().toDouble();
    if(n < 0.0) tabSourceFindingFieldThreshold2->setText("0.0");
    
    n = tabInFilterFieldSmoothingSpatialLon->text().toDouble();
    if(n < 0.0) tabInFilterFieldSmoothingSpatialLon->setText("0.0");
    n = tabInFilterFieldSmoothingSpatialLat->text().toDouble();
    if(n < 0.0) tabInFilterFieldSmoothingSpatialLat->setText("0.0");
    n = tabInFilterFieldSmoothingSpectral->text().toDouble();
    if(n < 0.0) tabInFilterFieldSmoothingSpectral->setText("0.0");
    
    tabOutputButtonParameterID->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterX->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterY->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterZ->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterLon->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterLat->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterFreq->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterVrad->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterW50->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterW20->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterFpeak->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterFint->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterRel->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterFlags->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterEllMaj->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterEllMin->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterEllPA->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterRms->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterBFPar->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterBFPhys->setEnabled(!tabOutputButtonParameterAll->isChecked());
    tabOutputButtonParameterBFInfo->setEnabled(!tabOutputButtonParameterAll->isChecked());
    
    updateActions();
    
    return;
}



// --------------------------------------
// Function to update actions and buttons
// --------------------------------------

void SoFiA::updateActions()
{
    // Activate or de-activate actions and buttons
    
    tabOutputButtonGo->setEnabled(!((tabInputFieldData->text()).isEmpty()) and (pipelineProcess->state() == QProcess::NotRunning));
    actionRun        ->setEnabled(!((tabInputFieldData->text()).isEmpty()) and (pipelineProcess->state() == QProcess::NotRunning));
    
    actionAbort->setEnabled(pipelineProcess->state() == QProcess::Running);
    //actionExit->setEnabled(pipelineProcess->state() == QProcess::NotRunning);
    
    actionSaveLogAs->setEnabled(outputText->toPlainText() != "" and pipelineProcess->state() == QProcess::NotRunning);
    actionClearLog->setEnabled(outputText->toPlainText() != "" and pipelineProcess->state() == QProcess::NotRunning);
    actionShowCatalogue->setEnabled(!((tabInputFieldData->text()).isEmpty()));
    
    return;
}



// ------------------------------
// Slot to select input data cube
// ------------------------------

void SoFiA::selectInputDataFile()
{
    selectFile(tabInputFieldData);
    
    return;
}



// ---------------------------------
// Slot to select input weights cube
// ---------------------------------

void SoFiA::selectInputWeightsFile()
{
    selectFile(tabInputFieldWeights);
    
    return;
}



// ---------------------------------
// Slot to select input weights cube
// ---------------------------------

void SoFiA::selectInputMaskFile()
{
    selectFile(tabInputFieldMask);
    
    return;
}



// ----------------------------
// Slot to display previous tab
// ----------------------------

void SoFiA::displayPrevTab()
{
    int i = tabs->currentIndex();
    
    if(i > 0) tabs->setCurrentIndex(i - 1);
    
    return;
}



// ------------------------
// Slot to display next tab
// ------------------------

void SoFiA::displayNextTab()
{
    int i = tabs->currentIndex();
    
    if(i < 6) tabs->setCurrentIndex(i + 1);
    
    return;
}



// ---------------------------------
// Slot to reset settings to default
// ---------------------------------

void SoFiA::resetToDefault()
{
    QMessageBox messageBox(this);
    messageBox.setWindowTitle(tr("SoFiA - New Parameter File"));
    messageBox.setText(tr("<p>This action will reset all parameters to their default values and close the current file. All unsaved changes will be lost.</p><p>Do you wish to create a new parameter file?</p>"));
    messageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.setIcon(QMessageBox::Warning);
    int choice = messageBox.exec();
    
    if(choice == QMessageBox::Ok)
    {
        setDefaults();
    }
    
    return;
}



// ---------------------------
// Slot to show SoFiA handbook
// ---------------------------

void SoFiA::showHandbook(const QString &page)
{
    if(page.isEmpty()) HelpBrowser::showPage("index.html");
    else HelpBrowser::showPage(page);
    
    return;
}



// --------------------------
// Slot to show About message
// --------------------------

void SoFiA::aboutSoFiA()
{
    QString messageText = tr("<h3>About SoFiA</h3><p>Version 0.2</p><p>SoFiA, the <b>Source Finding Application</b>, is an HI source finding pipeline designed to find and parametrise galaxies in HI data cubes. The name SoFiA is based on the Greek word %1, which means wisdom.</p><p>SoFiA is free software: you can redistribute it and/or modify it under the terms of the <b>GNU General Public License</b> as published by the Free Software Foundation, either version 3 of the licence, or (at your option) any later version.</p><p>SoFiA is distributed in the hope that it will be useful, but <b>without any warranty</b>; without even the implied warranty of merchantability or fitness for a particular purpose. See the GNU General Public License for more details.</p><p>You should have received a copy of the GNU General Public License along with SoFiA. If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.</p><p>SoFiA uses the Oxygen icon set which is licensed under version&nbsp;3 of the <a href=\"http://www.gnu.org/licenses/lgpl-3.0.txt\">GNU Lesser General Public License</a>. For more details please visit the website of the <a href=\"http://www.oxygen-icons.org/\">Oxygen project</a>.</p><p>&copy; 2013&ndash;2014 The SoFiA Authors</p>").arg(QString::fromUtf8("σοφία"));
    QString statusText = QString("");
    showMessage(0, messageText, statusText);
    
    return;
}



// --------------------
// Slot to run pipeline
// --------------------

void SoFiA::runPipeline()
{
    /*QMessageBox messageBox(this);
    messageBox.setWindowTitle(tr("SoFiA - Run Pipeline"));
    messageBox.setText(tr("<p>This action will run the pipeline with the parameters as currently set in the user interface. Depending on your hardware, the size of your data file, and the actual parameters, this can take up to several hours.</p><p>Do you wish to run the pipeline?</p>"));
    messageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.setIcon(QMessageBox::Information);
    int choice = messageBox.exec();
    
    if(choice == QMessageBox::Ok)
    {
    }*/
    
    if(pipelineProcess->state() != QProcess::NotRunning)
    {
        QString messageText = tr("<p>The pipeline is already running.</p>");
        QString statusText = tr("Pipeline already running.");
        showMessage(0, messageText, statusText);
    }
    else
    {
        bool flagTmpFile = false;
        if(dockWidgetOutput->isHidden()) dockWidgetOutput->show();
        
        QString command("python");
        QStringList arguments;
        
        if(currentFileName.isEmpty())
        {
            currentFileName = SOFIA_TEMP_FILE;
            flagTmpFile = true;
        }
        
        char *SOFIA_FULL_PATH = getenv("SOFIA_PIPELINE_PATH");
        
        arguments << SOFIA_FULL_PATH << currentFileName;
        saveSettings();
        
        if(flagTmpFile == true)
        {
            currentFileName.clear();
            this->setWindowTitle(tr("SoFiA"));
        }
        
        pipelineProcess->start(command, arguments);
    }
    
    return;
}



// -----------------------------------------
// Slot to read output from pipeline process
// -----------------------------------------

void SoFiA::pipelineProcessReadStd()
{
    QByteArray output = pipelineProcess->readAllStandardOutput();
    QString    outputStd(QString::fromUtf8(output));
    
    outputStd.remove(QChar('\r'));       // Get rid of carriage returns in the output; they produce line break!
    
    unsigned int progress = outputProgress->value();
    
    if(outputStd.contains(QString("--- SoFiA: Reading default parameters ---"))) progress = 0;
    if(outputStd.contains(QString("--- SoFiA: Reading user parameters ---")))    progress = 5;
    if(outputStd.contains(QString("--- SoFiA: Reading data cube(s) ---")))       progress = 10;
    if(outputStd.contains(QString("--- SoFiA: Running input filters ---")))      progress = 15;
    if(outputStd.contains(QString("--- SoFiA: Running source finder ---")))      progress = 25;
    if(outputStd.contains(QString("--- SoFiA: Merging detections ---")))         progress = 40;
    if(outputStd.contains(QString("--- SoFiA: Writing mask cube ---")))          progress = 50;
    if(outputStd.contains(QString("--- SoFiA: Determining reliability ---")))    progress = 55;
    if(outputStd.contains(QString("--- SoFiA: Writing raw catalogue ---")))      progress = 65;
    if(outputStd.contains(QString("--- SoFiA: Writing moment maps ---")))        progress = 70;
    if(outputStd.contains(QString("--- SoFiA: Parametrising sources ---")))      progress = 75;
    if(outputStd.contains(QString("--- SoFiA: Writing cubelets ---")))           progress = 90;
    if(outputStd.contains(QString("--- SoFiA: Writing output catalogue ---")))   progress = 95;
    if(outputStd.contains(QString("--- SoFiA: Pipeline finished ---")))          progress = 100;
    
    outputProgress->setValue(progress);
    
    if(!outputStd.isEmpty())
    {
        outputText->setTextColor(Qt::black);
        outputText->append(outputStd);
    }
    
    return;
}



// -----------------------------------------
// Slot to read errors from pipeline process
// -----------------------------------------

void SoFiA::pipelineProcessReadErr()
{
    QByteArray output = pipelineProcess->readAllStandardError();
    QString    outputErr(output);
    
    outputErr.remove(QChar('\r'));       // Get rid of carriage returns in the output
    
    if(!outputErr.isEmpty())
    {
        outputText->setTextColor(Qt::red);
        outputText->append(outputErr);
    }
    
    return;
}



// -------------------------------
// Slot to react to pipeline start
// -------------------------------

void SoFiA::pipelineProcessStarted()
{
    QString messageText("");
    QString statusText = tr("Pipeline started.");
    showMessage(0, messageText, statusText);
    
    outputProgress->setValue(0);
    
    updateActions();
    
    return;
}



// --------------------------------
// Slot to react to pipeline finish
// --------------------------------

void SoFiA::pipelineProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus == QProcess::NormalExit)
    {
        if(exitCode == 0)
        {
            // Pipeline finished successfully:
            QString messageText("");
            QString statusText = tr("Pipeline finished.");
            showMessage(0, messageText, statusText);
            
            outputText->setTextColor(Qt::darkGreen);
            outputText->append(QString("Pipeline finished with exit code %1.\n").arg(exitCode));
            
            if(!spreadsheet->isHidden()) showCatalogue();  // Reload catalogue if currently visible.
        }
        else
        {
            // Pipeline finished with error:
            QString messageText("");
            QString statusText = tr("Pipeline failed.");
            showMessage(0, messageText, statusText);
            
            outputText->setTextColor(Qt::red);
            outputText->append(QString("Pipeline failed with exit code %1.\n").arg(exitCode));
        }
    }
    else
    {
        // Pipeline was aborted or crashed:
        QString messageText("");
        QString statusText = tr("Pipeline aborted.");
        showMessage(2, messageText, statusText);
        
        outputText->setTextColor(Qt::red);
        outputText->append(QString("Pipeline aborted with exit code %1.\n").arg(exitCode));
    }
    
    updateActions();
    
    return;
}



// ---------------------------
// Slot to cancel pipeline run
// ---------------------------

void SoFiA::pipelineProcessCancel()
{
    actionAbort->setEnabled(false);
    
    if(pipelineProcess->state() == QProcess::Running)
    {
        pipelineProcess->terminate();                      // First try to terminate process.
    }
    
    if(!pipelineProcess->waitForFinished(30000))           // Give it 30 s to terminate.
    {
        if(pipelineProcess->state() == QProcess::Running)
        {
            pipelineProcess->kill();                       // If it doesn't, kill it.
        }
    }
    
    updateActions();
    
    return;
}



// ---------------------------------------
// Slot to react to pipeline process error
// ---------------------------------------

void SoFiA::pipelineProcessError(QProcess::ProcessError error)
{
    QString messageText("");
    QString statusText("");
    
    switch(error)
    {
        case QProcess::FailedToStart:
            outputText->setTextColor(Qt::red);
            outputText->append(QString("Error: Failed to launch pipeline.\n"));
            
            messageText = tr("<p>Failed to launch pipeline. Please ensure that the pipeline is installed on your computer and you have permission to execute it.</p>");
            statusText = tr("Failed to launch pipeline.");
            showMessage(2, messageText, statusText);
            break;
            
        case QProcess::Crashed:
            outputText->setTextColor(Qt::red);
            outputText->append(QString("Error: Pipeline terminated prematurely.\n"));
            break;
            
        case QProcess::Timedout:
            outputText->setTextColor(Qt::red);
            outputText->append(QString("Error: Pipeline timed out.\n"));
            break;
            
        case QProcess::WriteError:
            outputText->setTextColor(Qt::red);
            outputText->append(QString("Error: Pipeline failed to receive input.\n"));
            break;
            
        case QProcess::ReadError:
            outputText->setTextColor(Qt::red);
            outputText->append(QString("Error: Failed to receive output from pipeline.\n"));
            break;
            
        default:
            outputText->setTextColor(Qt::red);
            outputText->append(QString("Error: An unspecified error occurred.\n"));
    }
    
    return;
}



// -----------------------------
// Slot to show source catalogue
// -----------------------------

void SoFiA::showCatalogue()
{
    QString filename = tabInputFieldData->text();
    filename.replace(".fits", "_cat.xml", Qt::CaseInsensitive);
    
    if(spreadsheet->loadCatalog(filename))
    {
        QString messageText = tr("<p>Failed to load source catalogue:</p><p>\"%1\"</p>").arg(filename);
        QString statusText  = tr("Failed to load source catalogue.");
        showMessage(2, messageText, statusText);
    }
    else
    {
        spreadsheet->show();
        spreadsheet->raise();
    }
    
    return;
}



// --------------------------------------------
// Function to create and set up user interface
// --------------------------------------------

void SoFiA::createInterface()
{
    // Load icons
    
    QIcon iconSoFiA;
    iconSoFiA.addFile(QString(":/icons/32/SoFiA.png"), QSize(32, 32));
    
    QIcon iconDocumentNew     = QIcon::fromTheme("document-new", QIcon(":/icons/22/document-new.png"));
    QIcon iconDocumentOpen    = QIcon::fromTheme("document-open", QIcon(":/icons/22/document-open.png"));
    QIcon iconDocumentPreview = QIcon::fromTheme("document-preview", QIcon(":/icons/22/document-preview.png"));
    QIcon iconDocumentSave    = QIcon::fromTheme("document-save", QIcon(":/icons/22/document-save.png"));
    QIcon iconDocumentSaveAs  = QIcon::fromTheme("document-save-as", QIcon(":/icons/22/document-save-as.png"));
    QIcon iconApplicationExit = QIcon::fromTheme("application-exit", QIcon(":/icons/22/application-exit.png"));
    QIcon iconDialogOkApply   = QIcon::fromTheme("dialog-ok-apply", QIcon(":/icons/22/dialog-ok-apply.png"));
    QIcon iconDialogCancel    = QIcon::fromTheme("dialog-cancel", QIcon(":/icons/22/dialog-cancel.png"));
    QIcon iconDialogClose     = QIcon::fromTheme("dialog-close", QIcon(":/icons/22/dialog-close.png"));
    QIcon iconGoPreviousView  = QIcon::fromTheme("go-previous-view", QIcon(":/icons/22/go-previous-view.png"));
    QIcon iconGoNextView      = QIcon::fromTheme("go-next-view", QIcon(":/icons/22/go-next-view.png"));
    QIcon iconEditClearList   = QIcon::fromTheme("edit-clear-list", QIcon(":/icons/22/edit-clear-list.png"));
    QIcon iconHelpContents    = QIcon::fromTheme("help-contents", QIcon(":/icons/22/help-contents.png"));
    QIcon iconHelpAbout       = QIcon::fromTheme("help-about", QIcon(":/icons/22/help-about.png"));
    
    // Create main widget that contains everything else
    
    widgetMain = new QWidget(this);
    
    // Set up tabs
    
    tabs = new QTabWidget(widgetMain);
    
    tabInput = new QWidget(tabs);
    tabInFilter = new QWidget(tabs);
    tabSourceFinding = new QWidget(tabs);
    tabMerging = new QWidget(tabs);
    tabParametrisation = new QWidget(tabs);
    tabOutFilter = new QWidget(tabs);
    tabOutput = new QWidget(tabs);
    
    tabs->addTab(tabInput, tr("Input"));
    tabs->addTab(tabInFilter, tr("Input Filter"));
    tabs->addTab(tabSourceFinding, tr("Source Finding"));
    tabs->addTab(tabMerging, tr("Merging"));
    tabs->addTab(tabParametrisation, tr("Parametrisation"));
    tabs->addTab(tabOutFilter, tr("Output Filter"));
    tabs->addTab(tabOutput, tr("Output"));
    
    tabs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    tabs->setUsesScrollButtons(false);
    
    // Note that additional spaces at the end of the display text for checkboxes 
    // and radio buttons are a workaround for rendering issues under Mac OS X 
    // whereby the last character of the text was partly cut off.
    
    // Set up input tab
    
    tabInputLayout = new QVBoxLayout;
    
    tabInputGroupBox1 = new QGroupBox(tr("Input files"), tabInput);
    
    tabInputForm1 = new QFormLayout;
    
    tabInputFieldData  = new QLineEdit(tabInputGroupBox1);
    tabInputFieldData->setObjectName("import.inFile");
    tabInputFieldData->setToolTip("Name of input data cube (required)");
    connect(tabInputFieldData, SIGNAL(editingFinished()), this, SLOT(updateFields()));
    tabInputButtonData = new QPushButton(tr("Select..."), tabInputGroupBox1);
    connect(tabInputButtonData, SIGNAL(clicked()), this, SLOT(selectInputDataFile()));
    tabInputButtonData->setIcon(iconDocumentOpen);
    
    tabInputWidgetData = new QWidget(tabInputGroupBox1);
    tabInputLayoutData = new QHBoxLayout;
    tabInputLayoutData->addWidget(tabInputFieldData);
    tabInputLayoutData->addWidget(tabInputButtonData);
    tabInputLayoutData->setContentsMargins(0, 0, 0, 0);
    tabInputWidgetData->setLayout(tabInputLayoutData);
    
    tabInputFieldWeights  = new QLineEdit(tabInputGroupBox1);
    tabInputFieldWeights->setObjectName("import.weightsFile");
    tabInputFieldWeights->setToolTip("Name of data cube containing weights (optional)");
    tabInputButtonWeights = new QPushButton(tr("Select..."), tabInputGroupBox1);
    connect(tabInputButtonWeights, SIGNAL(clicked()), this, SLOT(selectInputWeightsFile()));
    tabInputButtonWeights->setIcon(iconDocumentOpen);
    
    tabInputWidgetWeights = new QWidget(tabInputGroupBox1);
    tabInputLayoutWeights = new QHBoxLayout;
    tabInputLayoutWeights->addWidget(tabInputFieldWeights);
    tabInputLayoutWeights->addWidget(tabInputButtonWeights);
    tabInputLayoutWeights->setContentsMargins(0, 0, 0, 0);
    tabInputWidgetWeights->setLayout(tabInputLayoutWeights);
    
    tabInputFieldMask  = new QLineEdit(tabInputGroupBox1);
    tabInputFieldMask->setObjectName("import.maskFile");
    tabInputFieldMask->setToolTip("Name of mask cube (optional)");
    tabInputButtonMask = new QPushButton(tr("Select..."), tabInputGroupBox1);
    connect(tabInputButtonMask, SIGNAL(clicked()), this, SLOT(selectInputMaskFile()));
    tabInputButtonMask->setIcon(iconDocumentOpen);
    
    tabInputWidgetMask = new QWidget(tabInputGroupBox1);
    tabInputLayoutMask = new QHBoxLayout;
    tabInputLayoutMask->addWidget(tabInputFieldMask);
    tabInputLayoutMask->addWidget(tabInputButtonMask);
    tabInputLayoutMask->setContentsMargins(0, 0, 0, 0);
    tabInputWidgetMask->setLayout(tabInputLayoutMask);
    
    tabInputForm1->addRow(tr("Data cube:"), tabInputWidgetData);
    tabInputForm1->addRow(tr("Weights cube:"), tabInputWidgetWeights);
    tabInputForm1->addRow(tr("Mask cube:"), tabInputWidgetMask);
    
    tabInputButtonNext = new QPushButton(tr("Next"), tabInput);
    tabInputButtonNext->setIcon(iconGoNextView);
    connect(tabInputButtonNext, SIGNAL(clicked()), this, SLOT(displayNextTab()));
    tabInputLayoutControls = new QHBoxLayout();
    tabInputLayoutControls->setContentsMargins(0, 0, 0, 0);
    tabInputLayoutControls->setSpacing(0);
    tabInputLayoutControls->addStretch();
    tabInputLayoutControls->addWidget(tabInputButtonNext);
    tabInputWidgetControls = new QWidget(tabInput);
    tabInputWidgetControls->setLayout(tabInputLayoutControls);
    
    tabInputGroupBox1->setLayout(tabInputForm1);
    tabInputLayout->addWidget(tabInputGroupBox1);
    tabInputLayout->addStretch();
    tabInputLayout->addWidget(tabInputWidgetControls);
    tabInput->setLayout(tabInputLayout);
    
    
    
    // Set up input filter tab
    
    tabInFilterLayout = new QVBoxLayout;
    
    tabInFilterGroupBox1 = new QGroupBox(tr("Smoothing"), tabInFilter);
    tabInFilterGroupBox1->setObjectName("steps.doSmooth");
    tabInFilterGroupBox1->setCheckable(true);
    tabInFilterGroupBox1->setChecked(false);
    
    tabInFilterForm1 = new QFormLayout;
    
    tabInFilterFieldSmoothingSpatialLon  = new QLineEdit(tabInFilterGroupBox1);
    tabInFilterFieldSmoothingSpatialLon->setObjectName("smooth.kernelX");
    tabInFilterFieldSmoothingSpatialLon->setToolTip(tr("Longitude smoothing scale in pixels"));
    tabInFilterFieldSmoothingSpatialLon->setMaxLength(10);
    tabInFilterFieldSmoothingSpatialLon->setMaximumWidth(100);
    connect(tabInFilterFieldSmoothingSpatialLon, SIGNAL(editingFinished()), this, SLOT(updateFields()));
    tabInFilterFieldSmoothingSpatialLat  = new QLineEdit(tabInFilterGroupBox1);
    tabInFilterFieldSmoothingSpatialLat->setObjectName("smooth.kernelY");
    tabInFilterFieldSmoothingSpatialLat->setToolTip(tr("Latitude smoothing scale in pixels"));
    tabInFilterFieldSmoothingSpatialLat->setMaxLength(10);
    tabInFilterFieldSmoothingSpatialLat->setMaximumWidth(100);
    connect(tabInFilterFieldSmoothingSpatialLat, SIGNAL(editingFinished()), this, SLOT(updateFields()));
    tabInFilterFieldSmoothingSpectral = new QLineEdit(tabInFilterGroupBox1);
    tabInFilterFieldSmoothingSpectral->setObjectName("smooth.kernelZ");
    tabInFilterFieldSmoothingSpectral->setToolTip(tr("Spectral smoothing scale in channels"));
    tabInFilterFieldSmoothingSpectral->setMaxLength(10);
    tabInFilterFieldSmoothingSpectral->setMaximumWidth(100);
    connect(tabInFilterFieldSmoothingSpectral, SIGNAL(editingFinished()), this, SLOT(updateFields()));
    
    tabInFilterFieldKernel = new QComboBox(tabInFilterGroupBox1);
    tabInFilterFieldKernel->setObjectName("smooth.kernel");
    tabInFilterFieldKernel->addItem(tr("gaussian"));
    tabInFilterFieldKernel->addItem(tr("boxcar"));
    tabInFilterFieldKernel->addItem(tr("median"));
    
    tabInFilterFieldBorder = new QComboBox(tabInFilterGroupBox1);
    tabInFilterFieldBorder->setObjectName("smooth.edgeMode");
    tabInFilterFieldBorder->addItem(tr("constant"));
    tabInFilterFieldBorder->addItem(tr("reflect"));
    tabInFilterFieldBorder->addItem(tr("mirror"));
    tabInFilterFieldBorder->addItem(tr("nearest"));
    tabInFilterFieldBorder->addItem(tr("wrap"));
    
    tabInFilterForm1->addRow(tr("Kernel:"), tabInFilterFieldKernel);
    tabInFilterForm1->addRow(tr("Edge:"), tabInFilterFieldBorder);
    tabInFilterForm1->addRow(tr("Scale X:"), tabInFilterFieldSmoothingSpatialLon);
    tabInFilterForm1->addRow(tr("Scale Y:"), tabInFilterFieldSmoothingSpatialLat);
    tabInFilterForm1->addRow(tr("Scale Z:"), tabInFilterFieldSmoothingSpectral);
    
    
    
    tabInFilterGroupBox2 = new QGroupBox(tr("Noise scaling"), tabInFilter);
    tabInFilterGroupBox2->setObjectName("steps.doScaleNoise");
    tabInFilterGroupBox2->setCheckable(true);
    tabInFilterGroupBox2->setChecked(false);
    
    tabInFilterForm2 = new QFormLayout;
    
    tabInFilterFieldEdgeX  = new QSpinBox(tabInFilterGroupBox2);
    tabInFilterFieldEdgeX->setObjectName("scaleNoise.edgeX");
    tabInFilterFieldEdgeX->setMaximumWidth(100);
    tabInFilterFieldEdgeX->setMinimum(0);
    tabInFilterFieldEdgeX->setMaximum(100);
    tabInFilterFieldEdgeY = new QSpinBox(tabInFilterGroupBox2);
    tabInFilterFieldEdgeY->setObjectName("scaleNoise.edgeY");
    tabInFilterFieldEdgeY->setMaximumWidth(100);
    tabInFilterFieldEdgeY->setMinimum(0);
    tabInFilterFieldEdgeY->setMaximum(100);
    tabInFilterFieldEdgeZ = new QSpinBox(tabInFilterGroupBox2);
    tabInFilterFieldEdgeZ->setObjectName("scaleNoise.edgeZ");
    tabInFilterFieldEdgeZ->setMaximumWidth(100);
    tabInFilterFieldEdgeZ->setMinimum(0);
    tabInFilterFieldEdgeZ->setMaximum(100);
    
    tabInFilterFieldStatistic = new QComboBox(tabInFilterGroupBox2);
    tabInFilterFieldStatistic->setObjectName("scaleNoise.statistic");
    tabInFilterFieldStatistic->addItem(tr("mad"));
    tabInFilterFieldStatistic->addItem(tr("std"));
    tabInFilterFieldStatistic->addItem(tr("negative"));
    
    tabInFilterForm2->addRow(tr("Statistic:"), tabInFilterFieldStatistic);
    tabInFilterForm2->addRow(tr("Edge X:"), tabInFilterFieldEdgeX);
    tabInFilterForm2->addRow(tr("Edge Y:"), tabInFilterFieldEdgeY);
    tabInFilterForm2->addRow(tr("Edge Z:"), tabInFilterFieldEdgeZ);
    
    tabInFilterButtonPrev = new QPushButton(tr("Previous"), tabInFilter);
    tabInFilterButtonPrev->setIcon(iconGoPreviousView);
    connect(tabInFilterButtonPrev, SIGNAL(clicked()), this, SLOT(displayPrevTab()));
    tabInFilterButtonNext = new QPushButton(tr("Next"), tabInFilter);
    tabInFilterButtonNext->setIcon(iconGoNextView);
    connect(tabInFilterButtonNext, SIGNAL(clicked()), this, SLOT(displayNextTab()));
    tabInFilterLayoutControls = new QHBoxLayout();
    tabInFilterLayoutControls->setContentsMargins(0, 0, 0, 0);
    tabInFilterLayoutControls->setSpacing(0);
    tabInFilterLayoutControls->addWidget(tabInFilterButtonPrev);
    tabInFilterLayoutControls->addStretch();
    tabInFilterLayoutControls->addWidget(tabInFilterButtonNext);
    tabInFilterWidgetControls = new QWidget(tabInFilter);
    tabInFilterWidgetControls->setLayout(tabInFilterLayoutControls);
    
    tabInFilterGroupBox1->setLayout(tabInFilterForm1);
    tabInFilterLayout->addWidget(tabInFilterGroupBox1);
    tabInFilterGroupBox2->setLayout(tabInFilterForm2);
    tabInFilterLayout->addWidget(tabInFilterGroupBox2);
    tabInFilterLayout->addStretch();
    tabInFilterLayout->addWidget(tabInFilterWidgetControls);
    
    tabInFilter->setLayout(tabInFilterLayout);
    
    
    
    
    
    // Set up source finding tab
    
    tabSourceFindingLayout = new QVBoxLayout;
    
    tabSourceFindingGroupBox1 = new QGroupBox(tr("Smooth + Clip finder"), tabSourceFinding);
    tabSourceFindingGroupBox1->setObjectName("steps.doSCfind");
    tabSourceFindingGroupBox1->setCheckable(true);
    tabSourceFindingGroupBox1->setChecked(true);
    
    tabSourceFindingWidget1Left = new QWidget(tabSourceFindingGroupBox1);
    tabSourceFindingWidget1Right = new QWidget(tabSourceFindingGroupBox1);
    tabSourceFindingForm1Left = new QFormLayout;
    tabSourceFindingForm1Right = new QFormLayout;
    tabSourceFindingForm1Layout = new QHBoxLayout;
    
    tabSourceFindingFieldThreshold  = new QLineEdit(tabSourceFindingGroupBox1);
    tabSourceFindingFieldThreshold->setObjectName("SCfind.threshold");
    tabSourceFindingFieldThreshold->setMaximumWidth(100);
    tabSourceFindingFieldThreshold->setMaxLength(10);
    connect(tabSourceFindingFieldThreshold, SIGNAL(editingFinished()), this, SLOT(updateFields()));
    
    tabSourceFindingFieldEdgeMode = new QComboBox(tabSourceFindingGroupBox1);
    tabSourceFindingFieldEdgeMode->setObjectName("SCfind.edgeMode");
    tabSourceFindingFieldEdgeMode->addItem(tr("constant"));
    tabSourceFindingFieldEdgeMode->addItem(tr("reflect"));
    tabSourceFindingFieldEdgeMode->addItem(tr("mirror"));
    tabSourceFindingFieldEdgeMode->addItem(tr("nearest"));
    tabSourceFindingFieldEdgeMode->addItem(tr("wrap"));
    
    tabSourceFindingFieldRmsMode = new QComboBox(tabSourceFindingGroupBox1);
    tabSourceFindingFieldRmsMode->setObjectName("SCfind.rmsMode");
    tabSourceFindingFieldRmsMode->addItem(tr("negative"));
    tabSourceFindingFieldRmsMode->addItem(tr("mad"));
    tabSourceFindingFieldRmsMode->addItem(tr("std"));
    
    tabSourceFindingFieldKunit = new QComboBox(tabSourceFindingGroupBox1);
    tabSourceFindingFieldKunit->setObjectName("SCfind.kernelUnit");
    tabSourceFindingFieldKunit->addItem(tr("pixel"));
    tabSourceFindingFieldKunit->addItem(tr("world"));
    
    tabSourceFindingFieldKernels = new QTextEdit(tabSourceFindingGroupBox1);
    tabSourceFindingFieldKernels->setObjectName("SCfind.kernels");
    tabSourceFindingFieldKernels->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tabSourceFindingFieldKernels->setMaximumHeight(120);
    
    tabSourceFindingForm1Left->addRow(tr("Threshold:"), tabSourceFindingFieldThreshold);
    tabSourceFindingForm1Left->addRow(tr("Edge mode:"), tabSourceFindingFieldEdgeMode);
    tabSourceFindingForm1Left->addRow(tr("RMS mode:"), tabSourceFindingFieldRmsMode);
    tabSourceFindingForm1Left->addRow(tr("Kernel units:"), tabSourceFindingFieldKunit);
    
    tabSourceFindingForm1Right->addRow(tr("Kernels:"), tabSourceFindingFieldKernels);
    
    tabSourceFindingWidget1Left->setLayout(tabSourceFindingForm1Left);
    tabSourceFindingWidget1Right->setLayout(tabSourceFindingForm1Right);
    
    tabSourceFindingForm1Layout->addWidget(tabSourceFindingWidget1Left);
    tabSourceFindingForm1Layout->addWidget(tabSourceFindingWidget1Right);
    
    tabSourceFindingGroupBox1->setLayout(tabSourceFindingForm1Layout);
    
    
    
    tabSourceFindingGroupBox2 = new QGroupBox(tr("Threshold finder"), tabSourceFinding);
    tabSourceFindingGroupBox2->setObjectName("steps.doThreshold");
    tabSourceFindingGroupBox2->setCheckable(true);
    tabSourceFindingGroupBox2->setChecked(false);
    
    tabSourceFindingForm2 = new QFormLayout;
    
    tabSourceFindingFieldThreshold2 = new QLineEdit(tabSourceFindingGroupBox2);
    tabSourceFindingFieldThreshold2->setObjectName("threshold.threshold");
    tabSourceFindingFieldThreshold2->setMaximumWidth(100);
    tabSourceFindingFieldThreshold2->setMaxLength(10);
    connect(tabSourceFindingFieldThreshold2, SIGNAL(editingFinished()), this, SLOT(updateFields()));
    
    tabSourceFindingFieldClipMethod = new QComboBox(tabSourceFindingGroupBox2);
    tabSourceFindingFieldClipMethod->setObjectName("threshold.mode");
    tabSourceFindingFieldClipMethod->addItem(tr("relative"));
    tabSourceFindingFieldClipMethod->addItem(tr("absolute"));
    
    tabSourceFindingFieldRmsMode2 = new QComboBox(tabSourceFindingGroupBox2);
    tabSourceFindingFieldRmsMode2->setObjectName("threshold.rmsMode");
    tabSourceFindingFieldRmsMode2->addItem(tr("negative"));
    tabSourceFindingFieldRmsMode2->addItem(tr("mad"));
    tabSourceFindingFieldRmsMode2->addItem(tr("std"));
    
    tabSourceFindingForm2->addRow(tr("Threshold:"), tabSourceFindingFieldThreshold2);
    tabSourceFindingForm2->addRow(tr("CLip mode:"), tabSourceFindingFieldClipMethod);
    tabSourceFindingForm2->addRow(tr("RMS mode:"), tabSourceFindingFieldRmsMode2);
    tabSourceFindingGroupBox2->setLayout(tabSourceFindingForm2);
    
    tabSourceFindingButtonPrev = new QPushButton(tr("Previous"), tabSourceFinding);
    tabSourceFindingButtonPrev->setIcon(iconGoPreviousView);
    connect(tabSourceFindingButtonPrev, SIGNAL(clicked()), this, SLOT(displayPrevTab()));
    tabSourceFindingButtonNext = new QPushButton(tr("Next"), tabSourceFinding);
    tabSourceFindingButtonNext->setIcon(iconGoNextView);
    connect(tabSourceFindingButtonNext, SIGNAL(clicked()), this, SLOT(displayNextTab()));
    tabSourceFindingLayoutControls = new QHBoxLayout();
    tabSourceFindingLayoutControls->setContentsMargins(0, 0, 0, 0);
    tabSourceFindingLayoutControls->setSpacing(0);
    tabSourceFindingLayoutControls->addWidget(tabSourceFindingButtonPrev);
    tabSourceFindingLayoutControls->addStretch();
    tabSourceFindingLayoutControls->addWidget(tabSourceFindingButtonNext);
    tabSourceFindingWidgetControls = new QWidget(tabSourceFinding);
    tabSourceFindingWidgetControls->setLayout(tabSourceFindingLayoutControls);
    
    tabSourceFindingLayout->addWidget(tabSourceFindingGroupBox1);
    tabSourceFindingLayout->addWidget(tabSourceFindingGroupBox2);
    tabSourceFindingLayout->addStretch();
    tabSourceFindingLayout->addWidget(tabSourceFindingWidgetControls);
    tabSourceFinding->setLayout(tabSourceFindingLayout);
    
    
    
    // Set up merging tab
    
    tabMergingLayout = new QVBoxLayout;
    
    tabMergingGroupBox1 = new QGroupBox(tr("Merge detections"), tabMerging);
    tabMergingGroupBox1->setObjectName("steps.doMerge");
    tabMergingGroupBox1->setCheckable(true);
    tabMergingGroupBox1->setChecked(true);
    tabMergingForm1 = new QFormLayout;
    
    tabMergingFieldMergeX = new QSpinBox(tabMergingGroupBox1);
    tabMergingFieldMergeX->setObjectName("merge.mergeX");
    tabMergingFieldMergeX->setMaximumWidth(100);
    tabMergingFieldMergeX->setMinimum(0);
    tabMergingFieldMergeX->setMaximum(50);
    tabMergingFieldMergeY = new QSpinBox(tabMergingGroupBox1);
    tabMergingFieldMergeY->setObjectName("merge.mergeY");
    tabMergingFieldMergeY->setMaximumWidth(100);
    tabMergingFieldMergeY->setMinimum(0);
    tabMergingFieldMergeY->setMaximum(50);
    tabMergingFieldMergeZ = new QSpinBox(tabMergingGroupBox1);
    tabMergingFieldMergeZ->setObjectName("merge.mergeZ");
    tabMergingFieldMergeZ->setMaximumWidth(100);
    tabMergingFieldMergeZ->setMinimum(0);
    tabMergingFieldMergeZ->setMaximum(50);
    tabMergingFieldMinSizeX = new QSpinBox(tabMergingGroupBox1);
    tabMergingFieldMinSizeX->setObjectName("merge.minSizeX");
    tabMergingFieldMinSizeX->setMaximumWidth(100);
    tabMergingFieldMinSizeX->setMinimum(1);
    tabMergingFieldMinSizeX->setMaximum(50);
    tabMergingFieldMinSizeY = new QSpinBox(tabMergingGroupBox1);
    tabMergingFieldMinSizeY->setObjectName("merge.minSizeY");
    tabMergingFieldMinSizeY->setMaximumWidth(100);
    tabMergingFieldMinSizeY->setMinimum(1);
    tabMergingFieldMinSizeY->setMaximum(50);
    tabMergingFieldMinSizeZ = new QSpinBox(tabMergingGroupBox1);
    tabMergingFieldMinSizeZ->setObjectName("merge.minSizeZ");
    tabMergingFieldMinSizeZ->setMaximumWidth(100);
    tabMergingFieldMinSizeZ->setMinimum(1);
    tabMergingFieldMinSizeZ->setMaximum(50);
    
    tabMergingForm1->addRow(tr("Radius X:"), tabMergingFieldMergeX);
    tabMergingForm1->addRow(tr("Radius Y:"), tabMergingFieldMergeY);
    tabMergingForm1->addRow(tr("Radius Z:"), tabMergingFieldMergeZ);
    tabMergingForm1->addRow(tr("Min. size X:"), tabMergingFieldMinSizeX);
    tabMergingForm1->addRow(tr("Min. size Y:"), tabMergingFieldMinSizeY);
    tabMergingForm1->addRow(tr("Min. size Z:"), tabMergingFieldMinSizeZ);
    tabMergingGroupBox1->setLayout(tabMergingForm1);
    
    tabMergingButtonPrev = new QPushButton(tr("Previous"), tabMerging);
    tabMergingButtonPrev->setIcon(iconGoPreviousView);
    connect(tabMergingButtonPrev, SIGNAL(clicked()), this, SLOT(displayPrevTab()));
    tabMergingButtonNext = new QPushButton(tr("Next"), tabMerging);
    tabMergingButtonNext->setIcon(iconGoNextView);
    connect(tabMergingButtonNext, SIGNAL(clicked()), this, SLOT(displayNextTab()));
    tabMergingLayoutControls = new QHBoxLayout();
    tabMergingLayoutControls->setContentsMargins(0, 0, 0, 0);
    tabMergingLayoutControls->setSpacing(0);
    tabMergingLayoutControls->addWidget(tabMergingButtonPrev);
    tabMergingLayoutControls->addStretch();
    tabMergingLayoutControls->addWidget(tabMergingButtonNext);
    tabMergingWidgetControls = new QWidget(tabMerging);
    tabMergingWidgetControls->setLayout(tabMergingLayoutControls);
    
    tabMergingLayout->addWidget(tabMergingGroupBox1);
    tabMergingLayout->addStretch();
    tabMergingLayout->addWidget(tabMergingWidgetControls);
    tabMerging->setLayout(tabMergingLayout);
    
    
    
    
    // Set up parametrisation tab
    
    tabParametrisationLayout = new QVBoxLayout;
    
    tabParametrisationGroupBox1 = new QGroupBox(tr("Parametrise sources"), tabParametrisation);
    tabParametrisationGroupBox1->setObjectName("steps.doParameterise");
    tabParametrisationGroupBox1->setCheckable(true);
    tabParametrisationGroupBox1->setChecked(true);
    tabParametrisationForm1 = new QFormLayout;
    
    tabParametrisationButtonMaskOpt = new QCheckBox(tr("Optimise mask "), tabParametrisationGroupBox1);
    tabParametrisationButtonMaskOpt->setObjectName("parameters.optimiseMask");
    tabParametrisationButtonMaskOpt->setToolTip("Run mask optimisation algorithm to improve parametrisation");
    tabParametrisationButtonMaskOpt->setEnabled(true);
    tabParametrisationButtonMaskOpt->setChecked(true);
    tabParametrisationButtonBusyFunction = new QCheckBox(tr("Fit Busy Function "), tabParametrisationGroupBox1);
    tabParametrisationButtonBusyFunction->setObjectName("parameters.fitBusyFunction");
    tabParametrisationButtonBusyFunction->setToolTip("Parametrise integrated spectrum by fitting Busy Function");
    tabParametrisationButtonBusyFunction->setEnabled(true);
    tabParametrisationButtonBusyFunction->setChecked(true);
    
    tabParametrisationForm1->addRow(tr(""), tabParametrisationButtonMaskOpt);
    tabParametrisationForm1->addRow(tr(""), tabParametrisationButtonBusyFunction);
    tabParametrisationGroupBox1->setLayout(tabParametrisationForm1);
    
    tabParametrisationGroupBox2 = new QGroupBox(tr("Calculate reliability"), tabParametrisation);
    tabParametrisationGroupBox2->setObjectName("steps.doReliability");
    tabParametrisationGroupBox2->setCheckable(true);
    tabParametrisationGroupBox2->setChecked(true);
    tabParametrisationForm2 = new QFormLayout;
    
    tabParametrisationFieldRelMin = new QLineEdit(tabParametrisationGroupBox2);
    tabParametrisationFieldRelMin->setObjectName("reliability.relThresh");
    tabParametrisationFieldRelMin->setMaximumWidth(100);
    tabParametrisationFieldRelMin->setMaxLength(10);
    connect(tabParametrisationFieldRelMin, SIGNAL(editingFinished()), this, SLOT(updateFields()));
    tabParametrisationFieldRelMax = new QLineEdit(tabParametrisationGroupBox2);
    tabParametrisationFieldRelMax->setObjectName("reliability.relThreshMax");
    tabParametrisationFieldRelMax->setMaximumWidth(100);
    tabParametrisationFieldRelMax->setMaxLength(10);
    tabParametrisationFieldRelMax->setEnabled(false);
    tabParametrisationFieldRelMax->setText("1.0");
    QLabel *labelRel   = new QLabel(QString::fromUtf8("–"), tabParametrisationGroupBox2);
    
    tabParametrisationWidgetRel = new QWidget(tabParametrisationGroupBox2);
    tabParametrisationWidgetRel->setToolTip("Reliability cutoff for output catalogue");
    tabParametrisationLayoutRel = new QHBoxLayout;
    tabParametrisationLayoutRel->addWidget(tabParametrisationFieldRelMin);
    tabParametrisationLayoutRel->addWidget(labelRel);
    tabParametrisationLayoutRel->addWidget(tabParametrisationFieldRelMax);
    tabParametrisationLayoutRel->addStretch();
    tabParametrisationLayoutRel->setContentsMargins(0, 0, 0, 0);
    tabParametrisationWidgetRel->setLayout(tabParametrisationLayoutRel);
    
    tabParametrisationFieldRelKernel = new QLineEdit(tabParametrisationGroupBox2);
    tabParametrisationFieldRelKernel->setObjectName("reliability.kernel");
    
    tabParametrisationForm2->addRow(tr("Accepted range:"), tabParametrisationWidgetRel);
    tabParametrisationForm2->addRow(tr("Kernel:"), tabParametrisationFieldRelKernel);
    tabParametrisationGroupBox2->setLayout(tabParametrisationForm2);
    
    tabParametrisationButtonPrev = new QPushButton(tr("Previous"), tabParametrisation);
    tabParametrisationButtonPrev->setIcon(iconGoPreviousView);
    connect(tabParametrisationButtonPrev, SIGNAL(clicked()), this, SLOT(displayPrevTab()));
    tabParametrisationButtonNext = new QPushButton(tr("Next"), tabParametrisation);
    tabParametrisationButtonNext->setIcon(iconGoNextView);
    connect(tabParametrisationButtonNext, SIGNAL(clicked()), this, SLOT(displayNextTab()));
    tabParametrisationLayoutControls = new QHBoxLayout();
    tabParametrisationLayoutControls->setContentsMargins(0, 0, 0, 0);
    tabParametrisationLayoutControls->setSpacing(0);
    tabParametrisationLayoutControls->addWidget(tabParametrisationButtonPrev);
    tabParametrisationLayoutControls->addStretch();
    tabParametrisationLayoutControls->addWidget(tabParametrisationButtonNext);
    tabParametrisationWidgetControls = new QWidget(tabParametrisation);
    tabParametrisationWidgetControls->setLayout(tabParametrisationLayoutControls);
    
    tabParametrisationLayout->addWidget(tabParametrisationGroupBox1);
    tabParametrisationLayout->addWidget(tabParametrisationGroupBox2);
    tabParametrisationLayout->addStretch();
    tabParametrisationLayout->addWidget(tabParametrisationWidgetControls);
    tabParametrisation->setLayout(tabParametrisationLayout);
    
    
    
    // Set up output filter tab
    
    tabOutFilterLayout = new QVBoxLayout;
    
    tabOutFilterGroupBox1 = new QGroupBox(tr("Parameter range"), tabOutFilter);
    tabOutFilterGroupBox1->setEnabled(false);
    
    tabOutFilterForm1 = new QFormLayout;
    
    tabOutFilterFieldW50Min   = new QLineEdit(tabOutFilterGroupBox1);
    tabOutFilterFieldW50Min->setObjectName("widthW50Min");
    tabOutFilterFieldW50Max   = new QLineEdit(tabOutFilterGroupBox1);
    tabOutFilterFieldW50Max->setObjectName("widthW50Max");
    tabOutFilterFieldW20Min   = new QLineEdit(tabOutFilterGroupBox1);
    tabOutFilterFieldW20Min->setObjectName("widthW20Min");
    tabOutFilterFieldW20Max   = new QLineEdit(tabOutFilterGroupBox1);
    tabOutFilterFieldW20Max->setObjectName("widthW20Max");
    tabOutFilterFieldFpeakMin = new QLineEdit(tabOutFilterGroupBox1);
    tabOutFilterFieldFpeakMin->setObjectName("fpeakMin");
    tabOutFilterFieldFpeakMax = new QLineEdit(tabOutFilterGroupBox1);
    tabOutFilterFieldFpeakMax->setObjectName("fpeakMax");
    tabOutFilterFieldFintMin  = new QLineEdit(tabOutFilterGroupBox1);
    tabOutFilterFieldFintMin->setObjectName("fintegrMin");
    tabOutFilterFieldFintMax  = new QLineEdit(tabOutFilterGroupBox1);
    tabOutFilterFieldFintMax->setObjectName("fintegrMax");
    
    tabOutFilterFieldW50Min->setMaximumWidth(100);
    tabOutFilterFieldW50Max->setMaximumWidth(100);
    tabOutFilterFieldW20Min->setMaximumWidth(100);
    tabOutFilterFieldW20Max->setMaximumWidth(100);
    tabOutFilterFieldFpeakMin->setMaximumWidth(100);
    tabOutFilterFieldFpeakMax->setMaximumWidth(100);
    tabOutFilterFieldFintMin->setMaximumWidth(100);
    tabOutFilterFieldFintMax->setMaximumWidth(100);
    
    tabOutFilterFieldW50Min->setMaxLength(10);
    tabOutFilterFieldW50Max->setMaxLength(10);
    tabOutFilterFieldW20Min->setMaxLength(10);
    tabOutFilterFieldW20Max->setMaxLength(10);
    tabOutFilterFieldFpeakMin->setMaxLength(10);
    tabOutFilterFieldFpeakMax->setMaxLength(10);
    tabOutFilterFieldFintMin->setMaxLength(10);
    tabOutFilterFieldFintMax->setMaxLength(10);
    
    QLabel *labelW50   = new QLabel(QString::fromUtf8("–"), tabOutFilterGroupBox1);
    QLabel *labelW20   = new QLabel(QString::fromUtf8("–"), tabOutFilterGroupBox1);
    QLabel *labelFpeak = new QLabel(QString::fromUtf8("–"), tabOutFilterGroupBox1);
    QLabel *labelFint  = new QLabel(QString::fromUtf8("–"), tabOutFilterGroupBox1);
    
    tabOutFilterButtonW50   = new QCheckBox(tr("Apply "), tabOutFilterGroupBox1);
    tabOutFilterButtonW50->setObjectName("applyW50Filt");
    tabOutFilterButtonW50->setEnabled(true);
    connect(tabOutFilterButtonW50, SIGNAL(toggled(bool)), this, SLOT(updateFields()));
    tabOutFilterButtonW20   = new QCheckBox(tr("Apply "), tabOutFilterGroupBox1);
    tabOutFilterButtonW20->setObjectName("applyW20Filt");
    tabOutFilterButtonW20->setEnabled(true);
    connect(tabOutFilterButtonW20, SIGNAL(toggled(bool)), this, SLOT(updateFields()));
    tabOutFilterButtonFpeak = new QCheckBox(tr("Apply "), tabOutFilterGroupBox1);
    tabOutFilterButtonFpeak->setObjectName("applyFpeakFilt");
    tabOutFilterButtonFpeak->setEnabled(true);
    connect(tabOutFilterButtonFpeak, SIGNAL(toggled(bool)), this, SLOT(updateFields()));
    tabOutFilterButtonFint  = new QCheckBox(tr("Apply "), tabOutFilterGroupBox1);
    tabOutFilterButtonFint->setObjectName("applyFintFilt");
    tabOutFilterButtonFint->setEnabled(true);
    connect(tabOutFilterButtonFint, SIGNAL(toggled(bool)), this, SLOT(updateFields()));
    
    tabOutFilterWidgetW50 = new QWidget(tabOutFilterGroupBox1);
    tabOutFilterLayoutW50 = new QHBoxLayout;
    tabOutFilterLayoutW50->addWidget(tabOutFilterFieldW50Min);
    tabOutFilterLayoutW50->addWidget(labelW50);
    tabOutFilterLayoutW50->addWidget(tabOutFilterFieldW50Max);
    tabOutFilterLayoutW50->addWidget(tabOutFilterButtonW50);
    tabOutFilterLayoutW50->addStretch();
    tabOutFilterLayoutW50->setContentsMargins(0, 0, 0, 0);
    tabOutFilterWidgetW50->setLayout(tabOutFilterLayoutW50);
    
    tabOutFilterWidgetW20 = new QWidget(tabOutFilterGroupBox1);
    tabOutFilterLayoutW20 = new QHBoxLayout;
    tabOutFilterLayoutW20->addWidget(tabOutFilterFieldW20Min);
    tabOutFilterLayoutW20->addWidget(labelW20);
    tabOutFilterLayoutW20->addWidget(tabOutFilterFieldW20Max);
    tabOutFilterLayoutW20->addWidget(tabOutFilterButtonW20);
    tabOutFilterLayoutW20->addStretch();
    tabOutFilterLayoutW20->setContentsMargins(0, 0, 0, 0);
    tabOutFilterWidgetW20->setLayout(tabOutFilterLayoutW20);
    
    tabOutFilterWidgetFpeak = new QWidget(tabOutFilterGroupBox1);
    tabOutFilterLayoutFpeak = new QHBoxLayout;
    tabOutFilterLayoutFpeak->addWidget(tabOutFilterFieldFpeakMin);
    tabOutFilterLayoutFpeak->addWidget(labelFpeak);
    tabOutFilterLayoutFpeak->addWidget(tabOutFilterFieldFpeakMax);
    tabOutFilterLayoutFpeak->addWidget(tabOutFilterButtonFpeak);
    tabOutFilterLayoutFpeak->addStretch();
    tabOutFilterLayoutFpeak->setContentsMargins(0, 0, 0, 0);
    tabOutFilterWidgetFpeak->setLayout(tabOutFilterLayoutFpeak);
    
    tabOutFilterWidgetFint = new QWidget(tabOutFilterGroupBox1);
    tabOutFilterLayoutFint = new QHBoxLayout;
    tabOutFilterLayoutFint->addWidget(tabOutFilterFieldFintMin);
    tabOutFilterLayoutFint->addWidget(labelFint);
    tabOutFilterLayoutFint->addWidget(tabOutFilterFieldFintMax);
    tabOutFilterLayoutFint->addWidget(tabOutFilterButtonFint);
    tabOutFilterLayoutFint->addStretch();
    tabOutFilterLayoutFint->setContentsMargins(0, 0, 0, 0);
    tabOutFilterWidgetFint->setLayout(tabOutFilterLayoutFint);
    
    tabOutFilterForm1->addRow(tr("Line width (w50):"), tabOutFilterWidgetW50);
    tabOutFilterForm1->addRow(tr("Line width (w20):"), tabOutFilterWidgetW20);
    tabOutFilterForm1->addRow(tr("Peak flux:"), tabOutFilterWidgetFpeak);
    tabOutFilterForm1->addRow(tr("Integrated flux:"), tabOutFilterWidgetFint);
    
    tabOutFilterButtonPrev = new QPushButton(tr("Previous"), tabOutFilter);
    tabOutFilterButtonPrev->setIcon(iconGoPreviousView);
    connect(tabOutFilterButtonPrev, SIGNAL(clicked()), this, SLOT(displayPrevTab()));
    tabOutFilterButtonNext = new QPushButton(tr("Next"), tabOutFilter);
    tabOutFilterButtonNext->setIcon(iconGoNextView);
    connect(tabOutFilterButtonNext, SIGNAL(clicked()), this, SLOT(displayNextTab()));
    tabOutFilterLayoutControls = new QHBoxLayout();
    tabOutFilterLayoutControls->setContentsMargins(0, 0, 0, 0);
    tabOutFilterLayoutControls->setSpacing(0);
    tabOutFilterLayoutControls->addWidget(tabOutFilterButtonPrev);
    tabOutFilterLayoutControls->addStretch();
    tabOutFilterLayoutControls->addWidget(tabOutFilterButtonNext);
    tabOutFilterWidgetControls = new QWidget(tabOutFilter);
    tabOutFilterWidgetControls->setLayout(tabOutFilterLayoutControls);
    
    tabOutFilterGroupBox1->setLayout(tabOutFilterForm1);
    tabOutFilterLayout->addWidget(tabOutFilterGroupBox1);
    tabOutFilterLayout->addStretch();
    tabOutFilterLayout->addWidget(tabOutFilterWidgetControls);
    tabOutFilter->setLayout(tabOutFilterLayout);
    
    // Set up output tab
    
    tabOutputLayout = new QVBoxLayout;
    
    tabOutputGroupBox1 = new QGroupBox(tr("Output files"), tabOutput);
    
    tabOutputForm1 = new QFormLayout;
    
    tabOutputFieldBaseName = new QLineEdit(tabOutputGroupBox1);
    tabOutputFieldBaseName->setObjectName("writeCat.basename");
    tabOutputFieldBaseName->setToolTip("Specify base name to be used for all output. Defaults to input file name.");
    tabOutputFieldBaseName->setEnabled(false);
    tabOutputButtonASCII = new QCheckBox(tr("ASCII "), tabOutputGroupBox1);
    tabOutputButtonASCII->setObjectName("writeCat.writeASCII");
    tabOutputButtonASCII->setToolTip(tr("Human-readable ASCII format"));
    tabOutputButtonASCII->setChecked(true);
    tabOutputButtonASCII->setEnabled(true);
    //connect(tabOutputButtonASCII, SIGNAL(toggled(bool)), this, SLOT(updateFields()));
    tabOutputButtonXML = new QCheckBox(tr("VO table "), tabOutputGroupBox1);
    tabOutputButtonXML->setObjectName("writeCat.writeXML");
    tabOutputButtonXML->setChecked(false);
    tabOutputButtonXML->setEnabled(true);
    tabOutputButtonXML->setToolTip(tr("Virtual Observatory XML table"));
    //connect(tabOutputButtonXML, SIGNAL(toggled(bool)), this, SLOT(updateFields()));
    tabOutputButtonSQL = new QCheckBox(tr("SQL "), tabOutputGroupBox1);
    tabOutputButtonSQL->setObjectName("writeCat.writeSQL");
    tabOutputButtonSQL->setChecked(false);
    tabOutputButtonSQL->setEnabled(false);
    tabOutputButtonSQL->setToolTip(tr("Structured Query Language"));
    //connect(tabOutputButtonSQL, SIGNAL(toggled(bool)), this, SLOT(updateFields()));
    
    tabOutputWidgetFormat = new QWidget(tabOutputGroupBox1);
    tabOutputLayoutFormat = new QHBoxLayout();
    tabOutputLayoutFormat->setContentsMargins(0, 0, 0, 0);
    tabOutputLayoutFormat->setSpacing(10);
    tabOutputLayoutFormat->addWidget(tabOutputButtonASCII);
    tabOutputLayoutFormat->addWidget(tabOutputButtonXML);
    tabOutputLayoutFormat->addWidget(tabOutputButtonSQL);
    tabOutputLayoutFormat->addStretch();
    tabOutputWidgetFormat->setLayout(tabOutputLayoutFormat);
    
    tabOutputButtonMask = new QCheckBox(tr("Mask "), tabOutputGroupBox1);
    tabOutputButtonMask->setObjectName("steps.doWriteMask");
    tabOutputButtonMask->setToolTip(tr("Source mask cube"));
    tabOutputButtonMask->setChecked(false);
    tabOutputButtonMom0 = new QCheckBox(tr("Mom. 0 "), tabOutputGroupBox1);
    tabOutputButtonMom0->setObjectName("steps.doMom0");
    tabOutputButtonMom0->setChecked(false);
    tabOutputButtonMom0->setToolTip(tr("Integrated flux map"));
    tabOutputButtonMom1 = new QCheckBox(tr("Mom. 1 "), tabOutputGroupBox1);
    tabOutputButtonMom1->setObjectName("steps.doMom1");
    tabOutputButtonMom1->setChecked(false);
    tabOutputButtonMom1->setToolTip(tr("Velocity field map"));
    tabOutputButtonCubelets = new QCheckBox(tr("Cubelets "), tabOutputGroupBox1);
    tabOutputButtonCubelets->setObjectName("steps.doCubelets");
    tabOutputButtonCubelets->setChecked(false);
    tabOutputButtonCubelets->setToolTip(tr("Create cubelet for each source"));
    
    tabOutputWidgetProducts = new QWidget(tabOutputGroupBox1);
    tabOutputLayoutProducts = new QHBoxLayout();
    tabOutputLayoutProducts->setContentsMargins(0, 0, 0, 0);
    tabOutputLayoutProducts->setSpacing(10);
    tabOutputLayoutProducts->addWidget(tabOutputButtonMask);
    tabOutputLayoutProducts->addWidget(tabOutputButtonMom0);
    tabOutputLayoutProducts->addWidget(tabOutputButtonMom1);
    tabOutputLayoutProducts->addWidget(tabOutputButtonCubelets);
    tabOutputLayoutProducts->addStretch();
    tabOutputWidgetProducts->setLayout(tabOutputLayoutProducts);
    
    tabOutputForm1->addRow(tr("Base name:"), tabOutputFieldBaseName);
    tabOutputForm1->addRow(tr("Catalogue:"), tabOutputWidgetFormat);
    tabOutputForm1->addRow(tr("Data products:"), tabOutputWidgetProducts);
    
    tabOutputGroupBox2 = new QGroupBox(tr("Output parameters"), tabOutput);
    tabOutputGroupBox2->setEnabled(true);
    
    tabOutputForm2 = new QFormLayout;
    
    tabOutputButtonParameterID    = new QCheckBox(tr("Source ID "), tabOutputGroupBox2);
    tabOutputButtonParameterID->setObjectName("parameterID");
    tabOutputButtonParameterID->setChecked(false);
    tabOutputButtonParameterX     = new QCheckBox(tr("Position X "), tabOutputGroupBox2);
    tabOutputButtonParameterX->setObjectName("parameterX");
    tabOutputButtonParameterX->setChecked(false);
    tabOutputButtonParameterX->setToolTip(tr("Source position in pixels"));
    tabOutputButtonParameterY     = new QCheckBox(tr("Position Y "), tabOutputGroupBox2);
    tabOutputButtonParameterY->setObjectName("parameterY");
    tabOutputButtonParameterY->setChecked(false);
    tabOutputButtonParameterY->setToolTip(tr("Source position in pixels"));
    tabOutputButtonParameterZ     = new QCheckBox(tr("Position Z "), tabOutputGroupBox2);
    tabOutputButtonParameterZ->setObjectName("parameterZ");
    tabOutputButtonParameterZ->setChecked(false);
    tabOutputButtonParameterZ->setToolTip(tr("Source position in pixels"));
    tabOutputButtonParameterLon   = new QCheckBox(tr("Longitude "), tabOutputGroupBox2);
    tabOutputButtonParameterLon->setObjectName("parameterLon");
    tabOutputButtonParameterLon->setChecked(false);
    tabOutputButtonParameterLon->setToolTip(tr("Longitude in world coordinates (e.g. right ascension, Galactic longitude, etc.)"));
    tabOutputButtonParameterLat   = new QCheckBox(tr("Latitude "), tabOutputGroupBox2);
    tabOutputButtonParameterLat->setObjectName("parameterLat");
    tabOutputButtonParameterLat->setChecked(false);
    tabOutputButtonParameterLat->setToolTip(tr("Latitude in world coordinates (e.g. declination, Galactic latitude, etc.)"));
    tabOutputButtonParameterFreq  = new QCheckBox(tr("Frequency "), tabOutputGroupBox2);
    tabOutputButtonParameterFreq->setObjectName("parameterFreq");
    tabOutputButtonParameterFreq->setChecked(false);
    tabOutputButtonParameterVrad  = new QCheckBox(tr("Velocity "), tabOutputGroupBox2);
    tabOutputButtonParameterVrad->setObjectName("parameterVelo");
    tabOutputButtonParameterVrad->setChecked(false);
    tabOutputButtonParameterVrad->setToolTip(tr("Radial velocity"));
    tabOutputButtonParameterW50   = new QCheckBox(tr("Line width (w50) "), tabOutputGroupBox2);
    tabOutputButtonParameterW50->setObjectName("parameterW50");
    tabOutputButtonParameterW50->setChecked(false);
    tabOutputButtonParameterW20   = new QCheckBox(tr("Line width (w20) "), tabOutputGroupBox2);
    tabOutputButtonParameterW20->setObjectName("parameterW20");
    tabOutputButtonParameterW20->setChecked(false);
    tabOutputButtonParameterFpeak = new QCheckBox(tr("Peak flux "), tabOutputGroupBox2);
    tabOutputButtonParameterFpeak->setObjectName("parameterFpeak");
    tabOutputButtonParameterFpeak->setChecked(false);
    tabOutputButtonParameterFint  = new QCheckBox(tr("Integrated flux "), tabOutputGroupBox2);
    tabOutputButtonParameterFint->setObjectName("parameterFint");
    tabOutputButtonParameterFint->setChecked(false);
    tabOutputButtonParameterRel   = new QCheckBox(tr("Reliability "), tabOutputGroupBox2);
    tabOutputButtonParameterRel->setObjectName("parameterRel");
    tabOutputButtonParameterRel->setChecked(false);
    tabOutputButtonParameterRel->setToolTip(tr("Reliability from negative detections (if enabled)"));
    tabOutputButtonParameterFlags = new QCheckBox(tr("Quality flags "), tabOutputGroupBox2);
    tabOutputButtonParameterFlags->setObjectName("parameterFlags");
    tabOutputButtonParameterFlags->setChecked(false);
    tabOutputButtonParameterEllMaj = new QCheckBox(tr("Major axis "), tabOutputGroupBox2);
    tabOutputButtonParameterEllMaj->setObjectName("parameterEllMaj");
    tabOutputButtonParameterEllMaj->setChecked(false);
    tabOutputButtonParameterEllMaj->setToolTip(tr("Major axis of ellipse fitted to source"));
    tabOutputButtonParameterEllMin = new QCheckBox(tr("Minor axis "), tabOutputGroupBox2);
    tabOutputButtonParameterEllMin->setObjectName("parameterEllMin");
    tabOutputButtonParameterEllMin->setChecked(false);
    tabOutputButtonParameterEllMin->setToolTip(tr("Minor axis of ellipse fitted to source"));
    tabOutputButtonParameterEllPA = new QCheckBox(tr("Position angle "), tabOutputGroupBox2);
    tabOutputButtonParameterEllPA->setObjectName("parameterEllPA");
    tabOutputButtonParameterEllPA->setChecked(false);
    tabOutputButtonParameterEllPA->setToolTip(tr("Position angle of ellipse fitted to source"));
    tabOutputButtonParameterRms = new QCheckBox(tr("RMS noise "), tabOutputGroupBox2);
    tabOutputButtonParameterRms->setObjectName("parameterRms");
    tabOutputButtonParameterRms->setChecked(false);
    tabOutputButtonParameterRms->setToolTip(tr("Measured RMS noise of the data"));
    tabOutputButtonParameterBFPar = new QCheckBox(tr("BF fit solution "), tabOutputGroupBox2);
    tabOutputButtonParameterBFPar->setObjectName("parameterBFPar");
    tabOutputButtonParameterBFPar->setChecked(false);
    tabOutputButtonParameterBFPar->setToolTip(tr("Best-fit Busy Function parameters (if enabled)"));
    tabOutputButtonParameterBFPhys = new QCheckBox(tr("BF parametrisation "), tabOutputGroupBox2);
    tabOutputButtonParameterBFPhys->setObjectName("parameterBFPhys");
    tabOutputButtonParameterBFPhys->setChecked(false);
    tabOutputButtonParameterBFPhys->setToolTip(tr("Physical parameters derived from Busy Function fit (if enabled)"));
    tabOutputButtonParameterBFInfo = new QCheckBox(tr("BF information "), tabOutputGroupBox2);
    tabOutputButtonParameterBFInfo->setObjectName("parameterBFInfo");
    tabOutputButtonParameterBFInfo->setChecked(false);
    tabOutputButtonParameterBFInfo->setToolTip(tr("Additional information (chi-squared, flags, etc.) about Busy Function fit (if enabled)"));
    tabOutputButtonParameterAll = new QCheckBox(tr("All "), tabOutputGroupBox2);
    tabOutputButtonParameterAll->setObjectName("parameterAll");
    tabOutputButtonParameterAll->setChecked(true);
    tabOutputButtonParameterAll->setToolTip(tr("Select all available parameters"));
    connect(tabOutputButtonParameterAll, SIGNAL(toggled(bool)), this, SLOT(updateFields()));
    
    tabOutputWidgetParameters = new QWidget(tabOutputGroupBox2);
    tabOutputLayoutParameters = new QGridLayout();
    tabOutputLayoutParameters->setContentsMargins(0, 0, 0, 0);
    tabOutputLayoutParameters->setSpacing(5);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterID, 0, 0);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterX, 1, 0);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterY, 2, 0);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterZ, 3, 0);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterLon, 4, 0);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterLat, 5, 0);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterFreq, 0, 1);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterVrad, 1, 1);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterW50, 2, 1);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterW20, 3, 1);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterFpeak, 4, 1);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterFint, 5, 1);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterEllMaj, 0, 2);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterEllMin, 1, 2);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterEllPA, 2, 2);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterRel, 3, 2);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterFlags, 4, 2);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterRms, 5, 2);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterBFPar, 0, 3);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterBFPhys, 1, 3);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterBFInfo, 2, 3);
    tabOutputLayoutParameters->addWidget(tabOutputButtonParameterAll, 5, 3);
    tabOutputWidgetParameters->setLayout(tabOutputLayoutParameters);
    
    tabOutputForm2->addRow(tr(""), tabOutputWidgetParameters);
    
    tabOutputButtonPrev = new QPushButton(tr("Previous"), tabOutput);
    tabOutputButtonPrev->setIcon(iconGoPreviousView);
    connect(tabOutputButtonPrev, SIGNAL(clicked()), this, SLOT(displayPrevTab()));
    tabOutputButtonGo   = new QPushButton(tr("Run Pipeline"), tabOutput);
    tabOutputButtonGo->setIcon(iconDialogOkApply);
    connect(tabOutputButtonGo, SIGNAL(clicked()), this, SLOT(runPipeline()));
    tabOutputLayoutControls = new QHBoxLayout();
    tabOutputLayoutControls->setContentsMargins(0, 0, 0, 0);
    tabOutputLayoutControls->setSpacing(0);
    tabOutputLayoutControls->addWidget(tabOutputButtonPrev);
    tabOutputLayoutControls->addStretch();
    tabOutputLayoutControls->addWidget(tabOutputButtonGo);
    tabOutputWidgetControls = new QWidget(tabOutput);
    tabOutputWidgetControls->setLayout(tabOutputLayoutControls);
    
    tabOutputGroupBox1->setLayout(tabOutputForm1);
    tabOutputGroupBox2->setLayout(tabOutputForm2);
    tabOutputLayout->addWidget(tabOutputGroupBox2);
    tabOutputLayout->addWidget(tabOutputGroupBox1);
    tabOutputLayout->addStretch();
    tabOutputLayout->addWidget(tabOutputWidgetControls);
    tabOutput->setLayout(tabOutputLayout);
    
    // Set up output widget
    
    widgetOutput = new QWidget(widgetMain);
    
    outputText = new QTextEdit(widgetOutput);
    //outputText->setToolTip(tr("Pipeline messages"));
    outputText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    outputText->setReadOnly(true);
    outputText->setLineWrapMode(QTextEdit::FixedColumnWidth);
    outputText->setLineWrapColumnOrWidth(80);
    outputText->setTabStopWidth(8 * outputText->fontMetrics().width("0"));     // setTabStopWidth() expects pixels!!!
    
    QFont font = QFont("Courier");
    font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
    font.setPointSize(10);
    font.setFixedPitch(true);
    font.setKerning(false);
    outputText->setFont(font);
    
    QFontMetrics fontMetrics(font);
    outputText->setMinimumSize(85 * fontMetrics.width(QChar('0')), 10 * fontMetrics.height());
    
    outputProgress = new QProgressBar(widgetOutput);
    //outputProgress->setToolTip(tr("Pipeline progress"));
    outputProgress->setMinimum(0);
    outputProgress->setMaximum(100);
    outputProgress->setValue(0);
    
    outputLayout = new QVBoxLayout();
    outputLayout->addWidget(outputText);
    outputLayout->addWidget(outputProgress);
    outputLayout->setContentsMargins(0, 0, 0, 0);
    outputLayout->setSpacing(5);
    
    widgetOutput->setLayout(outputLayout);
    widgetOutput->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    
    // Assemble main widget
    
    layoutMain = new QVBoxLayout();
    layoutMain->addWidget(tabs);
    layoutMain->setContentsMargins(5, 5, 5, 0);
    layoutMain->setSpacing(5);
    
    widgetMain->setLayout(layoutMain);
    
    dockWidgetOutput = new QDockWidget(tr("Pipeline Messages"), this);
    dockWidgetOutput->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockWidgetOutput->setWidget(widgetOutput);
    dockWidgetOutput->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dockWidgetOutput->setContentsMargins(5, 5, 5, 5);
    dockWidgetOutput->toggleViewAction()->setText(tr("Show Pipeline Messages"));
    this->addDockWidget(Qt::TopDockWidgetArea, dockWidgetOutput);
    
    // Set up actions
    
    actionOpen = new QAction(tr("Open..."), this);
    actionOpen->setShortcuts(QKeySequence::Open);
    actionOpen->setIcon(iconDocumentOpen);
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(loadSettings()));
    
    actionSave = new QAction(tr("Save"), this);
    actionSave->setShortcuts(QKeySequence::Save);
    actionSave->setIcon(iconDocumentSave);
    connect(actionSave, SIGNAL(triggered()), this, SLOT(saveSettings()));
    
    actionSaveAs = new QAction(tr("Save As..."), this);
    actionSaveAs->setShortcuts(QKeySequence::SaveAs);
    actionSaveAs->setIcon(iconDocumentSaveAs);
    connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(saveSettingsAs()));
    
    actionExit = new QAction(tr("Quit"), this);
    actionExit->setShortcuts(QKeySequence::Quit);
    actionExit->setIcon(iconApplicationExit);
    connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));
    
    actionRun = new QAction(tr("Run Pipeline"), this);
    actionRun->setShortcut(Qt::Key_F2);
    actionRun->setIcon(iconDialogOkApply);
    connect(actionRun, SIGNAL(triggered()), this, SLOT(runPipeline()));
    
    actionAbort = new QAction(tr("Abort Pipeline"), this);
    actionAbort->setShortcut(Qt::Key_Escape);
    actionAbort->setIcon(iconDialogClose);
    connect(actionAbort, SIGNAL(triggered()), this, SLOT(pipelineProcessCancel()));
    
    actionDefault = new QAction(tr("New"), this);
    actionDefault->setShortcuts(QKeySequence::New);
    actionDefault->setIcon(iconDocumentNew);
    connect(actionDefault, SIGNAL(triggered()), this, SLOT(resetToDefault()));
    
    actionSaveLogAs = new QAction(tr("Save Messages As..."), this);
    actionSaveLogAs->setEnabled(false);
    actionSaveLogAs->setIcon(iconDocumentSaveAs);
    connect(actionSaveLogAs, SIGNAL(triggered()), this, SLOT(saveLogAs()));
    
    actionClearLog = new QAction(tr("Clear Messages"), this);
    actionClearLog->setEnabled(false);
    actionClearLog->setIcon(iconEditClearList);
    connect(actionClearLog, SIGNAL(triggered()), this, SLOT(clearLog()));
    
    actionShowCatalogue = new QAction(tr("View Catalogue"), this);
    actionShowCatalogue->setEnabled(false);
    actionShowCatalogue->setIcon(iconDocumentPreview);
    connect(actionShowCatalogue, SIGNAL(triggered()), this, SLOT(showCatalogue()));
    
    actionHelp = new QAction(tr("SoFiA Handbook"), this);
    actionHelp->setShortcut(Qt::Key_F1);
    actionHelp->setIcon(iconHelpContents);
    connect(actionHelp, SIGNAL(triggered()), this, SLOT(showHandbook()));
    
    actionAbout = new QAction(tr("About SoFiA"), this);
    actionAbout->setIcon(iconHelpAbout);
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(aboutSoFiA()));
    
    // Set up toolbar
    
    toolBar = new QToolBar(tr("Toolbar"), this);
    toolBar->addAction(actionDefault);
    toolBar->addAction(actionOpen);
    toolBar->addAction(actionSave);
    toolBar->addSeparator();
    toolBar->addAction(actionRun);
    toolBar->addAction(actionAbort);
    toolBar->addAction(actionClearLog);
    
    toolBar->setIconSize(QSize(22, 22));
    toolBar->setMovable(false);
    toolBar->setAllowedAreas(Qt::AllToolBarAreas);
    toolBar->toggleViewAction()->setText(tr("Show Toolbar"));
    
    // Set up menu
    
    menuFile = new QMenu(tr("&File"), this);
    menuFile->addAction(actionDefault);
    menuFile->addSeparator();
    menuFile->addAction(actionOpen);
    menuFile->addSeparator();
    menuFile->addAction(actionSave);
    menuFile->addAction(actionSaveAs);
    menuFile->addSeparator();
    menuFile->addAction(actionExit);
    
    menuPipeline = new QMenu(tr("&Pipeline"), this);
    menuPipeline->addAction(actionRun);
    menuPipeline->addAction(actionAbort);
    menuPipeline->addSeparator();
    menuPipeline->addAction(actionSaveLogAs);
    menuPipeline->addAction(actionClearLog);
    
    menuView = new QMenu(tr("&Analysis"), this);
    menuView->addAction(actionShowCatalogue);
    
    menuSettings = new QMenu(tr("&Settings"), this);
    menuSettings->addAction(dockWidgetOutput->toggleViewAction());
    menuSettings->addAction(toolBar->toggleViewAction());
    
    menuHelp = new QMenu(tr("&Help"), this);
    menuHelp->addAction(actionHelp);
    menuHelp->addAction(actionAbout);
    
    this->menuBar()->addMenu(menuFile);
    this->menuBar()->addMenu(menuPipeline);
    this->menuBar()->addMenu(menuView);
    this->menuBar()->addMenu(menuSettings);
    this->menuBar()->addMenu(menuHelp);
    
    // Set up status bar
    
    this->statusBar()->setSizeGripEnabled(false);
    this->statusBar()->showMessage("");
    
    // Set up main window
    
    this->addToolBar(Qt::TopToolBarArea, toolBar);
    this->setWindowTitle(tr("SoFiA"));
    this->setCentralWidget(widgetMain);
    this->resize(600, 300);
    this->setWindowIcon(iconSoFiA);
    
    return;
}



// ------------------------------
// Support drag and drop of files
// ------------------------------

void SoFiA::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
    return;
}

void SoFiA::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
    return;
}

void SoFiA::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    
    // Check for single file:
    if(mimeData->hasUrls())
    {
        QString path;
        QList<QUrl> urlList = mimeData->urls();
        
        if(urlList.size() == 1)
        {
            path.append(urlList.at(0).toLocalFile());
            
            this->loadFile(path);
        }
    }
    
    return;
}




// --------------------------------
// Reimplementation of closeEvent()
// --------------------------------

void SoFiA::closeEvent(QCloseEvent *event)
{
    if(pipelineProcess->state() == QProcess::NotRunning)
    {
        event->accept();
    }
    else
    {
        event->ignore();
        
        QString messageText = tr("<p>The pipeline is still running!</p><p>If you wish to exit from SoFiA, you will need to either manually abort the pipeline or wait until the pipeline run has finished.</p>");
        QString statusText = tr("Pipeline still running.");
        showMessage(1, messageText, statusText);
    }
    
    return;
}
