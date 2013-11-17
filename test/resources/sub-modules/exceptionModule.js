// object type function prototype
var ExceptionModule = function () {

    // private function
    this.exceptionFunction = function (workParam) {
        var x = null;
        return {
            exceptionResult: x()
        };
    };
};

// replicate node.js module loading system
module.exports = ExceptionModule;