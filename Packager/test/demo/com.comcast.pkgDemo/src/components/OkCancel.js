import Button from "./Button";

export default class OkCancel extends lng.Component {
    static _template( )
    {
      let RR = lng.shaders.RoundedRectangle;

      var  pts = 25;
     // var buttonClr = 0xFF888888;  // #888888FF  // Background
      var frameClr  = 0xFFaaaaaa;  // #aaaaaaFF
      var textClr   = 0xFFffffff;  // #ffffffFF

      var stroke    = 2;
      var strokeClr = 0xFF444444;

      return {
        Dialog:
        {
            mount: 0.5, w: 600, h: 120, rtt: true, rect: true, color: frameClr, shader: { radius: 20, type: RR, stroke: stroke, strokeColor: strokeClr},

            Label:
            {
              mountX: 0.5, mountY: 0.0, x: (w => 0.5 * w), y:  (h => 0.15 * h),
              text: {  text: "Remove app ?", fontFace: 'Regular', fontSize: pts, textColor: textClr,
                        shadow: true,
                        shadowColor: 0xff444444,
                        shadowOffsetX: 1,
                        shadowOffsetY: 1,
                        shadowBlur: 2,
              },
            },

            OkButton:    { type: Button, label: "Ok",     clrFocus: 0xFF008800, mount: 0.5, x: (w => 0.25 * w), y: (h => h - 35), w: 150, h: 40 },
            CancelButton:{ type: Button, label: "Cancel", clrFocus: 0xFF880000, mount: 0.5, x: (w => 0.75 * w), y: (h => h - 35), w: 150, h: 40 },
        }//Dialog
      }
    };

    setLabel(s)
    {
      this.tag("Label").text = s;
    }

    _init()
    {
      this.buttons = [
                        this.tag("OkButton"),
                        this.tag("CancelButton")
                      ];

      this.buttonIndex = 1

      this.tag("Dialog").w = this.w;
      this.tag("Dialog").h = this.h;
    }

    static _states(){
      return [
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            class OKCState extends this
            {
              $enter()
              {
                this.tag("CancelButton").setFocus = true;
              }

              _handleLeft()
              {
                this.buttonIndex = 0;
              }

              _handleRight()
              {
                this.buttonIndex = 1;
              }

              handleEsc()
              {
                this.fireAncestors('$onRemoveCANCEL', this.pkgId, false);
              }

              _handleEnter() // could be OK or CANCEL button
              {
                var name = (this.buttonIndex == 0) ? '$onRemoveOK' : '$onRemoveCANCEL'

                this.fireAncestors(name);
              }

              _handleKey(k)
              {
                switch( k.keyCode )
                {
                  case 27: this.handleEsc(); break; // ESC key
                  default:
                    console.log("OKC ... GOT key code: " + k.keyCode)
                      break;
                }

                return true;
              }

              _getFocused()
              {
                return this.buttons[this.buttonIndex]
              }
            }
        ]}//_states
  }//CLASS
