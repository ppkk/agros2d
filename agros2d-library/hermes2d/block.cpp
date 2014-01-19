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

#include "block.h"
#include "util/global.h"

#include "field.h"
#include "problem.h"
#include "coupling.h"
#include "scene.h"
#include "logview.h"
#include "solver.h"
#include "module.h"
#include "problem_config.h"
#include "plugin_interface.h"


Block::Block(QList<FieldInfo *> fieldInfos, QList<CouplingInfo*> couplings) :
    m_couplings(couplings), m_wf(NULL)
{
    foreach (FieldInfo *fi, fieldInfos)
    {
        Field* field = new Field(fi);

        foreach (CouplingInfo* couplingInfo, Agros2D::problem()->couplingInfos())
        {
            if (couplingInfo->isWeak() && (couplingInfo->targetField() == fi))
            {
                field->addCouplingInfo(couplingInfo);
            }
        }

        m_fields.append(field);
    }
}

Block::~Block()
{
    // clear fields
    foreach (Field *field, m_fields)
        delete field;
    m_fields.clear();

    // clear boundary conditions
    foreach (Hermes::Hermes2D::EssentialBCs<double> *bc, m_bcs)
    {
        for (Hermes::vector<Hermes::Hermes2D::EssentialBoundaryCondition<double> *>::const_iterator it = bc->begin(); it < bc->end(); ++it)
        {
            // delete bc
            delete *it;
        }

        delete bc;
    }
    m_bcs.clear();

    // delete weakform
    if (m_wf)
        delete m_wf;
    m_wf = NULL;
}

QList<FieldInfo* > Block::sourceFieldInfosCoupling() const
{
    QList<FieldInfo* > result;
    foreach(Field* field, m_fields)
    {
        foreach(CouplingInfo* couplingInfo, field->couplingInfos())
        {
            assert(couplingInfo->targetField() == field->fieldInfo());
            assert(couplingInfo->couplingType() == CouplingType_Weak);
            if(! result.contains(couplingInfo->sourceField()))
                result.push_back(couplingInfo->sourceField());
        }
    }

    foreach(CouplingInfo* couplingInfo, couplings())
        if(couplingInfo->couplingType() == CouplingType_Weak)
            assert(result.contains(couplingInfo->sourceField()));

    return result;
}

void Block::createBoundaryConditions()
{
    // todo: memory leak? boundary conditions are probably released in space, but really?
    m_bcs.clear();

    // essential boundary conditions
    for (int i = 0; i < numSolutions(); i++)
        m_bcs.push_back(new Hermes::Hermes2D::EssentialBCs<double>());

    m_exactSolutionFunctions.clear();

    foreach(Field* field, this->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        ProblemID problemId;

        problemId.targetFieldId = fieldInfo->fieldId();
        problemId.analysisTypeTarget = fieldInfo->analysisType();
        problemId.coordinateType = Agros2D::problem()->config()->coordinateType();
        problemId.linearityType = fieldInfo->linearityType();

        int index = 0;
        foreach(SceneEdge* edge, Agros2D::scene()->edges->items())
        {
            SceneBoundary *boundary = edge->marker(fieldInfo);

            if (boundary && (!boundary->isNone()))
            {
                Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

                foreach (FormInfo form, boundaryType.essential())
                {
                    // exact solution - Dirichlet BC
                    Hermes::Hermes2D::MeshFunctionSharedPtr<double> function = Hermes::Hermes2D::MeshFunctionSharedPtr<double>(fieldInfo->plugin()->exactSolution(problemId, &form, fieldInfo->initialMesh()));
                    static_cast<ExactSolutionScalarAgros<double> *>(function.get())->setMarkerTarget(boundary);

                    // save function - boundary pairs, so thay can be easily updated in each time step;
                    m_exactSolutionFunctions[function] = boundary;

                    Hermes::Hermes2D::EssentialBoundaryCondition<double> *custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(index).toStdString(), function);

                    this->bcs().at(form.i - 1 + this->offset(field))->add_boundary_condition(custom_form);
                    //  cout << "adding BC i: " << form->i - 1 + this->offset(field) << " ( form i " << form->i << ", " << this->offset(field) << "), expression: " << form->expression << endl;
                }
            }
            index++;
        }
    }
}

void Block::updateExactSolutionFunctions()
{
    foreach(Hermes::Hermes2D::MeshFunctionSharedPtr<double> function, m_exactSolutionFunctions.keys())
    {
        SceneBoundary* boundary = m_exactSolutionFunctions[function];
        static_cast<ExactSolutionScalarAgros<double> *>(function.get())->setMarkerTarget(boundary);
    }
}

void Block::setWeakForm(WeakFormAgros<double> *wf)
{
    if (m_wf)
        delete m_wf;
    m_wf = wf;
}

