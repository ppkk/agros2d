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

#include "problemdialog.h"

#include "hermes2d/plugin_interface.h"

#include "util/global.h"

#include "scene.h"
#include "moduledialog.h"
#include "pythonlab/pythonengine_agros.h"

#include "hermes2d/module.h"

#include "hermes2d/coupling.h"
#include "hermes2d/problem_config.h"

#include "gui/lineeditdouble.h"
#include "gui/groupbox.h"
#include "gui/common.h"

#include "pythonlab/pythoneditor.h"

FieldSelectDialog::FieldSelectDialog(QList<QString> fields, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Select field"));
    setModal(true);

    m_selectedFieldId = "";

    lstFields = new QListWidget(this);
    lstFields->setIconSize(QSize(32, 32));
    lstFields->setMinimumHeight(36*9);

    QMapIterator<QString, QString> it(Module::availableModules());
    while (it.hasNext())
    {
        it.next();
        // add only missing fields
        if (!fields.contains(it.key()))
        {
            QListWidgetItem *item = new QListWidgetItem(lstFields);
            item->setIcon(icon("fields/" + it.key()));
            item->setText(it.value());
            item->setData(Qt::UserRole, it.key());

            lstFields->addItem(item);
        }
    }

    connect(lstFields, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(doItemDoubleClicked(QListWidgetItem *)));
    connect(lstFields, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(doItemSelected(QListWidgetItem *)));
    connect(lstFields, SIGNAL(itemPressed(QListWidgetItem *)),
            this, SLOT(doItemSelected(QListWidgetItem *)));

    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstFields);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (lstFields->count() > 0)
    {
        lstFields->setCurrentRow(0);
        doItemSelected(lstFields->currentItem());
    }
}

void FieldSelectDialog::doAccept()
{
    accept();
}

void FieldSelectDialog::doReject()
{
    reject();
}

int FieldSelectDialog::showDialog()
{
    return exec();
}

void FieldSelectDialog::doItemSelected(QListWidgetItem *item)
{
    m_selectedFieldId = item->data(Qt::UserRole).toString();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void FieldSelectDialog::doItemDoubleClicked(QListWidgetItem *item)
{
    if (lstFields->currentItem())
    {
        m_selectedFieldId = lstFields->currentItem()->data(Qt::UserRole).toString();
        accept();
    }
}

// ********************************************************************************************************

FieldWidget::FieldWidget(FieldInfo *fieldInfo, QWidget *parent)
    : QWidget(parent), m_fieldInfo(fieldInfo)
{
    createContent();
    load();
}

void FieldWidget::createContent()
{
    // equations
    // equationLaTeX = new LaTeXViewer(this);
    // equationLaTeX->setMaximumWidth(400);
    equationImage = new QLabel();

    cmbAdaptivityType = new QComboBox();
    cmbAnalysisType = new QComboBox();
    cmbLinearityType = new QComboBox();
    cmbLinearSolver = new QComboBox();

    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAnalysisTypeChanged(int)));
    connect(cmbLinearityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doLinearityTypeChanged(int)));
    connect(cmbLinearSolver, SIGNAL(currentIndexChanged(int)), this, SLOT(doLinearSolverChanged(int)));

    // mesh
    txtNumberOfRefinements = new QSpinBox(this);
    txtNumberOfRefinements->setMinimum(0);
    txtNumberOfRefinements->setMaximum(5);
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(10);

    // table
    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutGeneral->setColumnStretch(1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Analysis:")), 0, 0);
    layoutGeneral->addWidget(cmbAnalysisType, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Solver:")), 1, 0);
    layoutGeneral->addWidget(cmbLinearityType, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Matrix solver:")), 2, 0);
    layoutGeneral->addWidget(cmbLinearSolver, 2, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // mesh
    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutMesh->setColumnStretch(1, 1);
    layoutMesh->addWidget(new QLabel(tr("Number of refinements:")), 0, 0);
    layoutMesh->addWidget(txtNumberOfRefinements, 0, 1);
    layoutMesh->addWidget(new QLabel(tr("Polynomial order:")), 1, 0);
    layoutMesh->addWidget(txtPolynomialOrder, 1, 1);
    layoutMesh->addWidget(new QLabel(tr("Space adaptivity:")), 2, 0);
    layoutMesh->addWidget(cmbAdaptivityType, 2, 1);
    layoutMesh->setRowStretch(50, 1);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh parameters"));
    grpMesh->setLayout(layoutMesh);

    // left
    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addWidget(grpGeneral);
    layoutLeft->addStretch();

    // right
    QVBoxLayout *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(grpMesh);
    layoutRight->addStretch();

    // both
    QHBoxLayout *layoutPanel = new QHBoxLayout();
    layoutPanel->addLayout(layoutLeft);
    layoutPanel->addLayout(layoutRight);

    // equation
    QVBoxLayout *layoutEquation = new QVBoxLayout();
    // layoutEquation->addWidget(equationLaTeX);
    layoutEquation->addWidget(equationImage);
    layoutEquation->addStretch();

    // tabs
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->addTab(createSolverWidget(), tr("Solver"));
    tabWidget->addTab(createAdaptivityWidget(), tr("Space adaptivity"));
    tabWidget->addTab(createTransientAnalysisWidget(), tr("Transient analysis"));
    tabWidget->addTab(createLinearSolverWidget(), tr("Matrix solver"));

    QGroupBox *grpEquation = new QGroupBox(tr("Partial differential equation"));
    grpEquation->setLayout(layoutEquation);

    QVBoxLayout *layoutProblem = new QVBoxLayout();
    layoutProblem->addWidget(grpEquation);
    layoutProblem->addLayout(layoutPanel);
    layoutProblem->addWidget(tabWidget);

    // fill combobox
    fillComboBox();

    setLayout(layoutProblem);

    setMinimumSize(sizeHint());
}

QWidget *FieldWidget::createSolverWidget()
{
    // linearity
    chkNonlinearResidual = new QCheckBox(this);
    connect(chkNonlinearResidual, SIGNAL(stateChanged(int)), this, SLOT(doNonlinearResidual(int)));
    txtNonlinearResidual = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearResidualNorm).toDouble());
    txtNonlinearResidual->setBottom(0.0);

    chkNonlinearRelativeChangeOfSolutions = new QCheckBox(this);
    connect(chkNonlinearRelativeChangeOfSolutions, SIGNAL(stateChanged(int)), this, SLOT(doNonlinearRelativeChangeOfSolutions(int)));
    txtNonlinearRelativeChangeOfSolutions = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble());
    txtNonlinearRelativeChangeOfSolutions->setBottom(0.0);

    QGridLayout *layoutSolverConvergence = new QGridLayout();
    layoutSolverConvergence->setColumnMinimumWidth(0, columnMinimumWidth());

    layoutSolverConvergence->addWidget(new QLabel(tr("Relative change of solutions (%):")), 0, 0);
    layoutSolverConvergence->addWidget(chkNonlinearRelativeChangeOfSolutions, 0, 1);
    layoutSolverConvergence->addWidget(txtNonlinearRelativeChangeOfSolutions, 0, 2);
    layoutSolverConvergence->addWidget(new QLabel(tr("Residual:")), 1, 0);
    layoutSolverConvergence->addWidget(chkNonlinearResidual, 1, 1);
    layoutSolverConvergence->addWidget(txtNonlinearResidual, 1, 2);

    QGroupBox *grpSolverConvergence = new QGroupBox(tr("Convergence"));
    grpSolverConvergence->setLayout(layoutSolverConvergence);

    cmbNonlinearDampingType = new QComboBox();
    connect(cmbNonlinearDampingType, SIGNAL(currentIndexChanged(int)), this, SLOT(doNonlinearDampingChanged(int)));
    txtNonlinearDampingCoeff = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearDampingCoeff).toDouble());
    txtNonlinearDampingCoeff->setBottom(0.0);
    txtNonlinearDampingStepsForFactorIncrease = new QSpinBox(this);
    txtNonlinearDampingStepsForFactorIncrease->setMinimum(1);
    txtNonlinearDampingStepsForFactorIncrease->setMaximum(5);
    txtNonlinearDampingRatioForFactorDecrease = new LineEditDouble();

    QGridLayout *layoutSolverDamping = new QGridLayout();

    layoutSolverDamping->addWidget(new QLabel(tr("Damping type:")), 1, 0);
    layoutSolverDamping->addWidget(cmbNonlinearDampingType, 1, 1);
    layoutSolverDamping->addWidget(new QLabel(tr("Factor:")), 1, 2);
    layoutSolverDamping->addWidget(txtNonlinearDampingCoeff, 1, 3);
    layoutSolverDamping->addWidget(new QLabel(tr("Min. residual ratio for factor decrease:")), 2, 0, 1, 3);
    layoutSolverDamping->addWidget(txtNonlinearDampingRatioForFactorDecrease, 2, 3);
    layoutSolverDamping->addWidget(new QLabel(tr("Min. steps for factor increase:")), 3, 0, 1, 3);
    layoutSolverDamping->addWidget(txtNonlinearDampingStepsForFactorIncrease, 3, 3);

    QGroupBox *grpSolverDamping = new QGroupBox(tr("Damping"));
    grpSolverDamping->setLayout(layoutSolverDamping);

    chkNewtonReuseJacobian = new QCheckBox(tr("Reuse Jacobian if possible"));
    connect(chkNewtonReuseJacobian, SIGNAL(toggled(bool)), this, SLOT(doNewtonReuseJacobian(bool)));
    txtNewtonSufficientImprovementFactorForJacobianReuse = new LineEditDouble();
    txtNewtonMaximumStepsWithReusedJacobian = new QSpinBox(this);
    txtNewtonMaximumStepsWithReusedJacobian->setMinimum(0);
    txtNewtonMaximumStepsWithReusedJacobian->setMaximum(100);

    QGridLayout *layoutNewtonSolverReuse = new QGridLayout();

    layoutNewtonSolverReuse->addWidget(chkNewtonReuseJacobian, 1, 0);
    layoutNewtonSolverReuse->addWidget(new QLabel(tr("Max. residual ratio for Jacobian reuse:")), 2, 0);
    layoutNewtonSolverReuse->addWidget(txtNewtonSufficientImprovementFactorForJacobianReuse, 2, 1);
    layoutNewtonSolverReuse->addWidget(new QLabel(tr("Max. steps with the same Jacobian:")), 3, 0);
    layoutNewtonSolverReuse->addWidget(txtNewtonMaximumStepsWithReusedJacobian, 3, 1);

    QGroupBox *grpNewtonSolverReuse = new QGroupBox(tr("Jacobian reuse"));
    grpNewtonSolverReuse->setLayout(layoutNewtonSolverReuse);

    // Newton's solver
    QGridLayout *layoutNewtonSolver = new QGridLayout();
    layoutNewtonSolver->addWidget(grpNewtonSolverReuse, 3, 0, 1, 2);

    QWidget *widgetNewtonSolver = new QWidget(this);
    widgetNewtonSolver->setLayout(layoutNewtonSolver);

    // Picard's solver
    chkPicardAndersonAcceleration = new QCheckBox(tr("Use Anderson acceleration"));
    connect(chkPicardAndersonAcceleration, SIGNAL(stateChanged(int)), this, SLOT(doPicardAndersonChanged(int)));
    txtPicardAndersonBeta = new LineEditDouble(0.2);
    txtPicardAndersonBeta->setBottom(0.0);
    txtPicardAndersonBeta->setTop(1.0);
    txtPicardAndersonNumberOfLastVectors = new QSpinBox(this);
    txtPicardAndersonNumberOfLastVectors->setMinimum(1);
    txtPicardAndersonNumberOfLastVectors->setMaximum(5);

    QGridLayout *layoutPicardSolver = new QGridLayout(this);
    layoutPicardSolver->addWidget(chkPicardAndersonAcceleration, 0, 0, 1, 2);
    layoutPicardSolver->addWidget(new QLabel(tr("Anderson beta:")), 1, 0);
    layoutPicardSolver->addWidget(txtPicardAndersonBeta, 1, 1);
    layoutPicardSolver->addWidget(new QLabel(tr("Num. of last used iter.:")), 2, 0);
    layoutPicardSolver->addWidget(txtPicardAndersonNumberOfLastVectors, 2, 1);
    layoutPicardSolver->setRowStretch(50, 1);

    QWidget *widgetPicardSolver = new QWidget(this);
    widgetPicardSolver->setLayout(layoutPicardSolver);

    QTabWidget *tab = new QTabWidget(this);
    tab->addTab(widgetNewtonSolver, tr("Newton's solver"));
    tab->addTab(widgetPicardSolver, tr("Picard's solver"));

    QVBoxLayout *layoutSolver = new QVBoxLayout(this);
    layoutSolver->addWidget(grpSolverConvergence);
    layoutSolver->addWidget(grpSolverDamping);
    layoutSolver->addWidget(tab);

    QWidget *widSolver = new QWidget(this);
    widSolver->setLayout(layoutSolver);

    return widSolver;
}

