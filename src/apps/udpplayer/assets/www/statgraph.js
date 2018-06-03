var wsUri = "ws://localhost:38080/stats";
var websocket = null;

var testStatsObj = {action:"get_stats",
                    programs : [
                        {program_number: 1,  bitrate : 5200000,
                            streams:[
                                {pid:21, bitrate:20000}
                            ]
                        },
                        {program_number: 2,  bitrate : 4200000,
                            streams:[
                                {pid:31, bitrate:20000}
                            ]
                        },
                        {program_number: 3,  bitrate : 5200000,
                            streams:[
                                {pid:41, bitrate:20000}
                            ]
                        },
                    ]
                 };
var testStatsResponse = JSON.stringify(testStatsObj);

var testProgramsObj = {action:"get_programs",
                    programs : [
                        {program_number: 1, pid: 20,
                            streams:[
                                {pid:21, type: 3, codec:23}
                            ]
                        },
                        {program_number: 2, pid: 30,
                            streams:[
                                {pid:31, type: 3, codec:23}
                            ]
                        },
                        {program_number: 3, pid: 40,
                            streams:[
                                {pid:41, type: 3, codec:23}
                            ]
                        },
                    ]
                 };
var testProgramsResponse = JSON.stringify(testStatsObj);

(function(){

    var Spectrum = function (){
        this.programs = [];
        this.histroy_depth = 24;
        this.muxrate = [];
        this.maxbitrate = 54000000;
        this.sample_count = 0;
        this.sample_time = 1000;
    };
    Spectrum.prototype =
    {
        addMuxrate : function(program, value){
            if(this.muxrate[program].length >= this.histroy_depth)
                this.muxrate[program].shift();

            this.muxrate[program].push(value);
        }
    };
    window.spectrum = new Spectrum();

})();

function initSpectrum(obj)
{
    for (var i in obj.programs) {
        var program = obj.programs[i];
        obj.muxrate[program.program_number] = []; // Initialize inner array
    }

    obj.fillStyle = ["#FF0000", "#00FF00", "#0000FF", "#0F0000", "#F00F00", "#00007F", "#F00000", "#00F000", "#0000F0", "#F0F0F0", "#0F0F0F=", "#00F0F0", "#00F0F0", "#F0F000", "#0F00F0", "#F0000F"];
}

function resizeDrawingArea()
{
   var canvas = document.getElementById( 'fft' )
          var ctx     = canvas.getContext( '2d' );

          canvas.style.width ='100%';
          canvas.style.height='100%';
          ctx.canvas.width  = window.innerWidth;
          ctx.canvas.height = window.innerHeight;
 }

function drawSpectrum() {

    var programs = spectrum.programs;
    var histroy_depth = spectrum.histroy_depth;
    var canvasEl = document.getElementById( 'fft' );

    var fillStyle = spectrum.fillStyle;
    var muxrate = spectrum.muxrate; // Initialize array
    var ctx     = canvasEl.getContext( '2d' );
    var h       = canvasEl.height;
    var w       = canvasEl.width;

    var borderW = w * 0.01;
    var borderH = h * 0.01;
    var labelW = w * 0.2;
    var labelH = h * 0.1;

    var drawW = w - 2 * borderW - labelW;
    var drawH = h - 2* borderH - labelH;
    var darwXstart = borderW + labelW;
    var drawYstart = borderH + labelH;
    spacing = drawW / histroy_depth;
    stackWidth   = spacing;
    stackHeight  = drawH ;

    ctx.clearRect( 0, 0, w, h );
    //console.log('spectrum w=' + w + ' h=' + h + ' stackWidth=' + spacing + ' stackHeight' + stackHeight);
    // Draw Y Label
    ctx.font = "20px Arial";
    var y = drawYstart;
    var x = borderW;
    // Draw stacked bar
    for ( var i in programs ) {
        var program = programs[i];
        var barBitrate = muxrate[program.program_number][0] * stackHeight / spectrum.maxbitrate;
        ctx.fillStyle = fillStyle[program.program_number];// "#FF0000";
        ctx.fillText('program:' + program.program_number , 0, stackHeight - y - barBitrate / 2 );
        y = y +  barBitrate;
    }


    // Draw Y Label
    ctx.font = "20px Arial";
    var y = h;
    var label_interval = 4;
    // Draw stacked bar
    ctx.fillStyle = "#FF0000";
    for ( k=0; k < histroy_depth; k += label_interval ) {
        var x = darwXstart + k * spacing;
        ctx.fillText(spectrum.sample_count + k , x, h - (labelH + borderH));
    }


    // Draw bar chart
    for (var j=0; j < histroy_depth; j++) {
        var y = drawYstart;
        var x = darwXstart + j * spacing;
        // Draw stacked bar
        for ( var i in programs ) {
            var program = programs[i];
            var barBitrate = muxrate[program.program_number][j] * stackHeight / spectrum.maxbitrate;
            ctx.fillStyle = fillStyle[program.program_number];// "#FF0000";

            ctx.fillRect(x , stackHeight - y - barBitrate, stackWidth, barBitrate );
            //console.log('bar x=' + x + 'y= ' + 'stackHeight - y - barBitrate= ' + stackHeight - y - barBitrate + ' stackWidth = ' + stackWidth + ' barBitrate=' + barBitrate);
            y = y +  barBitrate;
        }
    }

    //setTimeout(drawSpectrum, 100);
}


