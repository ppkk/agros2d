#ifndef COUPLING_H
#define COUPLING_H
#include "util.h"
#include "hermes2d.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

class FieldInfo;
class ProblemConfig;
class ParserFormExpression;

//namespace Hermes{
//namespace Module{

struct Coupling{
    // id
    std::string id;
    // name
    std::string name;
    // description
    std::string description;

    Coupling(CoordinateType coordinateType, CouplingType couplingType, AnalysisType sourceFieldAnalysis, AnalysisType targetFieldAnalysis);
    ~Coupling();

    // constants
    std::map<std::string, double> constants;

    // weak forms
    Hermes::vector<ParserFormExpression *> weakform_matrix_volume;
    Hermes::vector<ParserFormExpression *> weakform_vector_volume;

    mu::Parser *getParser();

    void read(std::string filename);
    void clear();

    inline CoordinateType get_coordinate_type() const { return m_coordinateType; }

private:
    CouplingType m_couplingType;
    CoordinateType m_coordinateType;
    AnalysisType m_sourceFieldAnalysis;
    AnalysisType m_targetFieldAnalysis;

};


//// available couplings
//QMap<QPair<FieldInfo*, FieldInfo*>, CouplingInfo* > availableCouplings();

bool isCouplingAvailable(FieldInfo* sourceField, FieldInfo* targetField);

// coupling factory
Coupling *couplingFactory(FieldInfo* sourceField, FieldInfo* targetField, CouplingType coupling_type,
                                           std::string filename_custom = "");


class CouplingInfo
{
public:
    CouplingInfo(FieldInfo* sourceField, FieldInfo* targetField);
    ~CouplingInfo();

    inline Coupling *coupling() const { return m_coupling; }

    QString couplingId() { if(m_coupling) return QString::fromStdString(m_coupling->id); return "None"; }
    CouplingType couplingType() { return m_couplingType; }
    inline bool isHard() { return m_couplingType == CouplingType_Hard;}
    inline bool isWeak() { return m_couplingType == CouplingType_Weak;}
    void setCouplingType(CouplingType couplingType);

    inline FieldInfo* sourceField() {return m_sourceField; }
    inline FieldInfo* targetField() {return m_targetField; }

    bool isRelated(FieldInfo* fieldInfo) { return((fieldInfo == sourceField()) || (fieldInfo == targetField())); }

    /// weakforms
    WeakFormsType weakFormsType;

    /// reloads the Coupling ("module"). Should be called when couplingType or AnalysisType of either fields changes
    void reload();

    /// goes through field infos and adds/removes coupling infos accordingly
    static void synchronizeCouplings(const QMap<QString, FieldInfo *>& fieldInfos, QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* >& couplingInfos);

    LinearityType linearityType();

private:
    /// module
    Coupling *m_coupling;

    /// pointer to problem info
    ProblemConfig *m_problemInfo;

    FieldInfo* m_sourceField;
    FieldInfo* m_targetField;

//    /// unique coupling info
//    QString m_couplingId;

    /// coupling type
    CouplingType m_couplingType;

};


//}
//}

#endif // COUPLING_H