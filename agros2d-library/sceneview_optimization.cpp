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
        m_activeNumber = front(num);
    else if(variant == "not_front")
        m_activeNumber = notFront(num);
    else
        assert(0);

}

void OptimizationWidget::generationChanged(int generation)
{
    m_activeGeneration = generation;
    refresh();
}

double difference(QList<double> a, QList<double> b)
{
    double dif = 0;

    assert(a.size() == b.size());
    for(int i = 0; i < a.size(); i++)
        dif += fabs(a[i] - b[i]);
    return dif;
}

const double TOL = 1e-5;
bool areSame(QList<double> a, QList<double> b)
{
    return difference(a, b) < TOL;
}

void OptimizationWidget::loadResults()
{
    m_parametersList.clear();
    m_resultsList.clear();
    int gen = 0;
    QString path("/home/pkus/sources/data/optimization/data");
    FILE *f = nullptr;
    QList<double> newParameters;
    QList<double> newResults;

    while(1)
    {
        QString fileName = QString("%1/population-%2.txt").arg(path).arg(gen);
        qDebug() << fileName.toAscii();
        f = fopen(fileName.toAscii(), "r");
        if(f == nullptr)
            break;

        QList<int> newFrontThisOnly;
        QList<int> newNotFrontThisOnly;

        while(!feof(f))
        {
            bool doNotInclude = false;
            QList<double> row;
            double item;
            for(int i = 0; i < numParameters; i++)
            {
                if(fscanf(f, "%lf", &item) == 1)
                    row.push_back(item);
                else
                    doNotInclude = true;
            }
            newParameters = row;
            row.clear();
            for(int i = 0; i < numFunctionals; i++)
            {
                if(fscanf(f, "%lf", &item) == 1)
                    row.push_back(item);
                else
                    doNotInclude = true;
            }
            newResults = row;

            if(!doNotInclude)
            {
                for(int index = 0; index < m_parametersList.size(); index++)
                {
                    QList<double> presentParameters = m_parametersList.at(index);
                    if(areSame(presentParameters, newParameters))
                    {
                        doNotInclude = true;
                        newNotFrontThisOnly.push_back(index);
                    }
                }
            }
            if(!doNotInclude)
            {
                m_parametersList.push_back(newParameters);
                m_resultsList.push_back(newResults);

                newNotFrontThisOnly.push_back(m_parametersList.size() - 1);
            }
        }

        QList<int> newFront;
        QList<int> newNotFront;
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
                newFront.push_back(i);
            else
                newNotFront.push_back(i);
        }
        m_frontWithPrevious.push_back(newFront);
        m_notFrontWithPrevious.push_back(newNotFront);
        newFrontThisOnly = newFront;
        m_frontThisOnly.push_back(newFrontThisOnly);
        m_notFrontThisOnly.push_back(newNotFrontThisOnly);

        gen++;
    }
}

void OptimizationWidget::runActualVariant()
{

    //QString str = QString("import unittest as ut; agros2d_suite = ut.TestSuite(); import %1; agros2d_suite.addTest(ut.TestLoader().loadTestsFromTestCase(%1.%2)); agros2d_result = test_suite.scenario.Agros2DTestResult(); agros2d_suite.run(agros2d_result); agros2d_result_report = agros2d_result.report()").
    //        arg(module).arg(cls);

    QList<double> parameterList = m_parametersList[m_activeNumber];
    QString parameters("[");
    for(int i = 0; i < numParameters; i++)
        parameters  += QString("%1,").arg(parameterList[i]);
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
    m_activeGeneration = 0;
    m_activeNumber = 0;
    m_showFrontWithPrevious = false;

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

int OptimizationWidget::front(int index)
{
    if(m_showFrontWithPrevious)
        return m_frontWithPrevious.at(m_activeGeneration).at(index);
    else
        return m_frontThisOnly.at(m_activeGeneration).at(index);
}

int OptimizationWidget::notFront(int index)
{
    if(m_showFrontWithPrevious)
        return m_notFrontWithPrevious.at(m_activeGeneration).at(index);
    else
        return m_notFrontThisOnly.at(m_activeGeneration).at(index);
}

int OptimizationWidget::frontSize()
{
    if(m_showFrontWithPrevious)
        return m_frontWithPrevious.at(m_activeGeneration).size();
    else
        return m_frontThisOnly.at(m_activeGeneration).size();
}

int OptimizationWidget::notFrontSize()
{
    if(m_showFrontWithPrevious)
        return m_notFrontWithPrevious.at(m_activeGeneration).size();
    else
        return m_notFrontThisOnly.at(m_activeGeneration).size();
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
    QString optimizationDataNotFront = "[";
    if((frontSize() > 0) || (notFrontSize() > 0))
    {
        for(int i = 0; i < frontSize(); i++)
        {
            int index = front(i);
            QList<double> row = m_resultsList[index];
            optimizationDataFront += QString("[%1,%2], ")
                    .arg(row[0])
                    .arg(row[1]);
        }
        for(int i = 0; i < notFrontSize(); i++)
        {
            int index = notFront(i);
            QList<double> row = m_resultsList[index];
            optimizationDataNotFront += QString("[%1,%2], ")
                    .arg(row[0])
                    .arg(row[1]);
        }
    }
    optimizationDataFront += "]";
    optimizationDataNotFront += "]";
    templateValues.SetValue("OPTIMIZATION_DATA_FRONT", optimizationDataFront.toStdString());
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

