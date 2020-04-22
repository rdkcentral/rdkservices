/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 */
 
#ifndef __CUSTOMCLASS_H
#define __CUSTOMCLASS_H

#include "Module.h"

#include "JavaScriptFunction.h"

#include <WPE/WebKit.h>
#include <vector>

namespace WPEFramework {
namespace JavaScript {

    class ClassDefinition {
    private:
        ClassDefinition() = delete;
        ClassDefinition(const ClassDefinition& copy) = delete;
        ClassDefinition& operator=(const ClassDefinition& copy) = delete;

        typedef std::map<string, ClassDefinition> ClassMap;
        typedef std::vector<const JavaScriptFunction*> FunctionVector;

    public:
        typedef Core::IteratorMapType<ClassMap, ClassDefinition&, string> Iterator;
        typedef Core::IteratorType<FunctionVector, const JavaScriptFunction*> FunctionIterator;

        // These are the only viable constructor, but only via the static creation method !!!
        ClassDefinition(const string& identifier);
        ~ClassDefinition() {}

    public:
        static ClassMap& getClassMap();
        static ClassDefinition& Instance(const string& className);
        inline static Iterator GetClassDefinitions()
        {
            return (Iterator(getClassMap()));
        }

        void Add(const JavaScriptFunction* javaScriptFunction);
        void Remove(const JavaScriptFunction* javaScriptFunction);

        FunctionIterator GetFunctions()
        {
            return (FunctionIterator(_customFunctions));
        }
        const string& GetClassName() const
        {
            return (_className);
        }
        const string& GetExtName() const
        {
            return (_extName);
        }

    private:
        FunctionVector _customFunctions;

        // JavaScript class properties.
        std::string _className;
        std::string _extName;

        // Instance declared in main.cpp, as this needs to be initialized before
        // a static in this cpp unit is called!!!
        //static ClassMap _classes;
    };
}
}

#endif // __CUSTOMCLASS_H
