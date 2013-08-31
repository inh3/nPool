try {
    var nPool = require('./../build/Release/npool');
}
catch (e) {
    var nPool = require('./../build/Debug/npool');
}

// load fib module and create thread pool
var numThreads = +process.argv[3] || 1;
nPool.loadFile(1, './../example/fibonacciNumber.js');
nPool.createThreadPool(numThreads);

// work complete callback
var callBackFunction = function (callbackObject, workId) {

    // context is of the http response that required work
    console.log("Response [" + workId + "/" + (workCount - 1) + "]: " + callbackObject.fibCalcResult);
    this.end(callbackObject.fibCalcResult.toString() + ' ' + workId.toString());
};

var workCount = 0;
// work object to be passed to background threadpool
var workObject = {
    workId: workCount,
    fileKey: 1,
    workFunction: "calcFibonacciNumber",
    workParam: {
        fibNumber: 35
    },

    callbackFunction: callBackFunction
};

// http response function
var i = 0;
var httpFunc = function f (req, res) {
    if((++i) % 10) {
        res.end("QUICK");
        console.log(".");
    }
    else
    {
        console.log("**** Request " + workCount);
        workObject.workId = workCount++;
        workObject.callbackContext = res;
        nPool.queueWork(workObject);
    }
};

var cluster = require('cluster');
var http = require('http');
var numCPUs = require('os').cpus().length;

if (cluster.isMaster) {

    console.log("[ Master ]");

    // Fork workers.
    for (var i = 0; i < numCPUs; i++) {
        cluster.fork();
    }

    cluster.on('exit', function(worker, code, signal) {
        console.log('worker ' + worker.process.pid + ' died');
    });
} else {
    console.log("[ Worker ]");
    var port = process.argv[2] || 1234;
    var http = require('http');
    http.createServer(httpFunc).listen(port);
    console.log('Application running! ' + port);
}