QWidget *FieldWidget::createAdaptivityWidget()
{
    txtAdaptivitySteps = new QSpinBox(this);
    txtAdaptivitySteps->setMinimum(1);
    txtAdaptivitySteps->setMaximum(100);
    txtAdaptivitySteps->setValue(m_fieldInfo->defaultValue(FieldInfo::AdaptivitySteps).toInt());
    chkAdaptivityTolerance = new QCheckBox(this);
    connect(chkAdaptivityTolerance, SIGNAL(stateChanged(int)), this, SLOT(doAdaptivityTolerance(int)));
    txtAdaptivityTolerance = new LineEditDouble(1.0);
    txtAdaptivityTolerance->setBottom(0.0);
    cmbAdaptivityStoppingCriterionType = new QComboBox();
    foreach (QString type, adaptivityStoppingCriterionTypeStringKeys())
        if (adaptivityStoppingCriterionFromStringKey(type) != AdaptivityStoppingCriterionType_Undefined)
            cmbAdaptivityStoppingCriterionType->addItem(adaptivityStoppingCriterionTypeString(adaptivityStoppingCriterionFromStringKey(type)),
                                                        adaptivityStoppingCriterionFromStringKey(type));
    txtAdaptivityThreshold = new LineEditDouble(0.60);
    txtAdaptivityThreshold->setValue(m_fieldInfo->defaultValue(FieldInfo::AdaptivityThreshold).toDouble());
    cmbAdaptivityErrorCalculator = new QComboBox();
    chkAdaptivityUseAniso = new QCheckBox(tr("Use anisotropic refinements"));
    chkAdaptivityFinerReference = new QCheckBox(tr("Use hp reference solution for h and p adaptivity"));
    txtAdaptivityOrderIncrease = new QSpinBox(this);
    txtAdaptivityOrderIncrease->setMinimum(1);
    txtAdaptivityOrderIncrease->setMaximum(10);
    chkAdaptivitySpaceRefinement = new QCheckBox(tr("Space refinement for hp adaptivity"));
    txtAdaptivityBackSteps = new QSpinBox(this);
    txtAdaptivityBackSteps->setMinimum(0);
    txtAdaptivityBackSteps->setMaximum(100);
    txtAdaptivityRedoneEach = new QSpinBox(this);
    txtAdaptivityRedoneEach->setMinimum(1);
    txtAdaptivityRedoneEach->setMaximum(100);

    // control
    QGridLayout *layoutAdaptivityControl = new QGridLayout();
    layoutAdaptivityControl->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutAdaptivityControl->addWidget(new QLabel(tr("Maximum steps:")), 0, 0, 1, 2);
    layoutAdaptivityControl->addWidget(txtAdaptivitySteps, 0, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Tolerance (%):")), 1, 0);
    layoutAdaptivityControl->addWidget(chkAdaptivityTolerance, 1, 1, 1, 1, Qt::AlignRight);
    layoutAdaptivityControl->addWidget(txtAdaptivityTolerance, 1, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Stopping criterion:")), 0, 3);
    layoutAdaptivityControl->addWidget(cmbAdaptivityStoppingCriterionType, 0, 4);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Threshold:")), 1, 3);
    layoutAdaptivityControl->addWidget(txtAdaptivityThreshold, 1, 4);
    layoutAdaptivityControl->setRowStretch(50, 1);

    QGroupBox *grpControl = new QGroupBox(tr("Control"), this);
    grpControl->setLayout(layoutAdaptivityControl);

    // reference solution
    QGridLayout *layoutAdaptivityReferenceSolution = new QGridLayout();
    layoutAdaptivityReferenceSolution->addWidget(new QLabel(tr("Order increase:")), 0, 0);
    layoutAdaptivityReferenceSolution->addWidget(txtAdaptivityOrderIncrease, 0, 1);
    layoutAdaptivityReferenceSolution->addWidget(new QLabel(tr("Error calculator:")), 1, 0);
    layoutAdaptivityReferenceSolution->addWidget(cmbAdaptivityErrorCalculator, 1, 1);
    layoutAdaptivityReferenceSolution->addWidget(chkAdaptivitySpaceRefinement, 0, 2);
    layoutAdaptivityReferenceSolution->addWidget(chkAdaptivityUseAniso, 1, 2);
    layoutAdaptivityReferenceSolution->addWidget(chkAdaptivityFinerReference, 2, 2);
    layoutAdaptivityReferenceSolution->setRowStretch(50, 1);

    QGroupBox *grpReferenceSolution = new QGroupBox(tr("Reference solution"), this);
    grpReferenceSolution->setLayout(layoutAdaptivityReferenceSolution);

    // transient
    QGridLayout *layoutAdaptivityTransient = new QGridLayout();
    layoutAdaptivityTransient->addWidget(new QLabel(tr("Steps back in transient:")), 0, 0);
    layoutAdaptivityTransient->addWidget(txtAdaptivityBackSteps, 0, 1);
    layoutAdaptivityTransient->addWidget(new QLabel(tr("Redone each trans. step:")), 1, 0);
    layoutAdaptivityTransient->addWidget(txtAdaptivityRedoneEach, 1, 1);
    layoutAdaptivityTransient->setRowStretch(50, 1);

    QGroupBox *grpTransient = new QGroupBox(tr("Transient analysis"), this);
    grpTransient->setLayout(layoutAdaptivityTransient);

    QVBoxLayout *layoutAdaptivity = new QVBoxLayout();
    layoutAdaptivity->addWidget(grpControl);
    layoutAdaptivity->addWidget(grpReferenceSolution);
    layoutAdaptivity->addWidget(grpTransient);
    layoutAdaptivity->addStretch(1);

    QWidget *widAdaptivity = new QWidget(this);
    widAdaptivity->setLayout(layoutAdaptivity);

    return widAdaptivity;
}

QWidget *FieldWidget::createTransientAnalysisWidget()
{
    // transient
    txtTransientInitialCondition = new LineEditDouble(0.0);
    txtTransientTimeSkip = new LineEditDouble(0.0);
    txtTransientTimeSkip->setBottom(0.0);

    // transient analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransientAnalysis->setColumnStretch(1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial condition:")), 0, 0);
    layoutTransientAnalysis->addWidget(txtTransientInitialCondition, 0, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Time skip (s):")), 1, 0);
    layoutTransientAnalysis->addWidget(txtTransientTimeSkip, 1, 1);
    layoutTransientAnalysis->setRowStretch(50, 1);

    QWidget *widTransientAnalysis = new QWidget(this);
    widTransientAnalysis->setLayout(layoutTransientAnalysis);

    return widTransientAnalysis;
}

QWidget *FieldWidget::createLinearSolverWidget()
{
    cmbIterLinearSolverMethod = new QComboBox();
    cmbIterLinearSolverPreconditioner = new QComboBox();
    txtIterLinearSolverToleranceAbsolute = new LineEditDouble(1e-15);
    txtIterLinearSolverToleranceAbsolute->setBottom(0.0);
    txtIterLinearSolverIters = new QSpinBox();
    txtIterLinearSolverIters->setMinimum(1);
    txtIterLinearSolverIters->setMaximum(10000);

    QGridLayout *iterSolverLayout = new QGridLayout();
    iterSolverLayout->addWidget(new QLabel(tr("Method:")), 0, 0);
    iterSolverLayout->addWidget(cmbIterLinearSolverMethod, 0, 1);
    iterSolverLayout->addWidget(new QLabel(tr("Preconditioner:")), 1, 0);
    iterSolverLayout->addWidget(cmbIterLinearSolverPreconditioner, 1, 1);
    iterSolverLayout->addWidget(new QLabel(tr("Absolute tolerance:")), 2, 0);
    iterSolverLayout->addWidget(txtIterLinearSolverToleranceAbsolute, 2, 1);
    iterSolverLayout->addWidget(new QLabel(tr("Maximum number of iterations:")), 3, 0);
    iterSolverLayout->addWidget(txtIterLinearSolverIters, 3, 1);

    QGroupBox *iterSolverGroup = new QGroupBox(tr("Iterative solver"));
    iterSolverGroup->setLayout(iterSolverLayout);

    QVBoxLayout *layoutLinearSolver = new QVBoxLayout();
    layoutLinearSolver->addWidget(iterSolverGroup);
    layoutLinearSolver->addStretch();

    QWidget *widLinearSolver = new QWidget(this);
    widLinearSolver->setLayout(layoutLinearSolver);

    return widLinearSolver;
}

void FieldWidget::fillComboBox()
{
    cmbNonlinearDampingType->clear();
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Automatic), DampingType_Automatic);
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Fixed), DampingType_Fixed);
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Off), DampingType_Off);

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_None), AdaptivityType_None);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_H), AdaptivityType_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_P), AdaptivityType_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_HP), AdaptivityType_HP);

    foreach(LinearityType linearityType, m_fieldInfo->availableLinearityTypes())
    {
        cmbLinearityType->addItem(linearityTypeString(linearityType), linearityType);
    }

    QMapIterator<AnalysisType, QString> it(m_fieldInfo->analyses());
    while (it.hasNext())
    {
        it.next();
        cmbAnalysisType->addItem(it.value(), it.key());
    }

    cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_UMFPACK), Hermes::SOLVER_UMFPACK);
