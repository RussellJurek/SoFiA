/// ____________________________________________________________________ ///
///                                                                      ///
/// SoFiA 0.3 (SoFiA.h) - Source Finding Application                     ///
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

#ifndef PIPELINEINTERFACE_H
#define PIPELINEINTERFACE_H

#define SOFIA_DEFAULT_SETTINGS ":/SoFiA_default_input.txt"
#define SOFIA_TEMP_FILE "SoFiA_parameters.tmp"

#include <iostream>

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtGui/QDockWidget>
#include <QtGui/QStyle>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>
#include <QtGui/QAction>
#include <QtGui/QLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QProgressBar>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QList>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QMimeData>

#include "WidgetSpreadsheet.h"

class SoFiA : public QMainWindow
{
    Q_OBJECT
    
public:
    SoFiA(int argc, char *argv[]);
    ~SoFiA();
    
private slots:
    void selectInputDataFile();
    void loadSettings();
    void saveSettings();
    void saveSettingsAs();
    void saveLogAs();
    void clearLog();
    void resetToDefault();
    void showHandbook(const QString &page = QString("index.html"));
    void aboutSoFiA();
    void selectInputWeightsFile();
    void selectInputMaskFile();
    void displayPrevTab();
    void displayNextTab();
    void updateFields();
    void runPipeline();
    void pipelineProcessReadStd();
    void pipelineProcessReadErr();
    void pipelineProcessStarted();
    void pipelineProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void pipelineProcessCancel();
    void pipelineProcessError(QProcess::ProcessError error);
    void showCatalogue();
    
private:
    QString currentFileName;
    
    //QMap<QWidget*, QString> parameter;
    QMap<QString, QString> parameters;
    
    QProcess   *pipelineProcess;
    
    WidgetSpreadsheet *spreadsheet;
    
    int  selectFile(QLineEdit *target);
    int  loadFile(QString &fileName);
    int  updateVariables();
    int  setFields();
    int  setDefaults();
    int  showMessage(int severity, QString &messageText, QString &statusText);
    void createInterface();
    void updateActions();
    
    QMenu      *menuFile;
    QMenu      *menuPipeline;
    QMenu      *menuView;
    QMenu      *menuSettings;
    QMenu      *menuHelp;
    
    QToolBar   *toolBar;
    
    QAction    *actionOpen;
    QAction    *actionSave;
    QAction    *actionSaveAs;
    QAction    *actionExit;
    QAction    *actionRun;
    QAction    *actionAbort;
    QAction    *actionDefault;
    QAction    *actionSaveLogAs;
    QAction    *actionClearLog;
    QAction    *actionShowCatalogue;
    QAction    *actionHelp;
    QAction    *actionAbout;
    
    QWidget     *widgetMain;
    QVBoxLayout *layoutMain;
    
    QTabWidget *tabs;
    QWidget    *tabInput;
    QWidget    *tabInFilter;
    QWidget    *tabSourceFinding;
    QWidget    *tabMerging;
    QWidget    *tabParametrisation;
    QWidget    *tabOutFilter;
    QWidget    *tabOutput;
    
    
    
    // Input tab
    
    QVBoxLayout  *tabInputLayout;
    QHBoxLayout  *tabInputLayoutData;
    QWidget      *tabInputWidgetData;
    QHBoxLayout  *tabInputLayoutWeights;
    QWidget      *tabInputWidgetWeights;
    QHBoxLayout  *tabInputLayoutMask;
    QWidget      *tabInputWidgetMask;
    
    QGroupBox    *tabInputGroupBox1;
    QFormLayout  *tabInputForm1;
    QLineEdit    *tabInputFieldData;
    QPushButton  *tabInputButtonData;
    QLineEdit    *tabInputFieldWeights;
    QPushButton  *tabInputButtonWeights;
    QLineEdit    *tabInputFieldMask;
    QPushButton  *tabInputButtonMask;
    
    QGroupBox    *tabInputGroupBox2;
    QFormLayout  *tabInputForm2;
    QLineEdit    *tabInputFieldWeightsFunction;
    
    QGroupBox    *tabInputGroupBox3;
    QFormLayout  *tabInputForm3;
    QLineEdit    *tabInputFieldFlags;
    
    QHBoxLayout  *tabInputLayoutControls;
    QWidget      *tabInputWidgetControls;
    QPushButton  *tabInputButtonNext;
    
    
    
    // Input filter tab
    
    QVBoxLayout  *tabInFilterLayout;
    
    QWidget      *tabInFilterWidget2;
    QHBoxLayout  *tabInFilterLayout2;
    
