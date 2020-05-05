/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include <bits/stdc++.h>
#include<ctime>
#include<string>
#include "Module.h"

using namespace std;
using namespace WPEFramework;

JSONRPC::LinkType<Core::JSON::IElement> *remoteObject = NULL;
void showMenu()
{
        cout<<"Enter your choice\n";
        cout<<"1.getValues\n";
        cout<<"2.registerListeners\n";
        cout<<"3.unregisterListeners\n";
	cout<<"4.getName\n";
	cout<<"5.getApiVersionNumber\n";
	cout<<"6.setApiVersionNumber\n";
}


int main(int argc, char** argv)
{
        int choice;
	JsonObject result;
        JsonObject param;
        uint32_t ret;
	int size;
	int i;
	string req;
	
	
	Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
	if(NULL == remoteObject) {
		remoteObject = new JSONRPC::LinkType<Core::JSON::IElement>(_T("StateObserver.1"), _T(""));
		while(1)
		{
			showMenu();
                	cin>>choice;
                	switch(choice)
                	{
				case 1: 
				{
					cout<<"Enter the number of properties\n";
					cin>>size;
					string pName[size];
					cout<<"Enter the properties whose value is required\n";
					for(i=0;i<size;i++){
						cin>>pName[i];
					}
					//To create the json format request string
					req="[\"" + pName[0] +"\"";
					i=1;
					while(i<size){
						req+=",\"" + pName[i] + "\"";
						i++;
					}		
					req+="]";
					cout<<"request string is"<<req<<"\n";
					param["PropertyNames"].FromString(req);
					ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getValues"), param, result);
                                	if (result["success"].Boolean())
                                	{
						string resp=result["properties"].String();
						cout<<"StateObserver the response properties value is "<<resp<<"\n";
                                        	cout<<"StateObserver getValue call Success\n";
                                	}	
                                	else
                                	{
                                        	cout<<"StateObserver getValue call failed \n";
                                	}
				}
                        	break;

				case 2:
				{
					cout<<"Enter the number of properties\n";
                                	cin>>size;
                                	string pName[size];
                                	cout<<"Enter the properties which needs to be registered\n";
                                	for(i=0;i<size;i++){
                                        	cin>>pName[i];
                                	}	
                                	//To create the json format request string
                                	req="[\"" + pName[0] +"\"";
                                	i=1;
                                	while(i<size){
                                        	req+=",\"" + pName[i] + "\"";
                                        	i++;
                                	}
                                	req+="]";
                                	cout<<"request string is"<<req<<"\n";
                                	param["PropertyNames"].FromString(req);

					ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("registerListeners"), param, result);
                                	if (result["success"].Boolean())
                                	{
                                        	string resp=result["properties"].String();
                                        	cout<<"StateObserver the response properties value is "<<resp<<"\n";
                                        	cout<<"StateObserver register listeners call Success\n";
                                	}	
                                	else
                                	{
                                        	cout<<"StateObserver register Listeners  call failed \n";
                                	}
				}
                        	break;

				case 3:
				{
                                	cout<<"Enter the number of properties\n";
                                	cin>>size;
                                	string pName[size];
                                	cout<<"Enter the properties which needs to be unregistered\n";
                                	for(i=0;i<size;i++){
                                        	cin>>pName[i];
                                	}
                                	//To create the json format request string
                                	req="[\"" + pName[0] +"\"";
                                	i=1;
                                	while(i<size){
                                        	req+=",\"" + pName[i] + "\"";
                                        	i++;
                                	}
                                	req+="]";
                                	cout<<"request string is"<<req<<"\n";
                                	param["PropertyNames"].FromString(req);

                                	ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("unregisterListeners"), param, result);
                                	if (result["success"].Boolean())
                                	{  
                                	        cout<<"StateObserver unregister listeners call Success\n";
                                	}
                                	else
                                	{
                                        	cout<<"StateObserver unregister Listeners  call failed \n";
                                	}
				}
                        	break;

				case 4:
				{
					cout<<"getName API\n";
					ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getName"), param, result);
					if (result["success"].Boolean())
                                	{
						string pluginName=result["Name"].String();
						cout<<"getName Api plugin is "<<pluginName<<"\n";
                                        	cout<<"StateObserver getName call Success\n";
                                	}	
                                	else
                                	{
                                        	cout<<"StateObserver getName call failed \n";
                                	}
				}
				break;

				case 5:
				{
					cout<<"getApiVersionNumber API\n";
					ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getApiVersionNumber"), param, result);
					if (result["success"].Boolean())
                                	{
                                	        string version=result["version"].String();
                                		cout<<"getApiVersionNumber Api plugin version  is "<<version<<"\n";
                                        	cout<<"StateObserver getApiVersionNumber call Success\n";
                                	}
                                	else
                                	{
                                        	cout<<"StateObserver getApiVersionNumber call failed \n";
                                	}
				}
				break;

				case 6:
				{
                                	cout<<"setApiVersionNumber API\n";
					cout<<"enter the version number \n";
					int version ;
					cin>>version;
					string ver="\"" + std::to_string(version) + "\"";
					param["version"].FromString(ver);
                                	ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setApiVersionNumber"), param, result);
                                	if (result["success"].Boolean())
                                	{
                                        	cout<<"StateObserver setApiVersionNumber call Success\n";
                                	}
                                	else
                                	{
                                	        cout<<"StateObserver setApiVersionNumber call failed \n";
                                	}
				}
                        	break;
			
				default:
				
				break;
			}
		
			cout<<"to continue press y to quit press any other key \n";
			char c;
			cin>>c;
			if(c=='y')
				continue;
			else
				break;

		}
	}

	
}	