#ifdef WITH_MUMPS
    cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_MUMPS), Hermes::SOLVER_MUMPS);
#endif
#ifdef WITH_SUPERLU
    cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_SUPERLU), Hermes::SOLVER_SUPERLU);
#endif
#ifdef WITH_PETSC
    cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_PETSC), Hermes::SOLVER_PETSC);
#endif
#ifdef HAVE_AMESOS
    cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_AMESOS), Hermes::SOLVER_AMESOS);
#endif
#ifdef HAVE_AZTECOO
    cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_AZTECOO), Hermes::SOLVER_AZTECOO);
#endif
    cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_PARALUTION_ITERATIVE), Hermes::SOLVER_PARALUTION_ITERATIVE);
    cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_PARALUTION_AMG), Hermes::SOLVER_PARALUTION_AMG);
    // cmbLinearSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_EXTERNAL), Hermes::SOLVER_EXTERNAL);

    foreach(Module::ErrorCalculator calc, m_fieldInfo->errorCalculators())
        cmbAdaptivityErrorCalculator->addItem(calc.name(), calc.id());

    cmbIterLinearSolverMethod->clear();
    foreach (QString method, iterLinearSolverMethodStringKeys())
        cmbIterLinearSolverMethod->addItem(iterLinearSolverMethodString(iterLinearSolverMethodFromStringKey(method)), iterLinearSolverMethodFromStringKey(method));

    cmbIterLinearSolverPreconditioner->clear();
    foreach (QString type, iterLinearSolverPreconditionerTypeStringKeys())
        cmbIterLinearSolverPreconditioner->addItem(iterLinearSolverPreconditionerTypeString(iterLinearSolverPreconditionerTypeFromStringKey(type)), iterLinearSolverPreconditionerTypeFromStringKey(type));
}

