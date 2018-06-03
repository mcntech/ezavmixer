function drawSpectrum(canvas_name, spectrum, label) {
    var canvas = document.getElementById( canvas_name );

    if(canvas == null)
        return;

    var ctx     = canvas.getContext( '2d' );
    var h       = canvas.height;
    var w       = canvas.width;

    var spacing = w / spectrum.length;
    var width   = spacing * 2 / 3;
    var height  = h * 0.8;
    var borderH = w * 0.1;
    var labelWidth  = w * 0.1;
    var mariginH  = w * 0.05;
    var startX = borderH + labelWidth + mariginH;
    ctx.clearRect( 0, 0, w, h );

    ctx.fillText(label , borderH, h/2);
    for ( var i = 0, l = spectrum.length; i < l; i++ ) {
        ctx.fillRect( startX + i * spacing, h, width, -spectrum[ i ] * height / 64000 );
    }
}