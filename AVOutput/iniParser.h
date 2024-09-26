/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2022 Sky UK
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

//This is a ODM specific header file. To be depricated.
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>


class CIniFile 
{
	std::string m_path;
	std::string opt_path;
	boost::property_tree::ptree m_data;

public: 
	CIniFile(const std::string & filename, const std::string & filepath = "/etc/" )
	{
		opt_path = "/opt/panel/";
		m_path = filepath;
		m_path.append(filename);
		opt_path.append(filename);

		if(!boost::filesystem::exists( opt_path)) {
			std::cout << "DS HAL : Using " << m_path <<std::endl;
			boost::property_tree::ini_parser::read_ini(m_path, m_data);
		}
		else {
			std::cout << "DS HAL : Using " << opt_path << std::endl;
			boost::property_tree::ini_parser::read_ini(opt_path, m_data);
	        }
	}

	~CIniFile()
	{
	}

	template <typename T>
	T Get(const std::string & key)
	{
		return m_data.get<T>(key);
	}

	template <typename T>
	void Set(const std::string & key, const T & value){
		//TODO DD: Not required currently
             	//m_data.put(key, value);
	}
};
