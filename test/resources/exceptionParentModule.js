var ExceptionModule = require(__dirname + '/sub-modules/exceptionModule.js');
var exceptionModule = new ExceptionModule();

// object type function prototype
var ExceptionParentModule = function () {

    this.exceptionParentFunction = function (workParam) {
        return {
            exceptionResult: exceptionModule.exceptionFunction()
        };
    };
};

// replicate node.js module loading system
module.exports = ExceptionParentModule;