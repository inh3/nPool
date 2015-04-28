// utilize a node.js like require
var _ = require(__dirname + './underscore.js');

var ApplesOranges = function() {

    // function that matches the unit of work defined work function
    this.getFruitNames = function (workParam) {

        // total number of fruits
        var fruitCount = 0;

        // count number of fruits
        _.each(workParam.fruitArray, function(element) {
            fruitCount++;
        });

        // return callback object
        return {
            // return passed in parameter
            origFruitArray: workParam.fruitArray,

            // using underscore.js
            fruitNames: _.pluck(workParam.fruitArray, "name"),
            fruitCount: fruitCount
        };
    };
}

// replicate Node.js module loading behavior
module.exports = ApplesOranges;
