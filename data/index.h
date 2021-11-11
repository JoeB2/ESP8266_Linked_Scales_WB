extern const char index_html[] PROGMEM =  R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta name="file" content="index.html">
    <meta name="author" content="Joe Belson 20210822">
    <meta name="what" content="esp8266-01 html for WiFi connected weight scales.  Intended for W&B for small airplane M5">
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=480, initial-scale=1.0" />
    <title id="title">Plane Scale </title>
  </head>
  <style type="text/css">
    body {text-align: center; background-color: #ffffff;  font-family: Calibri, Myriad;  }
    table {margin-left:10px; margin-right: auto;  width: 315px;  border: 1px solid black; border-style:solid;}
    table th{text-align: center;width: auto;border-bottom:2px solid rgb(8, 8, 8); height: 26px;}
    table tr{text-align: center; height:30px;}
    table td{text-align: center; height: 30px;}
    table.scaleTable caption {
      background-color: #f79646;
      color: #fff;
      font-size: x-large;
      font-weight: bold;
      letter-spacing: .3em;
    }
    table.scaleTable .scale{text-align:center;font-weight: bold; border: 1pt solid black;}
    table.scaleTable .weight{
      text-align:center;
      background-color: rgba(31, 175, 50, 0.295);
      color:rgb(8, 8, 8);
      font-weight: bolder;
      text-align:center;
      width: 33%;
      border: 2px solid #ad0505;
    }
    table.scaleTable .tare{
      width: 80%;
      color: aliceblue; 
      background-color: rgb(96, 182, 96);
      color: black;
      font-weight: bold;
      text-align:center;
      font-size: 13px;
    }
    table.scaleTable .calib{
      font-size: 13px;
      color: black;
      background-color: yellow;
      font-weight:bold;
      width: 70px;
    }
    input[type=submit] {
      border: 0;
      line-height: 2.5;
      padding: 0 20px;
      font-size: 13px;
      text-align: center;
      
      font-weight: bolder;
      padding: 3px 3px;
      text-decoration: none;
      margin: 0px 0px;
      cursor: pointer;
      width: auto;
      box-shadow: inset 2px 2px 3px rgba(255, 255, 255, .6),
                  inset -2px -2px 3px rgba(0, 0, 0, .6);
   }
   .viewCode{
      width: 114px;
      cursor: pointer;
      color: #fff;
      text-shadow: 1px 1px 1px #000;
      border-radius: 10px;
      background-color: rgba(220, 0, 0, 1);
      background-image: linear-gradient(to top left,
                                        rgba(0, 0, 0, .2),
                                        rgba(0, 0, 0, .2) 30%,
                                        rgba(0, 0, 0, 0));
      box-shadow: inset 2px 2px 3px rgba(255, 255, 255, .6),
                  inset -2px -2px 3px rgba(0, 0, 0, .6);
    }
  </style>
  <body style="text-align: left;"><input id="bt_help" type=button onclick="f_help()" value="Help"></input>
  <hr>
    <table class="scaleTable">
      <caption id="caption">Plane Scale</caption>
        <thead>
            <tr>
                <th>Scale</th>
                <th>Units</th>
                <th >Tare</th>
                <th>Calibrate</th>
            </tr>
        </thead>
        <tbody>
            <tr id="Scale1">
                <td class="scale">1</td>
                <td class="weight" id="s1_reading">inop</td>
                <td><input class="tare" type="submit" value="Tare" onclick="f_tare(1)"></td>
                <td><input class="calib"  type="submit" value="Calibrate" onclick="f_redirect('calib', 1)"></td>
            </tr>
            <tr id="Scale2">
                <td class="scale">2</td>
                <td class="weight" id="s2_reading">inop</td>
                <td><input class="tare" type="submit" value="Tare" onclick="f_tare(2)"></td>
                <td><input class="calib" type="submit" value="Calibrate" onclick="f_redirect('calib', 2)"></td>
            </tr>
            <tr id="Scale3">
                <td class="scale">3</td>
                <td class="weight" id="s3_reading">inop</td>
                <td><input class="tare" type="submit" value="Tare" onclick="f_tare(3)"></td>
                <td><input class="calib" type="submit" value="Calibrate" onclick="f_redirect('calib', 3)"></td>
                </tr>
            <tr id="Scale4">
                <td class="scale">4</td>
                <td class="weight" id="s4_reading">inop</td>
                <td><input class="tare" type="submit" value="Tare" onclick="f_tare(4)"></td>
                <td><input class="calib" type="submit" value="Calibrate" onclick="f_redirect('calib', 4)"></td>
              </tr>
             <td><b>Total:</td>
                <td id="total" style="text-align: center;font-weight: bold;"></td>
              </tr>
              <tr>
                <td><b>1 & 2:</td>
                <td id="total_1.2" class="weight"></td>
              </tr>
              <tr>
                <td><b>3 & 4:</td>
                <td id="total_3.4" class="weight"></td>
              </tr>
    </table>
    <hr>
    <id id="help" style="visibility:collapse;text-align: left;">
            1) Click Tare to zero scale
    <br>2) Click Calibrate to goto calibration page.
    <br><br id="helpMessage" style="text-align: left;"></id>
  </p>
  <p id="msgArea0" style="text-align: left; visibility: collapse;">Message Area</p>
  <p id="msgArea1" style="text-align: left; visibility: collapse;">Scale1 JSON</p>
  <p id="msgArea2" style="text-align: left; visibility: collapse;">Scale2 JSON</p>
  <p id="msgArea3" style="text-align: left; visibility: collapse;">Scale3 JSON</p>
  <p id="msgArea4" style="text-align: left; visibility: collapse;">Scale4 JSON</p>
  <p id="codeButtons" style="text-align: left; visibility: collapse;">
    <input class="viewCode" type="button" value="Show cpp" onclick="f_redirect('cpp')">
    <input class="viewCode" type="button" value="Show index.h" onclick="f_redirect('indexH')"><br>
    <input class="viewCode" type="button" value="Show index.html" onclick="f_redirect('html_index')">
    <input class="viewCode" type="button" value="Show calib.html" onclick="f_redirect('html_calib')" width="113px">
    <input class="viewCode" type="button" value="Show Platformio.ini" onclick="f_redirect('platformio')" width="auto">
   </p>
  
