var OtherModule = require('this.module.is.fake');

// object type function prototype
var FailedRequireModule = function () {

    // private function
    this.failedFunction = function (workParam) {
        return {
            resultString: "This should never happen."
        };
    };
};

// replicate node.js module loading system
module.exports = FailedRequireModule;