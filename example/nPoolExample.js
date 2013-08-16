// load nPool module
var nPool = require('./../build/Release/npool');

// work complete callback from thread pool 
var fibonacciCallbackFunction = function (callbackObject, workId) {

    console.log("----------------------------------------");

    console.log("Callback Function: Fibonacci\n");

    console.log("Callback Context:");
    console.log(this);
    console.log("");

    console.log("WorkId: " + workId + "\n");

    console.log("Callback Object:");
    console.log(callbackObject);
    console.log("");
}

// work complete callback from thread pool 
var helloWorldCallbackFunction = function (callbackObject, workId) {

    console.log("----------------------------------------");

    console.log("Callback Function: Hello World\n");

    console.log("Callback Context:");
    console.log(this);
    console.log("");

    console.log("WorkId: " + workId + "\n");

    console.log("Callback Object:");
    console.log(callbackObject);
    console.log("");
}

// work complete callback from thread pool 
var applesOrangesCallbackFunction = function (callbackObject, workId) {

    console.log("----------------------------------------");

    console.log("Callback Function: Apples Oranges\n");

    console.log("Callback Context:");
    console.log(this);
    console.log("");

    console.log("WorkId: " + workId + "\n");

    console.log("Callback Object:");
    console.log(callbackObject);
    console.log("");
}

// object type to be used to demonstrate context param within unit of work
function ContextA() {

    this.contextAProperty = "[Context A] Property",
    this.contextAFunction = function() { console.log("[Context A] Function"); }
}

// object type to be used to demonstrate context param within unit of work
function ContextB() {

    this.contextBProperty = "[Context B] Property",
    this.contextBFunction = function() { console.log("[Context B] Function"); }
}

// object type to be used to demonstrate context param within unit of work
function ContextC() {

    this.contextCProperty = "[Context C] Property",
    this.contextCFunction = function() { console.log("[Context C] Function"); }
}

// load files defining object types
nPool.loadFile(1, './fibonacciNumber.js');
nPool.loadFile(2, './helloWorld.js');
nPool.loadFile(3, './applesOranges.js');

// create thread pool with two threads
nPool.createThreadPool(2);

// object instances to demonstrate context param
var ContextAObject = new ContextA();
var ContextBObject = new ContextB();
var ContextCObject = new ContextC();

// set continous timeout on main thread every 250ms
var startTime = (new Date()).getTime();
(function spinForever () {
    var diffTime = (new Date()).getTime() - startTime;
    console.log("** Time since last timeout: " + diffTime + " ms");
    startTime = (new Date()).getTime();
    setTimeout(spinForever, 250);
})();

// add 6 units of work to thread pool queue
for(var workCount = 0; workCount < 7; workCount++) {

    // create the unit of work object
    var unitOfWork = {
        workId: workCount,
        fileKey: 1,
        workFunction: "calcFibonacciNumber",
        workParam: {
            fibNumber: 40
        },

        callbackFunction: fibonacciCallbackFunction,
        callbackContext: ContextAObject
    }

    // queue some other work on a special condition
    if(workCount == 3) {
        unitOfWork.workId = 99;
        unitOfWork.fileKey = 2;
        unitOfWork.workFunction = "getHelloWorld";
        unitOfWork.workParam = {
            userName: "nPool",
            utf8InputString: "grâwen tägelîch"
        };

        unitOfWork.callbackFunction = helloWorldCallbackFunction;
        unitOfWork.callbackContext = ContextBObject

    }

    // this utilizes the node.js like module loading system
    // for pure javascript modules
    if(workCount == 6) {
        unitOfWork.workId = 1234;
        unitOfWork.fileKey = 3;
        unitOfWork.workFunction = "getFruitNames";
        unitOfWork.workParam = {
            fruitArray: [ { name: "apple", color: "red"}, { name: "strawberry", color: "red" }, { name: "banana", color: "yellow" } ]
        };

        unitOfWork.callbackFunction = applesOrangesCallbackFunction;
        unitOfWork.callbackContext = ContextCObject

    }

    // queue the unit of work
    nPool.queueWork(unitOfWork);
}