#pragma once

#if defined(DEBUG)

#include <cmath>
#include <string>
#include <iostream>
#include <sstream>

typedef size_t (*ValueInpector)(const std::string& tag, const float32_t valueToTest, const float32_t _min, float32_t _max, bool outDebugInfo);

inline size_t nanValueInspector(const std::string& tag, const float32_t valueToTest, const float32_t _min, float32_t _max, bool outDebugInfo)
{
    if(std::isnan(valueToTest))
    {
        if(outDebugInfo)
        {
            std::cerr << tag << ": nan" << std::endl;
        }
        return 1u;
    }

    return 0u;
}

inline size_t normalizedValueInspector(const std::string& tag, const float32_t valueToTest, const float32_t _min, float32_t _max, bool outDebugInfo)
{
    if(valueToTest > _max || valueToTest < _min)
    {
        if(outDebugInfo)
        {
            std::cerr << tag << ": out of bounds - value: " << valueToTest << " - boundaries: [" << _min << ", " << _max << "]" << std::endl;
        }
        return 1u;
    }

    return 0u;
}

inline size_t fullInspector(const std::string& tag, const float32_t valueToTest, const float32_t _min = -1.0f, float32_t _max = 1.0f, bool outDebugInfo = true)
{
    if(std::isnan(valueToTest))
    {
        if(outDebugInfo)
        {
            std::cerr << tag << ": nan" << std::endl;
        }
        return 1u;
    }
    else if(valueToTest > _max || valueToTest < _min)
    {
        if(outDebugInfo)
        {
            std::cerr << tag << ": out of bounds - value: " << valueToTest << " - boundaries: [" << _min << ", " << _max << "]" << std::endl;
        }
        return 1u;
    }

    return 0u;
}

#define SS_RESET(ss, prec, align)               ss.str(""); ss.precision(prec); ss << align; ss << std::fixed
#define SS_SPACE(ss, spc, nb, align, sep)       ss.fill(spc); ss.width(nb); ss << "" << sep
#define SS__TEXT(ss, spc, nb, align, sep, txt)  ss << align; ss.fill(spc); ss.width(nb); ss << txt << sep

class ValueInpectorDebugger
{
public:
    ValueInpectorDebugger() {}
    virtual ~ValueInpectorDebugger() {}

    virtual void dump(std::ostream& out, bool deepInspection = true, const std::string& tag = "") const = 0;
    virtual size_t inspect(ValueInpector inspector, bool deepInspection = true, const std::string& tag = "") const = 0;
};

#define INSPECTABLE(clazz) clazz : public ValueInpectorDebugger
#define IMPLEMENT_DUMP(code) \
public:\
    virtual void dump(std::ostream& out, bool deepInspection = true, const std::string& tag = "") const override\
    {\
        code\
    }

#define DUMP(clazz, out) clazz->dump(out, true, "")
#define DUMP2(clazz, out, tag) clazz->dump(out, true, tag)
#define FAST_DUMP(clazz, out, tag) clazz->dump(out, false, "")
#define FAST_DUMP2(clazz, out, tag) clazz->dump(out, false, tag)

#define IMPLEMENT_INSPECT(code) \
public:\
    virtual size_t inspect(ValueInpector inspector, bool deepInspection = true, const std::string& tag = "") const override\
    {\
        code\
    }

#define INSPECT(obj, inspector) obj->inspect(inspector, true)
#define INSPECT2(obj, inspector, deepInspection) obj->inspect(inspector, deepInspection)
#define FULL_INSPECT(obj, deepInspection) obj->inspect(fullInspector, deepInspection)
#define FULL_INSPECT2(obj, deepInspection, tag) obj->inspect(fullInspector, deepInspection, tag)

#else

#define INSPECTABLE(clazz) clazz
#define IMPLEMENT_DUMP(code)
#define IMPLEMENT_INSPECT(code) 

#define DUMP(clazz, out)
#define DUMP2(clazz, out, tag)
#define FAST_DUMP(clazz, out, tag)
#define FAST_DUMP2(clazz, out, tag)

#define INSPECT(obj, inspector)
#define INSPECT2(obj, inspector, deepInspection)
#define FULL_INSPECT(obj, deepInspection)
#define FULL_INSPECT2(obj, deepInspection, tag)

#endif