// load appropriate npool module
try {
    var nPool = require(__dirname + '/../../build/Release/npool');
}
catch (e) {
    var nPool = require(__dirname + '/../../build/Debug/npool');
}

describe("[ loadFile() - Unit Tests ]", function() {
  it("OK", function() {
    expect(nPool).not.toBe(undefined);
  });
});

describe("loadFile() shall execute without throwing an exception when a valid file is successfully loaded.", function() {
  it("Executed without throwing an exception.", function() {
    
    var thrownException = null;
    try {
        nPool.loadFile(1, __dirname + '/../resources/helloWorld.js');
    }
    catch(exception) {
        thrownException = exception;
    }

    nPool.removeFile(1);
    expect(thrownException).toBe(null);
  });
});

describe("loadFile() shall throw an exception when a file does not exist.", function() {
  it("Threw an exception when a file doesn't exist.", function() {
    
    var thrownException = null;
    try {
        nPool.loadFile(1, __dirname + '/../resources/fileDoesNotExist.js');    
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });
});

describe("loadFile() shall throw an exception when a file key is used twice.", function() {
  it("Threw an exception because the file key has already been used.", function() {
    nPool.loadFile(1, __dirname + '/../resources/helloWorld.js');
    
    var thrownException = null;
    try {
      nPool.loadFile(1, __dirname + '/../resources/helloWorld.js');
    }
    catch(exception) {
        thrownException = exception;
    }
    nPool.removeFile(1);
    expect(thrownException).not.toBe(null);
  });
});

describe("loadFile() shall throw an exception when passed invalid arguments.", function() {
  it("Exception thrown when there are no parameters.", function() {
    var thrownException = null;
    try {
        nPool.loadFile();
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown when there are more than 2 parameters.", function() {
    var thrownException = null;
    try {
        nPool.loadFile(1, __dirname + '/../resources/helloWorld.js', 'extraParameter');
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown for invalid file key type.", function() {
    var thrownException = null;
    try {
        nPool.loadFile([ 1, 2, 3 ], __dirname + '/../resources/helloWorld.js', 'extraParameter');
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown for invalid file path type.", function() {
    var thrownException = null;
    try {
        nPool.loadFile(2, 1234);
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown when both parameters are invalid types.", function() {
    var thrownException = null;
    try {
        nPool.loadFile({ invalidProp: 'invalid' }, 4321);
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });
});