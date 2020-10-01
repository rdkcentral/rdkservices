import { Utils } from 'wpe-lightning-sdk'

export default class IconButton extends lng.Component
{
  static _template( )
  {
    let RR = lng.shaders.RoundedRectangle;

    return {
      flexItem: { margin: 40 },
      Button:
      {
        mount: 0.5,
        w: 45, h: 45,

        RRect:  {
          w: 45, h: 45, rect: true, color: 0x88888888, pivot: 0.5, alpha: 0.5, shader: { radius: 8, type: RR, stroke: 1, strokeColor: 0xFF333333 }
        },
        Image: {
          mount: 0.50,
          scale: 0.45,
          x: (45 * 0.5 ),
          y: (45 * 0.5 ),

          // alpha: 0.5,
          // shader:  { type: lng.shaders.Grayscale}
        },
      },
      }
    };

  _focus()
  {
    var bb  = this.tag("Button")
    var bg  = this.tag("RRect")
    var clr = this.clrFocus;

    bb.setSmooth('alpha', 1.00, {duration: 0.3});
    bg.setSmooth('alpha', 1.00, {duration: 0.3});

    bb.setSmooth('scale', 1.18, {duration: 0.3});
    bg.setSmooth('color',  clr, {duration: 0.3});
  }

  _unfocus()
  {
    var bb  = this.tag("Button")
    var bg  = this.tag("RRect")
    var clr = this.clrBlur;

   // bb.setSmooth('alpha', 0.50, {duration: 0.3});
    bg.setSmooth('alpha', 0.50, {duration: 0.3});

    bb.setSmooth('scale', 1.00, {duration: 0.3});
    bg.setSmooth('color',  clr, {duration: 0.3});
  }

  _init()
  {
    if(this.clrFocus == undefined) this.clrFocus = 0xFFaaaaaa; // #aaaaaaFF
    if(this.clrBlur  == undefined) this.clrBlur  = 0xFF888888; // #888888FF

    // var button = this.tag("Button");

    // button.w = this.w;
    // button.h = this.h;

   var image = this.tag("Image");

   image.patch( {src: this.src1 } );
  }

  setClrFocus(clr)      { this.clrFocus = clr};
  setClrBlur(clr)       { this.clrBlur  = clr};

}//CLASS
