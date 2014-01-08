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

#include "enums.h"

#include "util.h"
#include "util/global.h"
#include "scene.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

// QMap lookup is faster than in a QMap for less than about 10 elements
static QMap<CoordinateType, QString> coordinateTypeList;
static QMap<PhysicFieldVariableComp, QString> physicFieldVariableCompList;
static QMap<SceneViewPost3DMode, QString> sceneViewPost3DModeList;
static QMap<WeakFormKind, QString> weakFormList;
static QMap<WeakFormVariant, QString> weakFormVariantList;
static QMap<AdaptivityType, QString> adaptivityTypeList;
static QMap<AdaptivityStoppingCriterionType, QString> adaptivityStoppingCriterionTypeList;
static QMap<Hermes::Hermes2D::NormType, QString> adaptivityNormTypeList;
static QMap<TimeStepMethod, QString> timeStepMethodList;
static QMap<SolutionMode, QString> solutionTypeList;
static QMap<AnalysisType, QString> analysisTypeList;
static QMap<CouplingType, QString> couplingTypeList;
static QMap<LinearityType, QString> linearityTypeList;
static QMap<DampingType, QString> dampingTypeList;
static QMap<MeshType, QString> meshTypeList;
static QMap<Hermes::MatrixSolverType, QString> matrixSolverTypeList;
static QMap<Hermes::Algebra::MatrixExportFormat, QString> dumpFormatList;
static QMap<Hermes::Hermes2D::SpaceType, QString> spaceTypeList;
static QMap<PaletteType, QString> paletteTypeList;
static QMap<PaletteOrderType, QString> paletteOrderTypeList;
static QMap<VectorType, QString> vectorTypeList;
static QMap<VectorCenter, QString> vectorCenterList;
static QMap<DataTableType, QString> dataTableTypeList;
static QMap<SpecialFunctionType, QString> specialFunctionTypeList;
static QMap<Hermes::ButcherTableType, QString> butcherTableTypeList;
static QMap<Hermes::Solvers::IterSolverType, QString> iterLinearSolverMethodList;
static QMap<Hermes::Solvers::PreconditionerType, QString> iterLinearSolverPreconditionerTypeList;

QStringList coordinateTypeStringKeys() { return coordinateTypeList.values(); }
QString coordinateTypeToStringKey(CoordinateType coordinateType) { return coordinateTypeList[coordinateType]; }
CoordinateType coordinateTypeFromStringKey(const QString &coordinateType) { return coordinateTypeList.key(coordinateType); }

QStringList analysisTypeStringKeys() { return analysisTypeList.values(); }
QString analysisTypeToStringKey(AnalysisType analysisType) { return analysisTypeList[analysisType]; }
AnalysisType analysisTypeFromStringKey(const QString &analysisType) { return analysisTypeList.key(analysisType); }

QStringList couplingTypeStringKeys() { return couplingTypeList.values(); }
QString couplingTypeToStringKey(CouplingType couplingType) { return couplingTypeList[couplingType]; }
CouplingType couplingTypeFromStringKey(const QString &couplingType) { return couplingTypeList.key(couplingType); }

QStringList weakFormStringKeys() { return weakFormList.values(); }
QString weakFormToStringKey(WeakFormKind weakForm) { return weakFormList[weakForm]; }
WeakFormKind weakFormFromStringKey(const QString &weakForm) { return weakFormList.key(weakForm); }

QStringList weakFormVariantStringKeys() { return weakFormVariantList.values(); }
QString weakFormVariantToStringKey(WeakFormVariant weakFormVariant) { return weakFormVariantList[weakFormVariant]; }
WeakFormVariant weakFormVariantFromStringKey(const QString &weakFormVariant) { return weakFormVariantList.key(weakFormVariant); }

QStringList meshTypeStringKeys() { return meshTypeList.values(); }
QString meshTypeToStringKey(MeshType meshType) { return meshTypeList[meshType]; }
MeshType meshTypeFromStringKey(const QString &meshType) { return meshTypeList.key(meshType); }

QStringList physicFieldVariableCompTypeStringKeys() { return physicFieldVariableCompList.values(); }
QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp) { return physicFieldVariableCompList[physicFieldVariableComp]; }
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp) { return physicFieldVariableCompList.key(physicFieldVariableComp); }

QStringList adaptivityTypeStringKeys() { return adaptivityTypeList.values(); }
QString adaptivityTypeToStringKey(AdaptivityType adaptivityType) { return adaptivityTypeList[adaptivityType]; }
AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType) { return adaptivityTypeList.key(adaptivityType); }

QStringList adaptivityStoppingCriterionTypeStringKeys() { return adaptivityStoppingCriterionTypeList.values(); }
QString adaptivityStoppingCriterionTypeToStringKey(AdaptivityStoppingCriterionType adaptivityStoppingCriterionType) { return adaptivityStoppingCriterionTypeList[adaptivityStoppingCriterionType]; }
AdaptivityStoppingCriterionType adaptivityStoppingCriterionFromStringKey(const QString &adaptivityStoppingCriterionType) { return adaptivityStoppingCriterionTypeList.key(adaptivityStoppingCriterionType); }

