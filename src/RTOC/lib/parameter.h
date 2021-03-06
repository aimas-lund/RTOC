#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <typeinfo>
#include <vector>

#include "boost/serialization/nvp.hpp"
#include "boost/serialization/serialization.hpp"

/** @brief ParameterBase is used to encapsulate parameters used in a Process.
 *  Through getOptions, we can deduce the type of the parameter (see below) - which will enable the
 * GUI to spawn the correct editor for the value.
 * To maintain a unified interface into Parameter subclasses, getters and setters all have a string
 * based variant..
 */

/** @brief Interfacing into a ParameterBase
 * calling ParameterBase::getOptions() will return a string vector
 *
 * stringstream a:
 *  a[0] = type (ie. int, float, ENUM type
 *  a[1] = name
 *  the values following the type is dependant on the type.
 *  for float/int:
 *      a[1..2] = min, max values
 *  for enums:
 *      A[1..n] = strings denoting the valid options for the enum
 *
 */

#define SPLITCHAR '|'

#define RANGE_SET (1 << 0)
#define DEFAULT_SET (1 << 1)

using namespace std;

enum class ParameterType { Int, Double, Enum };

static void getParameterStreamToken(std::istream& s, std::string& str) {
    std::getline(s, str, SPLITCHAR);
}

class ParameterBase {
public:
    ParameterBase(string name) : m_name(name) {}
    void setModifiable(bool val) { m_isModifiable = val; }
    stringstream getOptions() const {
        stringstream s;
        for (const auto& i : m_options) {
            s << i << SPLITCHAR;
        }
        return s;
    }

    // String based setters and getters, used through GUI
    virtual const string getValueStr() const = 0;
    virtual void setValueStr(string) = 0;

protected:
    // Function that is called to update m_options whenever a range is changed/new options are
    // added. Must be implemented by subclass
    virtual void updateOptions() = 0;

    std::vector<string> m_options;
    string m_name;
    ParameterType m_type;
    bool m_isModifiable = true;
    int m_isInitialized = 0x00;  // Set when setting range or options. Assert is thrown if this is
                                 // not done, to force implementer to set range or options

private:
    // Private assignment operator. This ensures us that even though the Parameter is declared as a
    // public member in a Process, it cannot be assigned to
    void operator=(const ParameterBase&) {}
};

/** @anchor ValueParameter
 *  @brief Class for containing parameters which are numbers (floats, doubles, char, int).
 *  Contains range-information for the valid range of m_val
 */
template <typename T>
class ValueParameter : public ParameterBase {
public:
    ValueParameter(vector<ParameterBase*>& parent, std::string name, T initialValue = T());

    void setRange(T start, T stop);
    void setValue(T v);
    const T& getValue() const {
        if (!(m_isInitialized & RANGE_SET))
            throw std::runtime_error("parameter range/options not set.");
        if (!(m_isInitialized & DEFAULT_SET))
            throw std::runtime_error("parameter default not set.");
        return m_val;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_val);
    }

    const string getValueStr() const override;
    void setValueStr(string) override;

    void updateOptions() override;

private:
    T m_val;
    pair<T, T> m_range;
};

template <typename T>
const string ValueParameter<T>::getValueStr() const {
    return std::to_string(getValue());
}

template <typename T>
void ValueParameter<T>::setValueStr(string value) {
    if (typeid(T).name() == typeid(int).name()) {
        m_val = stoi(value);
    } else if (typeid(T).name() == typeid(double).name()) {
        m_val = stod(value);
    } else if (typeid(T).name() == typeid(float).name()) {
        m_val = stof(value);
    } else {
        m_val = T();
    }
}

template <typename T>
ValueParameter<T>::ValueParameter(vector<ParameterBase*>& parentProcessContainer, std::string name,
                                  T v)
    : ParameterBase(name), m_val(v) {
    // Upon construction, append parameter to parent process' Parameter container
    parentProcessContainer.push_back(this);
}

template <typename T>
void ValueParameter<T>::setRange(T start, T stop) {
    m_isInitialized |= RANGE_SET;
    m_range = std::pair<T, T>(start, stop);
    updateOptions();
}

