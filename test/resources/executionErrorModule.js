var x = null;
x();

// object type function prototype
var ExecutionErrorModule = function () {

    this.compileErrorFunction = function (workParam) {
        return {
            resultString: "This should never happen."
        };
    };
};

// replicate node.js module loading system
module.exports = ExecutionErrorModule;