void FieldWidget::load()
{
    // analysis type
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(m_fieldInfo->analysisType()));
    if (cmbAnalysisType->currentIndex() == -1)
        cmbAnalysisType->setCurrentIndex(0);
    // adaptivity
    cmbAdaptivityType->setCurrentIndex(cmbAdaptivityType->findData(m_fieldInfo->adaptivityType()));
    txtAdaptivitySteps->setValue(m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt());
    chkAdaptivityTolerance->setChecked(m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble() > 0);
    txtAdaptivityTolerance->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble());
    txtAdaptivityThreshold->setValue(m_fieldInfo->value(FieldInfo::AdaptivityThreshold).toDouble());
    cmbAdaptivityStoppingCriterionType->setCurrentIndex(cmbAdaptivityStoppingCriterionType->findData((AdaptivityStoppingCriterionType) m_fieldInfo->value(FieldInfo::AdaptivityStoppingCriterion).toInt()));
    cmbAdaptivityErrorCalculator->setCurrentIndex(cmbAdaptivityErrorCalculator->findData(m_fieldInfo->value(FieldInfo::AdaptivityErrorCalculator).toString()));
    chkAdaptivityUseAniso->setChecked(m_fieldInfo->value(FieldInfo::AdaptivityUseAniso).toBool());
    chkAdaptivityFinerReference->setChecked(m_fieldInfo->value(FieldInfo::AdaptivityFinerReference).toBool());
    txtAdaptivityOrderIncrease->setValue(m_fieldInfo->value(FieldInfo::AdaptivityOrderIncrease).toInt());
    chkAdaptivitySpaceRefinement->setChecked(m_fieldInfo->value(FieldInfo::AdaptivitySpaceRefinement).toBool());
    txtAdaptivityBackSteps->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTransientBackSteps).toInt());
    txtAdaptivityRedoneEach->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTransientRedoneEach).toInt());
    // matrix solver
    cmbLinearSolver->setCurrentIndex(cmbLinearSolver->findData(m_fieldInfo->matrixSolver()));
    //mesh
    txtNumberOfRefinements->setValue(m_fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
    txtPolynomialOrder->setValue(m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
    // transient
    txtTransientInitialCondition->setValue(m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble());
    txtTransientTimeSkip->setValue(m_fieldInfo->value(FieldInfo::TransientTimeSkip).toDouble());
    // linearity
    cmbLinearityType->setCurrentIndex(cmbLinearityType->findData(m_fieldInfo->linearityType()));
    txtNonlinearResidual->setValue(m_fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble());
    chkNonlinearResidual->setChecked(m_fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble() > 0);
    chkNonlinearRelativeChangeOfSolutions->setChecked(m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() > 0);
    txtNonlinearRelativeChangeOfSolutions->setValue(m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble());
    cmbNonlinearDampingType->setCurrentIndex(cmbNonlinearDampingType->findData((DampingType) m_fieldInfo->value(FieldInfo::NonlinearDampingType).toInt()));
    txtNonlinearDampingCoeff->setValue(m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble());
    txtNewtonSufficientImprovementFactorForJacobianReuse->setValue(m_fieldInfo->value(FieldInfo::NewtonJacobianReuseRatio).toDouble());
    txtNonlinearDampingRatioForFactorDecrease->setValue(m_fieldInfo->value(FieldInfo::NonlinearDampingFactorDecreaseRatio).toDouble());
    txtNewtonMaximumStepsWithReusedJacobian->setValue(m_fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt());
    txtNonlinearDampingStepsForFactorIncrease->setValue(m_fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt());
    chkNewtonReuseJacobian->setChecked((m_fieldInfo->value(FieldInfo::NewtonReuseJacobian)).toBool());
    chkPicardAndersonAcceleration->setChecked(m_fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool());
    txtPicardAndersonBeta->setValue(m_fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble());
    txtPicardAndersonNumberOfLastVectors->setValue(m_fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt());
    // linear solver
    cmbIterLinearSolverMethod->setCurrentIndex((Hermes::Solvers::IterSolverType) cmbIterLinearSolverMethod->findData(m_fieldInfo->value(FieldInfo::LinearSolverIterMethod).toInt()));
    cmbIterLinearSolverPreconditioner->setCurrentIndex((Hermes::Solvers::PreconditionerType) cmbIterLinearSolverPreconditioner->findData(m_fieldInfo->value(FieldInfo::LinearSolverIterPreconditioner).toInt()));
    txtIterLinearSolverToleranceAbsolute->setValue(m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble());
    txtIterLinearSolverIters->setValue(m_fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt());

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

bool FieldWidget::save()
{
    // analysis type
    m_fieldInfo->setAnalysisType((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt());
    // adaptivity
    m_fieldInfo->setAdaptivityType((AdaptivityType) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivitySteps, txtAdaptivitySteps->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTolerance, chkAdaptivityTolerance->isChecked() ? txtAdaptivityTolerance->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::AdaptivityThreshold, txtAdaptivityThreshold->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityStoppingCriterion, (AdaptivityStoppingCriterionType) cmbAdaptivityStoppingCriterionType->itemData(cmbAdaptivityStoppingCriterionType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivityErrorCalculator, cmbAdaptivityErrorCalculator->itemData(cmbAdaptivityErrorCalculator->currentIndex()).toString());
    m_fieldInfo->setValue(FieldInfo::AdaptivityUseAniso, chkAdaptivityUseAniso->isChecked());
    m_fieldInfo->setValue(FieldInfo::AdaptivityFinerReference, chkAdaptivityFinerReference->isChecked());
    m_fieldInfo->setValue(FieldInfo::AdaptivityOrderIncrease, txtAdaptivityOrderIncrease->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivitySpaceRefinement, chkAdaptivitySpaceRefinement->isChecked());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTransientBackSteps, txtAdaptivityBackSteps->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTransientRedoneEach, txtAdaptivityRedoneEach->value());
    // matrix solver
    m_fieldInfo->setMatrixSolver((Hermes::MatrixSolverType) cmbLinearSolver->itemData(cmbLinearSolver->currentIndex()).toInt());
    //mesh
    m_fieldInfo->setValue(FieldInfo::SpaceNumberOfRefinements, txtNumberOfRefinements->value());
    m_fieldInfo->setValue(FieldInfo::SpacePolynomialOrder, txtPolynomialOrder->value());
    // transient
    m_fieldInfo->setValue(FieldInfo::TransientInitialCondition, txtTransientInitialCondition->value());
    m_fieldInfo->setValue(FieldInfo::TransientTimeSkip, txtTransientTimeSkip->value());
    // linearity
    m_fieldInfo->setLinearityType((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::NonlinearResidualNorm, chkNonlinearResidual->isChecked() ? txtNonlinearResidual->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::NonlinearRelativeChangeOfSolutions, chkNonlinearRelativeChangeOfSolutions->isChecked() ? txtNonlinearRelativeChangeOfSolutions->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingCoeff, txtNonlinearDampingCoeff->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingType, (DampingType) cmbNonlinearDampingType->itemData(cmbNonlinearDampingType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::NewtonReuseJacobian, chkNewtonReuseJacobian->isChecked());
    m_fieldInfo->setValue(FieldInfo::NewtonJacobianReuseRatio, txtNewtonSufficientImprovementFactorForJacobianReuse->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingFactorDecreaseRatio, txtNonlinearDampingRatioForFactorDecrease->value());
    m_fieldInfo->setValue(FieldInfo::NewtonMaxStepsReuseJacobian, txtNewtonMaximumStepsWithReusedJacobian->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearStepsToIncreaseDampingFactor, txtNonlinearDampingStepsForFactorIncrease->value());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonAcceleration, chkPicardAndersonAcceleration->isChecked());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonBeta, txtPicardAndersonBeta->value());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonNumberOfLastVectors, txtPicardAndersonNumberOfLastVectors->value());
    // linear solver
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterMethod, cmbIterLinearSolverMethod->itemData(cmbIterLinearSolverMethod->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterPreconditioner, cmbIterLinearSolverPreconditioner->itemData(cmbIterLinearSolverPreconditioner->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterToleranceAbsolute, txtIterLinearSolverToleranceAbsolute->value());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterIters, txtIterLinearSolverIters->value());

    return true;
}

void FieldWidget::refresh()
{
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

FieldInfo *FieldWidget::fieldInfo()
{
    return m_fieldInfo;
}

void FieldWidget::doAnalysisTypeChanged(int index)
{
    // initial condition
    txtTransientInitialCondition->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);

    // time steps skip
    bool otherFieldIsTransient = false;
    foreach (FieldInfo* otherFieldInfo, Agros2D::problem()->fieldInfos())
        if (otherFieldInfo->analysisType() == AnalysisType_Transient && otherFieldInfo->fieldId() != m_fieldInfo->fieldId())
            otherFieldIsTransient = true;

    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(index).toInt();
    txtTransientTimeSkip->setEnabled(analysisType != AnalysisType_Transient && otherFieldIsTransient);

    LinearityType previousLinearityType = (LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt();
    cmbLinearityType->clear();
    int idx = 0, nextIndex = 0;
    foreach(LinearityType linearityType, m_fieldInfo->availableLinearityTypes(analysisType))
    {
        cmbLinearityType->addItem(linearityTypeString(linearityType), linearityType);
        if(linearityType == previousLinearityType)
            nextIndex = idx;
        idx++;
    }
    cmbLinearityType->setCurrentIndex(nextIndex);

    doShowEquation();
    doAdaptivityChanged(cmbAdaptivityType->currentIndex());
}

void FieldWidget::doShowEquation()
{
    // equationLaTeX->setLatex(m_fieldInfo->equation());
    QPixmap pixmap(QString("%1/resources/images/equations/%2_equation_%3.png").
                   arg(datadir()).
                   arg(m_fieldInfo->fieldId()).
                   arg(analysisTypeToStringKey((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt())));

    equationImage->setPixmap(pixmap);
    equationImage->setMask(pixmap.mask());
}

void FieldWidget::doAdaptivityChanged(int index)
{
    txtAdaptivitySteps->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    chkAdaptivityTolerance->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    txtAdaptivityTolerance->setEnabled(((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None) && chkAdaptivityTolerance->isChecked());
    txtAdaptivityThreshold->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    cmbAdaptivityStoppingCriterionType->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    cmbAdaptivityErrorCalculator->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    chkAdaptivityUseAniso->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    chkAdaptivitySpaceRefinement->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    txtAdaptivityOrderIncrease->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    chkAdaptivityFinerReference->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);

    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();
    txtAdaptivityBackSteps->setEnabled(Agros2D::problem()->isTransient() && analysisType != AnalysisType_Transient && (AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    txtAdaptivityRedoneEach->setEnabled(Agros2D::problem()->isTransient() && analysisType != AnalysisType_Transient && (AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
}

void FieldWidget::doLinearityTypeChanged(int index)
{
    chkNonlinearResidual->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    txtNonlinearResidual->setEnabled(((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear) && chkNonlinearResidual->isChecked());
    chkNonlinearRelativeChangeOfSolutions->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    txtNonlinearRelativeChangeOfSolutions->setEnabled(((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear) && chkNonlinearRelativeChangeOfSolutions->isChecked());

    cmbNonlinearDampingType->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    doNonlinearDampingChanged(cmbNonlinearDampingType->currentIndex());

    txtNewtonMaximumStepsWithReusedJacobian->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    chkNewtonReuseJacobian->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    txtNewtonSufficientImprovementFactorForJacobianReuse->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    doNewtonReuseJacobian(chkNewtonReuseJacobian->isChecked());

    chkPicardAndersonAcceleration->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    txtPicardAndersonBeta->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    txtPicardAndersonNumberOfLastVectors->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    doPicardAndersonChanged(chkPicardAndersonAcceleration->checkState());
}

void FieldWidget::doLinearSolverChanged(int index)
{
    Hermes::MatrixSolverType solverType = (Hermes::MatrixSolverType) cmbLinearSolver->itemData(cmbLinearSolver->currentIndex()).toInt();
    bool isIterative = ((solverType == Hermes::SOLVER_PARALUTION_ITERATIVE) || (solverType == Hermes::SOLVER_PARALUTION_AMG));

    cmbIterLinearSolverMethod->setEnabled(isIterative);
    cmbIterLinearSolverPreconditioner->setEnabled(isIterative);
    txtIterLinearSolverToleranceAbsolute->setEnabled(isIterative);
    txtIterLinearSolverIters->setEnabled(isIterative);
}

void FieldWidget::doNonlinearDampingChanged(int index)
{
    txtNonlinearDampingCoeff->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                         ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() != DampingType_Off));
    txtNonlinearDampingStepsForFactorIncrease->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                                          ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() == DampingType_Automatic));
    txtNonlinearDampingRatioForFactorDecrease->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                                          ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() == DampingType_Automatic));

}

void FieldWidget::doNewtonReuseJacobian(bool checked)
{
    txtNewtonMaximumStepsWithReusedJacobian->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Newton) &&
                                                        (chkNewtonReuseJacobian->isChecked()));
    txtNewtonSufficientImprovementFactorForJacobianReuse->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Newton) &&
                                                                     (chkNewtonReuseJacobian->isChecked()));
}

