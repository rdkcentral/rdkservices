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
//namespace {
    
    void inline ReplaceString(string& subject, const string& search,const string& replace)
    {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != string::npos) {
             subject.replace(pos, search.length(), replace);
             pos += replace.length();
        }
    }
    
    string inline CreateRegex(const string& input)
    {
        string regex = input;
        
        // order of replacing is important
        ReplaceString(regex,"*","^[a-zA-Z0-9.]+$");
        ReplaceString(regex,".","\\.");
        
        return regex;
    }
    
    string inline CreateUrlRegex(const string& input)
    {
        string regex = input;
        
        // order of replacing is important
        ReplaceString(regex,"/","\\/");
        ReplaceString(regex,"[","\\[");
        ReplaceString(regex,"]","\\]");
        ReplaceString(regex,":*",":[0-9]+");
        ReplaceString(regex,"*:","[a-z]+:");
        ReplaceString(regex,".","\\.");
        ReplaceString(regex,"*","[a-zA-Z0-9\\.\\-]+");
        regex.insert(regex.begin(),'(');
        regex.insert(regex.end(),')');
        regex.insert(regex.begin(),'^');
        regex.insert(regex.end(),'$');
        
        return regex;
    }

    string inline GetUrlOrigin(const string& input)
    {
        // see https://tools.ietf.org/html/rfc3986
        auto path = input.find('/', input.find("//") + 2);
        auto fragment = input.rfind('#', path);
        auto end = fragment == string::npos ? path : fragment;
        auto query = input.rfind('?', end);
        if (query != string::npos)
            end = query;

        return input.substr(0, end);
    }
//}

namespace WPEFramework {
namespace Plugin {

    //Allow -> Check first
    //if Block then check for Block[] and block if present
    //else must be explicitly allowed

    // "xreapps.net": {
    //   "thunder": {
    //     "default": "blocked",
    //     "DeviceInfo": {
    //       "default": "allowed",
    //       "methods": [ "register", "unregister" ]
    //     }
    //   }
    // },

    class EXTERNAL AccessControlList {
    public:
        enum mode {
            BLOCKED,
            ALLOWED
        };
    private:
        class EXTERNAL JSONACL : public Core::JSON::Container {
        public:
            class Plugins : public Core::JSON::Container {
            public:
                class Rules : public Core::JSON::Container {
                public:
                    Rules(const Rules&) = delete;
                    Rules& operator=(const Rules&) = delete;

                    Rules()
                        : Core::JSON::Container()
                        , Default(BLOCKED)
                        , Methods()
                    {
                        Add(_T("default"), &Default);
                        Add(_T("methods"), &Methods);
                    }
                    ~Rules() override
                    {
                    }

                public:
                    Core::JSON::EnumType<mode> Default;
                    Core::JSON::ArrayType<Core::JSON::String> Methods;
                };
 
                using PluginsMap = std::map<string, Rules>;

            public:
                using Iterator = Core::IteratorMapType<const PluginsMap, const Rules&, const string&, PluginsMap::const_iterator>;

                Plugins(const Plugins&) = delete;
                Plugins& operator=(const Plugins&) = delete;

                Plugins()
                    : Core::JSON::Container()
                    , Default(BLOCKED)
                    , _plugins()
                {
                    Add(_T("default"), &Default);
                }
                ~Plugins() override
                {
                }

            public:
                Core::JSON::EnumType<mode> Default;

                inline Iterator Elements() const
                {
                    return (Iterator(_plugins));
                }

            private:
                virtual bool Request(const TCHAR label[])
                {
                    if (_plugins.find(label) == _plugins.end()) {
                        auto element = _plugins.emplace(std::piecewise_construct,
                            std::forward_as_tuple(label),
                            std::forward_as_tuple());
                        Add(element.first->first.c_str(), &(element.first->second));
                    }
                    return (true);
                }

            private:
                PluginsMap _plugins;
            };

            class Roles : public Core::JSON::Container {
            private:
                using RolesMap = std::map<string, Plugins>;

            public:
                using Iterator = Core::IteratorMapType<const RolesMap, const Plugins&, const string&, RolesMap::const_iterator>;

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
        private:
            class Plugin {
            public:
                Plugin() = delete;
                Plugin(const Plugin&) = delete;
                Plugin& operator= (const Plugin&) = delete;

                Plugin (const JSONACL::Plugins::Rules& rules)
                    : _defaultBlocked(rules.Default.Value() == mode::BLOCKED) 
                    , _methods() {
                    Core::JSON::ArrayType<Core::JSON::String>::ConstIterator index(rules.Methods.Elements());
                    while (index.Next() == true) {
                        string str = index.Current().Value();
                        _methods.emplace_back(CreateRegex(str));
                    }
                }
                ~Plugin() {
                }

            public:
                bool Allowed(const string& method) const
                {
                    bool found = false;

                    std::list<string>::const_iterator index(_methods.begin());

                    while ((index != _methods.end()) && (found == false)) { 
                        std::regex expression(index->c_str());
                        std::smatch matchList;
                        found = std::regex_search(method, matchList, expression);
                        if (found == false) {
                            index++;
                        }
                    }
                    return !(_defaultBlocked ^ found);
                }

            private:
                bool _defaultBlocked;
                std::list<string> _methods;
            };

        public:
            Filter() = delete;
            Filter(const Filter&) = delete;
            Filter& operator=(const Filter&) = delete;

            Filter(const JSONACL::Plugins& plugins)
                : _defaultBlocked(plugins.Default.Value() == mode::BLOCKED)
                , _plugins()
            {
                JSONACL::Plugins::Iterator index(plugins.Elements());
          
                while (index.Next() == true) {
                    _plugins.emplace(std::piecewise_construct,
                            std::forward_as_tuple(CreateRegex(index.Key())),
                            std::forward_as_tuple(index.Current()));
                }
            }
            ~Filter()
            {
            }

        public:
            bool Allowed(const string callsign, const string& method) const
            {
                bool pluginFound = false;

                std::map<string, Plugin>::const_iterator index(_plugins.begin());
                while ((index != _plugins.end()) && (pluginFound == false)) {
                    std::regex expression(index->first.c_str());
                    std::smatch matchList;
                    pluginFound = std::regex_search(callsign, matchList, expression);
                    if (pluginFound == false) {
                        index++;
                    }
                }

                return (pluginFound == false ? !_defaultBlocked : index->second.Allowed(method));
            }

        private:
            bool _defaultBlocked;
            std::map<string, Plugin> _plugins;
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
            auto origin = GetUrlOrigin(URL);

            const Filter* result = nullptr;
            std::smatch matchList;
            URLList::const_iterator index = _urlMap.begin();

            while ((index != _urlMap.end()) && (result == nullptr)) {
                // regex_search() for searching the regex pattern
                // 'r' in the string 's'. 'm' is flag for determining
                // matching behavior.
                std::regex expression(index->first.c_str());

                if (std::regex_search(origin, matchList, expression) == true) {
                    result = &(index->second);
                }
                else {
                    index++;
                }
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
                    std::forward_as_tuple(rolesIndex.Current()));
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
                    string url_regex = CreateUrlRegex(index.Current().URL.Value());
                    
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