QStringList adaptivityNormTypeStringKeys() { return adaptivityNormTypeList.values(); }
QString adaptivityNormTypeToStringKey(Hermes::Hermes2D::NormType adaptivityNormType) { return adaptivityNormTypeList[adaptivityNormType]; }
Hermes::Hermes2D::NormType adaptivityNormTypeFromStringKey(const QString &adaptivityNormType) { return adaptivityNormTypeList.key(adaptivityNormType); }

QStringList timeStepMethodStringKeys() { return timeStepMethodList.values(); }
QString timeStepMethodToStringKey(TimeStepMethod timeStepMethod) { return timeStepMethodList[timeStepMethod]; }
TimeStepMethod timeStepMethodFromStringKey(const QString &timeStepMethod) { return timeStepMethodList.key(timeStepMethod); }

QStringList solutionTypeStringKeys() { return solutionTypeList.values(); }
QString solutionTypeToStringKey(SolutionMode solutionType) { return solutionTypeList[solutionType]; }
SolutionMode solutionTypeFromStringKey(const QString &solutionType) { return solutionTypeList.key(solutionType); }

QStringList linearityTypeStringKeys() { return linearityTypeList.values(); }
QString linearityTypeToStringKey(LinearityType linearityType) { return linearityTypeList[linearityType]; }
LinearityType linearityTypeFromStringKey(const QString &linearityType) { return linearityTypeList.key(linearityType); }

QStringList dampingTypeStringKeys() { return dampingTypeList.values(); }
QString dampingTypeToStringKey(DampingType dampingType) { return dampingTypeList[dampingType]; }
DampingType dampingTypeFromStringKey(const QString &dampingType) { return dampingTypeList.key(dampingType); }

QStringList matrixSolverTypeStringKeys() { return matrixSolverTypeList.values(); }
QString matrixSolverTypeToStringKey(Hermes::MatrixSolverType matrixSolverType) { return matrixSolverTypeList[matrixSolverType]; }
Hermes::MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType) { return matrixSolverTypeList.key(matrixSolverType); }

QStringList dumpFormatStringKeys() { return dumpFormatList.values(); }
QString dumpFormatToStringKey(Hermes::Algebra::MatrixExportFormat format) { return dumpFormatList[format]; }
Hermes::Algebra::MatrixExportFormat dumpFormatFromStringKey(const QString &format) { return dumpFormatList.key(format); }

QStringList spaceTypeStringKeys() { return spaceTypeList.values(); }
QString spaceTypeToStringKey(Hermes::Hermes2D::SpaceType spaceType) { return spaceTypeList[spaceType]; }
Hermes::Hermes2D::SpaceType spaceTypeFromStringKey(const QString &spaceType) { return spaceTypeList.key(spaceType); }

QStringList sceneViewPost3DModeStringKeys() { return sceneViewPost3DModeList.values(); }
QString sceneViewPost3DModeToStringKey(SceneViewPost3DMode sceneViewPost3DMode) { return sceneViewPost3DModeList[sceneViewPost3DMode]; }
SceneViewPost3DMode sceneViewPost3DModeFromStringKey(const QString &sceneViewPost3DMode) { return sceneViewPost3DModeList.key(sceneViewPost3DMode); }

QStringList paletteTypeStringKeys() { return paletteTypeList.values(); }
QString paletteTypeToStringKey(PaletteType paletteType) { return paletteTypeList[paletteType]; }
PaletteType paletteTypeFromStringKey(const QString &paletteType) { return paletteTypeList.key(paletteType); }

QStringList paletteOrderTypeStringKeys() { return paletteOrderTypeList.values(); }
QString paletteOrderTypeToStringKey(PaletteOrderType paletteType) { return paletteOrderTypeList[paletteType]; }
PaletteOrderType paletteOrderTypeFromStringKey(const QString &paletteType) { return paletteOrderTypeList.key(paletteType); }

QStringList vectorTypeStringKeys() { return vectorTypeList.values(); }
QString vectorTypeToStringKey(VectorType vectorType) { return vectorTypeList[vectorType]; }
VectorType vectorTypeFromStringKey(const QString &vectorType) { return vectorTypeList.key(vectorType); }

QStringList vectorCenterStringKeys() { return vectorCenterList.values(); }
QString vectorCenterToStringKey(VectorCenter vectorCenter) { return vectorCenterList[vectorCenter]; }
VectorCenter vectorCenterFromStringKey(const QString &vectorCenter) { return vectorCenterList.key(vectorCenter); }

QStringList dataTableTypeStringKeys() { return dataTableTypeList.values(); }
QString dataTableTypeToStringKey(DataTableType dataTableType) { return dataTableTypeList[dataTableType]; }
DataTableType dataTableTypeFromStringKey(const QString &dataTableType) { return dataTableTypeList.key(dataTableType); }

