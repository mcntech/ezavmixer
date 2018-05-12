var wsUri = "ws://localhost:38080/stats";
var websocket = null;

var testStatsObj = {action:"get_stats",
                    programs : [
                        {program_number: 1,  bitrate : 5200000,
                            streams:[
                                {pid:21, bitrate:20000}
                            ]
                        },
                        {program_number: 2,  bitrate : 7200000,
                            streams:[
                                {pid:31, bitrate:20000}
                            ]
                        },
                        {program_number: 3,  bitrate : 2200000,
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
        this.maxbitrate = 19000000;
    };
    Spectrum.prototype =
    {
        addMuxrate : function(program, value){
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
        for (var j = 0; j < obj.histroy_depth; j++) { // i++ needs to be j++
            obj.muxrate[program.program_number][j] = Math.random();
        }
    }

    obj.fillStyle = ["#FF0000", "#00FF00", "#0000FF", "#0F0000", "#000F0000", "#00000F", "#F00000", "#00F000", "#0000F0", "#F0F0F0", "#0F0F0F=", "#00F0F0", "#00F0F0", "#F0F000", "#0F00F0", "#F0000F"];
}

function resizeDrawingArea()
{
   var canvas = document.getElementById( 'fft' )
        canvas.height=window.innerHeight*220/480;
        canvas.width=window.innerWidth*360/360;
 }

function drawSpectrum() {

    var programs = spectrum.programs;
    var histroy_depth = spectrum.histroy_depth;
    var canvasEl = document.getElementById( 'fft' )

    var fillStyle = spectrum.fillStyle;
    var muxrate = spectrum.muxrate; // Initialize array

    ctx     = canvasEl.getContext( '2d' ),
    h       = canvasEl.height,
    w       = canvasEl.width,

    spacing = w / histroy_depth,
    width   = spacing,
    height  = h ;

    ctx.clearRect( 0, 0, w, h );
    for (var j=0; j < histroy_depth; j++) {
        var y = 0;
        for ( var i in programs ) {
            var program = programs[i];
            var barBitrate = muxrate[program.program_number][j] * height / spectrum.maxbitrate;
            ctx.fillStyle = fillStyle[program.program_number];// "#FF0000";
            ctx.fillRect( j * spacing, h - y, width, barBitrate );
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
    setTimeout(doGetStats, 1000);
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
    console.log("onMessage: " + evt.data);
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
    console.log("SENT: " + msg);
    websocket.send(msg);
}