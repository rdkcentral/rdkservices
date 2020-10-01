import { Lightning, Utils } from 'wpe-lightning-sdk'

import beautify   from 'json-beautify'
import ThunderJS  from 'ThunderJS'
import Events     from './components/Events'
import AppList    from "./components/AppList";
import OkCancel   from "./components/OkCancel";

import { DefaultApps as DefaultApps } from "./DefaultApps.js";

const HOME_KEY      = 77;
const LIGHTNING_APP = "lightningapp";

var AvailableApps   = [];
var InstalledApps   = [];
var InstalledAppMap = {};

const thunder_cfg = {
  host: '127.0.0.1',
  port: 9999,
  debug: false, // VERY USEFUL
  versions: {
    default: 1, // use version 5 if plugin not specified
    Controller: 1,
    Packager: 1,
    // etc ..
  }
}

var thunderJS = null;

export default class App extends Lightning.Component
{
  static getFonts() {
    return [{ family: 'Regular', url: Utils.asset('fonts/Roboto-Regular.ttf') }]
  }

  static _template()
  {
    let RR = Lightning.shaders.RoundedRectangle;

    var ui =
    {
      Blind1: {
        x: 0, y: 0, w: 1920, h: 1080/2, rect: true, color: 0xff000000, zIndex: 998,

        // Bg: {mountX: 0.5,
        //   x: 1920/2, y: 1080/2, w: 600, h: 79, rect: true, color: 0xff000000, zIndex: 998,
        // },

        RDKlogo: {
          mount: 0.5,
          x: 1920/2,
          y: 1080/2,
          zIndex: 999,
          src: Utils.asset('images/RDKLogo400x79.png'),
        },
      },
      Blind2: {
        x: 0, y: 1080/2, w: 1920, h: 1080/2, rect: true, color: 0xff000000, zIndex: 997
      },


      Background: {
        w: 1920,
        h: 1080,
        color: 0xff8888aa,
        src: Utils.asset('images/background1.png'),
      },

      Title: {
        mountX: 0.5,
        mountY: 0,
        x: 1920/2,
        y: 20,
        text: {
          text: "Demo Store",
          fontFace: 'Regular',
          fontSize: 70,
          textColor: 0xFFffffff,

          shadow: true,
          shadowColor: 0xff000000,
          shadowOffsetX: 2,
          shadowOffsetY: 2,
          shadowBlur: 8,
        },
      },

      Lists:
      {
        mountX: 0.5, x: 1920/2, y: 150, w: 1450, h: 900,
        flex: {direction: 'column', alignItems: 'center'},
      //	rect: false, //rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,
        // rect: true, color: 0x88ccccFF,

        HelpText:
        {
          // rect: true, color: 0xff00ff00,

          flex: {direction: 'row'},

          HelpBox1:
          {
            w: 1450/2,
            h: 10,
            // rect: true, color: 0xffFF0000,

            HelpTip1:
            {
              text:
              {
                w: 1450/2,
                text: "Use  (A)ll or (I)nfo for package metadata",
                textAlign: 'center',
                fontFace: 'Regular',
                fontSize: 16,
                textColor: 0xFFffffff,

                shadow: true,
                shadowColor: 0xff000000,
                shadowOffsetX: 2,
                shadowOffsetY: 2,
                shadowBlur: 8,
              },
            },
          },//Box
          HelpBox2:
          {
            w: 1450/2,
            h: 10,
            // rect: true, color: 0x88FF00FF,

            HelpTip2:
            {
              text:
              {
                w: 1450/2,
                // h: 50,
                text: "Use  UP/DN  arrow keys for Console",
                textAlign: 'center',
                fontFace: 'Regular',
                fontSize: 16,
                textColor: 0xFFffffff,

                shadow: true,
                shadowColor: 0xff000000,
                shadowOffsetX: 2,
                shadowOffsetY: 2,
                shadowBlur: 8,
              },
            },
          },//Box
        }, // HelpText

        AvailableTXT: {
          // rect: true, color: 0x88FF00FF,
          x: 0,
          y: 60,
          w: 1450,
          text: {
            text: "AVAILABLE: ",

            // highlight: true,
            // highlightColor: 0xFF0000ff,

            // textAlign: 'left',
            fontFace: 'Regular',
            fontSize: 30,
            textColor: 0xFFffffff,

            shadow: true,
            shadowColor: 0xff000000,
            shadowOffsetX: 2,
            shadowOffsetY: 2,
            shadowBlur: 8,
          },
        },

        AvailableGroup:
        {
          mountX: 0.5, x: 1450/2, y: 0, w: 1450, h: 300, flex: {direction: 'row', padding: 15, wrap: false }, rect: true, rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,

          // Available PACKAGES from inventory ... injected here
          AvailableList: { x: 0, type: AppList }
        },


        InstalledTXT: {
          // rect: true, color: 0x88FF00FF,
          x: 0,
          y: 160,
          w: 1450,
          text: {
            text: "INSTALLED: ",

            // highlight: true,
            // highlightColor: 0xFF0000ff,

            // textAlign: 'left',
            fontFace: 'Regular',
            fontSize: 30,
            textColor: 0xFFffffff,

            shadow: true,
            shadowColor: 0xff000000,
            shadowOffsetX: 2,
            shadowOffsetY: 2,
            shadowBlur: 8,
          },
        },
        InstalledGroup:
        {
          mountX: 0.5, x: 1450/2, y: 100, w: 1450, h: 300, flex: {direction: 'row', padding: 15, wrap: true}, rect: true, rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,

          InstalledList: { type: AppList }

        }, // InstalledGroup
      },//Lists

      SpaceLeft:
      {
        x: 1400, y: 600,
        text: {
          text: "Space: 0 Kb",
          textAlign: 'right',
          fontFace: 'Regular',
          fontSize: 22,
          textColor: 0xaa00FF00,

          shadow: true,
          shadowColor: 0xFF000000,
          shadowOffsetX: 2,
          shadowOffsetY: 2,
          shadowBlur: 8,
        },

      },

      ConsoleBG:
      {
        mountX: 0.5,
        x: 1920/2, y: 150, w: 1450,
        h: 600, rect: true,
        alpha: 0.0, shader: { radius: 20, type: RR },
        color: 0xcc222222, // #222222ee
        // colorTop: 0xFF636EFB, colorBottom: 0xFF1C27bC,

        Console: {

          x: 10, y: 10,
          w: 1450,
          //h: 500,
          text: {
            fontFace: 'Regular',
            fontSize: 18,
            textColor: 0xFFffffff,
          },
        },
      }, // ConsoleBG

      // InfoButton: {
      //   w: 60,
      //   h: 60,

      //   src: Utils.asset('images/info.png'),
      // },

      OkCancel: { type: OkCancel, x: 1920/2, y: 400, w: 600, h: 180, alpha: 0.0 },

      // LineH: { mountY: 0.5, x: 0, y: 1080/2, w: 1920, h: 2, rect: true, color: 0xff00FF00 },
      // LineV: { mountX: 0.5, y: 0, x: 1920/2, h: 1080, w: 2, rect: true, color: 0xff00FF00 },
    };

    return ui;
  }