</body>
  <script language = "javascript" type = "text/javascript">

    var lastUpdated = [new Date().getTime(), new Date().getTime(), new Date().getTime(), new Date().getTime()];

    myVar = setInterval(function(){
      var now = new Date().getTime();

      for(i=0;i<4;i++)
        if(now - lastUpdated[i] > 14000){
            document.getElementById("s" + (i+1) + "_reading").innerHTML = "inop";
            document.getElementById("msgArea" + (i+1)).innerHTML = "Scale" + (i+1) + " JSON";
        }

      f_tallyTotals();
    }, 15000); // check for scale update every 15 sec - "inop" if has had no update 
    
    const webSock = new WebSocket('ws://'+window.location.hostname+':80');
    webSock.onopen = function(evt){f_webSockOnOpen(evt);}
    webSock.onmessage = function(evt){f_webSockOnMessage(evt);}
    webSock.onerror  = function(evt){f_webSockOnError(evt);}

    function f_webSockOnMessage(evt){
        if(typeof evt.data === "string")j=JSON.parse(evt.data);

        f_weight_update(Number(j.scaleNo), Number(j.reading), Number(j.SCALE), Number(j.offset))
        f_tallyTotals();
    }
    function f_webSockOnOpen(evt){
                var msg = document.getElementById("msgArea0");
                msg.style.wordWrap = "break-word";
                msg.style.color = "green";
                msg.innerHTML = "WebSock: CONNECTED"
    }
    function f_webSockOnError(evt){
                var msg = document.getElementById("msgArea0");
                msg.style.wordWrap = "break-word";
                msg.style.color = "red";
                msg.innerHTML = "WebSock: ERROR";
    }
    function f_weight_update(scaleNo, reading, SCALE, offset){

      const weight =(reading==-1)?"inop":Math.round(100*((reading-offset)/SCALE))/100;
      if(weight != -1)lastUpdated[scaleNo-1]=new Date().getTime();

      document.getElementById("s"+scaleNo+"_reading").innerHTML=weight;
      document.getElementById("msgArea"+j.scaleNo).innerHTML=JSON.stringify(j);
    } 
    function f_tare(scaleNo){
          const msg = document.getElementById("msgArea"+scaleNo).innerHTML;
          const scaleJSON = JSON.parse(msg);

          const reading = (isNaN(scaleJSON.reading))?-1:scaleJSON.reading;
          const SCALE = (isNaN(scaleJSON.SCALE))?1:scaleJSON.SCALE;
          const isSave = scaleJSON.isSave;

          let tmp = "{\"scaleNo\":"+scaleNo
              +",\"reading\":"+reading
              +",\"SCALE\":"+SCALE
              +",\"offset\":"+reading
              +",\"isTare\":1,\"isSave\":"+isSave+"}";

          webSock.send(tmp);
    }
    function f_tallyTotals(){  // tally totals from scale1, 2, 3, 4 values
        var scales = [0, 0, 0, 0];
        var total = Number(0);

        for(i=0;i<4;i++){
          scales[i]= isNaN(document.getElementById("s"+(i+1)+"_reading").innerHTML)? Number(0): Number(document.getElementById("s"+(i+1)+"_reading").innerHTML);
          total += scales[i];
        }
        document.getElementById("total").innerHTML=Math.round(100*(total))/100;
        document.getElementById("total_1.2").innerHTML=Math.round(100*(scales[0]+scales[1]))/100;
        document.getElementById("total_3.4").innerHTML=Math.round(100*(scales[2]+scales[3]))/100;
      }
      function f_redirect(where, scaleNo=-1){
                      window.location.href = "/"+where+"?scaleNo="+scaleNo;
      }
      function f_help(){
        nxt=document.getElementById("help").style.visibility=="collapse"?"initial":"collapse";
        document.getElementById("bt_help").value=nxt=="collapse"?"Help":"un-Help";
        rray=["help","msgArea0","msgArea1","msgArea2","msgArea3","msgArea4","codeButtons"];
        rray.forEach(function(val){document.getElementById(val).style.visibility=nxt;});
      }
  </script>
