// object type function prototype
var HelloWorld = function () {

    // private function
    function sayHelloWorld(workParam) {
        return "Hello World " + workParam.testString;
    };
};

// replicate node.js module loading system
module.exports = HelloWorld;