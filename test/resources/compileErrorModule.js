// object type function prototype
var CompileErrorModule = function () {

    function willFailToCompile() {
        return "This will fail to compile"!;
    }

    // private function
    this.compileErrorFunction = function (workParam) {
        return {
            resultString: willFailToCompile();
        };
    };
};

// replicate node.js module loading system
module.exports = CompileErrorModule;