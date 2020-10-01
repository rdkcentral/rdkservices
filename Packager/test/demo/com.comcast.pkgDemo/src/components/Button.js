export default class Button extends lng.Component
{
  static _template( )
  {
    let RR = lng.shaders.RoundedRectangle;

    return {
      Button:
      {
        RRect: { w: 150, h: 40, rect: true, color: 0xFF888888, shader: { radius: 8, type: RR, stroke: 1, strokeColor: 0xFF333333 } },
        Label: { mount: 0.5, x: (w => 0.5 * w), y: (h => 0.55 * h), text:{ text: '(unset)', fontSize: 20,  textColor: 0xFFffffff } },
      },
      }
    };

  setLabel(s)
  {
    var obj = this.tag("Label");
    obj.text.text = s;
  }

  _focus()
  {
    var bb  = this.tag("Button")
    var bg  = this.tag("RRect")
    var clr = this.clrFocus;

    bb.setSmooth('alpha', 1.00, {duration: 0.3});
    bb.setSmooth('scale', 1.18, {duration: 0.3});
    bg.setSmooth('color',  clr, {duration: 0.3});
  }

  _unfocus()
  {
    var bb  = this.tag("Button")
    var bg  = this.tag("RRect")
    var clr = this.clrBlur;

    bb.setSmooth('alpha', 1.00, {duration: 0.3});
    bb.setSmooth('scale', 1.00, {duration: 0.3});
    bg.setSmooth('color',  clr, {duration: 0.3});
  }

  _init()
  {
    this.pts       = 20;
    this.stroke    = 2;
    this.strokeClr = 0xCC888888;

    if(this.clrFrame == undefined) this.clrFrame = 0xFF666666; // #666666FF
    if(this.clrText  == undefined) this.clrText  = 0xFFffffff; // #ffffffFF

    if(this.clrFocus == undefined) this.clrFocus = 0xFFaaaaaa; // #aaaaaaFF
    if(this.clrBlur  == undefined) this.clrBlur  = 0xFF888888; // #888888FF

    var button = this.tag("Button");

    button.w = this.w;
    button.h = this.h;

    if(this.label)
    {
      this.setLabel(this.label)
    }
  }

  setClrBackground(clr) { this.clrBackground     = clr};
  setClrFocus(clr)      { this.clrFocus          = clr};
  setClrBlur(clr)       { this.clrBlur           = clr};
  setLabel(v)           { this.tag("Label").text = v; };

}//CLASS
