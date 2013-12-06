// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "sceneview_optimization.h"

#include "util/constants.h"
#include "util/global.h"

#include "gui/common.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "examplesdialog.h"
#include "pythonlab/pythonengine_agros.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutionstore.h"

#include "ctemplate/template.h"

#include "hermes2d.h"

OptimizationControl::OptimizationControl(QWidget *parent) : QWidget(parent)
{
    createActions();
    createControls();

//    updateControls();

    //setMinimumSize(sizeHint());
}

void OptimizationControl::createActions()
{
    actOptimization = new QAction(icon("document-properties"), tr("Optimization"), this);
    actOptimization->setShortcut(tr("Ctrl+9"));
    actOptimization->setCheckable(true);
}

void OptimizationControl::createControls()
{

//    // general
    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutGeneral->setColumnStretch(1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Optimization:")), 0, 0);
//    layoutGeneral->addWidget(cmbCoordinateType, 0, 1);
//    layoutGeneral->addWidget(new QLabel(tr("Mesh type:")), 1, 0);
//    layoutGeneral->addWidget(cmbMeshType, 1, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);


    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->setContentsMargins(0, 0, 0, 0);
    layoutArea->addWidget(grpGeneral);
//    layoutArea->addWidget(new QLabel(tr("Optimization:")), 0, 0);
    layoutArea->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutArea);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    QVBoxLayout *layout= new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(widgetArea);

    setLayout(layout);
  }

void OptimizationControl::fillComboBox()
{
}

void OptimizationControl::updateControls()
{
}


//*****************************************************************************************************************

void OptimizationJavaScriptBridge::call(QString number)
{
    m_optimizationWidget->m_number = number;
    m_optimizationWidget->runActualVariant();
    m_optimizationWidget->refresh();
}

const int numParameters = 8;
const int numFunctionals = 2;

void OptimizationWidget::loadResults()
{
    m_parametersList.clear();
    m_resultsList.clear();
    FILE *f = fopen("/home/pkus/sources/data/optimization/data/population-70.txt", "r");
    while(!feof(f))
    {
        QList<double> row;
        double item;
        for(int i = 0; i < numParameters; i++)
        {
            fscanf(f, "%lf", &item);
            row.push_back(item);
        }
        m_parametersList.push_back(row);
        qDebug() << row;
        row.clear();
        for(int i = 0; i < numFunctionals; i++)
        {
            fscanf(f, "%lf", &item);
            row.push_back(item);
        }
        qDebug() << row;
        m_resultsList.push_back(row);
    }
    m_parametersList.pop_back();
    m_resultsList.pop_back();
}

void OptimizationWidget::runActualVariant()
{
    int num = m_number.toInt();
    assert(num >= 0 && num < m_parametersList.size());
    assert(m_parametersList.size() == m_resultsList.size());

    //QString str = QString("import unittest as ut; agros2d_suite = ut.TestSuite(); import %1; agros2d_suite.addTest(ut.TestLoader().loadTestsFromTestCase(%1.%2)); agros2d_result = test_suite.scenario.Agros2DTestResult(); agros2d_suite.run(agros2d_result); agros2d_result_report = agros2d_result.report()").
    //        arg(module).arg(cls);

    QList<double> parameterList = m_parametersList[num];
    QString parameters("[");
    for(int i = 0; i < numParameters; i++)
        parameters += QString("%1,").arg(parameterList[i]);
    parameters += "]";

    QString fileName("/home/pkus/sources/data/optimization/laser.py");

    QFile file("/home/pkus/sources/data/optimization/laser.py");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&file);
    QString fileText = in.readAll();


    QString command = QString("create_problem(%1)").arg(parameters);
    fileText.append(command);
    currentPythonEngine()->runScript(fileText, fileName);

}

OptimizationWidget::OptimizationWidget(SceneViewPreprocessor *sceneView, QWidget *parent)
    : QWidget(parent), m_recentProblemFiles(NULL), m_recentScriptFiles(NULL)
{
    this->m_sceneViewGeometry = sceneView;

    // problem information
    webView = new QWebView();
    webView->page()->setNetworkAccessManager(new QNetworkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    webView->setMinimumSize(200, 200);

    m_bridge = new OptimizationJavaScriptBridge(this);
    connect(webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));

    //connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize()).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_common.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(2, 2, 2, 2);
    layoutMain->addWidget(webView);

    setLayout(layoutMain);

    loadResults();

    refresh();
}

void OptimizationWidget::javaScriptWindowObjectCleared() {
    webView->page()->mainFrame()->addToJavaScriptWindowObject("ehbridge", m_bridge);
}

OptimizationWidget::~OptimizationWidget()
{
    QFile::remove(tempProblemDir() + "/info.html");
}

void OptimizationWidget::refresh()
{
    show();
}


void OptimizationWidget::show()
{
    if (currentPythonEngine()->isScriptRunning() || Agros2D::problem()->isSolving())
        return;

    // template
    std::string info;
    ctemplate::TemplateDictionary problemInfo("info");

    problemInfo.SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());

    problemInfo.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    problemInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());

    problemInfo.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

    problemInfo.SetValue("NAME_LABEL", tr("Name:").toStdString());
    problemInfo.SetValue("NAME", QFileInfo(Agros2D::problem()->config()->fileName()).baseName().toStdString());


    problemInfo.SetValue("GEOMETRY_SVG", generateSvgGeometry(Agros2D::scene()->edges->items(), 350).toStdString());

    QString optimizationData = "[";
    foreach(QList<double> row, m_resultsList)
    {
        optimizationData += QString("[%1,%2], ")
                .arg(row[0])
                .arg(row[1]);
    }
    optimizationData += "]";
    problemInfo.SetValue("OPTIMIZATION_DATA", optimizationData.toStdString());

    problemInfo.SetValue("POINT_NUMBER", m_number.toStdString());

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/optimization.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &problemInfo, &info);

    // setHtml(...) doesn't work
    // webView->setHtml(QString::fromStdString(info));

    // load(...) works
    writeStringContent(tempProblemDir() + "/optimization.html", QString::fromStdString(info));
    webView->load(QUrl::fromLocalFile(tempProblemDir() + "/optimization.html"));

}

