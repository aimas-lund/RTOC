#ifndef CELLSORTER_PROCESS_H
#define CELLSORTER_PROCESS_H

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <opencv/cv.hpp>
#include <string>

#include "datacontainer.h"

#include "experiment.h"
#include "matlab_ext.h"
#include "parameter.h"

#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"

namespace {
// Compile time string length generation
// https://stackoverflow.com/a/36390498/6714645
constexpr size_t ct_strlen(const char* s) noexcept {
    return *s ? 1 + ct_strlen(s + 1) : 0;
}
constexpr bool checkDisplayName(char const* name) {
    // Compile time verify that display name does not contain space characters (will break the
    // string stream parsing when loading parameters in gui)
    for (int i = 0; i < ct_strlen(name); i++) {
        if (name[i] == ' ') {
            return false;
        }
    }
    return true;
}

#define PARAMETER_CONTAINER m_parameters

// Shorthand macros for creating process parameter member values
#define CREATE_ENUM_PARM(type, name, displayName) \
    EnumParameter<type> name = EnumParameter<type>(PARAMETER_CONTAINER, displayName)

#define CREATE_ENUM_PARM_DEFAULT(type, name, displayName, default) \
    EnumParameter<type> name = EnumParameter<type>(PARAMETER_CONTAINER, displayName, default)

#define CREATE_VALUE_PARM(type, name, displayName) \
    ValueParameter<type> name = ValueParameter<type>(PARAMETER_CONTAINER, displayName)

#define CREATE_VALUE_PARM_DEFAULT(type, name, displayName, default) \
    ValueParameter<type> name = ValueParameter<type>(PARAMETER_CONTAINER, displayName s, default)
}  // namespace

class ProcessBase {
    friend class ProcessInterface;

public:
    virtual std::string getTypeName() const = 0;
    ProcessBase();
    virtual void
    doProcessing(cv::Mat& img, cv::Mat& bg,
                 const Experiment& props) const = 0;  // General function for doing processing.
    const std::vector<ParameterBase*>& getParameters() { return PARAMETER_CONTAINER; }
    static std::vector<std::string>& get_processes();

protected:
    std::vector<ParameterBase*> PARAMETER_CONTAINER;
};

// ---------------------------------------------------
/* Process registration and getters
 * The following classes aid in "before runtime" generation of a list of subclasses of Process,
 * which can be accessed by GUI
 * https://stackoverflow.com/questions/34858341/c-compile-time-list-of-subclasses-of-a-class
 *
 */
class ProcessNameGenerator {
public:
    ProcessNameGenerator(const std::string& name) {
        // Notify when the static member is created
        add_process(name);
        m_typeName = name;
    }

    std::string m_typeName;

private:
    void add_process(const std::string& name);
};

template <typename T>
class NameGenerator {
protected:
    // Static member in a template class
    NameGenerator() { (void)m; }
    static ProcessNameGenerator m;
};

template <typename T>
ProcessNameGenerator NameGenerator<T>::m = ProcessNameGenerator(typeid(T).name());
// ---------------------------------------------------

#define SETUP_PROCESS(ProcessType, displayName)                                        \
    ProcessType();                                                                     \
    void doProcessing(cv::Mat& img, cv::Mat&, const Experiment& props) const override; \
    static std::string getName() { return displayName; }                               \
    std::string getTypeName() const override { return typeid(ProcessType).name(); }

class Morph : public ProcessBase, public NameGenerator<Morph> {
public:
    SETUP_PROCESS(Morph, "Morphology")

    CREATE_ENUM_PARM(cv::MorphTypes, m_morphType, "Morphology_type");
    CREATE_VALUE_PARM(int, m_morphValueX, "Structural_element_X_axis");
    CREATE_VALUE_PARM(int, m_morphValueY, "Structural_element_Y_axis");

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_morphType);
        ar& BOOST_SERIALIZATION_NVP(m_morphValueX);
        ar& BOOST_SERIALIZATION_NVP(m_morphValueY);
    }
};

class Binarize : public ProcessBase, public NameGenerator<Binarize> {
public:
    SETUP_PROCESS(Binarize, "Binarize")

    CREATE_VALUE_PARM(double, m_edgeThreshold, "Edge_threshold");
    CREATE_VALUE_PARM(double, m_maxVal, "Maximum_binary_value");

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_edgeThreshold);
        ar& BOOST_SERIALIZATION_NVP(m_maxVal);
    }
};

class Normalize : public ProcessBase, public NameGenerator<Normalize> {
public:
    SETUP_PROCESS(Normalize, "Normalize")

    CREATE_VALUE_PARM(int, m_normalizeStrength, "Normalize_strength");

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_normalizeStrength);
    }
};

class SubtractBG : public ProcessBase, public NameGenerator<SubtractBG> {
public:
    SETUP_PROCESS(SubtractBG, "Subtract background")

    CREATE_VALUE_PARM(double, m_edgeThreshold, "Edge_threshold");

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_edgeThreshold);
    }
};

class Canny : public ProcessBase, public NameGenerator<Canny> {
public:
    SETUP_PROCESS(Canny, "Canny")

    CREATE_VALUE_PARM(double, m_lowThreshold, "Low_threshold");    // first threshold
    CREATE_VALUE_PARM(double, m_highThreshold, "High_threshold");  // second threshold

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_lowThreshold);
        ar& BOOST_SERIALIZATION_NVP(m_highThreshold);
    }
};

class ClearBorder : public ProcessBase, public NameGenerator<ClearBorder> {
public:
    SETUP_PROCESS(ClearBorder, "Clear Border")

    CREATE_VALUE_PARM(int, m_borderWidth, "Border_width");

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_borderWidth);
    }
};

class FloodFillProcess : public ProcessBase, public NameGenerator<FloodFillProcess> {
public:
    SETUP_PROCESS(FloodFillProcess, "Flood fill")
};

class PropFilter : public ProcessBase, public NameGenerator<PropFilter> {
public:
    SETUP_PROCESS(PropFilter, "Property filter")

    CREATE_ENUM_PARM_DEFAULT(matlab::regionPropTypes, m_regionPropsTypes, "Regionprop types",
                             matlab::regionPropTypes::Area);

    CREATE_VALUE_PARM(double, m_lowerLimit, "Lower_Limit");
    CREATE_VALUE_PARM(double, m_upperLimit, "Upper_Limit");

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_lowerLimit);
        ar& BOOST_SERIALIZATION_NVP(m_upperLimit);
    }
};

typedef std::vector<std::unique_ptr<ProcessBase>>* processContainerPtr;

#endif  // CELLSORTER_PROCESS_H
