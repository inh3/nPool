// object type function prototype
var FibModule = function () {

	// private function
    function calcFib(n) {
        return n > 1 ? calcFib(n - 1) + calcFib(n - 2) : 1;
    };

    // function that matches the unit of work defined work function
    this.calcFibonacciNumber = function (workParam) {
        return { 
        	fibCalcResult: calcFib(workParam.fibNumber)
        };
    };
};

// replicate node.js module loading system
module.exports = FibModule;