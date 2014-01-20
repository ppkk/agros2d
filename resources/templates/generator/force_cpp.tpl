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

#include "{{ID}}_extfunction.h"
#include "{{ID}}_force.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"

bool hasForce{{CLASS}}(const FieldInfo *fieldInfo)
{
    {{#VARIABLE_SOURCE}}
    if ((fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
     && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
    {
        return true;
    }
    {{/VARIABLE_SOURCE}}

    return false;
}

Point3 force{{CLASS}}(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                      Hermes::Hermes2D::Element *element, SceneMaterial *material, const Point3 &point, const Point3 &velocity)
{
    int numberOfSolutions = fieldInfo->numberOfSolutions();

    FieldSolutionID fsid(fieldInfo, timeStep, adaptivityStep, solutionType);
    MultiArray<double> ma = Agros2D::solutionStore()->multiArray(fsid);

    {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}

    Point3 res;

    if (Agros2D::problem()->isSolved())
    {
        // update time functions
        if (fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(fieldInfo);
            Module::updateTimeFunctions(timeLevels[timeStep]);
        }

        // set variables
        double x = point.x;
        double y = point.y;

        double *value = new double[numberOfSolutions];
        double *dudx = new double[numberOfSolutions];
        double *dudy = new double[numberOfSolutions];

        for (int k = 0; k < numberOfSolutions; k++)
        {
            // point values
            // point values
            Hermes::Hermes2D::Func<double> *values = ma.solutions().at(k)->get_pt_value(point.x, point.y, true, element);
            if (!values)
            {
                throw AgrosException(QObject::tr("Point [%1, %2] does not lie in any element").arg(x).arg(y));

                delete [] value;
                delete [] dudx;
                delete [] dudy;

                return res;
            }

            double val;
            if ((fieldInfo->analysisType() == AnalysisType_Transient) && timeStep == 0)
                // const solution at first time step
                val = fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble();
            else
                val = values->val[0];

            // set variables
            value[k] = val;
            dudx[k] = values->dx[0];
            dudy[k] = values->dy[0];

            // values->free_fn();
            // values->free_ord();
            delete values;
        }

        {{#VARIABLE_SOURCE}}
        if ((fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
         && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
        {
            res.x = {{EXPRESSION_X}};
            res.y = {{EXPRESSION_Y}};
            res.z = {{EXPRESSION_Z}};
        }
        {{/VARIABLE_SOURCE}}

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }

    return res;
}
