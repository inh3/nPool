var OtherModule = require('./compileErrorModule.js');

// object type function prototype
var FailedCompileRequireModule = function () {

    this.failedFunction = function (workParam) {
        return {
            resultString: "This should never happen."
        };
    };
};

// replicate node.js module loading system
module.exports = FailedCompileRequireModule;
