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
 
#pragma once

#include "Module.h"

#include <regex>

// helper functions
namespace {
    
    void ReplaceString(std::string& subject, const std::string& search,const std::string& replace) 
    {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
             subject.replace(pos, search.length(), replace);
             pos += replace.length();
        }
    }
    
    std::string CreateRegex(const std::string& input)
    {
        std::string regex = input;
        
        // order of replacing is important
        ReplaceString(regex,"*","^[a-zA-Z0-9.]+$");
        ReplaceString(regex,".","\\.");
        
        return regex;
    }
    
    std::string CreateUrlRegex(const std::string& input)
    {
        std::string regex = input;
        
        // order of replacing is important
        ReplaceString(regex,"/","\\/");
        ReplaceString(regex,"[","\\[");
        ReplaceString(regex,"]","\\]");
        ReplaceString(regex,":*",":[0-9]+");
        ReplaceString(regex,"*:","[a-z]+:");
        ReplaceString(regex,".","\\.");
        ReplaceString(regex,"*","[a-zA-Z0-9\\.]+");
        regex.insert(regex.begin(),'(');
        regex.insert(regex.end(),')');
        
        return regex;
    }

}

namespace WPEFramework {
namespace Plugin {

    //Allow -> Check first
    //if Block then check for Block[] and block if present
    //else must be explicitly allowed

    class EXTERNAL AccessControlList {
    private:
        class EXTERNAL JSONACL : public Core::JSON::Container {
        public:
            class Config : public Core::JSON::Container {
            public:
                Config(const Config&) = delete;
                Config& operator=(const Config&) = delete;

                Config()
                {
                    Add(_T("allow"), &Allow);
                    Add(_T("block"), &Block);
                }
                virtual ~Config()
                {
                }

            public:
                Core::JSON::ArrayType<Core::JSON::String> Allow;
                Core::JSON::ArrayType<Core::JSON::String> Block;
            };
            class Role : public Core::JSON::Container {
            public:
                Role(const Role&) = delete;
                Role& operator=(const Role&) = delete;

                Role()
                    : Configuration()
                {
                    Add(_T("thunder"), &Configuration);
                }
                virtual ~Role()
                {
                }

            public:
                Config Configuration;
            };
            class Roles : public Core::JSON::Container {
            private:
                using RolesMap = std::map<string, Role>;

            public:
                using Iterator = Core::IteratorMapType<const RolesMap, const Role&, const string&, RolesMap::const_iterator>;

                Roles(const Roles&) = delete;
                Roles& operator=(const Roles&) = delete;

                Roles()
                    : _roles()
                {
                }
                virtual ~Roles()
                {
                }

                inline Iterator Elements() const
                {
                    return (Iterator(_roles));
                }

            private:
                virtual bool Request(const TCHAR label[])
                {
                    if (_roles.find(label) == _roles.end()) {
                        auto element = _roles.emplace(std::piecewise_construct,
                            std::forward_as_tuple(label),
                            std::forward_as_tuple());
                        Add(element.first->first.c_str(), &(element.first->second));
                    }
                    return (true);
                }

            private:
                RolesMap _roles;
            };
            class Group : public Core::JSON::Container {
            public:
                Group()
                    : URL()
                    , Role()
                {
                    Add(_T("url"), &URL);
                    Add(_T("role"), &Role);
                }
                Group(const Group& copy)
                    : URL()
                    , Role()
                {
                    Add(_T("url"), &URL);
                    Add(_T("role"), &Role);

                    URL = copy.URL;
                    Role = copy.Role;
                }
                virtual ~Group()
                {
                }

                Group& operator=(const Group& RHS)
                {
                    URL = RHS.URL;
                    Role = RHS.Role;

                    return (*this);
                }

            public:
                Core::JSON::String URL;
                Core::JSON::String Role;
            };

        public:
            JSONACL(const JSONACL&) = delete;
            JSONACL& operator=(const JSONACL&) = delete;
            JSONACL()
            {
                Add(_T("assign"), &Groups);
                Add(_T("roles"), &ACL);
            }
            virtual ~JSONACL()
            {
            }

        public:
            Core::JSON::ArrayType<Group> Groups;
            Roles ACL;
        };

    public:
        class Filter {
        public:
            Filter() = delete;
            Filter(const Filter&) = delete;
            Filter& operator=(const Filter&) = delete;

            Filter(const JSONACL::Config& filter)
            {
                Core::JSON::ArrayType<Core::JSON::String>::ConstIterator index(filter.Allow.Elements());
                std::string str;
                while (index.Next() == true) {
                    str = index.Current().Value();
                    _allow.emplace_back(CreateRegex(str));
                }
                index = (filter.Block.Elements());
                while (index.Next() == true) {
                    str = index.Current().Value();
                    _block.emplace_back(CreateRegex(str));
                }
            }
            ~Filter()
            {
            }