  setConsole(str)
  {
    this.tag('Console').text.text = str;
  }


  $fireRESUME(pkgId)
  {
    // console.log(">>>>>>>>>>>>  fireRESUME() ");

    let info = InstalledApps[this.installedButtonIndex];

    // console.log(">>>>>>>>>>>>  fireRESUME() info: "+ beautify(info, null, 2, 100) );

    if(info != null)
    {
      this.resumePkg(info.pkgId, info);
    }
  }

  $fireKILL()
  {
    // console.log(">>>>>>>>>>>>  fireKILL() ");

    let info = InstalledApps[this.installedButtonIndex];

    // console.log(">>>>>>>>>>>>  fireKILL() info: "+ beautify(info, null, 2, 100) );

    if(info != null)
    {
      this.killPkg(info.pkgId, info);
    }
  }

  $fireTRASH(pkgId)
  {
    let info = InstalledApps[this.installedButtonIndex];

    // console.log(">>>>>>>>>>>>  fireTRASH() info: "+ beautify(info, null, 2, 100) );

    this._setState('OKCStateEnter');
  }

  findInstalledButton(pkgId)
  {
    var bb = this.tag('InstalledList').children.filter( (o) =>
    {
      if(o.info)
      {
        return o.info.pkgId == pkgId;
      }
      else
      {
        return false;
      }
    });

    return bb.length == 0 ? null : bb[0];
  }

