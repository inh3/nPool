// load appropriate npool module
try {
    var nPool = require(__dirname + '/../../build/Release/npool');
}
catch (e) {
    var nPool = require(__dirname + '/../../build/Debug/npool');
}

describe("[ removeFile() - Unit Tests ]", function() {
  it("OK", function() {
    expect(nPool).not.toBe(undefined);
  });
});

describe("removeFile() shall execute without throwing an exception when a valid file key provided.", function() {
  it("Executed without throwing an exception.", function() {

    nPool.loadFile(1, __dirname + '/../resources/helloWorld.js');
    var thrownException = null;
    try {
       nPool.removeFile(1);
    }
    catch(exception) {
        thrownException = exception;
    }

    expect(thrownException).toBe(null);
  });
});

describe("removeFile() shall execute without throwing an exception when passed already removed file key.", function() {
  it("Executed without throwing an exception.", function() {

    var thrownException = null;
    try {
       nPool.removeFile(999);
    }
    catch(exception) {
        thrownException = exception;
    }

    expect(thrownException).toBe(null);
  });
});

describe("removeFile() shall throw an exception when passed an invalid argument.", function() {
  it("Exception thrown when there are no parameters.", function() {
    var thrownException = null;
    try {
        nPool.removeFile();
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown when there is more than 1 parameter.", function() {
    var thrownException = null;
    try {
        nPool.removeFile(1, 'extraParameter');
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown when the parameter is of the wrong type.", function() {
    var thrownException = null;
    try {
        nPool.removeFile([ 'invalid type' ]);
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });
});