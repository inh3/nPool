// object type function prototype
var ExtraModule = function () {

	// private function
    function privateExtra(n) {
        return "privateExtra";
    };

    // function that matches the unit of work defined work function
    this.publicExtra = function (whatever, i, want) {
        return privateExtra(i);
    };
};

// make this available to the calling context
this.ExtraModule = ExtraModule;