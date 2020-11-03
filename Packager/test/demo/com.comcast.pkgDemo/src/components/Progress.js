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

export default class Progress extends lng.Component
{
    static _template( )
    {
      let RR = lng.shaders.RoundedRectangle;

      var  barClr1  = 0xFFcccccc;  // #ccccccFF  // Background
      var  frameClr = 0xFF666666;  // #666666FF

    return {
        ProgressBar: {
          Background: { x: -2, y: 0, w: 4, h: 12, rtt: true, rect: true, color: frameClr, shader: { radius: 3, type: RR} },
          Progress:   { x:  0, y: 2, w: 0, h:  8, rtt: true, rect: true, color: barClr1,  shader: { radius: 3, type: RR} },
        }
      }
    };

    getProgress()
    {
      return this.value;
    }

    reset()
    {
      this.value = 0;
      this.tag("Progress").w = 0;
    }

    setProgress(pc)
    {
      this.value = pc;

      var ww = (this.w -4) * pc;

      this.tag("Progress").setSmooth('w', ww, {duration: 1});
    }

    _init()
    {
      this.tag("Background").w = this.w;
      this.reset();
    }
  }//CLASS