QStringList specialFunctionTypeStringKeys() { return specialFunctionTypeList.values(); }
QString specialFunctionTypeToStringKey(SpecialFunctionType specialFunctionType) { return specialFunctionTypeList[specialFunctionType]; }
SpecialFunctionType specialFunctionTypeFromStringKey(const QString &specialFunctionType) { return specialFunctionTypeList.key(specialFunctionType); }

QStringList butcherTableTypeStringKeys() { return butcherTableTypeList.values(); }
QString butcherTableTypeToStringKey(Hermes::ButcherTableType tableType) { return butcherTableTypeList[tableType]; }
Hermes::ButcherTableType butcherTableTypeFromStringKey(const QString &tableType) { return butcherTableTypeList.key(tableType); }

QStringList iterLinearSolverMethodStringKeys() { return iterLinearSolverMethodList.values(); }
QString iterLinearSolverMethodToStringKey(Hermes::Solvers::IterSolverType type) { return iterLinearSolverMethodList[type]; }
Hermes::Solvers::IterSolverType iterLinearSolverMethodFromStringKey(const QString &type) { return iterLinearSolverMethodList.key(type); }

QStringList iterLinearSolverPreconditionerTypeStringKeys() { return iterLinearSolverPreconditionerTypeList.values(); }
QString iterLinearSolverPreconditionerTypeToStringKey(Hermes::Solvers::PreconditionerType type) { return iterLinearSolverPreconditionerTypeList[type]; }
Hermes::Solvers::PreconditionerType iterLinearSolverPreconditionerTypeFromStringKey(const QString &type) { return iterLinearSolverPreconditionerTypeList.key(type); }

void initLists()
{
    // coordinate list
    coordinateTypeList.insert(CoordinateType_Planar, "planar");
    coordinateTypeList.insert(CoordinateType_Axisymmetric, "axisymmetric");

    // Analysis Type
    analysisTypeList.insert(AnalysisType_SteadyState, "steadystate");
    analysisTypeList.insert(AnalysisType_Transient, "transient");
    analysisTypeList.insert(AnalysisType_Harmonic, "harmonic");

    // coupling type
    couplingTypeList.insert(CouplingType_Hard, "hard");
    couplingTypeList.insert(CouplingType_Weak, "weak");
    couplingTypeList.insert(CouplingType_None, "none");

    // Weak form type
    weakFormList.insert(WeakForm_MatVol, "matvol");
    weakFormList.insert(WeakForm_MatSurf, "matsur");
    weakFormList.insert(WeakForm_VecVol, "vecvol");
    weakFormList.insert(WeakForm_VecSurf, "vecsur");

    // Weak form variant
    weakFormVariantList.insert(WeakFormVariant_Normal, "normal");
    weakFormVariantList.insert(WeakFormVariant_Residual, "residual");
    weakFormVariantList.insert(WeakFormVariant_TimeResidual, "time_residual");

    // Mesh Type
    meshTypeList.insert(MeshType_Triangle, "triangle");
    meshTypeList.insert(MeshType_Triangle_QuadFineDivision, "triangle_quad_fine_division");
    meshTypeList.insert(MeshType_Triangle_QuadRoughDivision, "triangle_quad_rough_division");
    meshTypeList.insert(MeshType_Triangle_QuadJoin, "triangle_quad_join");
    meshTypeList.insert(MeshType_GMSH_Triangle, "gmsh_triangle");
    meshTypeList.insert(MeshType_GMSH_Quad, "gmsh_quad");
    meshTypeList.insert(MeshType_GMSH_QuadDelaunay_Experimental, "gmsh_quad_delaunay");

    timeStepMethodList.insert(TimeStepMethod_Fixed, "fixed");
    timeStepMethodList.insert(TimeStepMethod_BDFTolerance, "adaptive");
    timeStepMethodList.insert(TimeStepMethod_BDFNumSteps, "adaptive_numsteps");
    //    timeStepMethodList.insert(TimeStepMethod_BDF2, "bdf2_adaptive");
    //    timeStepMethodList.insert(TimeStepMethod_BDF2Combine, "bdf2_combine");
    //    timeStepMethodList.insert(TimeStepMethod_FixedBDF2B, "fixed_bdf2b");
    //    timeStepMethodList.insert(TimeStepMethod_FixedCombine, "fixed_combine");

    // PHYSICFIELDVARIABLECOMP
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Scalar, "scalar");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Magnitude, "magnitude");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_X, "x");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Y, "y");

    // post3d
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_None, "none");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_ScalarView3D, "scalar");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_ScalarView3DSolid, "scalarsolid");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_Model, "model");

    // ADAPTIVITYTYPE
    adaptivityTypeList.insert(AdaptivityType_None, "disabled");
    adaptivityTypeList.insert(AdaptivityType_H, "h-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_P, "p-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_HP, "hp-adaptivity");

    // AdaptivityStoppingCriterionType
    adaptivityStoppingCriterionTypeList.insert(AdaptivityStoppingCriterionType_Cumulative, "cumulative");
    adaptivityStoppingCriterionTypeList.insert(AdaptivityStoppingCriterionType_SingleElement, "singleelement");
    adaptivityStoppingCriterionTypeList.insert(AdaptivityStoppingCriterionType_Levels, "levels");

    // ADAPTIVITYNORMTYPE
    adaptivityNormTypeList.insert(Hermes::Hermes2D::HERMES_H1_NORM, "h1_norm");
    adaptivityNormTypeList.insert(Hermes::Hermes2D::HERMES_L2_NORM, "l2_norm");
    adaptivityNormTypeList.insert(Hermes::Hermes2D::HERMES_H1_SEMINORM, "h1_seminorm");

    // SolutionType
    solutionTypeList.insert(SolutionMode_Normal, "normal");
    solutionTypeList.insert(SolutionMode_Reference, "reference");
    //solutionTypeList.insert(SolutionMode_Finer, "finer");

    // MatrixSolverType
    matrixSolverTypeList.insert(Hermes::SOLVER_EMPTY, "empty");
    matrixSolverTypeList.insert(Hermes::SOLVER_UMFPACK, "umfpack");
    matrixSolverTypeList.insert(Hermes::SOLVER_PARALUTION_ITERATIVE, "paralution_iterative");
    matrixSolverTypeList.insert(Hermes::SOLVER_PARALUTION_AMG, "paralution_amg");
