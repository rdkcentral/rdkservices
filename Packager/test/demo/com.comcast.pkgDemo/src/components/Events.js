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

export default class Events
{
  constructor(tt = null, pkdId)
  {
    if(tt == null)
    {
      throw "No Thunder !";
    }

    this.thunderJS = tt;
    this.pkdId     = pkdId
    this.events    = [];
  }

  async add(plugin, event, cb = null)
  {
    if( (plugin == null || plugin == "") &&
        (event  == null || event  == "") )
    {
      throw "No plugin/event !";
    }

    this.events.push( await this.handleEvent(plugin, event, cb) );
  }

  disposeAll()
  {
    // console.log( "EVENTS >>>   destroyAll() ")

    this.events.map( ee => { ee.dispose() } )
  }

  async handleEvent(plugin, event, cb = null)
  {
    // console.log('EVENTS >> Listen for >> ['+plugin+'] -> '+event+' ...');

    if(cb != null)
    {
      // console.log('Listen for ['+name+'] using CALLBACK ...');
      return await this.thunderJS.on(plugin, event, cb);
    }
    else
    {
      return await thunderJS.on(plugin, event, (notification) =>
      {
          var str = " " + event + " ...  Event" + JSON.stringify(notification);
          console.log('Handler GOT >> ' + str)
      })
    }
  }
}//CLASS
