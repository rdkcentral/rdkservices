
#pragma once

#include <string>
#include <memory>
#include <regex>
#include <vector>
#include <set>
#include <map>
#include <functional>
#include <type_traits>


namespace WPEFramework {
namespace Plugin {

    // Country and language codes from ISO-639/ISO-3166.
    static const std::set<std::string> expectedLanguageSetCollection = {
        "af", "af-ZA", "ar", "ar-AE", "ar-BH", "ar-DZ", "ar-EG", "ar-IQ", "ar-JO", "ar-KW", "ar-LB", "ar-LY",
        "ar-MA", "ar-OM", "ar-QA", "ar-SA", "ar-SY", "ar-TN", "ar-YE", "az", "az-AZ", "az-AZ",
        "be", "be-BY", "bg", "bg-BG", "bs-BA",
        "ca", "ca-ES", "cs", "cs-CZ", "cy", "cy-GB",
        "da", "da-DK", "de", "de-AT", "de-CH", "de-DE", "de-LI", "de-LU", "dv", "dv-MV",
        "el", "el-GR", "en", "en-AU", "en-BZ", "en-CA", "en-CB", "en-GB", "en-IE", "en-JM", "en-NZ", "en-PH",
        "en-TT", "en-US", "en-ZA", "en-ZW", "eo", "es", "es-AR", "es-BO", "es-CL", "es-CO", "es-CR", "es-DO",
        "es-EC", "es-ES", "es-ES", "es-GT", "es-HN", "es-MX", "es-NI", "es-PA", "es-PE", "es-PR", "es-PY", "es-SV",
        "es-UY", "es-VE", "et", "et-EE", "eu", "eu-ES",
        "fa", "fa-IR", "fi", "fi-FI", "fo", "fo-FO", "fr", "fr-BE", "fr-CA", "fr-CH", "fr-FR", "fr-LU", "fr-MC",
        "gl", "gl-ES", "gu", "gu-IN",
        "he", "he-IL", "hi", "hi-IN", "hr", "hr-BA", "hr-HR", "hu", "hu-HU", "hy", "hy-AM", "id", "id-ID",
        "is", "is-IS", "it", "it-CH", "it-IT",
        "ja", "ja-JP",
        "ka", "ka-GE", "kk", "kk-KZ", "kn", "kn-IN", "ko", "ko-KR", "kok", "kok-IN", "ky", "ky-KG",
        "lt", "lt-LT", "lv", "lv-LV",
        "mi", "mi-NZ", "mk", "mk-MK", "mn", "mn-MN", "mr", "mr-IN", "ms", "ms-BN", "ms-MY", "mt", "mt-MT",
        "nb", "nb-NO", "nl", "nl-BE", "nl-NL", "nn-NO", "ns", "ns-ZA",
        "pa", "pa-IN", "pl", "pl-PL", "ps", "ps-AR", "pt", "pt-BR", "pt-PT",
        "qu", "qu-BO", "qu-EC", "qu-PE",
        "ro", "ro-RO", "ru", "ru-RU",
        "sa", "sa-IN", "se", "se-FI", "se-FI", "se-FI", "se-NO", "se-NO", "se-NO", "se-SE", "se-SE",
        "se-SE", "sk", "sk-SK", "sl", "sl-SI", "sq", "sq-AL", "sr-BA", "sr-BA", "sr-SP", "sr-SP",
        "sv", "sv-FI", "sv-SE", "sw", "sw-KE", "syr", "syr-SY",
        "ta", "ta-IN", "te", "te-IN", "th", "th-TH", "tl", "tl-PH", "tn", "tn-ZA", "tr", "tr-TR", "tt", "tt-RU", "ts",
        "uk", "uk-UA", "ur", "ur-PK", "uz", "uz-UZ", "uz-UZ",
        "vi", "vi-VN",
        "xh", "xh-ZA",
        "zh", "zh-CN", "zh-HK", "zh-MO", "zh-SG", "zh-TW", "zu", "zu-ZA",""};

    static const std::set<std::string> expectedVoicesSetCollection = {"carol","amelie","Angelica","ava",""};

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
        ExpectedValues(std::set<T> values) : m_type(Discrete), m_values(values) {}
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
                m_pattern = copy.m_pattern;
                break;
            case Type::None:
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
                std::swap(m_values, copy.m_values);
                break;
            case Type::Range:
                std::swap(m_range, copy.m_range);
                break;
            case Type::RegExp:
                std::swap(m_pattern, copy.m_pattern);
                break;
            case Type::None:
                break;
            }

