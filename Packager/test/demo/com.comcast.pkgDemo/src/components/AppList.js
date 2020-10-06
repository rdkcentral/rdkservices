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

import StoreTile from "./StoreTile";
import AppTile   from "./AppTile";

export default class AppList extends lng.Component
{
    static _template() {
      return {
          flex: {direction: 'row', padding: 20, paddingTop: 50, wrap: false },
          children:[]
      }
    }

    _getFocused()
    {
      console.log('LIST >>> _getFocused() ... ENTER')
      return this.children[0]//this.storeButtonIndex]
    }

    addTile(n, info)
    {
      // console.log('addTile() ... ENTER')

      // console.log("LIST  addTile( n: "+n+",  info:  " + JSON.stringify(info, 2, null) )
      // console.log("LIST  addTile( )    ... this.children.length " + this.children.length  )

      this.children[n].setInfo( info );
      this.children[n].show();
    }

    set storeTiles( list )
    {
        // console.log("SETTING >>> tiles: " + JSON.stringify(list, 2, null) )
        this.children = list.map((tileInfo, index) =>
        {
            return {
              w: 210, h: 150,
              type: StoreTile,
              tileInfo
            }
        })
    }


    set appTiles( list )
    {
        // console.log("SETTING >>> tiles: " + JSON.stringify(list, 2, null) )
        this.children = list.map((tileInfo, index) =>
        {
            return {
              w: 210, h: 150,
              type: AppTile,
              tileInfo
            }
        })
    }
  }//CLASS
