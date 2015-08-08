var assert = require("assert");

// load appropriate npool module
var nPool = null;
try {
    nPool = require(__dirname + '/../build/Release/npool');
}
catch (e) {
    nPool = require(__dirname + '/../build/Debug/npool');
}

describe("[ createThreadPool() - Tests ]", function() {
    it("OK", function() {
        assert.notEqual(nPool, undefined);
    });
});

describe("createThreadPool() shall execute without throwing an exception when passed an integer number.", function() {
    after(function() {
        nPool.destroyThreadPool();
    });

    it("Executed without an exception.", function() {
        var thrownException = null;
        try {
            nPool.createThreadPool(2);
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.equal(thrownException, null);
    });
});

describe("createThreadPool() shall execute without throwing an exception when called after a previous thread pool has been destroyed.", function() {
    after(function() {
        nPool.destroyThreadPool();
    });

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
        assert.equal(thrownException, null);
    });
});

describe("createThreadPool() shall throw an exception when called more than once.", function() {
    after(function() {
        nPool.destroyThreadPool();
    });

    it("Exception thrown when called twice.", function() {

        nPool.createThreadPool(2);

        var thrownException = null;
        try {
            nPool.createThreadPool(2);
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(thrownException, null);
    });
});

describe("createThreadPool() shall throw an exception when passed zero arguments.", function() {
    var thrownException = null;

    after(function() {
        if(thrownException == null) {
            nPool.destroyThreadPool();
        }
    });

    it("Exception thrown with zero parameters.", function() {
        try {
            nPool.createThreadPool();
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(thrownException, null);
    });
});

describe("createThreadPool() shall throw an exception when passed a non-integer argument.", function() {
    var thrownException = null;

    beforeEach(function() {
        thrownException = null;
    });

    afterEach(function() {
        if(thrownException == null) {
            nPool.destroyThreadPool();
        }
    });

    it("Exception thrown for string parameter.", function() {
        try {
            nPool.createThreadPool('');
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(thrownException, null);
    });

    it("Exception thrown for array parameter.", function() {
        try {
            nPool.createThreadPool([ 1, 2, 3 ]);
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(thrownException, null);
    });

    it("Exception thrown for object parameter.", function() {
        try {
            nPool.createThreadPool({
                test: 'object'
            });
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(thrownException, null);
    });

    it("Exception thrown for function parameter.", function() {
        try {
            nPool.createThreadPool(function () {
                var x = 1;
                x = x + 1;
            });
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(thrownException, null);
    });
});