            return *this;
        };
        ExpectedValues(ExpectedValues &&copy)
        {
            operator=(std::forward(copy));
        };

        virtual ~ExpectedValues(){};

        ExpectedValues(T min, T max) : m_type(Range)
        {
            m_range.m_min = min;
            m_range.m_max = max;
        }
        ExpectedValues(std::string pattern) : m_type(RegExp), m_pattern(pattern) {}

        ExpectedValues(std::initializer_list<T> values) : m_type(Discrete), m_values(values) {}
        ExpectedValues(std::initializer_list<const char *> values) : m_type(Discrete)
        {
            for (auto &value : values)
                m_values.emplace(std::string(value));
        }

        ExpectedValues(std::vector<T> values) : m_type(Discrete), m_values(values) {}
        ExpectedValues(std::vector<const char *> values) : m_type(Discrete)
        {
            for (auto &value : values)
                m_values.emplace(std::string(value));
        }

        bool validate(T value) const
        {
            switch (m_type)
            {
            case Type::Discrete:
                return std::find(m_values.begin(), m_values.end(), value) != m_values.end();
            case Type::Range:
                return value >= m_range.m_min && value <= m_range.m_max;
            case Type::RegExp:
                return regexMatch(value);
            case Type::None:
                break;
            }
            return true;
        }

        bool validate(const char *value)
        {
            return validate(std::string(value));
        }

    private:
        bool regexMatch(const std::string &value) const
        {
            return std::regex_match(value, std::regex(m_pattern));
        }

        template <typename U>
        bool regexMatch(const U &value) const
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
        std::string m_pattern;
    };

    struct ValidatorBase
    {
        enum Type
        {
            UseExpectedValues,
            CustomValidation
        };

        virtual Type type() = 0;
        virtual ~ValidatorBase(){};
    };

    template <typename T>
    class Validator : public ValidatorBase
    {
    public:
        Validator() = delete;
        Validator(const Validator &) = delete;
        Validator(Validator &&) = delete;
        Validator &operator=(const Validator &) = delete;
        Validator &operator=(Validator &&) = delete;
        virtual ~Validator(){};

        virtual Type type() override
        {
            return m_type;
        }

        static std::shared_ptr<ValidatorBase> create(const ExpectedValues<T> &expectedValues)
        {
            return std::shared_ptr<ValidatorBase>(static_cast<ValidatorBase *>(new Validator<T>(expectedValues)));
        }

        static std::shared_ptr<ValidatorBase> create(std::function<bool(const T &)> func)
        {
            return std::shared_ptr<ValidatorBase>(static_cast<ValidatorBase *>(new Validator<T>(func)));
        }

    public:
        bool validate(T value) const
        {

            if (m_type == Type::CustomValidation)
                return m_func(value);
            else{
                bool result = m_expectedValues.validate(value);
                return result;
            }
        }

    private:
        Validator(ExpectedValues<T> expectedValues) : m_type(UseExpectedValues), m_expectedValues(expectedValues) {}
        Validator(std::function<bool(const T &)> &func) : m_type(CustomValidation), m_func(func) {}

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

    public:
        void addValidator(std::string name, std::shared_ptr<ValidatorBase> validator)
        {
            m_validators[name].push_back(validator);
        }

        template <typename T>
        void addValidator(std::string name,const ExpectedValues<T> &expectedValues)
        {
            addValidator(name, Validator<T>::create(expectedValues));
        }

        template <typename T>
        void addValidator(std::string name, std::function<bool(const T &)> func)
        {
            addValidator(name, Validator<T>::create(func));
        }

        template <typename T>
        bool validate(std::string name, const T &value)
        {
            auto it = m_validators.find(name);
            if (it != m_validators.end())
            {
                for (auto &strValidatorBase : it->second)
                {
                    auto validator = dynamic_cast<Validator<T> *>(strValidatorBase.get());
                    if (!validator)
                    {
                        continue;
                    }

                    if (!validator->validate(value))
                        return false;
                }
            }
            //  If no validator is found or validator is found and all validators return true
            return true;
        }

        bool validate(std::string name, const char *value)
        {
            return validate(name, std::string(value));
        }

    private:
        ValidatorMap m_validators;
    };

    struct InputValidation
    {
        InputValidation &operator=(const InputValidation &copy) = delete;
        InputValidation(const InputValidation &copy) = delete;
        InputValidation &operator=(InputValidation &&copy) = delete;
        InputValidation(InputValidation &&copy) = delete;
        ~InputValidation() = default;

        static ValidationManager &Instance()
        {
            static InputValidation s_instance;
            return s_instance.m_validationManager;
        }

        private:
            InputValidation() = default;

        private:
            ValidationManager m_validationManager;
    };

} // namespace Plugin
} // namespace WPEFramework
