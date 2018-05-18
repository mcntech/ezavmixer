function drawSpectrum(canvas_name, spectrum) {
    var canvas = document.getElementById( canvas_name )
    var ctx     = canvas.getContext( '2d' );
    var h       = canvas.height;
    var w       = canvas.width;

    var spacing = w / spectrum.length;
    var width   = spacing * 2 / 3;
    var height  = h * 0.8;

    ctx.clearRect( 0, 0, w, h );
    for ( var i = 0, l = spectrum.length; i < l; i++ ) {
        ctx.fillRect( i * spacing, h, width, -spectrum[ i ] * height / 64000 );
    }
}