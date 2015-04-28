var ExtraModule = require('./extraModule.js');

var md5 = require('./md5.js');

var extraModule = new ExtraModule();

// object type function prototype
var HelloWorld = function () {

	// private function
    function sayHelloWorld(workParam) {
        return "Hello World " + workParam.userName;
    };

    // function that matches the unit of work defined work function
    this.getHelloWorld = function (workParam) {
        return { 
        	helloWorld: sayHelloWorld(workParam),
        	utf8OutputString: "Τη γλώσσα μου έδωσαν ελληνική]" + workParam.utf8InputString,
            extra: md5(extraModule.getExtra()),
            underscoreVersion: extraModule.underscoreVersion
        };
    };
};

// replicate node.js module loading system
module.exports = HelloWorld;