void FieldWidget::doPicardAndersonChanged(int index)
{
    txtPicardAndersonBeta->setEnabled((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Picard && chkPicardAndersonAcceleration->isChecked());
    txtPicardAndersonNumberOfLastVectors->setEnabled((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Picard && chkPicardAndersonAcceleration->isChecked());
}

void FieldWidget::doAdaptivityTolerance(int state)
{
    txtAdaptivityTolerance->setEnabled(((AdaptivityType) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt() != AdaptivityType_None) && chkAdaptivityTolerance->isChecked());
}

void FieldWidget::doNonlinearResidual(int state)
{
    txtNonlinearResidual->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) && chkNonlinearResidual->isChecked());
}

void FieldWidget::doNonlinearRelativeChangeOfSolutions(int state)
{
    txtNonlinearRelativeChangeOfSolutions->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) && chkNonlinearRelativeChangeOfSolutions->isChecked());
}

// ********************************************************************************************

FieldDialog::FieldDialog(FieldInfo *fieldInfo, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(fieldInfo->name());

    fieldWidget = new FieldWidget(fieldInfo, this);

    // dialog buttons
    QPushButton *btnDeleteField = new QPushButton(tr("Delete field"));
    btnDeleteField->setDefault(false);
    btnDeleteField->setEnabled(Agros2D::problem()->hasField(fieldInfo->fieldId()));
    connect(btnDeleteField, SIGNAL(clicked()), this, SLOT(deleteField()));

#ifdef QT_DEBUG
    QPushButton *btnModuleEditor = new QPushButton(tr("Module editor"));
    btnModuleEditor->setDefault(false);
    btnModuleEditor->setVisible(false);
    connect(btnModuleEditor, SIGNAL(clicked()), this, SLOT(moduleEditor()));
#endif

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnDeleteField, QDialogButtonBox::ActionRole);
#ifdef QT_DEBUG
    // buttonBox->addButton(btnModuleEditor, QDialogButtonBox::ActionRole);
#endif
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(fieldWidget);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    // setMaximumSize(sizeHint());

    QSettings settings;
    restoreGeometry(settings.value("FieldDialog/Geometry", saveGeometry()).toByteArray());
}

