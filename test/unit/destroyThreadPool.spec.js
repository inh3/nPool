// load appropriate npool module
try {
    var nPool = require(__dirname + '/../../build/Release/npool');
}
catch (e) {
    var nPool = require(__dirname + '/../../build/Debug/npool');
}

describe("[ destroyThreadPool() - Unit Tests ]", function() {
  it("Test Suite Ready", function() {
    var health = true;
    expect(health).toBe(true);
  });
});

describe("destroyThreadPool() shall execute without throwing an exception when called and a thread pool exists.", function() {
  it("Executed without an exception.", function() {
    nPool.createThreadPool(2);
    var thrownException = null;
    try {
        nPool.destroyThreadPool();
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).toBe(null);
  });
});

describe("destroyThreadPool() shall throw an exception when called and a thread pool does not exist.", function() {
  it("Exception thrown when called and a thread pool doesn't exist.", function() {
    var thrownException = null;
    try {
        nPool.destroyThreadPool();
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });
});