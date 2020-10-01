
import StoreTile   from "./StoreTile";
import AppTile from "./AppTile";

export default class AppList extends lng.Component {

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