FieldDialog::~FieldDialog()
{
    QSettings settings;
    settings.setValue("FieldDialog/Geometry", saveGeometry());
}

void FieldDialog::doAccept()
{
    fieldWidget->save();
    accept();
}

void FieldDialog::deleteField()
{
    if (QMessageBox::question(this, tr("Delete"), tr("Physical field '%1' will be pernamently deleted. Are you sure?").
                              arg(fieldWidget->fieldInfo()->name()), tr("&Yes"), tr("&No")) == 0)
    {
        Agros2D::problem()->removeField(fieldWidget->fieldInfo());
        accept();
    }
}

void FieldDialog::moduleEditor()
{
    ModuleDialog moduleDialog(fieldWidget->fieldInfo()->fieldId(), this);
    moduleDialog.exec();
}

// ********************************************************************************************

FieldsToobar::FieldsToobar(QWidget *parent) : QWidget(parent)
{
    createControls();

    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), this, SLOT(refresh()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));

    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(refresh()));

    refresh();
}

void FieldsToobar::createControls()
{
    QButtonGroup *buttonBar = new QButtonGroup(this);
    connect(buttonBar, SIGNAL(buttonClicked(int)), this, SLOT(fieldDialog(int)));

    QGridLayout *layoutFields = new QGridLayout();

    // buttons and labels
    int numberOfFields = Module::availableModules().count();

    int row = 0;
    for (int i = 0; i < numberOfFields; i++)
    {
        QLabel *label = new QLabel(this);
        label->setVisible(false);
        label->setStyleSheet("QLabel { font-size: 8.5pt; }");

        QToolButton *button = new QToolButton(this);
        button->setVisible(false);
        button->setMinimumWidth(columnMinimumWidth());
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setStyleSheet("QToolButton { font-size: 8pt; }");
        button->setIconSize(QSize(36, 36));

        // add to button bar
        buttonBar->addButton(button, row);

        // add to layout
        layoutFields->addWidget(button, row, 0);
        layoutFields->addWidget(label, row, 1);

        // add to lists
        m_buttons.append(button);
        m_labels.append(label);

        row++;
    }

    // dialog buttons
    QPushButton *btnAddField = new QPushButton(tr("Add field")); // icon("tabadd")
    connect(btnAddField, SIGNAL(clicked()), SLOT(addField()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnAddField);

    QVBoxLayout *layoutToolBar = new QVBoxLayout();
    layoutToolBar->setContentsMargins(0, 1, 0, 1);
    // layoutToolBar->addWidget(tlbFields);
    layoutToolBar->addLayout(layoutFields);
    layoutToolBar->addLayout(layoutButtons);
    layoutToolBar->addStretch();

    setLayout(layoutToolBar);
}

void FieldsToobar::refresh()
{
    // disable during script running
    if (currentPythonEngineAgros()->isScriptRunning())
        return;

    setUpdatesEnabled(false);

    // fields
    int row = 0;
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        QString hint = tr("<table>"
                          "<tr><td><b>Analysis:</b></td><td>%1</td></tr>"
                          "<tr><td><b>Solver:</b></td><td>%3</td></tr>"
                          "<tr><td><b>Adaptivity:</b></td><td>%2</td></tr>"
                          "<tr><td><b>Number of ref. / order:</b></td><td>%4 / %5</td></tr>"
                          "</table>")
                .arg(analysisTypeString(fieldInfo->analysisType()))
                .arg(adaptivityTypeString(fieldInfo->adaptivityType()))
                .arg(linearityTypeString(fieldInfo->linearityType()))
                .arg(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt())
                .arg(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());

        m_labels[row]->setText(hint);
        m_labels[row]->setVisible(true);

        m_buttons[row]->setText(fieldInfo->name());
        m_buttons[row]->setIcon(icon("fields/" + fieldInfo->fieldId()));
        m_buttons[row]->setVisible(true);

        row++;
    }
    for (int i = row; i < m_buttons.count(); i++)
    {
        m_labels[i]->setVisible(false);
        m_buttons[i]->setVisible(false);
    }

    setUpdatesEnabled(true);
}

void FieldsToobar::fieldDialog(int index)
{
    FieldInfo *fieldInfo = Agros2D::problem()->fieldInfos().values().at(index);
    if (fieldInfo)
    {
        FieldDialog fieldDialog(fieldInfo, this);
        if (fieldDialog.exec() == QDialog::Accepted)
        {
            Agros2D::problem()->clearSolution();

            refresh();
            emit changed();
        }
    }
}

void FieldsToobar::addField()
{
    // select field dialog
    FieldSelectDialog dialog(Agros2D::problem()->fieldInfos().keys(), this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        // add field
        FieldInfo *fieldInfo = new FieldInfo(dialog.selectedFieldId());

        FieldDialog fieldDialog(fieldInfo, this);
        if (fieldDialog.exec() == QDialog::Accepted)
        {
            Agros2D::problem()->clearSolution();
            Agros2D::problem()->addField(fieldInfo);

            refresh();
            emit changed();
        }
        else
        {
            delete fieldInfo;
        }
    }
}

// ********************************************************************************************

CouplingsWidget::CouplingsWidget(QWidget *parent) : QWidget(parent)
{
    Agros2D::problem()->synchronizeCouplings();

    createContent();

    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), this, SLOT(refresh()));
}

void CouplingsWidget::createContent()
{
    QGridLayout *layoutTable;layoutTable = new QGridLayout();
    layoutTable->setContentsMargins(0, 0, 0, 0);
    layoutTable->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTable->setColumnStretch(1, 1);

    for (int i = 0; i < couplingList()->availableCouplings().count(); i++)
    {
        QLabel *label = new QLabel(this);
        label->setVisible(false);

        QComboBox *combo = new QComboBox(this);
        combo->setVisible(false);
        combo->addItem(couplingTypeString(CouplingType_None), CouplingType_None);
        combo->addItem(couplingTypeString(CouplingType_Weak), CouplingType_Weak);
        combo->addItem(couplingTypeString(CouplingType_Hard), CouplingType_Hard);

        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(itemChanged(int)));

        layoutTable->addWidget(label, i, 0);
        layoutTable->addWidget(combo, i, 1);

        m_labels.append(label);
        m_comboBoxes.append(combo);
    }

    setLayout(layoutTable);
}

void CouplingsWidget::save()
{
    int row = 0;
    foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos())
    {
        couplingInfo->setCouplingType((CouplingType) m_comboBoxes[row]->itemData(m_comboBoxes[row]->currentIndex()).toInt());

        row++;
    }
}

void CouplingsWidget::refresh()
{
    Agros2D::problem()->synchronizeCouplings();

    setUpdatesEnabled(false);

    int row = 0;
    foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos())
    {
        m_comboBoxes[row]->blockSignals(true);
        m_comboBoxes[row]->setUpdatesEnabled(false);
        m_comboBoxes[row]->setCurrentIndex(m_comboBoxes[row]->findData(couplingInfo->couplingType()));
        m_comboBoxes[row]->setUpdatesEnabled(true);
        m_comboBoxes[row]->setVisible(true);
        m_comboBoxes[row]->blockSignals(false);

        m_labels[row]->setText(couplingInfo->name());
        m_labels[row]->setVisible(true);

        row++;
    }
    for (int i = row; i < m_comboBoxes.count(); i++)
    {
        m_comboBoxes[i]->setVisible(false);
        m_labels[i]->setVisible(false);
    }

    setUpdatesEnabled(true);
}

void CouplingsWidget::itemChanged(int index)
{
    // qDebug() << "void CouplingsWidget::itemChanged(int index)";
    emit changed();
}

// ********************************************************************************************

ProblemWidget::ProblemWidget(QWidget *parent) : QWidget(parent)
{
    createActions();
    createControls();

    updateControls();

    // global signals
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(updateControls()));
    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), this, SLOT(updateControls()));
    connect(fieldsToolbar, SIGNAL(changed()), this, SLOT(updateControls()));

    // resend signal
    connect(fieldsToolbar, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(couplingsWidget, SIGNAL(changed()), this, SIGNAL(changed()));

    setMinimumSize(sizeHint());
}

