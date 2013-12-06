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

#ifndef SCENEVIEW_OPTIMIZATION_H
#define SCENEVIEW_OPTIMIZATION_H

#include "util.h"
#include "util/form_script.h"
#include "sceneview_common.h"

#include <QWebView>

class SceneViewPreprocessor;

class OptimizationControl: public QWidget
{
    Q_OBJECT
public:
    OptimizationControl(QWidget *parent = 0);

    QAction *actOptimization;

signals:
    void changed();

public slots:
    void updateControls();

private:
    void createActions();
    void createControls();

    void fillComboBox();

    QSlider *sliderTransientAnimate;


private slots:
};


class OptimizationWidget;

class OptimizationJavaScriptBridge : public QObject {

    Q_OBJECT

public:

    OptimizationJavaScriptBridge(OptimizationWidget* optimizationWidget) : m_optimizationWidget(optimizationWidget) {}
    Q_INVOKABLE void call(QString number, QString type);

private:

    OptimizationWidget* m_optimizationWidget;

};


class OptimizationWidget : public QWidget
{
    Q_OBJECT

public:
    OptimizationWidget(SceneViewPreprocessor *sceneView, QWidget *parent = 0);
    ~OptimizationWidget();

    OptimizationJavaScriptBridge* m_bridge;

    void loadResults();
    void runActualVariant();
    void setActiveNumber(QString number, QString variant);

    int m_activeNumber;

signals:

public slots:
    void refresh();
    void javaScriptWindowObjectCleared();

private:
    SceneViewPreprocessor *m_sceneViewGeometry;
    QString m_cascadeStyleSheet;

    QWebView *webView;

    QStringList *m_recentProblemFiles;
    QStringList *m_recentScriptFiles;

    QList<QList<double> > m_parametersList;
    QList<QList<double> > m_resultsList;

    QList<int> m_front, m_notFront;

private slots:
    void show();

//    void linkClicked(const QUrl &url);
};

#endif // SCENEINFOVIEW_H
