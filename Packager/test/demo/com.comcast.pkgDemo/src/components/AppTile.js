import { Utils } from 'wpe-lightning-sdk'

import Tile from "./Tile";
import IconButton from "./IconButton";
import Progress from "./Progress";

export default class StoreTile extends Tile {
    static _template( )
    {
      var pts = 20;
      var frameClr = 0xFF666666;  // #666666FF
      var textClr  = 0xFFffffff;  // #ffffffFF

      var stroke    = 2;
      var strokeClr = 0xFF444444;

      let RR = { radius: 20, type: lng.shaders.RoundedRectangle, stroke: stroke, strokeColor: strokeClr}

      return {
        flexItem: { margin: 40 },
        alpha: 1.0,
        pivot: 0.5,
        w: 210, h: 150,
          Button:
          {
            scale: 0.0,
            pivotY: 1.0,

            RRect:
            {
                w: 210, h: 150, rtt: true, rect: true, pivot: 0.5, alpha: 1.0, color: frameClr, shader: RR,

                Image: {
                  mount: 0.5,
                  src: Utils.asset('images/crate2_80x80.png'),
                  x: (w => 0.45 * w),
                  y: 150 * 0.40 //(h => 0.40 * h)
                },

                Label:
                { mountX: 0.5, mountY: 1.0,
                  x: (w => 0.5 * w),
                  y: 150 - 5, //(h => h - 5),
                  text: {  text: "Label 11", fontFace: 'Regular', fontSize: pts, textColor: textClr,

                  shadow: true,
                  shadowColor: 0xFF000000,
                  shadowOffsetX: 2,
                  shadowOffsetY: 2,
                  shadowBlur: 8,
                  },
                },

                Icon: {
                  alpha: 1.0,
                  mountX: 1.0,
                  scale: 0.52,
                  x: (w => w + 4),
                  y: (h => 0)
                },

                Buttons:
                {
                  flex: {direction: 'row'},
                  alpha: 0.5,
                  mountX: 0.5,
                  x: 210 / 2,
                  y: 150,

                  Button1: { btnId: "RESUME", flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/check_mark80x80.png')  },
                  Button2: { btnId: "KILL",   flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/x_mark.png')           },
                  Button3: { btnId: "TRASH",  flexItem: { margin: 30 }, type: IconButton, src1: Utils.asset('images/TrashCan80x80.png')    },
                }
              }, //RRect

              Progress: { type: Progress, mountX: 0.0, x: 0, y: 190, w: 210, h: 8, alpha: 0.0 },

          },//Button
      }
    };

    setSuspended()
    {
      var btn = this.tag("Button");
      var icn = btn.tag("Icon")

      icn.patch( {src: Utils.asset('images/sleep80x80.png')  } );

      this.startSnooze();
    }

    stopSuspended()
    {
      this.stopSnooze();
    }

    startSnooze()
    {
        this.tag('Icon').setSmooth('alpha', 1.0, {duration: 0.3});

        this.snoozeAnim = this.tag('Icon').animation({
            duration: 1.75,
            repeat: -1,
            actions: [
              {
                t: '',
                repeat: -1,
                p: 'scale',
                v: { 0.00: 0.30, 0.25: 0.60, 0.50: 0.65, 0.75: 0.60, 1.00: 0.30 },
              },
            ],
          });

        this.snoozeAnim.start()
    }

    stopSnooze()
    {
      if(this.snoozeAnim == null)
      {
        console.log("stopSnooze() ... Already stopped.  Null")
        return;
      }

      this.snoozeAnim.on('stop', ()=>
      {
        this.tag('Icon').setSmooth('alpha', 0.0, {duration: 1.73});

        this.snoozeAnim = null;
      });

      this.snoozeAnim.stop();
    }

    _init()
    {
      this.buttonIndex = 0;

      this._enabled = true;
      this.tag("Button").scale = 0;

      if(this.w && this.h)
      {
        let button = this.tag("Button");

        button.w = this.w;
        button.h = this.h;
      }

      this.info = this.tileInfo;

      this._setState('PassiveState');
    }

    startWiggle()
    {
        this.wiggleAnim = this.tag('Image').animation({
            duration: 0.11*3,
            repeat: -1,
            actions: [
              {
                t: '',
                repeat: -1,
                p: 'rotation', v: { 0: 0.0, 0.25: -0.14, 0.5: 0.0, 0.75: 0.14, 1: 0.0},
              },
            ],
          });

        this.wiggleAnim.start()
    }

    stopWiggle()
    {
      if(this.wiggleAnim)
      {
        this.wiggleAnim.stop();
        this.wiggleAnim = null;
      }
    }

    setInstalled(v)
    {
      if(this._info)
      {
        this._info.pkgInstalled = v;
      }
    }

    isInstalled()
    {
      return this._info.pkgInstalled;
    }

    clickAnim()
    {
      var anim = this.tag('Button').animation({
        duration: 0.35,
        repeat: 1,
        actions: [
          {
            t: '',
            p: 'scale', v: { 0: 1.0, 0.5: 1.2, 1: 1.0},
          },
        ],
      });

      anim.start()
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////

    static _states(){
      return [
            class PassiveState extends this
            {
              $enter()
              {
                // console.log(">>>>>>>>>>>>   STATE:  PassiveState");

                var btn  = this.tag("Button");
                var tile =  btn.tag("RRect")

                tile.setSmooth('h', 150 , {duration: 0.3});

                this._setState('PassiveState');
              }

              _handleDown()
              {
                this._setState('AppStateButtons');
              }
            },
            class AppStateButtons extends this
            {
              $enter()
              {
                console.log(">>>>>>>>>>>>   STATE:  AppStateButtons");

                var btn  = this.tag("Button");
                var tile =  btn.tag("RRect")

                tile.setSmooth('h', 210 , {duration: 0.3});
              }

              _handleUp()
              {
                this._setState('PassiveState');
              }

              _handleLeft()
              {
                // console.log(">>>>>>>>>>>>   LEFT:  this.buttonIndex: " + this.buttonIndex);

                if(--this.buttonIndex < 0) this.buttonIndex = 0;
              }

              _handleRight()
              {
                var btns = this.tag("Buttons");

                // console.log(">>>>>>>>>>>>   RIGHT:  this.buttonIndex: " + this.buttonIndex);
                if(++this.buttonIndex >= btns.children.length) this.buttonIndex = btns.children.length - 1;
              }

              _handleEnter()
              {
                console.log(">>>>>>>>>>>>  _handleEnter()  " + this.buttonIndex );

                var btns = this.tag("Buttons");
                var button = btns.children[this.buttonIndex];

                var fireThis = '$fire' + button.btnId;

                button.fireAncestors(fireThis);

                if(button.btnId == 'RESUME')
                {
                  this.setSuspended();
                }
                else
                if(button.btnId == 'KILL')
                {
                  this.stopSuspended();
                }
              }

              _getFocused()
              {
                // console.log(">>>>>>>>>>>>  _getFocused()  ");

                var btns  = this.tag("Buttons");
                return btns.children[this.buttonIndex]
              }
            },
        ]
      };
  }//CLASS
