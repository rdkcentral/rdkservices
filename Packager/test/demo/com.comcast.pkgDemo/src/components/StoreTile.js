import { Utils } from 'wpe-lightning-sdk'

import Tile from "./Tile";
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
        Button:
        {
          scale: 0.0,

          RRect:
          {
              w: 210, h: 150, rtt: true, rect: true, pivot: 0.5, alpha: 1.0, color: frameClr, shader: RR,

              Image: {
                mountX: 0.5,
                mountY: 0.5,
                src: Utils.asset('images/crate2_80x80.png'),
                x: (w => 0.45 * w),
                y: (h => 0.4 * h)
              },

              Label:
              { mountX: 0.5, mountY: 1.0, x: (w => 0.5 * w), y: (h => h - 5),
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
            },

            Progress: { type: Progress, mountX: 0.0, x: 0, y: 190, w: 210, h: 8, alpha: 0.0 },
        }//Button
      }
    };

    _init()
    {
      this._enabled = true;
      this.tag("Button").scale = 0;

      if(this.w && this.h)
      {
        let button = this.tag("Button");

        button.w = this.w;
        button.h = this.h;
      }

      this.info = this.tileInfo;

      var download_PNG = Utils.asset('images/download3.png');
      this.setIcon(download_PNG);
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

  }//CLASS
