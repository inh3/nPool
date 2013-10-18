// load appropriate npool module
try {
    var nPool = require(__dirname + '/../../build/Release/npool');
}
catch (e) {
    var nPool = require(__dirname + '/../../build/Debug/npool');
}

describe("[ createThreadPool() - Unit Tests ]", function() {
  it("OK", function() {
    expect(nPool).not.toBe(undefined);
  });
});

describe("createThreadPool() shall execute without throwing an exception when passed an integer number.", function() {
  it("Executed without an exception.", function() {

    var thrownException = null;
    try {
        nPool.createThreadPool(2);
    }
    catch(exception) {
        thrownException = exception;
    }
    nPool.destroyThreadPool();
    
    expect(thrownException).toBe(null);
  });
});

describe("createThreadPool() shall execute without throwing an exception when called after a previous thread pool has been destroyed.", function() {
  it("Executed without an exception.", function() {

    nPool.createThreadPool(2);
    nPool.destroyThreadPool();

    var thrownException = null;
    try {
        nPool.createThreadPool(2);
    }
    catch(exception) {
        thrownException = exception;
    }
    nPool.destroyThreadPool();
    
    expect(thrownException).toBe(null);
  });
});

describe("createThreadPool() shall throw an exception when called more than once.", function() {
  it("Exception thrown when called twice.", function() {

    nPool.createThreadPool(2);
    
    var thrownException = null;
    try {
        nPool.createThreadPool(2);
    }
    catch(exception) {
        thrownException = exception;
    }
    nPool.destroyThreadPool();

    expect(thrownException).not.toBe(null);
  });
});

describe("createThreadPool() shall throw an exception when passed zero arguments.", function() {
  it("Exception thrown with zero parameters.", function() {

    var thrownException = null;
    try {
        nPool.createThreadPool();
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });
});

describe("createThreadPool() shall throw an exception when passed a non-integer argument.", function() {
  it("Exception thrown for string parameter.", function() {
    var thrownException = null;
    try {
        nPool.createThreadPool('');
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown for array parameter.", function() {
    var thrownException = null;
    try {
        nPool.createThreadPool([ 1, 2, 3 ]);
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown for object parameter.", function() {
    var thrownException = null;
    try {
        nPool.createThreadPool({
            test: 'object'
        });
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown for function parameter.", function() {
    var thrownException = null;
    try {
        nPool.createThreadPool(function () {
            var x = 1;
            x = x + 1;
        });
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });
});