#ifdef WITH_MUMPS
    matrixSolverTypeList.insert(Hermes::SOLVER_MUMPS, "mumps");
#endif
#ifdef WITH_SUPERLU
    matrixSolverTypeList.insert(Hermes::SOLVER_SUPERLU, "superlu");
#endif
#ifdef WITH_PETSC
    matrixSolverTypeList.insert(Hermes::SOLVER_PETSC, "petsc");
#endif
#ifdef HAVE_AMESOS
    matrixSolverTypeList.insert(Hermes::SOLVER_AMESOS, "trilinos_amesos");
#endif
#ifdef HAVE_AZTECOO
    matrixSolverTypeList.insert(Hermes::SOLVER_AZTECOO, "trilinos_aztecoo");
#endif
    matrixSolverTypeList.insert(Hermes::SOLVER_EXTERNAL, "external");

    // dump format
    dumpFormatList.insert(Hermes::Algebra::EXPORT_FORMAT_PLAIN_ASCII, "plain_ascii");
    dumpFormatList.insert(Hermes::Algebra::EXPORT_FORMAT_MATLAB_MATIO, "matlab_mat");
    dumpFormatList.insert(Hermes::Algebra::EXPORT_FORMAT_MATRIX_MARKET, "matrix_market");

    // space
    spaceTypeList.insert(Hermes::Hermes2D::HERMES_H1_SPACE, "h1");
    spaceTypeList.insert(Hermes::Hermes2D::HERMES_HCURL_SPACE, "hcurl");
    spaceTypeList.insert(Hermes::Hermes2D::HERMES_HDIV_SPACE, "hdiv");
    spaceTypeList.insert(Hermes::Hermes2D::HERMES_L2_SPACE, "l2");
    spaceTypeList.insert(Hermes::Hermes2D::HERMES_L2_MARKERWISE_CONST_SPACE, "const");

    // LinearityType
    linearityTypeList.insert(LinearityType_Linear, "linear");
    linearityTypeList.insert(LinearityType_Picard, "picard");
    linearityTypeList.insert(LinearityType_Newton, "newton");

    // DampingType
    dampingTypeList.insert(DampingType_Off, "disabled");
    dampingTypeList.insert(DampingType_Automatic, "automatic");
    dampingTypeList.insert(DampingType_Fixed, "fixed");

    // PaletteType
    paletteTypeList.insert(Palette_Agros2D, "agros2d");
    paletteTypeList.insert(Palette_Jet, "jet");
    paletteTypeList.insert(Palette_Copper, "copper");
    paletteTypeList.insert(Palette_Hot, "hot");
    paletteTypeList.insert(Palette_Bone, "bone");
    paletteTypeList.insert(Palette_Pink, "pink");
    paletteTypeList.insert(Palette_Spring, "spring");
    paletteTypeList.insert(Palette_Summer, "summer");
    paletteTypeList.insert(Palette_Autumn, "autumn");
    paletteTypeList.insert(Palette_Winter, "winter");
    paletteTypeList.insert(Palette_HSV, "hsv");
    paletteTypeList.insert(Palette_BWAsc, "bw_ascending");
    paletteTypeList.insert(Palette_BWDesc, "bw_descending");

    // PaletteOrderType
    paletteOrderTypeList.insert(PaletteOrder_Hermes, "hermes");
    paletteOrderTypeList.insert(PaletteOrder_Jet, "jet");
    paletteOrderTypeList.insert(PaletteOrder_Copper, "copper");
    paletteOrderTypeList.insert(PaletteOrder_Hot, "hot");
    paletteOrderTypeList.insert(PaletteOrder_Bone, "bone");
    paletteOrderTypeList.insert(PaletteOrder_Pink, "pink");
    paletteOrderTypeList.insert(PaletteOrder_Spring, "spring");
    paletteOrderTypeList.insert(PaletteOrder_Summer, "summer");
    paletteOrderTypeList.insert(PaletteOrder_Autumn, "autumn");
    paletteOrderTypeList.insert(PaletteOrder_Winter, "winter");
    paletteOrderTypeList.insert(PaletteOrder_HSV, "hsv");
    paletteOrderTypeList.insert(PaletteOrder_BWAsc, "bw_ascending");
    paletteOrderTypeList.insert(PaletteOrder_BWDesc, "bw_descending");

    // VectorType
    vectorTypeList.insert(VectorType_Arrow, "arrow");
    vectorTypeList.insert(VectorType_Cone, "cone");

    // VectorCenter
    vectorCenterList.insert(VectorCenter_Tail, "tail");
    vectorCenterList.insert(VectorCenter_Head, "head");
    vectorCenterList.insert(VectorCenter_Center, "center");

    // DataTableType
    dataTableTypeList.insert(DataTableType_CubicSpline, "cubic_spline");
    dataTableTypeList.insert(DataTableType_PiecewiseLinear, "piecewise_linear");
    dataTableTypeList.insert(DataTableType_Constant, "constant");

    // SpecialFunctionType
    specialFunctionTypeList.insert(SpecialFunctionType_Constant, "constant");
    specialFunctionTypeList.insert(SpecialFunctionType_Function1D, "function_1d");

    // ButcherTableType
    butcherTableTypeList.insert(Hermes::Explicit_HEUN_EULER_2_12_embedded, "heun-euler");
    butcherTableTypeList.insert(Hermes::Explicit_BOGACKI_SHAMPINE_4_23_embedded, "bogacki-shampine");
    butcherTableTypeList.insert(Hermes::Explicit_FEHLBERG_6_45_embedded, "fehlberg");
    butcherTableTypeList.insert(Hermes::Explicit_CASH_KARP_6_45_embedded, "cash-karp");
    butcherTableTypeList.insert(Hermes::Explicit_DORMAND_PRINCE_7_45_embedded, "dormand-prince");

    // Iterative solver
    iterLinearSolverMethodList.insert(Hermes::Solvers::CG, "cg");
    iterLinearSolverMethodList.insert(Hermes::Solvers::GMRES, "gmres");
    iterLinearSolverMethodList.insert(Hermes::Solvers::BiCGStab, "bicgstab");
    iterLinearSolverMethodList.insert(Hermes::Solvers::CR, "cr");
    // iterLinearSolverMethodList.insert(Hermes::Solvers::IDR, "idr");

    iterLinearSolverPreconditionerTypeList.insert(Hermes::Solvers::Jacobi, "jacobi");
    iterLinearSolverPreconditionerTypeList.insert(Hermes::Solvers::ILU, "ilu");
    iterLinearSolverPreconditionerTypeList.insert(Hermes::Solvers::MultiColoredSGS, "multicoloredsgs");
    iterLinearSolverPreconditionerTypeList.insert(Hermes::Solvers::MultiColoredILU, "multicoloredilu");
    iterLinearSolverPreconditionerTypeList.insert(Hermes::Solvers::SaddlePoint, "saddlepoint");
    // iterLinearSolverPreconditionerTypeList.insert(Hermes::Solvers::AIChebyshev, "aichebyshev");
    iterLinearSolverPreconditionerTypeList.insert(Hermes::Solvers::IC, "ic");
    iterLinearSolverPreconditionerTypeList.insert(Hermes::Solvers::MultiElimination, "multielimination");
}

