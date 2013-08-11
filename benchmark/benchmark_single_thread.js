function fib (n) {
    return (n < 2) ? 1 : fib(n-2) + fib(n-1);
}

var i = 0;
var n = 35;
var count = 0;

function f (req, res) {
    if((++i) % 10) {
        res.end("QUICK");
        console.log(".");
    }
    else {
        var txt = count++ + ": " + fib(n);
        res.end(txt);
        console.log(txt);
    }
}

var port = process.argv[2] || 1234;
var http = require('http');
http.globalAgent.maxSockets = 8192 + 2048;
http.createServer(f).listen(port);

console.log('Fibonacci server (NO THREADS) running @port: ' + port);