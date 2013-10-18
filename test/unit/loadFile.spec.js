// load appropriate npool module
try {
    var nPool = require(__dirname + '/../../build/Release/npool');
}
catch (e) {
    var nPool = require(__dirname + '/../../build/Debug/npool');
}

describe("[ loadFile() - Unit Tests ]", function() {
  it("Test Suite Ready", function() {
    var health = true;
    expect(health).toBe(true);
  });
});

describe("loadFile() shall throw an exception when a file does not exist.", function() {
  it("Threw an exception when a file doesn't exist.", function() {
    
    var thrownException = null;
    try {
        nPool.loadFile(1, __dirname + '/../resources/test.js');    
    }
    catch(exception) {
        thrownException = exception;
    }
    console.log(thrownException);
    expect(thrownException).not.toBe(null);
  });
});