QString errorNormString(Hermes::Hermes2D::NormType projNormType)
{
    switch (projNormType)
    {
    case Hermes::Hermes2D::HERMES_H1_NORM:
        return QObject::tr("H1 norm");
    case Hermes::Hermes2D::HERMES_L2_NORM:
        return QObject::tr("L2 norm");
    case Hermes::Hermes2D::HERMES_H1_SEMINORM:
        return QObject::tr("H1 seminorm");
    case Hermes::Hermes2D::HERMES_HDIV_NORM:
        return QObject::tr("Hdiv norm");
    case Hermes::Hermes2D::HERMES_HCURL_NORM:
        return QObject::tr("Hcurl norm");
    case Hermes::Hermes2D::HERMES_UNSET_NORM:
        return QObject::tr("Custom norm");
    default:
        std::cerr << "Norm '" + QString::number(projNormType).toStdString() + "' is not implemented. QString errorNormString(ProjNormType projNormType)" << endl;
        throw;
    }
}

QString analysisTypeString(AnalysisType analysisType)
{
    switch (analysisType)
    {
    case AnalysisType_SteadyState:
        return QObject::tr("Steady state");
    case AnalysisType_Transient:
        return QObject::tr("Transient");
    case AnalysisType_Harmonic:
        return QObject::tr("Harmonic");
    default:
        std::cerr << "Analysis type '" + QString::number(analysisType).toStdString() + "' is not implemented. analysisTypeString(AnalysisType analysisType)" << endl;
        throw;
    }
}