</html>
)=====";

extern const char calib_html[] PROGMEM =  R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta name="file" content="Calib.html">
    <meta name="author" content="Joe Belson 20210822">
    <meta name="what" content="esp8266-01 html for WiFi connected weight scales: HX711 Calibration.  Intended for W&B for small airplane M5">
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Plane Scales Calibration</title>
  </head>
<style type="text/css">
      body {font-size: 12px; background-color: #f6f6ff;font-family: Calibri, Myriad;text-align: center;}
      body p{text-align: left;}
      table th{text-align: center;width: 23pt;border-bottom:2px solid rgb(8, 8, 8);}
      table.calibrate {font-size:smaller ; margin-left:10px; margin-right: auto;  width: 300px;  border: 1px solid black; border-style:solid;}
      table.calibrate caption {	background-color: #f79646;	color: #fff;	font-size: large;	font-weight: bold;	letter-spacing: .3em;}
      table.calibrate thead th {
                  padding: 3px;
                  background-color: #fde9d9;
                  font-size:16px;
                  border-width: 1px;
                  border-style: solid;
                  border-color: #f79646 #ccc;
      }
      table.calibrate td {
                  font-size: 12px;
                  font-weight: bold;
                  text-align: center;
                  padding: 2px;
                  background-color:shite;
                  color:black;
                  font-weight: bold;
      }
</style>
<body style="text-align: left;"></input><tab id="saveMsg" style="padding: 50px">Calculates & Saves SCALE Factor.<br><tab style="padding: 58px; font-weight: bolder;">REMOVE Weight & Click Reset<tab style="padding: 33px"><input id="bt_help" type=submit onclick="f_help()" value="Help">
  <hr>
    <table class="calibrate">
      <caption>Plane Scales Calibration</caption>
        <thead>
            <tr>
                <th>Scale</th>
                <th>RAW</th>
                <th>Actual</th>
                <th>SCALE</th>
                <th>Offset</th>
                <th><input id="reset" type="submit" value="Reset" onclick="f_reset()" style="color: black; background-color: yellow; font-weight: bold;"></th>
                <th><input id="save" type="submit" value="Save" onclick="f_save()" style="color: aliceblue; background-color: red; font-weight: bold; visibility: hidden;"></th>
            </tr>
        </thead>
        <tbody>
            <tr>
                    <td id="scaleNo"></td>
                    <td id="reading"></td>
                    <td>
                              <input id="actual" disabled onmouseout="f_setDivider()" style="width: 80px; border: hidden;"></input>
                  </td>
                    <td id="SCALE"></td>
                    <td id="offset"></td>
                    <td></td>
            </tr>
    </table>
              <hr>
              <p id="help" style="visibility:collapse;">
                      1) Clear scale of any weight.
              <br>2) Click Reset.  SCALE goes to 1.  "Save" button appears. Offset Value changes.
              <br>3) Place known weight on scale.
              <br>4) Enter the known weight's value under "Actual Weight".
              <br>5) Verify SCALE value has computed. (Raw Weight &divide; Known Weight)
              <br>6) Click Save.
            </p>
            <p id="msgArea" style="visibility: collapse;">Message Area</p>
            <p id="msgJSON" style="visibility: collapse;">sock Message</p>
            <p id="msgReset" style="visibility: collapse;">reset Message</p>
            <p id="msgSave" style="visibility: collapse;">save Message</p>
          </body>
  <script language = "javascript" type = "text/javascript">

      const args = location.search;
      const urlParams = new URLSearchParams(args);
      window.scale = urlParams.get('scaleNo');
      document.getElementById("scaleNo").innerHTML=window.scale;

      let webSock       = new WebSocket('ws://'+window.location.hostname+':80');
      webSock.onopen    = function(evt){f_webSockOnOpen(evt);}
      webSock.onmessage = function(evt){f_webSockOnMessage(evt);}
      webSock.onerror   = function(evt){f_webSockOnError(evt);}

      function f_webSockOnMessage(evt){
        if(typeof evt.data === "string"){

          j=JSON.parse(evt.data);
              
                    if(scale==j.scaleNo){
                          window.reading =Number(j.reading);
                          window.SCALE   =Number(isNaN(j.SCALE)?1:j.SCALE);
                          window.offset  =Number(j.offset);

                          window.weight  = Math.round(100*(window.reading-window.offset)/window.SCALE)/100;

                          document.getElementById("reading").innerHTML=window.reading;
                          document.getElementById("SCALE").innerHTML=window.SCALE;
                          document.getElementById("offset").innerHTML=window.offset;

                          if( j.isSave && !j.isTare){
                                document.getElementById("save").style.visibility="visible";
                                document.getElementById("actual").disabled=false;
                          }
                          j.weight=window.weight;
                          document.getElementById("msgJSON").innerHTML=JSON.stringify(j);
                    }// if scaaleNO
        }
      }  // END: Func def: webSockOnMessage
      function f_webSockOnOpen(evt){
                var msg = document.getElementById("msgArea");
                msg.style.wordWrap = "break-word";
                msg.style.color = "green";
                msg.innerHTML = "WebSock: CONNECTED"
      }
      function f_webSockOnError(evt){
                var msg = document.getElementById("msgArea");
                msg.style.wordWrap = "break-word";
                msg.style.color = "red";
                msg.innerHTML = "WebSock: ERROR";
      }
      function f_setDivider(){
        const reading =document.getElementById("reading").innerHTML;
        const actual = document.getElementById("actual").value;
        const offset = document.getElementById("offset").innerHTML;

        document.getElementById("SCALE").innerHTML =
                    (reading != "0" && actual.length)?
                    Math.round(10000*(reading-offset)/actual)/10000:1;
      }
      function f_reset(){
        const reading=document.getElementById("reading").innerHTML;
        const SCALE=document.getElementById("SCALE").innerHTML;

        var tmp =
               "{\"scaleNo\":"+document.getElementById("scaleNo").innerHTML
                +",\"reading\":"+reading
                +",\"SCALE\":1"+",\"offset\":"+reading
                +",\"isTare\":1,\"isSave\":1}";

                document.getElementById("msgReset").innerHTML = "reset: "+tmp;

            webSock.send(tmp);
      }
      function f_save(){
        const reading=document.getElementById("reading").innerHTML;
        const actual=document.getElementById("actual").value;
        const offset=document.getElementById("offset").innerHTML;

        const SCALE=Math.round(10000*((reading-offset)/actual))/10000;
        if(isNaN(SCALE))SCALE=1;

        console.log("SCALE: " + scale + "\treading:"+reading+"\tSCALE: "+SCALE+"\tactual: "+actual+"\n");
        
        var tmp = "{\"scaleNo\":" +document.getElementById("scaleNo").innerHTML
                        +",\"reading\":" +reading
                        +",\"SCALE\":" +SCALE
                        +",\"offset\":"+offset
                        +",\"isTare\":0,\"isSave\":1}";

        document.getElementById("msgSave").innerHTML="Save: " + tmp;

        webSock.send(tmp);
      }
      function f_help(){
        if(document.getElementById("help").style.visibility=="collapse"){
          document.getElementById("help").style.visibility="initial";
          document.getElementById("msgArea").style.visibility="initial";
          document.getElementById("msgJSON").style.visibility="initial";
          document.getElementById("msgReset").style.visibility="initial";
          document.getElementById("msgSave").style.visibility="initial";
          document.getElementById("bt_help").value="un-Help";
        }
        else{
           document.getElementById("help").style.visibility="collapse";
           document.getElementById("bt_help").value="Help";
           document.getElementById("msgArea").style.visibility="collapse";
           document.getElementById("msgJSON").style.visibility="collapse";
           document.getElementById("msgReset").style.visibility="collapse";
           document.getElementById("msgSave").style.visibility="collapse";
        }
      }
  </script>
</html>
)=====";
