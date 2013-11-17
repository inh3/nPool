var assert = require("assert");

// load appropriate npool module
var nPool = null;
try {
    nPool = require(__dirname + '/../build/Release/npool');
}
catch (e) {
    nPool = require(__dirname + '/../build/Debug/npool');
}

describe("[ destroyThreadPool() - Tests ]", function() {
    it("OK", function() {
        assert.notEqual(nPool, undefined);
    });
});

describe("destroyThreadPool() shall execute without throwing an exception when called and a thread pool exists.", function() {

    before(function() {
        nPool.createThreadPool(2);
    });

    it("Executed without an exception.", function() {
        var thrownException = null;
        try {
            nPool.destroyThreadPool();
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.equal(thrownException, null);
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
        assert.notEqual(thrownException, null);
    });
});