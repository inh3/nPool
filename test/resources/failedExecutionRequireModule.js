var OtherModule = require('./executionErrorModule.js');

// object type function prototype
var FailedExecutionRequireModule = function () {

    this.failedFunction = function (workParam) {
        return {
            resultString: "This should never happen."
        };
    };
};

// replicate node.js module loading system
module.exports = FailedExecutionRequireModule;