  findStoreButton(pkgId)
  {
    var bb = this.tag('AvailableList').children.filter( (o) =>
    {
      if(o.info)
      {
        return o.info.pkgId == pkgId;
      }
      else
      {
        return false;
      }
    });

    return bb.length == 0 ? null : bb[0];
  }

  $onRemoveOK() // 'okButton = true' indicates the OK button was clicked
  {
    var dlg = this.tag("OkCancel");
    var pkgId = dlg.pkgId;

    console.log("onRemoveOK ENTER - ... pkgId: " + pkgId);

    if(pkgId == undefined)
    {
      console.log("onRemoveOK() >>>  ERROR - ... pkgId: " + pkgId)
      return;
    }

    let info = InstalledAppMap[pkgId];
    if(info.appState == "SUSPENDED")
    {
      this.killPkg(pkgId);
    }

    // this.removePkg(pkgId);

    dlg.setSmooth('alpha', 0, {duration: 0.3}); // HIDE

    // Enable STORE button - as it's UNINSTALLED
    let removeMe = this.tag('InstalledList').children[this.installedButtonIndex];
    removeMe.stopWiggle();

    //removeMe.hide();

    if(removeMe.tag("Button").scale == 1.0)
    {
      const anim = removeMe.tag('Button').animation({
        duration: 0.5,
        actions: [
            { p: 'scale', v: { 0: 1, 0.5: 0.50, 1: 0.0 } },
        ]
      });
      anim.start();

      anim.on('finish', () =>
      {
        this.tag('InstalledList').childList.remove(removeMe);
        this.tag('InstalledList').childList.add(removeMe); // move to end

        this.removePkg(pkgId);
      });
    }

    var storeButton = this.findStoreButton(pkgId);
    if(storeButton != null)
    {
      storeButton.enable();
    }

    this._setState('InstalledRowState');
}

  $onRemoveCANCEL()
  {
    var dlg = this.tag("OkCancel");

    // console.log("onRemoveCANCEL ENTER - ... pkgId: " + pkgId);

    dlg.setSmooth('alpha', 0, {duration: 0.3}); // HIDE

    // console.log("onRemoveCANCEL ENTER - ... info: " + pkgId)

    var dontRemoveMe = this.tag('InstalledList').children[this.installedButtonIndex];
    dontRemoveMe.stopWiggle();

    this._setState('InstalledRowState');
  }

  $fireINSTALL(pkgId)
  {
    // console.log("INSTALL >>  fireINSTALL() - ENTER .. pkgId: " + pkgId);

    let button = this.tag('AvailableList').children[this.storeButtonIndex];

    this.isInstalled(pkgId).then( (ans) =>
    {
      if( ans['available'] == false)
      {
        var progress = button.tag("Progress")

        progress.reset(); // reset
        progress.setSmooth('alpha', 1, {duration: .1});

        var info = button.info;

        this.installPkg(pkgId, info);
        info.appState = "STOPPED";
      }
      else
      {
        console.log("CALL >> this.installPkg() ALREADY have ... pkgId: " + pkgId)
      }
    });
  }

  $LaunchClicked(pkgId)
  {
    // console.log("$LaunchClicked() >>>  ENTER - ... pkgId: " + pkgId)

    let info = InstalledAppMap[pkgId];
    if(info)
    {
      if(info.appState == "STOPPED" || info.appState == undefined)
      {
        this.launchPkg(pkgId, info);

        this.setConsole("Launched: " + beautify(info, null, 2, 100) );

      }
      else if(info.appState == "SUSPENDED")
      {
        this.resumePkg(pkgId, info);

        this.setConsole("Resumed: " + beautify(info, null, 2, 100) );
      }
      else
      {
        console.log("$LaunchClicked() >>> Error:  invalid app state: " + info);
      }
    }
    else
    {
      console.log("$LaunchClicked() >>> Error:  NO  info: " + info);
    }
  }

