var NotConstructorModule = require(__dirname + '/sub-modules/notConstructorModule.js');
var ApplesOranges = require(__dirname + '/sub-modules/applesOranges.js');

// object type function prototype
var SubModuleWorker = function () {

    this.executeNotConstructorSubModuleFunction = function (workParam) {
        return {
            resultString: NotConstructorModule.testFunction()
        };
    };

    this.executeSubModuleFunction = function (workParam) {
        var applesOranges = new ApplesOranges();
        return applesOranges.getFruitNames(workParam);
    };
};

// replicate node.js module loading system
module.exports = SubModuleWorker;