QString couplingTypeString(CouplingType couplingType)
{
    switch (couplingType)
    {
    case CouplingType_None:
        return QObject::tr("Not used");
    case CouplingType_Hard:
        return QObject::tr("Hard");
    case CouplingType_Weak:
        return QObject::tr("Weak");
    default:
        std::cerr << "Coupling type '" + QString::number(couplingType).toStdString() + "' is not implemented. couplingTypeString(CouplingType couplingType)" << endl;
        throw;
    }
}

QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    case PhysicFieldVariableComp_Scalar:
        return QObject::tr("Scalar");
    case PhysicFieldVariableComp_Magnitude:
        return QObject::tr("Magnitude");
    case PhysicFieldVariableComp_X:
        return Agros2D::problem()->config()->labelX();
    case PhysicFieldVariableComp_Y:
        return Agros2D::problem()->config()->labelY();
    default:
        return QObject::tr("Undefined");
    }
}

QString coordinateTypeString(CoordinateType coordinateType)
{
    return ((coordinateType == CoordinateType_Planar) ? QObject::tr("Planar") : QObject::tr("Axisymmetric"));
}

QString solutionTypeString(SolutionMode solutionMode)
{
    switch (solutionMode)
    {
    case SolutionMode_Normal:
        return QObject::tr("Normal");
    case SolutionMode_Reference:
        return QObject::tr("Reference");
    case SolutionMode_Finer:
        return QObject::tr("Finer"); // used to choose reference if exists, normal otherwise
    default:
        std::cerr << "Solution mode '" + QString::number(solutionMode).toStdString() + "' is not implemented. solutionTypeString(SolutionMode solutionMode)" << endl;
        throw;
    }
}

QString adaptivityTypeString(AdaptivityType adaptivityType)
{
    switch (adaptivityType)
    {
    case AdaptivityType_None:
        return QObject::tr("Disabled");
    case AdaptivityType_H:
        return QObject::tr("h-adaptivity");
    case AdaptivityType_P:
        return QObject::tr("p-adaptivity");
    case AdaptivityType_HP:
        return QObject::tr("hp-adaptivity");
    default:
        std::cerr << "Adaptivity type '" + QString::number(adaptivityType).toStdString() + "' is not implemented. adaptivityTypeString(AdaptivityType adaptivityType)" << endl;
        throw;
    }
}

QString adaptivityStoppingCriterionTypeString(AdaptivityStoppingCriterionType adaptivityStoppingCriterionType)
{
    switch (adaptivityStoppingCriterionType)
    {
    case AdaptivityStoppingCriterionType_Cumulative:
        return QObject::tr("Cumulative");
    case AdaptivityStoppingCriterionType_SingleElement:
        return QObject::tr("Single element");
    case AdaptivityStoppingCriterionType_Levels:
        return QObject::tr("Levels");
    default:
        std::cerr << "Adaptivity stopping criterion type '" + QString::number(adaptivityStoppingCriterionType).toStdString() + "' is not implemented. adaptivityStoppingCriterionTypeString(AdaptivityStoppingCriterionType adaptivityStoppingCriterionType)" << endl;
        throw;
    }
}

QString timeStepMethodString(TimeStepMethod timeStepMethod)
{
    switch (timeStepMethod)
    {
    case TimeStepMethod_Fixed:
        return QObject::tr("Fixed step");
    case TimeStepMethod_BDFTolerance:
        return QObject::tr("Adaptive (tolerance)");
    case TimeStepMethod_BDFNumSteps:
        return QObject::tr("Adaptive (num steps)");
    default:
        std::cerr << "Time step method '" + QString::number(timeStepMethod).toStdString() + "' is not implemented. timeStepMethodString(TimeStepMethod timeStepMethod)" << endl;
        throw;
    }
}

QString weakFormString(WeakFormKind weakForm)
{
    switch (weakForm)
    {
    case WeakForm_MatVol:
        return QObject::tr("Matrix volume");
    case WeakForm_MatSurf:
        return QObject::tr("Matrix surface");
    case WeakForm_VecVol:
        return QObject::tr("Vector volume");
    case WeakForm_VecSurf:
        return QObject::tr("Vector surface");
    default:
        std::cerr << "Weak form '" + QString::number(weakForm).toStdString() + "' is not implemented. weakFormString(WeakForm weakForm)" << endl;
        throw;
    }
}

QString weakFormVariantString(WeakFormVariant weakFormVariant)
{
    switch (weakFormVariant)
    {
    case WeakFormVariant_Normal:
        return QObject::tr("Normal");
    case WeakFormVariant_Residual:
        return QObject::tr("Residual");
    case WeakFormVariant_TimeResidual:
        return QObject::tr("Time residual");
    default:
        std::cerr << "Weak form variant '" + QString::number(weakFormVariant).toStdString() + "' is not implemented. weakFormVariantString(WeakFormVariant weakFormVariant)" << endl;
        throw;
    }
}

