var wsUri = "ws://localhost:38080/audspectrum";
var websocket = null;




var testDataObj = {action:"get_data",
                    programs : [
                        {program_number: 1,
                            streams:[
                                {pid:21, channels:[{data:[1,2,3]}, {data:[1,2,3]}]},
                                {pid:22, channels:[{data:[1,2,3]}, {data:[1,2,3]}]}
                            ]
                        },
                        {program_number: 2,
                            streams:[
                                {pid:31, channels:[{data:[1,2,3]}, {data:[1,2,3]}, {data:[1,2,3]}, {data:[1,2,3]}, {data:[1,2,3]}, {data:[1,2,3]}]}
                            ]
                        },
                        {program_number: 3,
                            streams:[
                                {pid:41, channels:[{data:[1,2,3]}, {data:[1,2,3]}]}
                            ]
                        },
                    ]
                 };

var testInfoObj = {action:"get_info",
                    programs : [
                        {program_number: 1, pid: 20,
                            streams:[
                                {pid:21, type: 3, codec:2,  channels:[{label:"left"},{label:"right"}]},
                                {pid:22, type: 3, codec:3,  channels:[{label:"left"},{label:"right"}]}
                            ]
                        },
                        {program_number: 2, pid: 30,
                            streams:[
                                {pid:31, type: 3, codec:2, channels:[{label:"left"},{label:"right"},{label:"center"},{label:"sub"},{label:"sleft"},{label:"sright"}]}
                            ]
                        },
                        {program_number: 3, pid: 40,
                            streams:[
                                {pid:41, type: 3, codec:3, channels:[{label:"left"},{label:"right"}]}
                            ]
                        },
                    ]
                 };



(function(){
   var Spectrum = function (){
        this.num_chan = 2;
        this.chan_data = [];
        this.programs = [];
        this.spectrum_ids = [];
    };
    Spectrum.prototype =
    {
        updateChanData : function(program_num, stream_num, chan_num, data){

        }
    };
    window.spectrum = new Spectrum();
}
)();

function resizeCanvas(cavas_id, w, h)
{
   var canvas = document.getElementById( cavas_id )
          var ctx     = canvas.getContext( '2d' );

          //canvas.style.width ='100%';
          //canvas.style.height='100%';
          ctx.canvas.width  = w;
          ctx.canvas.height = h;
 }

function resizeDrawingArea()
{
    for(var i=0; i < spectrum.num_chan; i++){
        resizeCanvas(spectrum.spectrum_ids[i], window.innerWidth, window.innerHeight/ spectrum.num_chan);
    }
}

function initAudioSpectrum(spectrum_id) {
        var canvas = document.createElement("canvas");
        canvas.className  = "spectrumChan";
        canvas.id = spectrum_id;
        document.getElementsByTagName('body')[0].appendChild(canvas);
        var br = document.createElement("br");
        document.getElementsByTagName('body')[0].appendChild(br);
}

function initAudioSpectrums()
{
    spectrum.num_chan = 0;
    var programs = spectrum.programs;
    for (i in programs) {
        var program = programs[i];
        if(program.hasOwnProperty("streams")) {
            var streams = program.streams;
            for (j in streams) {
                var stream = streams[j];
                if(stream.hasOwnProperty("channels")){
                    var channels = stream.channels;
                    for(k in channels) {
                        var channel = channels[k];

                        var spectrum_id = "fft" + i + j + k;
                        initAudioSpectrum(spectrum_id);
                        spectrum.spectrum_ids[spectrum.num_chan] = spectrum_id;
                        spectrum.chan_data[spectrum.num_chan]=[];
                        spectrum.num_chan++;
                    }
                }
            }
        }
    }
    resizeDrawingArea();
}

function drawAudioSpectrum(spectrum_id, chan_data) {
    //var canvas = document.getElementById( 'fft' );
    // Testing
    //chan_data = Array.from({length: 48}, () => Math.random());
    //console.log("drawAudioSpectrum: " + chan_data);
    drawSpectrum(spectrum_id, chan_data);
}

function drawSpectrums(resp)
{
    //console.log("drawSpectrums: ");
    var programs = resp.programs;
    for (i in programs) {
        var program = programs[i];
        if(program.hasOwnProperty("streams")) {
            var streams = program.streams;
            for (j in streams) {
                var stream = streams[j];
                if(stream.hasOwnProperty("channels")){
                    var channels = stream.channels;
                    for(k in channels) {
                        var channel = channels[k];
                        if(channel.hasOwnProperty("data")) {
                            var spectrum_id = "fft" + i + j + k;
                            drawAudioSpectrum(spectrum_id, channel.data);
                        }
                    }
                }
            }
        }
    }
}




function updateData(resp)
{
    //console.log("updateData: " + JSON.stringify(resp));
    drawSpectrums(resp);
    setTimeout(doGetData, 1000);
}

function doGetData()
{
    //console.log("doGetData: ");

    if("AudFreqData" in window) {
        var data = AudFreqData.getData(1);
        //console.log("doGetData: " + data);
        var obj = JSON.parse(data);
        updateData(obj);
    } else {
        updateData(testDataObj); // For Testing
    }

}

function updateInfo(resp)
{
    console.log("updateInfo: " + JSON.stringify(resp));
    if(resp.hasOwnProperty('programs')){
        var programs = resp.programs;
        for (i in programs) {
            var program = programs[i];
            spectrum.programs[i] = program;
        }
    }
    initAudioSpectrums();
    doGetData();
}

function doGetInfo()
{
    console.log("doGetInfo: ");
    if("AudFreqData" in window) {
        var data = AudFreqData.getInfo(1);
        console.log("doGetInfo: data " + data);
        var obj = JSON.parse(data);
        updateInfo(obj);
    } else {
        updateInfo(testInfoObj); // For Testing
    }
}

function initAudInterface()
{
    console.log('Initializing Audio Interface');
}