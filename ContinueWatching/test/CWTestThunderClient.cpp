/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#include<iostream>
#include <bits/stdc++.h>
#include<ctime>
#include<string>

#include "Module.h"

using namespace std;
using namespace WPEFramework;

JSONRPC::Client* remoteObject = NULL;
void showMenu()
{
	cout<<"Enter your choice\n";
	cout<<"1.setApplicationToken\n";
	cout<<"2.getApplicationToken\n";
	cout<<"3.deleteApplicationToken\n";
}

int main(int argc, char** argv)
{
	int choice,repeat=1;
	string value,token,tokenData,Cookie,intermediate;
	string esn="CMCSTX1003-0000010A03001F34-44AAF5286490";

	time_t result = time(NULL);
       	char *time1;
	time1=asctime(localtime(&result));
	time1[strlen(time1) - 1] = '\0';
	string dt=string(time1);

	uint32_t ret;
	vector <string> tokens;

	Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
	JsonObject response,response1;
	JsonObject param,param1;
	string getresponse;

	if(NULL == remoteObject)
	{
		remoteObject = new JSONRPC::Client(_T("ContinueWatching.1"), _T(""));
		while(repeat == 1)
		{
			showMenu();
			cin>>choice;
			switch(choice)
			{
				case 1:
					cout<<"Enter the token:";
					cin>>token;
					cout<<endl;

					Cookie = token + "12345678901234567890";  //Cookie value=12345678901234567890
					tokenData = "{\"DateModified\" : \"" + dt + "\", \"Token\" : \"" + Cookie + "\", \"ESN\" : \"" + esn + "\"}";

					param["ApplicationName"] = "netflix";
					param["Token"].FromString(tokenData);

					ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setApplicationToken"),param, response);
					if (response["success"].Boolean())
					{
						cout<<endl<<"CW setApplication call Success"<<endl;
					}
					else
					{
						value = response["success"].String();
						cout<<"CW setApplication call failed: "<<value<<endl;
					}
				break;

				case 2:
					param1["ApplicationName"] = "netflix";
					ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getApplicationToken"), param1,response1);
					if (response1["success"].Boolean())
					{
						cout<<"CW getApplication call Success\n";
						response1.ToString(getresponse);
        	                                cout<<"Application Token:"<<getresponse<<endl;
						stringstream check1(getresponse);
						while(getline(check1, intermediate, ','))
    						{
   							tokens.push_back(intermediate);
    						}
						cout << tokens[1] << endl;
					}
					else
					{
						value = response["success"].String();
						cout<<"CW getApplication call failed: "<<value<<endl;
					}
				break;

				case 3:
					param["ApplicationName"] = "netflix";
					ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("deleteApplicationToken"), param, response);
					if (response["success"].Boolean())
					{
						cout<<"CW deleteApplication call Success"<<endl;
					}
					else
					{
						value = response["success"].String();
						cout<<"CW deleteApplication call failed: "<<value<<endl;
					}

				break;

				default:
					cout<<"Invalid choice"<<endl;
				break;
			}
			cout<<"\nEnter 1 to continue:";
			cin>>repeat;
		}
	}
	else
		cout<<"Failed to instantiate remote object"<<endl;

	cout<<"CW C++ application ends"<<endl;

	return 0;
}