    QGroupBox    *tabInFilterGroupBox1;
    QFormLayout  *tabInFilterForm1;
    QLineEdit    *tabInFilterFieldSmoothingSpatialLon;
    QLineEdit    *tabInFilterFieldSmoothingSpatialLat;
    QLineEdit    *tabInFilterFieldSmoothingSpectral;
    QComboBox    *tabInFilterFieldKernel;
    QComboBox    *tabInFilterFieldBorder;
    
    QGroupBox    *tabInFilterGroupBox2;
    QFormLayout  *tabInFilterForm2;
    QHBoxLayout  *tabInFilterLayoutSigma;
    QWidget      *tabInFilterWidgetSigma;
    QSpinBox     *tabInFilterFieldEdgeX;
    QSpinBox     *tabInFilterFieldEdgeY;
    QSpinBox     *tabInFilterFieldEdgeZ;
    QComboBox    *tabInFilterFieldStatistic;
    
    QHBoxLayout  *tabInFilterLayoutControls;
    QWidget      *tabInFilterWidgetControls;
    QPushButton  *tabInFilterButtonPrev;
    QPushButton  *tabInFilterButtonNext;
    
    
    
    // Source finding tab
    
    QVBoxLayout  *tabSourceFindingLayout;
    
    QGroupBox    *tabSourceFindingGroupBox1;
    QWidget      *tabSourceFindingWidget1Left;
    QWidget      *tabSourceFindingWidget1Right;
    QFormLayout  *tabSourceFindingForm1Left;
    QFormLayout  *tabSourceFindingForm1Right;
    QHBoxLayout  *tabSourceFindingForm1Layout;
    QLineEdit    *tabSourceFindingFieldThreshold;
    QComboBox    *tabSourceFindingFieldEdgeMode;
    QComboBox    *tabSourceFindingFieldRmsMode;
    QComboBox    *tabSourceFindingFieldKunit;
    QTextEdit    *tabSourceFindingFieldKernels;
    
    QGroupBox    *tabSourceFindingGroupBox2;
    QFormLayout  *tabSourceFindingForm2;
    QLineEdit    *tabSourceFindingFieldThreshold2;
    QComboBox    *tabSourceFindingFieldRmsMode2;
    QComboBox    *tabSourceFindingFieldClipMethod;
    
    QHBoxLayout  *tabSourceFindingLayoutControls;
    QWidget      *tabSourceFindingWidgetControls;
    QPushButton  *tabSourceFindingButtonPrev;
    QPushButton  *tabSourceFindingButtonNext;
    
    
    
    // Merging tab
    
    QVBoxLayout  *tabMergingLayout;
    
    QGroupBox    *tabMergingGroupBox1;
    QFormLayout  *tabMergingForm1;
    
    QSpinBox     *tabMergingFieldMergeX;
    QSpinBox     *tabMergingFieldMergeY;
    QSpinBox     *tabMergingFieldMergeZ;
    QSpinBox     *tabMergingFieldMinSizeX;
    QSpinBox     *tabMergingFieldMinSizeY;
    QSpinBox     *tabMergingFieldMinSizeZ;
    
    QHBoxLayout  *tabMergingLayoutControls;
    QWidget      *tabMergingWidgetControls;
    QPushButton  *tabMergingButtonPrev;
    QPushButton  *tabMergingButtonNext;
    
    
    
    // Parametrisation tab
    
    QVBoxLayout  *tabParametrisationLayout;
    
    QGroupBox    *tabParametrisationGroupBox1;
    QFormLayout  *tabParametrisationForm1;
    QCheckBox    *tabParametrisationButtonBusyFunction;
    QCheckBox    *tabParametrisationButtonMaskOpt;
    
    QGroupBox    *tabParametrisationGroupBox2;
    QFormLayout  *tabParametrisationForm2;
    QLineEdit    *tabParametrisationFieldRelMin;
    QLineEdit    *tabParametrisationFieldRelMax;
    QLineEdit    *tabParametrisationFieldRelKernel;
    QHBoxLayout  *tabParametrisationLayoutRel;
    QWidget      *tabParametrisationWidgetRel;
    
    QHBoxLayout  *tabParametrisationLayoutControls;
    QWidget      *tabParametrisationWidgetControls;
    QPushButton  *tabParametrisationButtonPrev;
    QPushButton  *tabParametrisationButtonNext;
    
    
    
    // Output filter tab
    
    QVBoxLayout  *tabOutFilterLayout;
    
