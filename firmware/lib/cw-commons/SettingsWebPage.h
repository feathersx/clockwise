#pragma once

#include <Arduino.h>
// TODO: Move the whole JS to a CDN soon
const char SETTINGS_PAGE[] PROGMEM = R""""(
<!DOCTYPE html>
<html>
<title>LED矩阵时钟设置</title>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
<link rel="shortcut icon" type="image/x-icon"
  href="https://github.com/jnthas/clockwise/raw/gh-pages/static/images/favicon.png">

<body>
  <div class="w3-container" style="background-image: linear-gradient(120deg, #155799, #159957);">

  </div>

  <div class="w3-bar w3-black w3-medium">
    <div id="fw-version" class="w3-bar-item w3-black w3-hover-red"></div>
    <div id="ssid" class="w3-bar-item w3-hover-blue w3-right"></div>
    <div class="w3-bar-item w3-button w3-hover-yellow w3-right" onclick="restartDevice();"><i class='fa fa-power-off'></i> 重启</div>
    <div id="status" class="w3-bar-item w3-green" style="display:none"><i class='fa fa-floppy-o'></i> 保存! 请重启设备</div>
  </div>

  <div class="w3-row-padding w3-padding">
    <div id="base" class="w3-col s3 m3 s12" style="display: none;">
      <div class="w3-card-4 w3-margin-bottom">
        <header class="w3-container w3-blue-gray">
          <h4 id="title">{{TITLE}}</h4>
        </header>
        <div class="w3-container">
          <p style="min-height: 45px;" id="description">{{DESCRIPTION}}</p>
          <hr>
          <div class="w3-row w3-section">
            <div class="w3-col" style="width:50px"><i id="icon" class="w3-xxlarge w3-text-dark-grey fa"></i>
            </div>
            <div class="w3-rest" id="formInput">
              {{FORM_INPUT}}
            </div>
          </div>
        </div>
        <button id="cardButton" class="w3-button w3-block w3-light-blue">保存</button>
      </div>
    </div>
  </div>
  <script>
    function createCards(settings) {
      console.log(settings);
      const cards = [
        {
          title: "屏幕亮度",
          description: "0 = 最暗 (屏幕关闭) / 255 = 刺眼的亮 | 已选值: <strong><output id='rangevalue'>" + settings.displaybright + "</output></strong>",
          formInput: "<input class='w3-input w3-border' type='range' min='0' max='255' value='" + settings.displaybright + "' class='slider' id='bright' oninput='rangevalue.value=value'>",
          icon: "fa-adjust",
          save: "updatePreference('displayBright', bright.value)",
          property: "displayBright"
        },
        {
          title: "使用24小时制显示?",
          description: "改变时间显示格式从8:00PM到20:00",
          formInput: "<input class='w3-check' type='checkbox' id='use24h' " + (settings.use24hformat == '1' ? "checked" : "") + "><label for='use24h'> Yep</label>",
          icon: "fa-clock-o",
          save: "updatePreference('use24hFormat', Number(use24h.checked))",
          property: "use24hFormat"
        },
        {
          title: "显示色彩不正常，切换色彩",
          description: "切换红绿蓝针脚顺序",
          formInput: "<select name='swapRGB' id='swapRGB'>"
          +"<option value='0'" + (settings.swapRGB == 0 ? " selected='selected'" : "") + ">RGB</option>"
          +"<option value='1'" + (settings.swapRGB == 1 ? " selected='selected'" : "") + ">RBG</option>"
          +"<option value='2'" + (settings.swapRGB == 2 ? " selected='selected'" : "") + ">GRB</option>"
          +"<option value='3'" + (settings.swapRGB == 3 ? " selected='selected'" : "") + ">GBR</option>"
          +"<option value='4'" + (settings.swapRGB == 4 ? " selected='selected'" : "") + ">BGE</option>"
          +"<option value='5'" + (settings.swapRGB == 5 ? " selected='selected'" : "") + ">BEG</option>"
          +"</select>",
          icon: "fa-random",
          save: "updatePreference('swapRGB', Number(swapRGB.checked))",
          property: "swapRGB"
        },
        {
          title: "屏幕显示重影",
          description: "开关屏幕时钟相位",
          formInput: "<input class='w3-check' type='checkbox' id='clockPhase' " + (settings.clockphase == '1' ? "checked" : "") + "><label for='clockPhase'> Yep</label>",
          icon: "fa-random",
          save: "updatePreference('clockPhase', clockPhase.value)",
          property: "clockPhase"
        },
        {
          title: "时区",
          description: "点<a href='https://en.wikipedia.org/wiki/List_of_tz_database_time_zones' target='_blank'>这里</a>查看所有时区，示例: Asia/Shanghai, America/Sao_Paulo",
          formInput: "<input id='tz' class='w3-input w3-light-grey' name='tz' type='text' placeholder='Timezone' value='" + settings.timezone + "'>",
          icon: "fa-globe",
          save: "updatePreference('timeZone', tz.value)",
          property: "timeZone"
        },
        {
          title: "NTP 对时服务器",
          description: "设置自动对时服务器. 你可以在这里 <a href='https://www.ntppool.org' target='_blank'>NTP Pool Project</a> 选一个最近的",
          formInput: "<input id='ntp' class='w3-input w3-light-grey' name='ntp' type='text' placeholder='NTP Server' value='" + settings.ntpserver + "'>",
          icon: "fa-server",
          save: "updatePreference('ntpServer', ntp.value)",
          property: "ntpServer"
        },
        {
          title: "自动亮度范围",
          description: "根据环境亮度自动调节亮度的范围 0表示关闭屏幕 范围 0 - 4095",
          formInput: "<input id='autoBrightMin' class='w3-input w3-light-grey w3-cell w3-margin-right' name='autoBrightMin' style='width:45%;' type='number' min='0' max='4095' placeholder='Min value' value='" + settings.autobrightmin + "'>" + 
          "<input id='autoBrightMax' class='w3-input w3-light-grey w3-cell' name='autoBrightMax' style='width:45%;' type='number' min='0' max='4095' placeholder='Max value' value='" + settings.autobrightmax + "'>",
          icon: "fa-sun-o",
          save: "updatePreference('autoBright', autoBrightMin.value.padStart(4, '0') + ',' + autoBrightMax.value.padStart(4, '0'))",
          property: "autoBright"
        },
        {
          title: "光敏电阻针脚设置",
          description: "光敏电阻默认连接在ESP32 GPIO的35脚上，点击 | <a href='#' onclick='readPin(ldrPin.value);'>读取当前阻值: </a><strong id='ldrPinRead'>0</strong>",
          formInput: "<input id='ldrPin' class='w3-input w3-light-grey' name='ldrPin' type='number' min='0' max='39' value='" + settings.ldrpin + "'>",
          icon: "fa-microchip",
          save: "updatePreference('ldrPin', ldrPin.value)",
          property: "ldrPin"
        },
        {
          title: "表盘皮肤名称",
          description: "表盘皮肤名称",
          formInput: "<input id='themeName' class='w3-input w3-light-grey' name='themeName' type='text' placeholder='文件名' value='" + settings.themename + "'>",
          icon: "fa-file-image-o",
          save: "updatePreference('themeName', themeName.value)",
          property: "themeName"
        },
        // {
        //   title: "表盘文件名称",
        //   description: "要显示的文件名，不带扩展名",
        //   formInput: "<input id='descFile' class='w3-input w3-light-grey' name='descFile' type='text' placeholder='Description File' value='" + settings.canvasfile + "'>",
        //   icon: "fa-file-image-o",
        //   save: "updatePreference('canvasFile', descFile.value)",
        //   property: "canvasFile",
        // },
        {
          title: "表盘地址设置",
          description: "描述文件所在的服务器地址。更改后可在本地测试",
          formInput: "<input id='serverAddress' class='w3-input w3-light-grey' name='serverAddress' type='text' placeholder='Canvas Server' value='" + settings.canvasserver + "'>",
          icon: "fa-server",
          save: "updatePreference('canvasServer', serverAddress.value)",
          property: "canvasServer",
         
        },

        {
          title: "旋转屏幕",
          description: "旋转LED屏幕显示方向",
          formInput: "<select name='rotation' id='rotation'><option value='0'" + (settings.displayrotation == 0 ? " selected='selected'" : "") + ">0</option><option value='1'" + (settings.displayrotation == 1 ? " selected='selected'" : "") + ">90</option><option value='2'" + (settings.displayrotation == 2 ? " selected='selected'" : "") + ">180</option><option value='3'" + (settings.displayrotation == 3 ? " selected='selected'" : "") + ">270</option></select>",
          icon: "fa-rotate-right",
          save: "updatePreference('displayRotation', rotation.value)",
          property: "displayRotation"
        }
      ];

      var base = document.querySelector('#base');
   
      cards.forEach(c => {
        if (!c.hasOwnProperty('exclusive') || (c.hasOwnProperty('exclusive') && c.exclusive === settings.clockface_name)) {
          var clone = base.cloneNode(true);
          clone.id = c.property + "-card";
          clone.removeAttribute("style");

          Array.prototype.slice.call(clone.getElementsByTagName('*')).forEach(e => {
            e.id = e.id + "-" + c.property;
          });

          base.before(clone);
          document.getElementById("title-" + c.property).innerHTML = c.title
          document.getElementById("description-" + c.property).innerHTML = c.description
          document.getElementById("formInput-" + c.property).innerHTML = c.formInput
          document.getElementById("icon-" + c.property).classList.add(c.icon);
          document.getElementById("cardButton-" + c.property).setAttribute("onclick", c.save);
        }
      })

      document.getElementById("ssid").innerHTML = "<i class='fa fa-wifi'></i> " + settings.wifissid
      document.getElementById("fw-version").innerHTML = "<i class='fa fa-code-fork'></i> Firmware v" + settings.cw_fw_version
    }
   var nyancat= '{"name":"NyanCat","version":1,"author":"@jnthas","bgColor":396,"delay":100,"setup":[{"type":"datetime","content":"H:i","font":"","fgColor":65535,"bgColor":396,"x":34,"y":45},{"type":"datetime","content":"m-d","font":"","fgColor":65088,"bgColor":396,"x":1,"y":45},{"type":"line","color":29582,"x":10,"y":41,"x1":54,"y1":41},{"type":"image","x":3,"y":0,"image":"iVBORw0KGgoAAAANSUhEUgAAAAUAAAAFCAIAAAACDbGyAAAALUlEQVQI12NkME5jQAJMEOr/mZkwEeO0////w0lGiPr/Z2YymqQzMDAwoukHAEsuDvoaMg+MAAAAAElFTkSuQmCC"},{"type":"image","x":10,"y":34,"image":"iVBORw0KGgoAAAANSUhEUgAAAAUAAAAFCAIAAAACDbGyAAAALUlEQVQI12NkME5jQAJMEOr/mZkwEeO0////w0lGiPr/Z2YymqQzMDAwoukHAEsuDvoaMg+MAAAAAElFTkSuQmCC"},{"type":"text","content":"enjoy your day!","font":"picopixel","fgColor":26592,"bgColor":396,"x":6,"y":60}],"sprites":[[{"image":"iVBORw0KGgoAAAANSUhEUgAAAEAAAAAaCAYAAAAHfFpPAAACGElEQVRYw9VZS24CMQy1q1kgboOCuFFt9URVwo2qjuhpELt0AZlm8nVgYFJLLBgST/z8ezEIiiz0IKNZXqei6pIhfGCFB0GBcqk+BAB7Wh4A3FEVBLQAq0ZAZPzX++NKD0c4XyxsN1gF4Q3+q3wer5+MbDcoUtMXADXvFwxu0lNMAS3cycJ1FX3ItxRwh3ZGfiyTCrUUGMQGNxr29NrBfyFutZWB5wqyB8jQXW4LPI+MoLWefZ9AKLRZt4eZJxD6A+DJ4FV5AJBwp5GGaoIb2MZaUgltrXU5CjzvxwDQnWhL9zHMXs7MgBiAsIA4EFLPy13APpcIIcYHYeYpEhCDLuB7PePxsAa0ins/KIoB2AuVfAvX7QFgxNgj0yEgA0AmFfCHHjI+BGLY37m5ZZ+6QTwidCcvZ4LO81rr2JM1ppdJCSKaRVTLmpcC4CKBmdMHvpP9IWI51wtrUAU1YDSymqhIFs9ZfT6TK9WAoBi6AkieQaahpfj7kAgGqcFiw8R9y86BOByvIIQ9XxgVzBx1mtL37m6D58s6Y4luqPB2g9cogLLHrbZXwpPwZlhXUnXGWDvnAaDnCURCimuETDCnzwQ90Z7MNMUJIyI13MAdiZheEiTvNoik7Mtjz4wxk6u1seVaEfWXAn5Ilq6uLdfc8AKU09PPSEzRKq+NUsAIZ10kHAkl9aVG5T4A/u8SYHLrBXqwmz9GVpJf7j/+4Qx3B68AAAAASUVORK5CYII="},{"image":"iVBORw0KGgoAAAANSUhEUgAAAEAAAAAaCAYAAAAHfFpPAAACF0lEQVRYw9VZUW7DIAy1p3xUPc0qot5otnaiCnKjadF2mmp/3keahRBCTJs0DKkfQWDw4/nZUARDAnu31q1v05BqWOV/iHIjqDSusYcAIF/rA4AnUoGAArAbAybOf7w9bvTcwPVH4HhAFQgv8B/bpel+M+14QLWpcgBYOv2Ew1l2kiFglbNYOW7BHvItBPpN906+rxMKmhCo1E5nOPYU/eCB5mJFB14vyh4oVVGxrTx5ZARr7ehbXt1iqu3nMPMfCGUB8ATwknUAkHKWNm3TClqioLa1tmOBlVna+4wZA0B3wJY5B2/hKhtWHD0IsX49A7YQKxw2gcg6EC7NcOIZ1F5ydsIMQ2MAauXcT+W4WtmniudbKOA35TmaAIqZoarvMFA/sHiLUFTbPASMADAyWGsnJ8fMcdqnWMFTASIiQMQkM+bGPKUUNtI5O3J4pTTWaUucVv56c2PQyFAKt04n04ZQqTMRe4FSj0rhlBgGRRB5DrmM9OLPQyKotE4vOqZWIJmCcW46EMKcn8EKZh7Re+m7qNvg9We/R6kiSuHjATsWwPKJi5Wu4ImcZqgxMc1xIkO/IUCwQwCRssR1ykpwzp5rpzrQv+KEjJh73MAT5Rc/wUWo0wEjT+WfazGdDjdPSVRWCPh0TF1bc6+5fn/KVhlPYoZ2W3oUAk751kXKJ6GovdhTeQiAP0YDztz4cK2ILSzij5Ed2y9yGe3wdlfpIwAAAABJRU5ErkJggg=="},{"image":"iVBORw0KGgoAAAANSUhEUgAAAEAAAAAaCAYAAAAHfFpPAAACEElEQVRYw+VZXW7CMAy2UR8Qu8xQEdIOhC1OhNIeaNIE2i4zxNPMQ5suDU3jlIrSzVKltEocbH/+BSEngX9MKACNAjCn6AE5FTrGCl4NKXkmkfL+lgImsQAAyOf4CsA1qZQwqQJuhP/Y3c90W8L5IrBaokoJi1k67qGsngCtlqhmlYGpV6w8YZT7OFGomPUPJcB+p+OzLROCoJnQBbh2ASu8tep+HFfQuEA22/TFvzAXIzrl2WzjKCUDu9YGYm12GxLYlZZHRjDGtN7ltYimWnuGmRslzBYBQ5UXdgEa+Qdp+GkCpQLaxpgKBUaCsHcR8ydiQEgJXd97EbCpF0flRRvlvuM90rgpLwHaMWFvkJETYC7TpcETemlQ4Qr4RWmC9nkg80wrwRFp8Uhr+09nsOuL8gF3IKIqtfXF5MCe7FRIXRug0nV0HmP5WUG7YNv6QXdUf4gYhLi9N7QnSxVMH2Ok6flc4WOW0tb/5AoUiAnuvXbtnuPJukEj1ePU7Z1dXgIqfMXG3h9XBxgB5n73Ol8EVlP1FJSLAAAUykqQlDW+ih/joIGI3wtoXavVC9RNUaOAKahwUsH3u2kGGeeLRIcbuKb04sdrhJ6iFLYCvLzxoPNJQdVrhZ+vF8gp2LamtLju9xifrPBaMorMvArlrIvAJCMh2ZpjBMGH/zEiPwC4aE9ofHi632Oj7dDeEG+PrkNm45JCFy++AAAAAElFTkSuQmCC"},{"image":"iVBORw0KGgoAAAANSUhEUgAAAEAAAAAaCAYAAAAHfFpPAAACA0lEQVRYw+VZXW7CMAy2UR8Qpxkq4kbY4kQo7YEmTVTbZYZ4mvdAU9KUNE5BhG6WKqUlcfHnfxehJIF/TCgAHQBYUvSANJWOsYJXR0qeSaR8fw+ALBoAAPl8PAC4JhUIWQEYCP+xu5/ptobTWWC1RBUIi1k67qG+XAFaLVHNqgDTrlh5wij3caJQMe0faoD9TsdnWycEQZPRBbh1ASu81er+Ma6gcYFitumLr2YuRnTg2WzjgFKAXWsDsTa7TQnsSs0jIxhjevfyVkVTrT3DzB0Is7WAqeCFXYAe/Ic0/DSBUmHaxpiLFRgJmr1rMX8iBoRAuPV81AI27eKofNFGue94jzRuyksw7ZiwA8soCbCUfGmwQS8NKlwBvyhN0DEPZH6+CzT4Wq5TPFtwV3vMrK/07G88jK5EBIg4ahmhPUVTSVsboNJ1dB5j+d0S/JFpDAAAEYMmbt8b2lOkCqaPMdL1fL0CZGow9DXqChQA1wXdrt1znKUbNHK9nLr9ZpeXYBU+uLH7l2qHT+d8MxmkUgQAoFJWgqSs8Xv8bIHiV2qMkwYifi+gda+BK5Z0BSAHVU5O/H433SDDtYjQcAPXlF78eI1QzwJyCD85ON5D3nzgdXqBkoIt65Q2130+xq+ovJaMIjOvSjnrIjAwB8KnfxiRHwBc9L8FuJoJPY81NjFeAX6/S9HmbMNT2N8AAAAASUVORK5CYII="},{"image":"iVBORw0KGgoAAAANSUhEUgAAAEAAAAAaCAYAAAAHfFpPAAACBklEQVRYw+VZUW7CMAy1p34gbjNl4kbY4kQo6Y3QKnYaxJ/3AS1p0zROV0jRIlWiJXHjZ/vZThEMCfzjUQ0fSONUC9GQap5a3jO0U+wRBaC4ByAAyNktL/eTkiAUByBQ/rT/u9BdDZerwHaDSRA+3jZ4j/XtioztBudxAFjlBlg5z2bISVn/WAMcFB5y2gPsap0Hii0cAnwPgVb51qqHZUIhFQLVu0YA8sPFxYoOvDYjeYCsDwCF5ZERrLW9+w6E2Ghct4aZOxBCAEi5UW3WooW4ZCHwwhCgmS+jF3pFwrWttdNe4Fm/CAfgSEaSham3BWHseVYa/FK+8DtD+eEmmHkUlGjKU7p2StmAEA0BGnluGmwwvjnmGwH00mACFPyhPEWn6If5+SFg7vAy8qjFWhBKjZeVwkb6l+8BAdllsjwRJYGMzanC0NBFhCFUhppMl8g+cc2s/jBCKMzchUtsTqVVeC5Qi6TAEWDIVyjCCT5XtL/9dbyqbrBtXoZdntIrhu6dul9dO3y5lunJAg4gZYnrlJVgSp7z+/ddDQDTFhcrt4JnIq1OWd2JPJ4bAiQjRaB3TZ+U5Oy6U5yhR4wdbuAn5Rc/g0aoWDfoGgw23io0p5jJy8e0vvMA3yWnWtecNnfYAMXkVGFM6hAl5dmZVh4YKgI+FvkwMvatwAfA/18DTGy+Qs4vFq3iyqTgq5sAAAAASUVORK5CYII="},{"image":"iVBORw0KGgoAAAANSUhEUgAAAEAAAAAaCAYAAAAHfFpPAAACGElEQVRYw+VZUZKCMAxNdvhwvI1TxhuZjCdyWm7kLLO3cfzLfiBSSqGhqOhuZ/ygtoG8JI+XgmBI4B+PIpyQ2qk2oiHVOrW9Z3ineEYUgNUzAAFAftzj7e4oCcLqAAycPx+WG91XcLkKbDeYBOHrY4v3VDW/kbHdYB4HgFU+ACvX2Rl2UtE/VQBHRYacDwD7SpeBYlcuAb6VQOt8G9WEo8hdhMXKaCn0SqAlZK8kik/MfmQEa23vWqxMg1e7+x5mvoPwfgAcDy+93RAAUu7UvrXoQVwyRTPWdlkQG170hwBQ5l1n7sNbyUoO4yh4oQUhNj8vA55Rs9g9CCLngaDMBPWoHYChIQClcv+3cl2pnJt85XmRDwlwCVDMDEWZaaRc8AC1RqPE0v1UPTxrXlICRgAYGay1g+gxL2dBIgLE6cwYW/MyKWykcXbS4VSER0gQMZ5S/r3G1qCRvhKsnY6hDKGSayL2ArbuKUGlCCLPITeDVf19SASF1mGVY2oWklEwcvU/M/fSO3X9ft1g27yEXd6TleHbSOHLVWCrOWGy0gieSDRDfonxjRPp5g0Bgu0XECklrlMqwZQ9V2PWgUir+uZqAr8RanjAyCrtsKuHRNie4rQZkTrcwF2Gjg9Oh4q1nA8jl+XMX+EAvyanevdUp9dbG3SAY3aKYZerU2akPDvT2gOzTgbgKh9GYt8KfAD8/1PATK1V2PkF6t7s7LMI4OgAAAAASUVORK5CYII="}],[{"image":"iVBORw0KGgoAAAANSUhEUgAAAAUAAAAFCAIAAAACDbGyAAAALUlEQVQI12NkME5jQAJMEOr/mZkwEeO0////w0lGiPr/Z2YymqQzMDAwoukHAEsuDvoaMg+MAAAAAElFTkSuQmCC"},{"image":"iVBORw0KGgoAAAANSUhEUgAAAAUAAAAFCAIAAAACDbGyAAAAL0lEQVQI12XLMREAIAzAwFQTaCqa6qmewgQDLFn+EowE7Iq5ABip3oZ6BLs+f/4Ng8IcvVqv7xgAAAAASUVORK5CYII="},{"image":"iVBORw0KGgoAAAANSUhEUgAAAAUAAAAFCAIAAAACDbGyAAAALUlEQVQI12P8//8/o0k6AwMDAwPD/zMzGRmM0xiQABMDKsDg/z8zE875f2YmAAIEC2fkjmhRAAAAAElFTkSuQmCC"}]],"loop":[{"type":"sprite","movement":0,"sprite":0,"x":0,"y":5},{"type":"sprite","movement":0,"sprite":1,"x":58,"y":3},{"type":"sprite","movement":0,"sprite":1,"x":31,"y":31}]}';

   function updatePreference(key, value) {
      const xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status >= 200 && this.status < 299) {
          document.getElementById('status').style.display = 'block';
        }
      };
      if (value.indexOf("nyan-cat")>-1){
            var data = "themeJson="+ nyancat ;
            xhr.open('POST', '/set?' + key + '=' + value);
            xhr.setRequestHeader('Content-Type','application/x-www-form-urlencoded');
            xhr.send(data);
      } else {
            xhr.open('POST', '/set?' + key + '=' + value);
            xhr.send();
      }

      

      setTimeout(() => {
        document.getElementById('status').style.display = 'none';
      }, 2000);
    }

    function splitHeaders(request) {
      const headers = request.getAllResponseHeaders().trim().split(/[\r\n]+/);
      const headerMap = {};
      headers.forEach((line) => {
        const parts = line.split(": ");
        const header = parts.shift().substring(2);
        const value = parts.join(": ");
        headerMap[header] = value;
      });
      return headerMap;
    }

    function requestGet(path, cb) {
      var xmlhttp = new XMLHttpRequest();
      xmlhttp.onreadystatechange = function () {
        if (this.readyState === 2 && this.status === 204) {
          cb(this);
        }
      };
      xmlhttp.open("GET", path, true);
      xmlhttp.send();
    }
    
    function readPin(pin) {
      requestGet("/read?pin=" + pin, (req) => {
        var headers = splitHeaders(req);
        document.getElementById("ldrPinRead").innerHTML = headers.pin;  
      });  
    }

    function begin() {
      requestGet("/get", (req) => {
        createCards(splitHeaders(req));
      });  
    }

    function restartDevice() {
      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/restart');
      xhr.send();
    }

    //Local
    //createCards({ "displayBright": 30, "swapRGB": 1, "use24hFormat": 0, "timeZone": "Europe/Lisbon", "ntpServer": "pool.ntp.org", "wifiSsid": "test", "autoBrightMin":0, "autoBrightMax":800, "ldrPin":35, "cw_fw_version":"1.2.2", "clockface_name":"paint", "canvasServer":"raw.githubusercontent.com", "canvasFile":"star-wars.json" });

    //Embedded
    begin();

  </script>
</body>
</html>
)"""";
