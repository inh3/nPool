// object type function prototype
var RequireModuleA = function () {

    this.testFunctionA = function (workParam) {
        return {
            result: "RequireModuleA"
        };
    };
};

// replicate node.js module loading system
module.exports = RequireModuleA;