  async getAvailableSpace()
  {
    try
    {
      var result = await thunderJS.call('Packager', 'getAvailableSpace', null);

      this.tag('SpaceLeft').text.text = ("Space Remaining: " + result.availableSpaceInKB + " Kb");

      //this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      this.setConsole( 'getAvailableSpace() >>> CAUGHT:  e: ' + beautify(e, null, 2, 100) );
    }
  }

  async getPackageInfo(pkgId)
  {
    try
    {
      let params = { "pkgId": pkgId };

      var result = await thunderJS.call('Packager', 'getPackageInfo', params);

      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      this.setConsole( 'getPackageInfo() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async getInstalled()
  {
    // console.log("getInstalled() - ENTER ")

    try
    {
      var result = await thunderJS.call('Packager', 'getInstalled', null);

      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      this.setConsole( 'getInstalled() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      return;
    }

    this.getAvailableSpace();

    // Preserve App States ...

    for( var i = 0; i < result.applications.length; i++)
    {
      let app   = result.applications[i];
      let match = InstalledApps.filter( o => o.pkgId == app.id );

      if(match.length > 0)
      {
        // NOTE: If objects have a property with the same name,
        //       then the right-most object property overwrites the previous one.
        //
        var merged = { ...match[0], ...app }; // prefer new 'info' from getInstalled()

        result.applications[i] = merged;
      }
    }

    // Update Apps ...
    InstalledAppMap = {}    // reset
    InstalledApps   = null; // reset

    //
    // NOTE:  getInstalled() returns meta with 'id' -NOT- 'pkgId'
    //
    result.applications.map( (o) => InstalledAppMap[o.id] = o ); // populate info

    InstalledApps = result.applications; // update INSTALLED array

    //
    // APP STORE >>> DISABLE apps that are already installed...
    //
    InstalledApps.map( have =>
    {
      let disable = AvailableApps.filter( o => o.pkgId == have.id );

      var storeButton = this.findStoreButton(disable[0].pkgId);
      if(storeButton != null)
      {
        storeButton.disable(); // DISABLE
      }
    })

    // SHOW / HIDE tiles per installations
    this.tag("InstalledList").children.map( (button, i) =>
    {
      if(i < InstalledApps.length)
      {
        InstalledApps[i].pkgInstalled = true;

        button.info = InstalledApps[i];
        button.show(i * 0.15);
      }
      else
      {
        button.info = null; //  RESET
        // button.hide();
      }
    });

    if(InstalledApps.length == 0)
    {
      this._setState('StoreRowState'); // No apps installed >>> BACK TO STORE 
    }
  }

  async isInstalled(pkgId)
  {
    try
    {
      let params = { "pkgId": pkgId };

      let result = await thunderJS.call('Packager', 'isInstalled', params)

//      console.log( 'DEBUG:  IsInstalled  ' + beautify(result, null, 2, 100) )
//      this.setConsole(     'IsInstalled  ' + beautify(result, null, 2, 100) )

      return result;
    }
    catch(e)
    {
      console.log('DEBUG:  isInstalled() >>> CAUGHT:  e: ' + beautify(e, null, 2, 100) );
      this.setConsole(    'isInstalled() >>> CAUGHT:  e: ' + beautify(e, null, 2, 100) );
      return false;
    }
  }

  async addKeyIntercept()  // v1
  {
    let params =
    {
      "keyCode": HOME_KEY,
      "modifiers": ["ctrl"],
      "client": LIGHTNING_APP
    }

    try
    {
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'addKeyIntercept', params);
      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      console.log(     'addKeyIntercept() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'addKeyIntercept() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async removeKeyIntercept() // v1
  {
    let params =
    {
      "client": LIGHTNING_APP
    }

    try
    {
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'removeKeyIntercept', params);

      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      console.log(     'removeKeyIntercept() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'removeKeyIntercept() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async setFocus(pkgId)
  {
    let params =
    {
        "client": pkgId
    };

    try
    {
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'setFocus', params);

      this.setConsole( beautify(result, null, 2, 100) );
    }
    catch(e)
    {
      console.log(     'setFocus() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'setFocus() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async moveToFront(pkgId)
  {
    let params =
    {
        "client": pkgId
    }

    try
    {
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'moveToFront', params);

      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      console.log(     'moveToFront() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'moveToFront() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async moveToBack(pkgId)
  {
    let params =
    {
        "client": pkgId
    }

    try
    {
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'moveToBack', params);
      console.log(beautify(result, null, 2, 100));
      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      console.log(     'moveToBack() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'moveToBack() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async suspendPkg(pkgId, info)
  {
    let params =
    {
        "client": pkgId
    }

    try
    {
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'suspendApplication', params);

      console.log(beautify(result, null, 2, 100));
      this.setConsole( beautify(result, null, 2, 100) );

      if(result.success)
      {
        this.moveToBack(pkgId);
        this.setFocus(LIGHTNING_APP);

        info.appState      = "SUSPENDED";
        this.launchedPkgId = "";

        // TODO: APP BUTTON - setSuspended()
        var appButton = this.findInstalledButton(pkgId);
        if(appButton != null)
        {
          appButton.setSuspended();
        }
        else
        {
          console.log("suspendPkg() >>> Cannot find App Button for pkgId: " + pkgId);
        }
      }
      else
      {
        console.log( 'suspendPkg() failed!');
      }
    }
    catch(e)
    {
      console.log(     'suspendPkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'suspendPkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async resumePkg(pkgId, info)
  {
    let params =
    {
        "client": pkgId
    }

    try
    {
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'resumeApplication', params);

      console.log( beautify(result, null, 2, 100) );
      this.setConsole( beautify(result, null, 2, 100) );

      if(result.success)
      {
        this.moveToFront(pkgId);
        this.setFocus(pkgId);

        info.appState      = "LAUNCHED"; // RESUMED
        this.launchedPkgId = pkgId;

        // TODO: APP BUTTON - setLaunched()
        var appButton = this.findInstalledButton(pkgId);
        if(appButton != null)
        {
          appButton.stopSuspended();
        }
        else
        {
          console.log("resumePkg() >>> Cannot find App Button for pkgId: " + pkgId);
        }
      }
      else
      {
        console.log( 'resumePkg() failed!');
      }
    }
    catch(e)
    {
      console.log(     'resumePkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'resumePkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async killPkg(pkgId, info)
  {
    let params =
    {
        "client": pkgId
    }

    try
    {
      // Need to resume before stopping a container app....
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'resumeApplication', params);

      if(result.success)
      {
        info.appState = "STOPPED";
      }
      else
      {
        console.log( 'killPkg() >>> calling "resumeApplication" FAILED!');
      }

      // Next kill the App
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'kill', params);

      if(result.success)
      {
        info.appState = "STOPPED";
      }
      else
      {
        console.log( 'killPkg() >>> calling "kill" FAILED!');
      }

      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      console.log(     'killPkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'killPkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }


  async launchPkg(pkgId, info)
  {
    let params =
    {
        "client": pkgId,
        "uri": pkgId, //TODO:  Unexpected... check why ?
        // "uri": info.bundlePath,
        "mimeType": "application/dac.native"
    }

    try
    {
      var result = await thunderJS.call('org.rdk.RDKShell.1', 'launchApplication', params);

      console.log(beautify(result, null, 2, 100));
      this.setConsole( beautify(result, null, 2, 100) );

      if(result.success)
      {
        this.moveToFront(pkgId);
        this.setFocus(pkgId);

        info.appState      = "LAUNCHED"; // 1st LAUNCH
        this.launchedPkgId = pkgId;

        // TODO: APP BUTTON - setLaunched()
      }
      else
      {
        console.log( 'launchPkg() failed to launch app!!!');
      }
    }
    catch(e)
    {
      console.log(     'launchPkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
      this.setConsole( 'launchPkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }
  }

  async installPkg(thisPkgId, info)
  {
    var myEvents = new Events(thunderJS, thisPkgId);

    let buttons  = this.tag('AvailableList').children
    let button   = buttons[this.storeButtonIndex];
    let progress = button.tag('Progress')

    progress.reset(); // reset

    let handleFailure = (notification, str) =>
    {
      console.log("FAILURE >> '"+str+"' ... notification = " + JSON.stringify(notification) )

      if(thisPkgId == notification.pkgId)
      {
        button.setIcon(Utils.asset('images/x_mark.png'))

        progress.setSmooth('alpha', 0, {duration: 1.3});

        setTimeout( () =>
        {
          button.setIcon(Utils.asset('images/x_mark.png'))

          progress.reset(); // reset

          this.getAvailableSpace()

        }, 1.2 * 1000); //ms

        this.setConsole( beautify(notification, null, 2, 100) )
      }
    }

    let handleFailureDownload     = (notification) => { handleFailure(notification,'FailureDownload')     };
    let handleFailureDecryption   = (notification) => { handleFailure(notification,'FailureDecryption')   };
    let handleFailureExtraction   = (notification) => { handleFailure(notification,'FailureExtraction')   };
    let handleFailureVerification = (notification) => { handleFailure(notification,'FailureVerification') };
    let handleFailureInstall      = (notification) => { handleFailure(notification,'FailureInstall')      };

    let handleProgress = (notification) =>
    {
      // console.log("HANDLER >> pkgId: "+thisPkgId+" ... notification = " + JSON.stringify(notification) );

      if(thisPkgId == notification.pkgId)
      {
        let pc = notification.status / 8.0;
        progress.setProgress(pc);

        // console.log("HANDLER >> pkgId: "+thisPkgId+" ... progress = " + pc );

        if(pc == 1.0)
        {
          progress.setSmooth('alpha', 0, {duration: 2.3});

          var ans = AvailableApps.filter( (o) => { return o.pkgId == notification.pkgId; });

          if(ans.length == 1) // IGNORE OTHER NOTTIFICATIONS
          {
            var info = ans[0];
            this.onPkgInstalled(info, button)

            if(info.events)
            {
              info.events.disposeAll(); // remove event handlers
              info.events = null;
            }
          }
        }//ENDIF - 100%
      }
    }

    {
      myEvents.add( 'Packager', 'onDownloadCommence', handleProgress);
      myEvents.add( 'Packager', 'onDownloadComplete', handleProgress);

      myEvents.add( 'Packager', 'onExtractCommence',  handleProgress);
      myEvents.add( 'Packager', 'onExtractComplete',  handleProgress);

      myEvents.add( 'Packager', 'onInstallCommence',  handleProgress);
      myEvents.add( 'Packager', 'onInstallComplete',  handleProgress);

      myEvents.add( 'Packager', 'onDownload_FAILED',     handleFailureDownload,) ;
      myEvents.add( 'Packager', 'onDecryption_FAILED',   handleFailureDecryption) ;
      myEvents.add( 'Packager', 'onExtraction_FAILED',   handleFailureExtraction) ;
      myEvents.add( 'Packager', 'onVerification_FAILED', handleFailureVerification);
      myEvents.add( 'Packager', 'onInstall_FAILED',      handleFailureInstall);
    }

    try
    {
      var result = await thunderJS.call('Packager', 'install', info);

      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      this.setConsole( 'installPkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }

    info.events = myEvents
  }

  async removePkg(pkgId)
  {
    console.log("removePkg() >>>    ENTER - ... pkgId: " + pkgId)

    if(pkgId == undefined)
    {
      console.log("removePkg() >>>  ERROR - ... pkgId: " + pkgId)
      return;
    }

    var params = {
      "pkgId": pkgId
    }

    try
    {
      var result = await thunderJS.call('Packager', 'remove', params);

      this.setConsole( beautify(result, null, 2, 100) )
    }
    catch(e)
    {
      this.setConsole( 'removePkg() >>> CAUGHT:  e: ' +  beautify(e, null, 2, 100) );
    }

    // Update the Installed
    //
    this.getAvailableSpace()
    this.getInstalled();
  }

  onPkgInstalled(info, storeButton)
  {
    // console.log('onPkgInstalled() ... Installed >>> ' + info.pkgId)

    info.pkgInstalled = true;

    InstalledApps.push( info )
    InstalledAppMap[info.pkgId] = info; // populate

    this.tag('InstalledList').addTile(InstalledApps.length - 1, info)

    // Disable STORE button - as it's UNINSTALLED
    storeButton.disable();

    this.getAvailableSpace()
  }

  _init()
  {
    this.storeButtonIndex     = 0;
    this.installedButtonIndex = 0;
    this.launchedPkgId        = "";

    this.tag('Background').on('txLoaded', () =>
    {
      this._setState('IntroState');
    });
  }

  handleToggleConsole()
  {
    let a = this.tag("ConsoleBG").alpha;
    this.tag("ConsoleBG").setSmooth('alpha', (a == 1) ? 0 : 1, {duration: 0.3});
  }

  handleGetInfoALL()
  {
    this.getInstalled();
  }

  handleGetInfo()
  {
    let info = InstalledApps[this.installedButtonIndex];

    this.getPackageInfo(info.pkgId || info.id);
  }

  // GLOBAL key handling
  _handleKey(k)
  {
    switch( k.keyCode )
    {
      case HOME_KEY:
      case 72: //'H' key
        console.log("HOME code: " + k.keyCode);
        this.setConsole( "HOME code: " + k.keyCode);

        let info = InstalledAppMap[this.launchedPkgId];
        if(info == "" || info == null)
        {
          console.log("Ignoring HOME key, no apps running");
          break;
        }

        if(info.appState == "LAUNCHED")
        {
          console.log("Calling >>> this.suspendPkg()  pkgId: " + info.pkgId );
          this.suspendPkg(info.pkgId, info);
        }
        else
        {
          console.log("Ignoring HOME key, no apps running");
        }
        break

      case 65:  // 'A' key on keyboard
      case 403: // 'A' key on remote
          this.handleGetInfoALL();
          break;

      case 67:  // 'C' key on keyboard
      case 405: // 'C' key on remote
          this.handleToggleConsole();
          break;

      case 73:  // 'I' key on keyboard
                // 'INFO' key on remote
          this.handleGetInfo();
          break;

      default:
        // console.log("GOT key code: " + k.keyCode)
          break;
    }

    return true;
  }

  static _states(){
    return [
          class IntroState extends this
          {
            $enter()
            {
              // console.log(">>>>>>>>>>>>   STATE:  IntroState");

              var dlg = this.tag("OkCancel");
              dlg.setSmooth('alpha', 0, {duration: 0.0});

              let h1 =  (1080 + 79); // Move LOWER blind to below bottom (offscreen)
              let h2 = -(h1/2 + 79); // Move UPPER blins to above top    (offscreen)

              const anim = this.tag('RDKlogo').animation(
              {
                duration: 0.5,  delay: 1.5,
                actions: [ { p: 'alpha', v: { 0: 1.0, 0.5: 0.75, 1: 0.0 } } ]
              });

              anim.on('finish', ()=>
              {
                this.tag('Blind1' ).setSmooth('y', h2, { delay: 0.25, duration: 0.75 });
                this.tag('Blind2' ).setSmooth('y', h1, { delay: 0.25, duration: 0.75 });

                this._setState('SetupState');
              });

              anim.start();
            }
          },
          class SetupState extends this
          {
            fetchAppList(url)
            {
              // Fetch App List
              //
              fetch(url)
              .then(res => res.json())
              .then((apps) =>
              {
                apps.map( (o) => o.pkgInstalled = false); //default

                AvailableApps = apps;
                InstalledApps = apps;

                this.tag("AvailableList").storeTiles   = AvailableApps;
                this.tag("InstalledList").appTiles = InstalledApps;

                this._setState('StoreRowState')
              })
              .catch(err =>
              {
                console.log("Failed to get URL: " + url);

                AvailableApps = DefaultApps;

                console.log("... using DefaultApps");

                this.tag("AvailableList").tiles = AvailableApps;

                this._setState('StoreRowState')
              });
            }

            fetchThunderCfg(url)
            {
              // Fetch Thunder Cfg
              //
              fetch(url)
              .then( res => res.json())
              .then((cfg) =>
              {
                console.log(' >>> Creating CUSTOM ThunderJS ...')
                thunderJS = ThunderJS(cfg);

                this.getInstalled(); // <<< needs THUNDER
              })
              .catch(err =>
              {
                console.log("Failed to get URL: " + url);

                console.log(' >>> Creating DEFAULT ThunderJS ...')
                thunderJS = ThunderJS(thunder_cfg);

                this.getInstalled();

                console.log("... using default Thunder cfg.");
              });
            }

            $enter()
            {
              // console.log(">>>>>>>>>>>>   STATE:  SetupState");

              const URL_PARAMS = new window.URLSearchParams(window.location.search)
              var appURL       = URL_PARAMS.get('appList')
              var cfgURL       = URL_PARAMS.get('thunderCfg')

              this.fetchThunderCfg(cfgURL);
              this.fetchAppList(appURL);

              this.setFocus(LIGHTNING_APP);
              this.addKeyIntercept(); // Watch for HOME key

              // State advanced within 'fetchAppList()' above.
            }
          },  //CLASS - SetupState
          // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
          class StoreRowState extends this
          {
            $enter()
            {
              // console.log(">>>>>>>>>>>>   STATE:  StoreRowState");

              // Set FOCUS to 1st package
              //
              var av_children = this.tag('AvailableList').children
              if(av_children.length >0)
              {
                av_children[this.storeButtonIndex].setFocus = true;
              }

              av_children.map( (o,n) => o.show(n * 0.15) );
            }

            _handleEnter()
            {
              let info   = AvailableApps[this.storeButtonIndex];
              let button = this.tag('AvailableList').children[this.storeButtonIndex];

              if(info == undefined)
              {
                console.log("FIRE >>> INSTALL  NO info !")
                return // ignore
              }

              if(button.isEnabled() == false)
              {
                return // IGNORE CLICK
              }

              console.log("FIRE >>> INSTALL   pkgId:" + info.pkgId)

              button.fireAncestors('$fireINSTALL', info.pkgId);
            }

            _handleDown()
            {
              if(InstalledApps.length > 0)
              {
                this._setState('InstalledRowState');
              }
            }

            _handleLeft()
            {
              if(--this.storeButtonIndex < 0) this.storeButtonIndex = 0;
            }

            _handleRight()
            {
              if(++this.storeButtonIndex > AvailableApps.length) this.storeButtonIndex = AvailableApps.length - 1;
            }

            _getFocused()
            {
              return this.tag('AvailableList').children[this.storeButtonIndex]
            }
        }, //CLASS

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        class InstalledRowState extends this
        {
          $enter()
          {
            // console.log(">>>>>>>>>>>>   STATE:  InstalledRowState");
          }

          _handleUp()
          {
            this._setState('StoreRowState');
          }

          _handleLeft()
          {
            if(--this.installedButtonIndex < 0) this.installedButtonIndex = 0;
          }

          _handleRight()
          {
            if(++this.installedButtonIndex >= InstalledApps.length) this.installedButtonIndex = InstalledApps.length - 1;
          }

          _handleEnter()
          {
            let info   = InstalledApps[this.installedButtonIndex];
            let button = this.tag('InstalledList').children[this.installedButtonIndex];

            console.log("FIRE >>> LAUNCH   pkgId:" + info.pkgId)

            button.fireAncestors('$LaunchClicked', info.pkgId);
            button.clickAnim();
          }

          _handleBack() // BACK key on the "Installed Row" - triggers the Delete OK/Cancel dialog
          {
            this._setState('OKCStateEnter')
          }

          _getFocused()
          {
            return this.tag('InstalledList').children[this.installedButtonIndex]
          }
        },//class
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        class OKCStateEnter extends this
        {
          $enter()
          {
            // console.log(">>>>>>>>>>>>   STATE:  OKCStateEnter");

            if(this.installedButtonIndex <0)
            {
              console.error(  'BUTTON index:' + this.installedButtonIndex +'  - INVLAID');
              return;
            }

            var button = this.tag('InstalledList').children[this.installedButtonIndex]

            if(button == undefined || button.info == undefined)
            {
              console.error(  'BUTTON index:' + this.installedButtonIndex +'  - NOT FOUND')
              this.setConsole('BUTTON index:' + this.installedButtonIndex +'  - NOT FOUND');
              return;
            }
            var pkgId = button.info.pkgId;

            button.startWiggle();

            var dlg    = this.tag("OkCancel");
            dlg.pkgId  = pkgId; // needed later
            dlg.button = button;

            dlg.setLabel("Remove '" + pkgId + "' app ?");
            dlg.setSmooth('alpha', 1, {duration: 0.3});

            dlg._setState('OKCState')
          }

          _getFocused()
          {
            var dlg = this.tag("OkCancel");

            return dlg;
          }
        },
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      ]
  }//_states
}