template <typename T>
void ValueParameter<T>::setValue(T v) {
    m_isInitialized |= DEFAULT_SET;
    if (!((v >= m_range.first) && (v <= m_range.second))) {
        throw runtime_error("default value is outside parameter range");
    }
    m_val = v;
}

template <typename T>
void ValueParameter<T>::updateOptions() {
    m_options.resize(4);
    m_options[0] = typeid(T).name();
    m_options[1] = m_name;
    m_options[2] = to_string(m_range.first);
    m_options[3] = to_string(m_range.second);
}

/** @anchor EnumParameter
 *  @brief Used for options which are enumerated. Template argument type must be enumerateable - ie.
 *  an int must be constructible from T.
 */
template <typename T>
class EnumParameter : public ParameterBase {
public:
    EnumParameter(vector<ParameterBase*>& parent, std::string name, T initialValue = T());

    void setOptions(map<T, string>);

    void setValue(T v);
    const T& getValue() const {
        if (!(m_isInitialized & RANGE_SET))
            throw std::runtime_error("parameter range/options not set.");
        if (!(m_isInitialized & DEFAULT_SET))
            throw std::runtime_error("parameter default not set.");
        return m_val;
    }

    const string getValueStr() const override;
    void setValueStr(string) override;

    void updateOptions() override;

    template <typename Archive>
    void serialize(Archive& ar, unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_val);
    }

private:
    T m_val;
    map<T, string> m_enumOptions;
};

template <typename T>
EnumParameter<T>::EnumParameter(vector<ParameterBase*>& parentProcessContainer, std::string name,
                                T v)
    : ParameterBase(name), m_val(v) {
    // Check if an int is constructable from T.
    static_assert(std::is_constructible<int, T>::value,
                  "An integer must be constructable from the input template argument type.");
    // Upon construction, append parameter to parent process' Parameter container
    parentProcessContainer.push_back(this);
}

template <typename T>
void EnumParameter<T>::setOptions(map<T, string> options) {
    m_isInitialized |= RANGE_SET;
    m_enumOptions = options;
    updateOptions();
}

template <typename T>
void EnumParameter<T>::setValue(T v) {
    m_isInitialized |= DEFAULT_SET;
    m_val = v;
}

template <typename T>
void EnumParameter<T>::updateOptions() {
    m_options.resize(2 + m_enumOptions.size());
    m_options[0] = typeid(T).name();
    m_options[1] = m_name;
    int i = 2;
    for (const auto& item : m_enumOptions) {
        m_options[i] = item.second;
        i++;
    }
}

template <typename T>
const string EnumParameter<T>::getValueStr() const {
    if (!m_isInitialized)
        throw std::runtime_error("value parameter is not initialized");
    for (const auto& v : m_enumOptions) {
        if (v.first == m_val) {
            return v.second;
        }
    }
    return string("ERROR");
}

template <typename T>
void EnumParameter<T>::setValueStr(string value) {
    for (const auto& v : m_enumOptions) {
        if (v.second == value) {
            m_val = v.first;
        }
    }
}

// ----------------------

// Declare some smart pointer types for value and enum parameters
template <typename T>
using UniqueValuePtr = std::unique_ptr<ValueParameter<T>>;
template <typename T>
using SharedValuePtr = std::shared_ptr<ValueParameter<T>>;

template <typename T>
using UniqueEnumPtr = std::unique_ptr<EnumParameter<T>>;
template <typename T>
using SharedEnumPtr = std::shared_ptr<EnumParameter<T>>;

// Function for receiving range/options from a parameter
template <typename T>
void getRange(stringstream& stream, T& low, T& high) {
    stringstream tmp_stream;
    std::string tmp;
    getParameterStreamToken(stream, tmp);
    // let stringstream do the type-specific stream parsing
    tmp_stream << tmp;
    tmp_stream >> low;
    getParameterStreamToken(stream, tmp);
    tmp_stream.clear();
    tmp_stream << tmp;
    tmp_stream >> high;
}

inline void getOptions(stringstream& stream, std::vector<string>& options) {
    string str;
    getParameterStreamToken(stream, str);
    while (!str.empty()) {
        options.push_back(str);
        getParameterStreamToken(stream, str);
    }
}