    QGroupBox    *tabOutFilterGroupBox1;
    QFormLayout  *tabOutFilterForm1;
    QHBoxLayout  *tabOutFilterLayoutW50;
    QWidget      *tabOutFilterWidgetW50;
    QHBoxLayout  *tabOutFilterLayoutW20;
    QWidget      *tabOutFilterWidgetW20;
    QHBoxLayout  *tabOutFilterLayoutFpeak;
    QWidget      *tabOutFilterWidgetFpeak;
    QHBoxLayout  *tabOutFilterLayoutFint;
    QWidget      *tabOutFilterWidgetFint;
    QLineEdit    *tabOutFilterFieldW50Min;
    QLineEdit    *tabOutFilterFieldW50Max;
    QLineEdit    *tabOutFilterFieldW20Min;
    QLineEdit    *tabOutFilterFieldW20Max;
    QLineEdit    *tabOutFilterFieldFpeakMin;
    QLineEdit    *tabOutFilterFieldFpeakMax;
    QLineEdit    *tabOutFilterFieldFintMin;
    QLineEdit    *tabOutFilterFieldFintMax;
    QCheckBox    *tabOutFilterButtonW50;
    QCheckBox    *tabOutFilterButtonW20;
    QCheckBox    *tabOutFilterButtonFpeak;
    QCheckBox    *tabOutFilterButtonFint;
    
    QHBoxLayout  *tabOutFilterLayoutControls;
    QWidget      *tabOutFilterWidgetControls;
    QPushButton  *tabOutFilterButtonPrev;
    QPushButton  *tabOutFilterButtonNext;
    
    
    
    // Output tab
    
    QVBoxLayout  *tabOutputLayout;
    
    QGroupBox    *tabOutputGroupBox1;
    QFormLayout  *tabOutputForm1;
    
    QHBoxLayout  *tabOutputLayoutProducts;
    QWidget      *tabOutputWidgetProducts;
    QCheckBox    *tabOutputButtonMask;
    QCheckBox    *tabOutputButtonMom0;
    QCheckBox    *tabOutputButtonMom1;
    QCheckBox    *tabOutputButtonCubelets;
    
    QHBoxLayout  *tabOutputLayoutFormat;
    QWidget      *tabOutputWidgetFormat;
    QLineEdit    *tabOutputFieldBaseName;
    QCheckBox    *tabOutputButtonASCII;
    QCheckBox    *tabOutputButtonXML;
    QCheckBox    *tabOutputButtonSQL;
    
    QGroupBox    *tabOutputGroupBox2;
    QFormLayout  *tabOutputForm2;
    QGridLayout  *tabOutputLayoutParameters;
    QWidget      *tabOutputWidgetParameters;
    QCheckBox    *tabOutputButtonParameterID;
    QCheckBox    *tabOutputButtonParameterX;
    QCheckBox    *tabOutputButtonParameterY;
    QCheckBox    *tabOutputButtonParameterZ;
    QCheckBox    *tabOutputButtonParameterLon;
    QCheckBox    *tabOutputButtonParameterLat;
    QCheckBox    *tabOutputButtonParameterFreq;
    QCheckBox    *tabOutputButtonParameterVrad;
    QCheckBox    *tabOutputButtonParameterW50;
    QCheckBox    *tabOutputButtonParameterW20;
    QCheckBox    *tabOutputButtonParameterFpeak;
    QCheckBox    *tabOutputButtonParameterFint;
    QCheckBox    *tabOutputButtonParameterEllMaj;
    QCheckBox    *tabOutputButtonParameterEllMin;
    QCheckBox    *tabOutputButtonParameterEllPA;
    QCheckBox    *tabOutputButtonParameterRel;
    QCheckBox    *tabOutputButtonParameterFlags;
    QCheckBox    *tabOutputButtonParameterRms;
    QCheckBox    *tabOutputButtonParameterBFInfo;
    QCheckBox    *tabOutputButtonParameterBFPar;
    QCheckBox    *tabOutputButtonParameterBFPhys;
    QCheckBox    *tabOutputButtonParameterAll;
    
    QHBoxLayout  *tabOutputLayoutControls;
    QWidget      *tabOutputWidgetControls;
    QPushButton  *tabOutputButtonPrev;
    QPushButton  *tabOutputButtonGo;
    
    
    
    // Pipeline output terminal
    
    QDockWidget  *dockWidgetOutput;
    QWidget      *widgetOutput;
    QTextEdit    *outputText;
    QProgressBar *outputProgress;
    QVBoxLayout  *outputLayout;
    
protected:
    void closeEvent(QCloseEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
};

#endif
