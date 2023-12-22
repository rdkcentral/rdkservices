/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#pragma once

#include <string>
#include <memory>
#include <regex>
#include <vector>
#include <set>
#include <map>
#include <functional>
#include <type_traits>
#include <initializer_list>

namespace Utils {

#define NON_COPYABLE(Type) \
    Type(Type &) = delete; \
    Type& operator=(Type &) = delete;\

#define NON_MOVABLE(Type) \
    Type(Type &&) = delete; \
    Type& operator=(Type &&) = delete;\

template <typename T>
class ExpectedValues
{
private:
    enum Type
    {
        None,
        Discrete,
        Range,
        RegExp
    };

public:
    ExpectedValues() = default;

    ExpectedValues &operator=(const ExpectedValues &copy)
    {
        m_type = copy.m_type;
        switch (m_type)
        {
        case Type::Discrete:
            for (auto &value : copy.m_values)
                m_values.emplace(value);
            break;
        case Type::Range:
            m_range = copy.m_range;
            break;
        case Type::RegExp:
            m_regex = copy.m_regex;
            break;
        case None:
        default:
            break;
        }

        return *this;
    };
    ExpectedValues(const ExpectedValues &copy)
    {
        operator=(copy);
    }

    ExpectedValues &operator=(ExpectedValues &&copy)
    {
        std::swap(m_type, copy.m_type);
        switch (m_type)
        {
        case Type::Discrete:
            m_values = std::move(copy.m_values);
            break;
        case Type::Range:
            m_range = copy.m_range;
            break;
        case Type::RegExp:
            m_regex = std::move(copy.m_regex);
            break;
        case None:
        default:
            break;
        }

        return *this;
    };
    ExpectedValues(ExpectedValues &&copy)
    {
        operator=(std::forward<ExpectedValues<T>>(copy));
    };

    virtual ~ExpectedValues(){};

    ExpectedValues(T min, T max) : m_type(Range), m_range({min, max}) {}
    ExpectedValues(std::string regexStr) : m_type(RegExp), m_regex(std::regex(std::move(regexStr))) {}
    ExpectedValues(std::regex regex) : m_type(RegExp), m_regex(std::move(regex)) {}
    ExpectedValues(std::set<T> values) : m_type(Discrete), m_values(std::move(values)) {}

    ExpectedValues(std::initializer_list<T> values) : m_type(Discrete), m_values(std::move(values)) {}
    ExpectedValues(std::initializer_list<const char *> values) : m_type(Discrete)
    {
        for (auto *value : values) {
            if (!value)
                continue;
            m_values.emplace(std::string(value));
        }
    }

    ExpectedValues(std::vector<T> values) : m_type(Discrete), m_values(std::move(values)) {}
    ExpectedValues(std::vector<const char *> values) : m_type(Discrete)
    {
        for (auto *value : values) {
            if (!value)
                continue;
            m_values.emplace(std::string(value));
        }
    }

    inline bool validate(const T &value) const
    {
        switch (m_type)
        {
        case Type::Discrete:
            return std::find(m_values.begin(), m_values.end(), value) != m_values.end();
        case Type::Range:
            return value >= m_range.m_min && value <= m_range.m_max;
        case Type::RegExp:
            return regexMatch(value);
        case None:
        default:
            return true;
        }
    }

    inline bool validate(const char *value)
    {
        return value ? validate(std::string(value)) : false;
    }

private:
    inline bool regexMatch(const std::string &value) const
    {
        return std::regex_match(value, m_regex);
    }

    template <typename U>
    inline bool regexMatch(const U &value) const
    {
        return regexMatch(std::to_string(value));
    }

private:
    Type m_type{None};

    struct
    {
        T m_min;
        T m_max;
    } m_range;
    std::set<T> m_values;
    std::regex m_regex;
};

struct ValidatorBase
{
    virtual ~ValidatorBase(){};
};

template <typename T>
class Validator : public ValidatorBase
{
    using FunctionType = std::function<bool(const T &)>;

    enum Type
    {
        None,
        UseExpectedValues,
        CustomValidation
    };

public:
    NON_COPYABLE(Validator);
    NON_MOVABLE(Validator);
    virtual ~Validator(){};

