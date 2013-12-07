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

    sliderGeneration = new QSlider(Qt::Horizontal);
    sliderGeneration->setTickPosition(QSlider::TicksBelow);
    sliderGeneration->setMinimum(0);
    sliderGeneration->setMaximum(70);
    //connect(sliderTransientAnimate, SIGNAL(valueChanged(int)), this, SLOT(setTransientStep(int)));
    connect(sliderGeneration, SIGNAL(valueChanged(int)), this, SIGNAL(sliderMoved(int)));

    layoutGeneral->addWidget(sliderGeneration, 1, 0, 1, 3);
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

void OptimizationJavaScriptBridge::call(QString number, QString type)
{

    m_optimizationWidget->setActiveNumber(number, type);
    m_optimizationWidget->runActualVariant();
    m_optimizationWidget->refresh();
}

const int numParameters = 8;
const int numFunctionals = 2;

void OptimizationWidget::setActiveNumber(QString number, QString variant)
{
    int num = number.toInt();
    assert(num >= 0 && num < m_parametersList.size());
    assert(m_parametersList.size() == m_resultsList.size());
    if(variant == "front")
        m_activeNumber = m_front.at(num);
    else if(variant == "not_front")
        m_activeNumber = m_notFront.at(num);
    else
        assert(0);

}

void OptimizationWidget::generationChanged(int generation)
{
    m_activeGeneration = generation;
    refresh();
}

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
        row.clear();
        for(int i = 0; i < numFunctionals; i++)
        {
            fscanf(f, "%lf", &item);
            row.push_back(item);
        }
        m_resultsList.push_back(row);
    }
    m_parametersList.pop_back();
    m_resultsList.pop_back();

    for(int i = 0; i < m_resultsList.size(); i++)
    {
        bool front = true;
        for(int j = 0; j < m_resultsList.size(); j++)
        {
            if((m_resultsList[i][0] < m_resultsList[j][0]) && (m_resultsList[i][1] > m_resultsList[j][1]))
            {
                front = false;
                break;
            }
        }
        if(front)
            m_front.push_back(i);
        else
            m_notFront.push_back(i);
    }
    qDebug() << m_front;
    qDebug() << m_notFront;
}

void OptimizationWidget::runActualVariant()
{

    //QString str = QString("import unittest as ut; agros2d_suite = ut.TestSuite(); import %1; agros2d_suite.addTest(ut.TestLoader().loadTestsFromTestCase(%1.%2)); agros2d_result = test_suite.scenario.Agros2DTestResult(); agros2d_suite.run(agros2d_result); agros2d_result_report = agros2d_result.report()").
    //        arg(module).arg(cls);

    QList<double> parameterList = m_parametersList[m_activeNumber];
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
    ctemplate::TemplateDictionary templateValues("info");

    templateValues.SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());

    templateValues.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    templateValues.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());

    templateValues.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

    templateValues.SetValue("NAME_LABEL", tr("Name:").toStdString());
    templateValues.SetValue("NAME", QFileInfo(Agros2D::problem()->config()->fileName()).baseName().toStdString());


    templateValues.SetValue("GEOMETRY_SVG", generateSvgGeometry(Agros2D::scene()->edges->items(), 350).toStdString());

    if(m_activeNumber < m_resultsList.size())
    {
        templateValues.SetValue("FUNC1", QString::number(m_resultsList[m_activeNumber][0]).toStdString());
        templateValues.SetValue("FUNC2", QString::number(m_resultsList[m_activeNumber][1]).toStdString());
    }

    QString optimizationDataFront = "[";
    foreach(int index, m_front)
    {
        QList<double> row = m_resultsList[index];
        optimizationDataFront += QString("[%1,%2], ")
                .arg(row[0])
                .arg(row[1]);
    }
    optimizationDataFront += "]";
    templateValues.SetValue("OPTIMIZATION_DATA_FRONT", optimizationDataFront.toStdString());
    QString optimizationDataNotFront = "[";
    foreach(int index, m_notFront)
    {
        QList<double> row = m_resultsList[index];
        optimizationDataNotFront += QString("[%1,%2], ")
                .arg(row[0])
                .arg(row[1]);
    }
    optimizationDataNotFront += "]";
    templateValues.SetValue("OPTIMIZATION_DATA_NOT_FRONT", optimizationDataNotFront.toStdString());

    templateValues.SetValue("POINT_NUMBER", QString::number(m_activeNumber).toStdString());

    templateValues.SetValue("GENERATION", QString::number(m_activeGeneration).toStdString());

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/optimization.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &templateValues, &info);

    // setHtml(...) doesn't work
    // webView->setHtml(QString::fromStdString(info));

    // load(...) works
    writeStringContent(tempProblemDir() + "/optimization.html", QString::fromStdString(info));
    webView->load(QUrl::fromLocalFile(tempProblemDir() + "/optimization.html"));

}

