
$(document).ready(function() {
    // Use a "/test" namespace.
    // An application can open a connection on multiple namespaces, and
    // Socket.IO will multiplex all those connections on a single
    // physical channel. If you don't care about multiple channels, you
    // can set the namespace to an empty string.			
    namespace = '/test';

    // Connect to the Socket.IO server.
    // The connection URL has the following format, relative to the current page:
    //     http[s]://<domain>:<port>[/<namespace>]
    var socket = io(namespace);

    // Event handler for new connections.
    // The callback function is invoked when a connection with the
    // server is established.
    socket.on('connect', function(){
        console.log("Connect from js");
        socket.emit('my_event', {data: 'I\'m connected!'});
    });

    socket.on('disconnect', function() {
        console.log("Disconnect");
    });

    socket.on('simple', function(){
        console.log("Simple received 2");
    })

    socket.on('my_response', function(data) {
        console.log("Tick "+data["count"]);
    });

    // $('form#emit').submit(function(event) {
    //     socket.emit('my_event', {data: $('#emit_data').val()});
    //     return false;
    // });

})
