var assert = require("assert");

// load appropriate npool module
var nPool = null;
try {
    nPool = require(__dirname + '/../build/Release/npool');
}
catch (e) {
    nPool = require(__dirname + '/../build/Debug/npool');
}

describe("[ require() Emulation - Tests ]", function() {
    it("OK", function() {
        assert.notEqual(nPool, undefined);
    });
});

describe("require() shall consider paths that start with './' as relative to the file that is performing the require.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/require/requireBaseModule.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed without throwing an exception.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "testFunctionA",
            workParam: {
                testString: 1
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(callbackObject.result, 'RequireModuleA');
                    assert.equal(workId, 1);
                    assert.equal(exceptionObject, null);
                    done();
                }
                catch(exception) {
                    done(exception);
                }
            },
            callbackContext: this
        };

        try
        {
            nPool.queueWork(unitOfWork);
        }
        catch (exception) {
            thrownException = exception;
        }
    });
});

describe("require() shall consider paths that start with '../' as relative to the file that is performing the require.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/require/requireBaseModule.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed without throwing an exception.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "testFunctionB",
            workParam: {
                testString: null
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(callbackObject.result, 'RequireModuleB');
                    assert.equal(workId, 1);
                    assert.equal(exceptionObject, null);
                    done();
                }
                catch(exception) {
                    done(exception);
                }
            },
            callbackContext: this
        };

        try
        {
            nPool.queueWork(unitOfWork);
        }
        catch (exception) {
            thrownException = exception;
        }
    });
});

describe("require() shall properly handle __dirname in the path.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/require/requireBaseModule.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed without throwing an exception.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "testFunctionC",
            workParam: {
                testString: null
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(callbackObject.result, 'RequireModuleC');
                    assert.equal(workId, 1);
                    assert.equal(exceptionObject, null);
                    done();
                }
                catch(exception) {
                    done(exception);
                }
            },
            callbackContext: this
        };

        try
        {
            nPool.queueWork(unitOfWork);
        }
        catch (exception) {
            thrownException = exception;
        }
    });
});