QString meshTypeString(MeshType meshType)
{
    switch (meshType)
    {
    case MeshType_Triangle:
        return QObject::tr("Triangle - triangle");
    case MeshType_Triangle_QuadFineDivision:
        return QObject::tr("Triangle - quad fine div.");
    case MeshType_Triangle_QuadRoughDivision:
        return QObject::tr("Triangle - quad rough div.");
    case MeshType_Triangle_QuadJoin:
        return QObject::tr("Triangle - quad join");
    case MeshType_GMSH_Triangle:
        return QObject::tr("GMSH (exp.) - triangle");
    case MeshType_GMSH_Quad:
        return QObject::tr("GMSH (exp.) - quad");
    case MeshType_GMSH_QuadDelaunay_Experimental:
        return QObject::tr("GMSH (exp.) - quad Delaunay");
    default:
        std::cerr << "Mesh type '" + QString::number(meshType).toStdString() + "' is not implemented. meshTypeString(MeshType meshType)" << endl;
        throw;
    }
}

QString paletteTypeString(PaletteType paletteType)
{
    switch (paletteType)
    {
    case Palette_Agros2D:
        return QObject::tr("Agros2D");
    case Palette_Jet:
        return QObject::tr("Jet");
    case Palette_Copper:
        return QObject::tr("Copper");
    case Palette_Hot:
        return QObject::tr("Hot");
    case Palette_Cool:
        return QObject::tr("Cool");
    case Palette_Bone:
        return QObject::tr("Bone");
    case Palette_Pink:
        return QObject::tr("Pink");
    case Palette_Spring:
        return QObject::tr("Spring");
    case Palette_Summer:
        return QObject::tr("Summer");
    case Palette_Autumn:
        return QObject::tr("Autumn");
    case Palette_Winter:
        return QObject::tr("Winter");
    case Palette_HSV:
        return QObject::tr("HSV");
    case Palette_BWAsc:
        return QObject::tr("B/W ascending");
    case Palette_BWDesc:
        return QObject::tr("B/W descending");
    default:
        std::cerr << "Palette type '" + QString::number(paletteType).toStdString() + "' is not implemented. paletteTypeString(PaletteType paletteType)" << endl;
        throw;
    }
}

QString vectorTypeString(VectorType vectorType)
{
    switch (vectorType)
    {
    case VectorType_Arrow:
        return QObject::tr("Arrow");
    case VectorType_Cone:
        return QObject::tr("Cone");
    default:
        std::cerr << "Vector type '" + QString::number(vectorType).toStdString() + "' is not implemented. vectorTypeString(VectorType vectorType)" << endl;
        throw;
    }
}

QString vectorCenterString(VectorCenter vectorCenter)
{
    switch (vectorCenter)
    {
    case VectorCenter_Tail:
        return QObject::tr("Tail");
    case VectorCenter_Head:
        return QObject::tr("Head");
    case VectorCenter_Center:
        return QObject::tr("Center");
    default:
        std::cerr << "Vector center '" + QString::number(vectorCenter).toStdString() + "' is not implemented. vectorCenterString(VectorCenter vectorCenter)" << endl;
        throw;
    }
}

QString matrixSolverTypeString(Hermes::MatrixSolverType matrixSolverType)
{
    switch (matrixSolverType)
    {
    case Hermes::SOLVER_EMPTY:
        return QObject::tr("EMPTY");
    case Hermes::SOLVER_UMFPACK:
        return QObject::tr("UMFPACK");
    case Hermes::SOLVER_PETSC:
        return QObject::tr("PETSc");
    case Hermes::SOLVER_MUMPS:
        return QObject::tr("MUMPS");
    case Hermes::SOLVER_SUPERLU:
        return QObject::tr("SuperLU");
    case Hermes::SOLVER_PARALUTION_ITERATIVE:
        return QObject::tr("PARALUTION iter. (exp.)");
    case Hermes::SOLVER_PARALUTION_AMG:
        return QObject::tr("PARALUTION AMG (exp.)");
    case Hermes::SOLVER_AMESOS:
        return QObject::tr("Trilinos/Amesos");
    case Hermes::SOLVER_AZTECOO:
        return QObject::tr("Trilinos/AztecOO");
    case Hermes::SOLVER_EXTERNAL:
        return QObject::tr("External (out of core)");
    default:
        std::cerr << "Matrix solver type '" + QString::number(matrixSolverType).toStdString() + "' is not implemented. matrixSolverTypeString(MatrixSolverType matrixSolverType)" << endl;
        throw;
    }
}

QString dumpFormatString(Hermes::Algebra::MatrixExportFormat format)
{
    switch (format)
    {
    case Hermes::Algebra::EXPORT_FORMAT_MATLAB_MATIO:
        return QObject::tr("MATLAB MAT");
    case Hermes::Algebra::EXPORT_FORMAT_MATRIX_MARKET:
        return QObject::tr("Matrix Market");
    case Hermes::Algebra::EXPORT_FORMAT_PLAIN_ASCII:
        return QObject::tr("Plain ASCII");
    default:
        std::cerr << "Matrix dump format '" + QString::number(format).toStdString() + "' is not implemented. dumpFormatString(Hermes::Algebra::MatrixExportFormat format)" << endl;
        throw;
    }
}

