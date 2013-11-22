// object type function prototype
var HelloWorld = function () {

    this.sayHelloWorld = function (workParam) {
        return {
            resultString: "Hello World " + workParam.testString
        };
    };
};

// replicate node.js module loading system
module.exports = HelloWorld;