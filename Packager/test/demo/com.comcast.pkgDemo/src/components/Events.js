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
