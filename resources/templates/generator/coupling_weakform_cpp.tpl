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

#include "{{ID}}_weakform.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "hermes2d.h"
#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/bdf2.h"
 		
// quantities in volume weak forms:
{{#QUANTITY_INFO}}//{{QUANT_ID}} = ext[{{INDEX}}]
{{/QUANTITY_INFO}}

{{#VOLUME_MATRIX_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, int offsetI, int offsetJ, int* offsetPreviousTimeExt)
    : MatrixFormVolAgros<Scalar>(i, j, offsetI, offsetJ, offsetPreviousTimeExt)
{       
}


template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                          Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                                             Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;
}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    //return new {{FUNCTION_NAME}}(this->i, this->j, this->m_offsetI, this->m_offsetJ);
    return new {{FUNCTION_NAME}}(*this);
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerSource(const Marker *marker)
{
    FormAgrosInterface::setMarkerSource(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = this->m_markerSource->valueNakedPtr("{{VARIABLE}}");{{/VARIABLE_SOURCE}}
}
{{/VOLUME_MATRIX_SOURCE}}

// ***********************************************************************************************************************************

{{#VOLUME_VECTOR_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, int offsetI, int offsetJ, int* offsetPreviousTimeExt, int* offsetCouplingExt)
    : VectorFormVolAgros<Scalar>(i, offsetI, offsetJ, offsetPreviousTimeExt, offsetCouplingExt), j(j)
{
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                          Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;

    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                             Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;
}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    //return new {{FUNCTION_NAME}}(this->i, this->j, this->m_offsetI, this->m_offsetJ, this->m_offsetPreviousTimeExt, this->m_offsetCouplingExt);
    return new {{FUNCTION_NAME}}(*this);
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerSource(const Marker *marker)
{
    FormAgrosInterface::setMarkerSource(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = this->m_markerSource->valueNakedPtr("{{VARIABLE}}");{{/VARIABLE_SOURCE}}
}
{{/VOLUME_VECTOR_SOURCE}}

// ***********************************************************************************************************************************

{{#SOURCE}}template class {{FUNCTION_NAME}}<double>;
{{/SOURCE}}
