// object type function prototype
var HelloWorld = function () {

	// private function
    function sayHelloWorld(workParam) {
        return "Hello World " + workParam.userName;
    };

    // function that matches the unit of work defined work function
    this.getHelloWorld = function (workParam) {
        return { 
        	helloWorld: sayHelloWorld(workParam),
        	utf8OutputString: "Τη γλώσσα μου έδωσαν ελληνική]" + workParam.utf8InputString
        };
    };
};

// make this available to the calling context
this.HelloWorld = HelloWorld;