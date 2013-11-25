var RequireModuleA = require('./requireModuleA.js');
var RequireModuleB = require('../require/anotherDirectory/requireModuleB.js');
var RequireModuleC = require(__dirname + 'anotherDirectory/requireModuleC.js');

// object type function prototype
var RequireBaseModule = function () {

    this.testFunctionA = function (workParam) {
        var reqA = new RequireModuleA();
        return {
            result: reqA.testFunctionA().result
        };
    };

    this.testFunctionB = function (workParam) {
        return {
            result: new RequireModuleB().testFunctionB().result
        };
    };

    this.testFunctionC = function (workParam) {
        return {
            result: RequireModuleC.testFunctionC().result
        };
    };
};

// replicate node.js module loading system
module.exports = RequireBaseModule;