function Program(id, label) {
/*
	this._id = id;
	this._label = label;

    var widget = document.createElement('div');
    widget.className = 'device';
    var l = document.createElement('span');
    l.className = 'text';
    l.innerHTML = label;
    widget.appendChild(l);

	var x = document.createElement("INPUT");
	x.setAttribute("type", "range");
	x.setAttribute("id", id);
	x.setAttribute("min", 0);
	x.setAttribute("max", 100);
	x.setAttribute("step", 5);
	widget.appendChild(x);

	x.addEventListener("change", volumeChange, true);
	x.addEventListener("input", volumeChange, true);

	//document.body.appendChild(widget);
	$("device_panel").appendChild(widget);
*/
}

function initWebsocket()
{
    console.log('Initializing websocket');
    websocket = new WebSocket(wsUri);
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { onMessage(evt) };
    websocket.onerror = function(evt) { onError(evt) };
}

function onOpen(evt)
{
    console.log("CONNECTED");
    doGetProgramList();
}

function onClose(evt)
{
    console.log("DISCONNECTED");
}

function updateStats(resp)
{
    if (spectrum.sample_count > 0) {

        if(resp.hasOwnProperty('programs')){
            var programs = resp.programs;
            for (i in programs) {
                var program = programs[i];
                if(program.hasOwnProperty("bitrate"))
                    //console.log('program=' + program.program + ' bitrate=' + program.bitrate)
                    spectrum.addMuxrate(program.program_number, program.bitrate);
            }
        }
        drawSpectrum();
    }
    setTimeout(doGetStats, spectrum.sample_time);
    spectrum.sample_count++;
}

function updatePrograms(resp)
{
    console.log("updatePrograms: " + resp);
    if(resp.hasOwnProperty('programs')){
        var programs = resp.programs;
        for (i in programs) {
            var program = programs[i];
            spectrum.programs[i] = program;
        }
    }
    initSpectrum(spectrum);
    doGetStats();
}

function onMessage(evt)
{
    //console.log("onMessage: " + evt.data);
    try {
        var obj = JSON.parse(evt.data);
        if(obj.hasOwnProperty('action')  && obj.action == "get_programs"){
            updatePrograms(obj);
        } else if(obj.hasOwnProperty('action') && obj.action == "get_stats") {
            updateStats(obj);
        }
    } catch(e) {
      console.log(e);
    }
}

function onError(evt)
{
    console.log('Error' + evt.data);
    setTimeout(initWebsocket, 1000);
}


function doGetProgramList()
{
    if(websocket === null || websocket.readyState != WebSocket.OPEN) {
        updatePrograms(testProgramsObj); // For Testing
        return 0;
    }

    var obj = { "action":"get_programs"};

    var msg = JSON.stringify(obj);
    console.log("doGetProgramList: " + msg);
    websocket.send(msg);
}

function doGetStats()
{
    if(websocket === null || websocket.readyState != WebSocket.OPEN) {
        updateStats(testStatsObj); // FOr testing
        return 0;
    }
    var obj = { "action":"get_stats"};

    var msg = JSON.stringify(obj);
    //console.log("SENT: " + msg);
    websocket.send(msg);
}