void ProblemWidget::createActions()
{
    actProperties = new QAction(icon("document-properties"), tr("Properties"), this);
    actProperties->setShortcut(tr("Ctrl+1"));
    actProperties->setCheckable(true);
}

void ProblemWidget::createControls()
{
    // fields toolbar
    fieldsToolbar = new FieldsToobar();
    QVBoxLayout *layoutFields = new QVBoxLayout();
    layoutFields->addWidget(fieldsToolbar);
    layoutFields->addStretch();

    QGroupBox *grpFieldsToolbar = new QGroupBox(tr("Physical fields"));
    grpFieldsToolbar->setLayout(layoutFields);

    // problem
    cmbCoordinateType = new QComboBox();
    // mesh type
    cmbMeshType = new QComboBox();

    // general
    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutGeneral->setColumnStretch(1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Coordinate type:")), 0, 0);
    layoutGeneral->addWidget(cmbCoordinateType, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Mesh type:")), 1, 0);
    layoutGeneral->addWidget(cmbMeshType, 1, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // harmonic
    txtFrequency = new LineEditDouble(0);
    txtFrequency->setBottom(0.0);

    // harmonic analysis
    QGridLayout *layoutHarmonicAnalysis = new QGridLayout();
    layoutHarmonicAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutHarmonicAnalysis->addWidget(new QLabel(tr("Frequency (Hz):")), 0, 0);
    layoutHarmonicAnalysis->addWidget(txtFrequency, 0, 1);

    grpHarmonicAnalysis = new QGroupBox(tr("Harmonic analysis"));
    grpHarmonicAnalysis->setLayout(layoutHarmonicAnalysis);

    // transient
    cmbTransientMethod = new QComboBox();
    txtTransientOrder = new QSpinBox();
    txtTransientOrder->setMinimum(1);
    txtTransientOrder->setMaximum(3);
    txtTransientTimeTotal = new LineEditDouble(1.0);
    txtTransientTimeTotal->setBottom(0.0);
    lblTransientTimeTotal = new QLabel("Total time");
    txtTransientTolerance = new LineEditDouble(0.1);
    txtTransientTolerance->setBottom(0.0);
    chkTransientInitialStepSize = new QCheckBox(this);
    txtTransientInitialStepSize = new LineEditDouble(0.01);
    txtTransientInitialStepSize->setBottom(0);
    txtTransientSteps = new QSpinBox();
    txtTransientSteps->setMinimum(1);
    txtTransientSteps->setMaximum(10000);
    lblTransientTimeStep = new QLabel("0.0");
    lblTransientSteps = new QLabel(tr("Number of constant steps:"));

    // transient analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransientAnalysis->setColumnStretch(2, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Method:")), 0, 0, 1, 2);
    layoutTransientAnalysis->addWidget(cmbTransientMethod, 0, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Order:")), 1, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientOrder, 1, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Tolerance:")), 2, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientTolerance, 2, 2);
    layoutTransientAnalysis->addWidget(lblTransientTimeTotal, 3, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientTimeTotal, 3, 2);
    layoutTransientAnalysis->addWidget(lblTransientSteps, 4, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientSteps, 4, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial time step:")), 5, 0);
    layoutTransientAnalysis->addWidget(chkTransientInitialStepSize, 5, 1, 1, 1, Qt::AlignRight);
    layoutTransientAnalysis->addWidget(txtTransientInitialStepSize, 5, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Constant time step:")), 6, 0, 1, 2);
    layoutTransientAnalysis->addWidget(lblTransientTimeStep, 6, 2);

    grpTransientAnalysis = new QGroupBox(tr("Transient analysis"));
    grpTransientAnalysis->setLayout(layoutTransientAnalysis);

    // fill combobox
    fillComboBox();

    // couplings
    couplingsWidget = new CouplingsWidget(this);
    connect(Agros2D::problem(), SIGNAL(couplingsChanged()), couplingsWidget, SLOT(refresh()));
    connect(couplingsWidget, SIGNAL(changed()), couplingsWidget, SLOT(save()));

    QVBoxLayout *layoutCouplings = new QVBoxLayout();
    layoutCouplings->addWidget(couplingsWidget);

    grpCouplings = new QGroupBox(tr("Couplings"));
    grpCouplings->setLayout(layoutCouplings);

    // startup script
    txtStartupScript = new ScriptEditor(currentPythonEngine(), this);
    txtStartupScript->setLineNumbersVisible(false);
    lblStartupScriptError = new QLabel();

    QPalette palette = lblStartupScriptError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblStartupScriptError->setPalette(palette);
    lblStartupScriptError->setVisible(false);

    QPushButton *btnStartupScriptApply = new QPushButton(tr("Apply"));
    connect(btnStartupScriptApply, SIGNAL(clicked()), this, SLOT(startupScriptChanged()));

    QHBoxLayout *layoutStartupScriptButton = new QHBoxLayout();
    layoutStartupScriptButton->addStretch(1);
    layoutStartupScriptButton->addWidget(btnStartupScriptApply);

    QVBoxLayout *layoutStartupScriptWidget = new QVBoxLayout();
    layoutStartupScriptWidget->addWidget(txtStartupScript);
    layoutStartupScriptWidget->addWidget(lblStartupScriptError);
    layoutStartupScriptWidget->addLayout(layoutStartupScriptButton);

    widStartupScript = new QWidget();
    widStartupScript->setLayout(layoutStartupScriptWidget);

    QVBoxLayout *layoutStartupScript = new QVBoxLayout();
    layoutStartupScript->addWidget(widStartupScript);

    grpStartupScript = new CollapsableGroupBoxButton(tr("Script"));
    connect(grpStartupScript, SIGNAL(collapseEvent(bool)), this, SLOT(startupScriptCollapse(bool)));
    grpStartupScript->setCollapsed(true);
    grpStartupScript->setLayout(layoutStartupScript);

    // problem widget
    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->setContentsMargins(0, 0, 0, 0);
    layoutArea->addWidget(grpGeneral);
    layoutArea->addWidget(grpFieldsToolbar);
    layoutArea->addWidget(grpCouplings);
    layoutArea->addWidget(grpHarmonicAnalysis);
    layoutArea->addWidget(grpTransientAnalysis);
    layoutArea->addWidget(grpStartupScript, 1);
    // layoutArea->addStretch(1);

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

void ProblemWidget::fillComboBox()
{
    cmbCoordinateType->clear();
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Planar), CoordinateType_Planar);
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Axisymmetric), CoordinateType_Axisymmetric);

    cmbMeshType->addItem(meshTypeString(MeshType_Triangle), MeshType_Triangle);
    cmbMeshType->addItem(meshTypeString(MeshType_Triangle_QuadFineDivision), MeshType_Triangle_QuadFineDivision);
    cmbMeshType->addItem(meshTypeString(MeshType_Triangle_QuadRoughDivision), MeshType_Triangle_QuadRoughDivision);
    cmbMeshType->addItem(meshTypeString(MeshType_Triangle_QuadJoin), MeshType_Triangle_QuadJoin);
    cmbMeshType->addItem(meshTypeString(MeshType_GMSH_Triangle), MeshType_GMSH_Triangle);
    cmbMeshType->addItem(meshTypeString(MeshType_GMSH_Quad), MeshType_GMSH_Quad);
    cmbMeshType->addItem(meshTypeString(MeshType_GMSH_QuadDelaunay_Experimental), MeshType_GMSH_QuadDelaunay_Experimental);

    cmbTransientMethod->addItem(timeStepMethodString(TimeStepMethod_Fixed), TimeStepMethod_Fixed);
    cmbTransientMethod->addItem(timeStepMethodString(TimeStepMethod_BDFTolerance), TimeStepMethod_BDFTolerance);
    cmbTransientMethod->addItem(timeStepMethodString(TimeStepMethod_BDFNumSteps), TimeStepMethod_BDFNumSteps);
}