        public:
            bool Allowed(const string& method) const
            {
                bool allowed = false;
                if (_allowSet) {
                    std::list<string>::const_iterator index(_allow.begin());
                    while ((index != _allow.end()) && (allowed == false)) { 
                        std::regex expression(index->c_str());
                        std::smatch matchList;
                        allowed = std::regex_search(method, matchList, expression);
                        index++;
                    }
                } else {
                    allowed = true;
                    std::list<string>::const_iterator index(_block.begin());
                    while ((index != _block.end()) && (allowed == true)) {
                        std::regex expression(index->c_str());
                        std::smatch matchList;
                        allowed = !std::regex_search(method, matchList, expression);
                        index++;
                    }
                }
                return (allowed);
            }

        private:
            bool _allowSet;
            std::list<string> _allow;
            std::list<string> _block;
        };

        using URLList = std::list<std::pair<string, Filter&>>;
        using Iterator = Core::IteratorType<const std::list<string>, const string&, std::list<string>::const_iterator>;

    public:
        AccessControlList(const AccessControlList&) = delete;
        AccessControlList& operator=(const AccessControlList&) = delete;

        AccessControlList()
            : _urlMap()
            , _filterMap()
            , _unusedRoles()
            , _undefinedURLS()
        {
        }
        ~AccessControlList()
        {
        }

    public:
        inline Iterator Unreferenced() const
        {
            return (Iterator(_unusedRoles));
        }
        inline Iterator Undefined() const
        {
            return (Iterator(_undefinedURLS));
        }
        void Clear()
        {
            _urlMap.clear();
            _filterMap.clear();
            _unusedRoles.clear();
            _undefinedURLS.clear();
        }
        const Filter* FilterMapFromURL(const string& URL) const
        {
            const Filter* result = nullptr;
            std::smatch matchList;
            URLList::const_iterator index = _urlMap.begin();

            while ((index != _urlMap.end()) && (result == nullptr)) {
                // regex_search() for searching the regex pattern
                // 'r' in the string 's'. 'm' is flag for determining
                // matching behavior.
                std::regex expression(index->first.c_str());

                if (std::regex_search(URL, matchList, expression) == true) {
                    result = &(index->second);
                }
                index++;
            }

            return (result);
        }
        uint32_t Load(Core::File& source)
        {
            JSONACL controlList;
            Core::OptionalType<Core::JSON::Error> error;
            controlList.IElement::FromFile(source, error);
            if (error.IsSet() == true) {
                SYSLOG(Logging::ParsingError, (_T("Parsing failed with %s"), ErrorDisplayMessage(error.Value()).c_str()));
            }
            _unusedRoles.clear();

            JSONACL::Roles::Iterator rolesIndex = controlList.ACL.Elements();

            // Now iterate over the Rules
            while (rolesIndex.Next() == true) {
                const string& roleName = rolesIndex.Key();

                _unusedRoles.push_back(roleName);

                _filterMap.emplace(std::piecewise_construct,
                    std::forward_as_tuple(roleName),
                    std::forward_as_tuple(rolesIndex.Current().Configuration));
            }

            Core::JSON::ArrayType<JSONACL::Group>::Iterator index = controlList.Groups.Elements();

            // Let iterate over the groups
            while (index.Next() == true) {
                const string& role(index.Current().Role.Value());

                // Try to find the Role..
                std::map<string, Filter>::iterator selectedFilter(_filterMap.find(role));

                if (selectedFilter == _filterMap.end()) {
                    std::list<string>::iterator found = std::find(_undefinedURLS.begin(), _undefinedURLS.end(), role);
                    if (found == _undefinedURLS.end()) {
                        _undefinedURLS.push_front(role);
                    }
                } else {
                    Filter& entry(selectedFilter->second);
                    
                    // create regex for url
                    std::string url_regex = CreateUrlRegex(index.Current().URL.Value());
                    
                    _urlMap.emplace_back(std::pair<string, Filter&>(
                        url_regex, entry));

                    std::list<string>::iterator found = std::find(_unusedRoles.begin(), _unusedRoles.end(), role);

                    if (found != _unusedRoles.end()) {
                        _unusedRoles.erase(found);
                    }
                }
            }
            return ((_unusedRoles.empty() && _undefinedURLS.empty()) ? Core::ERROR_NONE : Core::ERROR_INCOMPLETE_CONFIG);
        }

    private:
	//_urlMap contains list of entries of urls under "groups" to the allow/block filters set for that role under "thunder"
        URLList _urlMap; 
        std::map<string, Filter> _filterMap;
        std::list<string> _unusedRoles;
        std::list<string> _undefinedURLS;
    };
}
}