QSharedPointer<ProblemSolver<double> > Block::prepareSolver()
{
    QSharedPointer<ProblemSolver<double> > solver = QSharedPointer<ProblemSolver<double> >(new ProblemSolver<double>());

    foreach (Field* field, m_fields)
    {
        // evaluate all values
        if (!field->solveInitVariables())
        {
            return QSharedPointer<ProblemSolver<double> >();
        }
    }

    solver.data()->init(this);

    return solver;
}

bool Block::isTransient() const
{
    foreach (Field *field, m_fields)
    {
        if (field->fieldInfo()->analysisType() == AnalysisType_Transient)
            return true;
    }

    return false;
}

double Block::timeSkip() const
{
    double skip = 0.;
    foreach (Field *field, m_fields)
    {
        if (field->fieldInfo()->analysisType() == AnalysisType_Transient)
            continue;

        double sActual = field->fieldInfo()->value(FieldInfo::TransientTimeSkip).toDouble();
        if ((skip == 0.) || (sActual < skip))
            skip = sActual;
    }
    return skip;
}

AdaptivityType Block::adaptivityType() const
{
    AdaptivityType at = m_fields.at(0)->fieldInfo()->adaptivityType();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->adaptivityType() == at);
    }

    return at;
}

int Block::adaptivitySteps() const
{
    int as = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivitySteps).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivitySteps).toInt() == as);
    }

    return as;
}

int Block::adaptivityBackSteps() const
{
    int abs = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivityTransientBackSteps).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivityTransientBackSteps).toInt() == abs);
    }

    return abs;
}

int Block::adaptivityRedoneEach() const
{
    int re = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivityTransientRedoneEach).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivityTransientRedoneEach).toInt() == re);
    }

    return re;
}

double Block::adaptivityTolerance() const
{
    double tolerance = numeric_limits<double>::max();

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble() < tolerance)
            tolerance = fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble();
    }

    return tolerance;
}

AdaptivityStoppingCriterionType Block::adaptivityStoppingCriterionType() const
{
    AdaptivityStoppingCriterionType sc = (AdaptivityStoppingCriterionType) m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivityStoppingCriterion).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert((AdaptivityStoppingCriterionType) field->fieldInfo()->value(FieldInfo::AdaptivityStoppingCriterion).toInt() == sc);
    }

    return sc;
}

double Block::adaptivityThreshold() const
{
    double threshold = numeric_limits<double>::max();

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::AdaptivityThreshold).toDouble() < threshold)
            threshold = fieldInfo->value(FieldInfo::AdaptivityThreshold).toDouble();
    }

    return threshold;
}

bool Block::adaptivityUseAniso() const
{
    bool useAniso = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivityUseAniso).toBool();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivityUseAniso).toBool() == useAniso);
    }

    return useAniso;
}

bool Block::adaptivityFinerReference() const
{
    bool finerReference = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivityFinerReference).toBool();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivityFinerReference).toBool() == finerReference);
    }

    return finerReference;
}

int Block::numSolutions() const
{
    int num = 0;
    foreach (Field *field, m_fields)
        num += field->fieldInfo()->numberOfSolutions();

    return num;
}

int Block::offset(Field *fieldParam) const
{
    int offset = 0;

    foreach (Field* field, m_fields)
    {
        if (field == fieldParam)
            return offset;
        else
            offset += field->fieldInfo()->numberOfSolutions();
    }

    assert(0);
}

LinearityType Block::linearityType() const
{
    int linear = 0;
    int newton = 0;
    int picard = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->linearityType() == LinearityType_Linear)
            linear++;
        if (fieldInfo->linearityType() == LinearityType_Newton)
            newton++;
        if (fieldInfo->linearityType() == LinearityType_Picard)
            picard++;
    }
    assert(linear * newton * picard == 0); // all hard coupled fields has to be solved by the same method

    if (linear)
        return LinearityType_Linear;
    else if (newton)
        return  LinearityType_Newton;
    else if (picard)
        return  LinearityType_Picard;
    else
        assert(0);
}

Hermes::MatrixSolverType Block::matrixSolver() const
{
    Hermes::MatrixSolverType mt = m_fields.at(0)->fieldInfo()->matrixSolver();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->matrixSolver() == mt);
    }

    return mt;
}

double Block::nonlinearResidualNorm() const
{
    double tolerance = numeric_limits<double>::max();

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble() < tolerance)
            tolerance = fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble();
    }

    // if we do not want to use this stopping criterion, the value is 0.
    // At the present moment, we want, however, Newton solver to satisfy both conditions. Thus we set very high number.
    if(tolerance == 0)
        tolerance = 1e20;

    return tolerance;
}

double Block::nonlinearRelativeChangeOfSolutions() const
{
    double tolerance = numeric_limits<double>::max();

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() < tolerance)
            tolerance = fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble();
    }

    // if we do not want to use this stopping criterion, the value is 0.
    // At the present moment, we want, however, Newton solver to satisfy both conditions. Thus we set very high number.
    if(tolerance == 0)
        tolerance = 1e20;

    return tolerance;
}

