var SubModuleContext = require(__dirname + '/sub-modules/subModuleContext.js');
var subModuleContext = new SubModuleContext();

// object type function prototype
var ModuleContextBase = function () {

    // private function
    this.checkModuleContext = function (workParam) {

        return {
            "workParam": workParam,
            "__dirname": __dirname,
            "__filename": __filename,
            "console.log": (typeof console.log == "function")
        };
    };

    this.checkSubModuleContext = function(workParam) {
        return subModuleContext.checkModuleContext(workParam);
    }
};

// replicate node.js module loading system
module.exports = ModuleContextBase;