void ProblemWidget::updateControls()
{
    // disconnect signals
    cmbCoordinateType->disconnect();
    cmbMeshType->disconnect();

    txtFrequency->disconnect();

    cmbTransientMethod->disconnect();
    txtTransientOrder->disconnect();
    txtTransientTimeTotal->disconnect();
    txtTransientTolerance->disconnect();
    chkTransientInitialStepSize->disconnect();
    txtTransientInitialStepSize->disconnect();
    txtTransientSteps->disconnect();
    txtStartupScript->disconnect();

    // main
    cmbCoordinateType->setCurrentIndex(cmbCoordinateType->findData(Agros2D::problem()->config()->coordinateType()));
    if (cmbCoordinateType->currentIndex() == -1)
        cmbCoordinateType->setCurrentIndex(0);

    // mesh type
    cmbMeshType->setCurrentIndex(cmbMeshType->findData(Agros2D::problem()->config()->meshType()));

    // harmonic magnetic
    grpHarmonicAnalysis->setVisible(Agros2D::problem()->isHarmonic());
    txtFrequency->setValue(Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble());
    // txtFrequency->setEnabled(Agros2D::problem()->isHarmonic());

    // transient
    grpTransientAnalysis->setVisible(Agros2D::problem()->isTransient());
    txtTransientSteps->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt());
    txtTransientTimeTotal->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble());
    txtTransientTolerance->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble());
    chkTransientInitialStepSize->setChecked(Agros2D::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0);
    txtTransientInitialStepSize->setEnabled(chkTransientInitialStepSize->isChecked());
    txtTransientInitialStepSize->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble());
    txtTransientOrder->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt());
    cmbTransientMethod->setCurrentIndex(cmbTransientMethod->findData((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()));
    if (cmbTransientMethod->currentIndex() == -1)
        cmbTransientMethod->setCurrentIndex(0);

    lblTransientTimeTotal->setText(QString("Total time (%1)").arg(Agros2D::problem()->timeUnit()));

    // couplings
    fieldsToolbar->refresh();
    couplingsWidget->refresh();

    grpCouplings->setVisible(Agros2D::problem()->couplingInfos().count() > 0);

    transientChanged();

    // startup script
    txtStartupScript->setPlainText(Agros2D::problem()->setting()->value(ProblemSetting::Problem_StartupScript).toString());
    grpStartupScript->setCollapsed(txtStartupScript->toPlainText().trimmed().isEmpty());

    // connect signals
    connect(cmbCoordinateType, SIGNAL(currentIndexChanged(int)), this, SLOT(changedWithClear()));
    connect(cmbMeshType, SIGNAL(currentIndexChanged(int)), this, SLOT(changedWithClear()));

    connect(txtFrequency, SIGNAL(textChanged(QString)), this, SLOT(changedWithClear()));

    // transient
    connect(cmbTransientMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(changedWithClear()));
    connect(txtTransientSteps, SIGNAL(valueChanged(int)), this, SLOT(changedWithClear()));
    connect(txtTransientTimeTotal, SIGNAL(textChanged(QString)), this, SLOT(changedWithClear()));
    connect(txtTransientOrder, SIGNAL(valueChanged(int)), this, SLOT(changedWithClear()));
    connect(txtTransientTolerance, SIGNAL(textChanged(QString)), this, SLOT(changedWithClear()));
    connect(chkTransientInitialStepSize, SIGNAL(stateChanged(int)), this, SLOT(changedWithClear()));
    connect(txtTransientInitialStepSize, SIGNAL(textChanged(QString)), this, SLOT(changedWithClear()));

    connect(cmbTransientMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(transientChanged()));
    connect(txtTransientSteps, SIGNAL(valueChanged(int)), this, SLOT(transientChanged()));
    connect(txtTransientTimeTotal, SIGNAL(textChanged(QString)), this, SLOT(transientChanged()));
    connect(txtTransientOrder, SIGNAL(valueChanged(int)), this, SLOT(transientChanged()));

    // startup
    // connect(txtStartupScript, SIGNAL(textChanged()), this, SLOT(changedWithClear()));
}

void ProblemWidget::changedWithClear()
{
    // save properties
    Agros2D::problem()->config()->blockSignals(true);

    Agros2D::problem()->config()->setCoordinateType((CoordinateType) cmbCoordinateType->itemData(cmbCoordinateType->currentIndex()).toInt());
    Agros2D::problem()->config()->setMeshType((MeshType) cmbMeshType->itemData(cmbMeshType->currentIndex()).toInt());

    Agros2D::problem()->config()->setValue(ProblemConfig::Frequency, txtFrequency->value());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeMethod, (TimeStepMethod) cmbTransientMethod->itemData(cmbTransientMethod->currentIndex()).toInt());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeOrder, txtTransientOrder->value());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeMethodTolerance, txtTransientTolerance->value());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeConstantTimeSteps, txtTransientSteps->value());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeTotal, txtTransientTimeTotal->value());
    txtTransientInitialStepSize->setEnabled(chkTransientInitialStepSize->isChecked());
    if (chkTransientInitialStepSize->isChecked())
    {
        Agros2D::problem()->config()->setValue(ProblemConfig::TimeInitialStepSize, txtTransientInitialStepSize->value());
    }
    else
    {
        txtTransientInitialStepSize->setValue(0.0);
        Agros2D::problem()->config()->setValue(ProblemConfig::TimeInitialStepSize, 0.0);
    }

    // save couplings
    couplingsWidget->save();

    Agros2D::problem()->config()->blockSignals(false);
    Agros2D::problem()->config()->refresh();

    emit changed();
}

void ProblemWidget::transientChanged()
{
    lblTransientTimeStep->setText(QString("%1 %2").arg(txtTransientTimeTotal->value() / txtTransientSteps->value()).arg(Agros2D::problem()->timeUnit()));

    if (((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_Fixed)
    {
        chkTransientInitialStepSize->setEnabled(false);
        txtTransientInitialStepSize->setEnabled(false);
        txtTransientTolerance->setEnabled(false);
        txtTransientSteps->setEnabled(true);

    }
    else if (((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFTolerance)
    {
        chkTransientInitialStepSize->setEnabled(true);
        txtTransientTolerance->setEnabled(true);
        txtTransientSteps->setEnabled(false);
    }
    else if (((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFNumSteps)
    {
        chkTransientInitialStepSize->setEnabled(true);
        txtTransientTolerance->setEnabled(false);
        txtTransientSteps->setEnabled(true);
    }

    if (((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_Fixed)
        lblTransientSteps->setText(tr("Number of steps:"));
    else
        lblTransientSteps->setText(tr("Aprox. number of steps:"));
}

void ProblemWidget::startupScriptCollapse(bool collapsed)
{
    widStartupScript->setVisible(!collapsed);
}

void ProblemWidget::startupScriptChanged()
{
    lblStartupScriptError->clear();
    lblStartupScriptError->setVisible(false);

    // run and check startup script
    if (!txtStartupScript->toPlainText().isEmpty())
    {
        currentPythonEngineAgros()->blockSignals(true);
        bool successfulRun = currentPythonEngineAgros()->runScript(txtStartupScript->toPlainText());
        currentPythonEngineAgros()->blockSignals(false);

        if (successfulRun)
        {
            Agros2D::problem()->setting()->setValue(ProblemSetting::Problem_StartupScript, txtStartupScript->toPlainText());

            // invalidate geometry
            Agros2D::scene()->invalidate();

            changedWithClear();
        }
        else
        {
            ErrorResult result = currentPythonEngineAgros()->parseError();
            lblStartupScriptError->setText(QObject::tr("Error: %1").arg(result.error()));
            lblStartupScriptError->setVisible(true);

            // original script
            currentPythonEngineAgros()->runScript(Agros2D::problem()->setting()->value(ProblemSetting::Problem_StartupScript).toString());
        }
    }
}