DampingType Block::nonlinearDampingType() const
{
    DampingType blockDt = DampingType_Undefined;
    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        DampingType dt = (DampingType)fieldInfo->value(FieldInfo::NonlinearDampingType).toInt();
        assert(dt != DampingType_Undefined);
        if(blockDt != DampingType_Undefined)
            assert (blockDt == dt);
        blockDt = dt;
    }

    return blockDt;
}

double Block::nonlinearDampingCoeff() const
{
    double coeff = 1.0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble() < coeff)
            coeff = fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble();
    }

    return coeff;
}

bool Block::newtonReuseJacobian() const
{
    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (!fieldInfo->value(FieldInfo::NewtonReuseJacobian).toBool())
            return false;
    }

    return true;
}

int Block::nonlinearStepsToIncreaseDampingFactor() const
{
    int number = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt() > number)
            number = fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt();
    }

    return number;
}

double Block::newtonSufficientImprovementFactorForJacobianReuse() const
{
    double number = 1e10;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NewtonJacobianReuseRatio).toDouble() < number)
            number = fieldInfo->value(FieldInfo::NewtonJacobianReuseRatio).toDouble();
    }

    return number;
}

double Block::nonlinearDampingFactorDecreaseRatio() const
{
    double number = 1e10;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NonlinearDampingFactorDecreaseRatio).toDouble() < number)
            number = fieldInfo->value(FieldInfo::NonlinearDampingFactorDecreaseRatio).toDouble();
    }

    return number;
}

int Block::newtonMaxStepsWithReusedJacobian() const
{
    int number = 10;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt() < number)
            number = fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt();
    }

    return number;
}

bool Block::picardAndersonAcceleration() const
{
    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (!fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool())
            return false;
    }

    return true;
}

double Block::picardAndersonBeta() const
{
    double number = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        // TODO: check ">"
        if (fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble() > number)
            number = fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble();
    }

    return number;
}

int Block::picardAndersonNumberOfLastVectors() const
{
    int number = 1;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt() > number)
            number = fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt();
    }

    return number;
}

IterSolverType Block::iterLinearSolverType() const
{
    Hermes::Solvers::IterSolverType method
            = (Hermes::Solvers::IterSolverType) m_fields.at(0)->fieldInfo()->value(FieldInfo::LinearSolverIterMethod).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert((Hermes::Solvers::IterSolverType) field->fieldInfo()->value(FieldInfo::LinearSolverIterMethod).toInt() == method );
    }

    return method ;
}

Hermes::Solvers::PreconditionerType Block::iterPreconditionerType() const
{
    Hermes::Solvers::PreconditionerType type
            = (Hermes::Solvers::PreconditionerType) m_fields.at(0)->fieldInfo()->value(FieldInfo::LinearSolverIterPreconditioner).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert((Hermes::Solvers::PreconditionerType) field->fieldInfo()->value(FieldInfo::LinearSolverIterPreconditioner).toInt() == type );
    }

    return type ;
}

double Block::iterLinearSolverToleranceAbsolute() const
{
    double coeff = 1.0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble() < coeff)
            coeff = fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble();
    }

    return coeff;
}

int Block::iterLinearSolverIters() const
{
    int iters = 1;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt() > iters)
            iters = fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt();
    }

    return iters;
}

bool Block::contains(const FieldInfo *fieldInfo) const
{
    foreach(Field* field, m_fields)
    {
        if (field->fieldInfo() == fieldInfo)
            return true;
    }
    return false;
}

Field* Block::field(const FieldInfo *fieldInfo) const
{
    foreach (Field* field, m_fields)
    {
        if (fieldInfo == field->fieldInfo())
            return field;
    }

    return NULL;
}

QList<FieldInfo*> Block::fieldInfos() const
{
    QList<FieldInfo*> result;
    foreach(Field* field, m_fields)
        result.push_back(field->fieldInfo());

    return result;
}

Hermes::vector<Hermes::Hermes2D::NormType> Block::projNormTypeVector() const
{
    Hermes::vector<Hermes::Hermes2D::NormType> vec;

    foreach (Field* field, m_fields)
    {
        for (int comp = 0; comp < field->fieldInfo()->numberOfSolutions(); comp++)
        {
            Hermes::Hermes2D::SpaceType spaceType = field->fieldInfo()->spaces()[comp+1].type();
            if (spaceType == Hermes::Hermes2D::HERMES_H1_SPACE)
                vec.push_back(Hermes::Hermes2D::HERMES_H1_NORM);
            else if (spaceType == Hermes::Hermes2D::HERMES_L2_SPACE)
                vec.push_back(Hermes::Hermes2D::HERMES_L2_NORM);
            else if (spaceType == Hermes::Hermes2D::HERMES_L2_MARKERWISE_CONST_SPACE)
                vec.push_back(Hermes::Hermes2D::HERMES_L2_NORM);
            else
                assert(0);
        }
    }
    return vec;
}

ostream& operator<<(ostream& output, const Block& id)
{
    output << "Block ";
    return output;
}