    inline static std::shared_ptr<ValidatorBase> create(ExpectedValues<T> &&expectedValues)
    {
        return std::shared_ptr<ValidatorBase>(static_cast<ValidatorBase *>(new Validator<T>(std::forward<ExpectedValues<T>>(expectedValues))));
    }

    inline static std::shared_ptr<ValidatorBase> create(FunctionType &&func)
    {
        return std::shared_ptr<ValidatorBase>(static_cast<ValidatorBase *>(new Validator<T>(std::forward<FunctionType>(func))));
    }

public:
    inline virtual bool validate(const T &value)
    {
        switch (m_type)
        {
            case Type::CustomValidation:
                return m_func(value);
                break;
            case Type::UseExpectedValues:
                return m_expectedValues.validate(value);
                break;
            case None:
            default:
                return false;
        }
    }

protected:
    Validator() : m_type(Type::None) {}

private:
    Validator(ExpectedValues<T> expectedValues) : m_type(UseExpectedValues), m_expectedValues(std::move(expectedValues)) {}
    Validator(std::function<bool(const T &)> func) : m_type(CustomValidation), m_func(std::move(func)) {}

private:
    Type m_type;

    union
    {
        ExpectedValues<T> m_expectedValues;
        std::function<bool(const T &)> m_func;
    };
};

class ValidationManager
{
public:
    using ValidatorMap = std::map<std::string, std::vector<std::shared_ptr<ValidatorBase>>>;
    using LoggerFunction = void(*)(const char *log);

public:
    inline void addValidator(std::string name, std::shared_ptr<ValidatorBase> validator)
    {
        m_validators[name].emplace_back(std::move(validator));
    }

    template <typename T>
    inline void addValidator(std::string name, ExpectedValues<T> &&expectedValues)
    {
        addValidator(name, Validator<T>::create(std::forward<ExpectedValues<T>>(expectedValues)));
    }

    template <typename T, typename FunctionType = std::function<bool(const T &)>>
    inline void addValidator(std::string name, FunctionType &&func)
    {
        addValidator(name, Validator<T>::create(std::forward<FunctionType>(func)));
    }

    template <typename T>
    inline bool validate(std::string name, const T &value)
    {
        auto it = m_validators.find(name);
        if (it != m_validators.end())
        {
            for (auto &strValidatorBase : it->second)
            {
                auto *validator = dynamic_cast<Validator<T> *>(strValidatorBase.get());
                if (validator == nullptr || !validator->validate(value))
                {
                    std::stringstream ss;
                    if (!validator)
                        ss << "Validator not found for key \"" << name << "\" with type \"" << typeid(T).name() << "\"" << std::endl;
                    ss << "Validation failed for key \"" << name << "\" with type \"" << typeid(T).name() << "\" & value \"" << value << "\"" << std::endl;

                    if (m_logger)
                        m_logger(ss.str().c_str());
                    else
                        std::cout << ss.str();

                    return false;
                }
            }

            return true;
        }

        return false;
    }

    inline bool validate(std::string name, const char *value)
    {
        return value ? validate(name, std::string(value)) : false;
    }

    void setLogger(LoggerFunction logger)
    {
        m_logger = logger;
    }

private:
    ValidatorMap m_validators;
    LoggerFunction m_logger { nullptr };
};

} // namespace Utils

#if 0
namespace {
    using namespace Utils;

    template <typename T>
    class MyCustomValidator : public Validator<T>
    {
    public:
        NON_COPYABLE(MyCustomValidator);
        NON_MOVABLE(MyCustomValidator);
        virtual ~MyCustomValidator(){};

        inline static std::shared_ptr<ValidatorBase> create()
        {
            return std::shared_ptr<ValidatorBase>(static_cast<ValidatorBase *>(new MyCustomValidator<T>()));
        }

    public:
        inline virtual bool validate(const T &value) override
        {
            return true;
        }

    private:
        MyCustomValidator() : Validator<T>() {}
    };

    ValidationManager validator;
    validator.addValidator("key", MyCustomValidator<std::string>::create());
}
#endif

