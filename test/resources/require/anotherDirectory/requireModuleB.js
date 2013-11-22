// object type function prototype
var RequireModuleB = function () {

    this.testFunctionB = function (workParam) {
        return {
            result: "RequireModuleB"
        };
    };
};

// replicate node.js module loading system
module.exports = RequireModuleB;