// utilize a node.js like require
var _ = require('./underscore.js');

// object type function prototype
var ExtraModule = function () {

    // function that matches the unit of work defined work function
    this.getExtra = function (workParam) {
        return "** EXTRA - Nested Module Loading **";
    };

    // store value of underscore version
    this.underscoreVersion = _.VERSION;
};

// make this available to the calling context
module.exports = ExtraModule;