bool isMatrixSolverIterative(Hermes::MatrixSolverType type)
{
    return ((type == Hermes::SOLVER_PARALUTION_ITERATIVE) || (type == Hermes::SOLVER_PARALUTION_AMG));
}

QString linearityTypeString(LinearityType linearityType)
{
    switch (linearityType)
    {
    case LinearityType_Linear:
        return QObject::tr("Linear");
    case LinearityType_Picard:
        return QObject::tr("Picard's method");
    case LinearityType_Newton:
        return QObject::tr("Newton's method");
    default:
        std::cerr << "Linearity type '" + QString::number(linearityType).toStdString() + "' is not implemented. linearityTypeString(LinearityType linearityType)" << endl;
        throw;
    }
}

QString dampingTypeString(DampingType dampingType)
{
    switch (dampingType)
    {
    case DampingType_Off:
        return QObject::tr("No damping");
    case DampingType_Fixed:
        return QObject::tr("Fixed");
    case DampingType_Automatic:
        return QObject::tr("Automatic");
    default:
        std::cerr << "Damping type '" + QString::number(dampingType).toStdString() + "' is not implemented. dampingTypeString(DampingType dampingType)" << endl;
        throw;
    }
}

QString dataTableTypeString(DataTableType dataTableType)
{
    switch (dataTableType)
    {
    case DataTableType_CubicSpline:
        return QObject::tr("Cubic spline");
    case DataTableType_PiecewiseLinear:
        return QObject::tr("Piecewise linear");
    case DataTableType_Constant:
        return QObject::tr("Constant");
    default:
        std::cerr << "Data table type '" + QString::number(dataTableType).toStdString() + "' is not implemented. dataTableTypeString(DataTableType dataTableType)" << endl;
        throw;
    }
}

QString specialFunctionTypeString(SpecialFunctionType specialFunctionType)
{
    switch (specialFunctionType)
    {
    case SpecialFunctionType_Constant:
        return QObject::tr("Constant");
    case SpecialFunctionType_Function1D:
        return QObject::tr("1D function");
    default:
        std::cerr << "Data table type '" + QString::number(specialFunctionType).toStdString() + "' is not implemented. specialFunctionTypeString(SpecialFunctionType specialFunctionType)" << endl;
        throw;
    }
}

QString butcherTableTypeString(Hermes::ButcherTableType tableType)
{
    switch (tableType)
    {
    case Hermes::Explicit_HEUN_EULER_2_12_embedded:
        return QObject::tr("Heun-Euler (2,1)");
    case Hermes::Explicit_BOGACKI_SHAMPINE_4_23_embedded:
        return QObject::tr("Bogacki-Shampine (2,3)");
    case Hermes::Explicit_FEHLBERG_6_45_embedded:
        return QObject::tr("Fehlberg (4,5)");
    case Hermes::Explicit_CASH_KARP_6_45_embedded:
        return QObject::tr("Cash-Karp (4,5)");
    case Hermes::Explicit_DORMAND_PRINCE_7_45_embedded:
        return QObject::tr("Dormand-Prince (4,5)");
    default:
        std::cerr << "Butcher table type'" + QString::number(tableType).toStdString() + "' is not implemented. butcherTableTypeString(Hermes::ButcherTableType tableType)" << endl;
        throw;
    }
}

QString iterLinearSolverMethodString(Hermes::Solvers::IterSolverType type)
{
    switch (type)
    {
    case Hermes::Solvers::CG:
        return QObject::tr("CG");
    case Hermes::Solvers::GMRES:
        return QObject::tr("GMRES");
    case Hermes::Solvers::BiCGStab:
        return QObject::tr("BiCGStab");
    case Hermes::Solvers::IDR:
        return QObject::tr("IDR");
    case Hermes::Solvers::CR:
        return QObject::tr("CR");
    default:
        std::cerr << "Iterative solver method '" + QString::number(type).toStdString() + "' is not implemented. iterLinearSolverTypeString(Hermes::Solvers::ParalutionSolverType type)" << endl;
        throw;
    }
}

QString iterLinearSolverPreconditionerTypeString(PreconditionerType type)
{
    switch (type)
    {
    case Hermes::Solvers::Jacobi:
        return QObject::tr("Jacobi");
    case Hermes::Solvers::ILU:
        return QObject::tr("ILU");
    case Hermes::Solvers::MultiColoredSGS:
        return QObject::tr("MultiColoredSGS");
    case Hermes::Solvers::MultiColoredILU:
        return QObject::tr("MultiColoredILU");
    case Hermes::Solvers::IC:
        return QObject::tr("IC");
    case Hermes::Solvers::AIChebyshev:
        return QObject::tr("AIChebyshev");
    case Hermes::Solvers::MultiElimination:
        return QObject::tr("MultiElimination");
    case Hermes::Solvers::SaddlePoint:
        return QObject::tr("SaddlePoint");
    default:
        std::cerr << "Iterative solver preconditioner '" + QString::number(type).toStdString() + "' is not implemented. iterLinearSolverPreconditionerTypeString(Hermes::Solvers::PreconditionerType type)" << endl;
        throw;
    }
}
