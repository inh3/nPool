var assert = require("assert");

// load appropriate npool module
var nPool = null;
try {
    nPool = require(__dirname + '/../build/Release/npool');
}
catch (e) {
    nPool = require(__dirname + '/../build/Debug/npool');
}

describe("[ queueWork() Exceptions - Tests ]", function() {
    it("OK", function() {
        assert.notEqual(nPool, undefined);
    });
});

describe("queueWork() shall throw an exception when passed an invalid type for the unit of work argument.", function() {

    before(function() {
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
    });

    it("Exception thrown when there are no parameters.", function() {
        var thrownException = null;
        try {
            nPool.queueWork();
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(nPool, null);
    });

    it("Exception thrown when there is more than 1 parameter.", function() {
        var thrownException = null;
        try {
            nPool.queueWork({}, 'extraParameter');
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(nPool, null);
    });

    it("Exception thrown when the parameter is of the wrong type.", function() {
        var thrownException = null;
        try {
            nPool.queueWork('invalid type');
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(nPool, null);
    });
});

describe("queueWork() shall throw an exception when passed a unit of work object without the proper properties.", function() {

    before(function() {
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
    });

    it("Exception thrown when there is a malformed unit of work object.", function() {
        var thrownException = null;
        try {
            nPool.queueWork({});
        }
        catch(exception) {
            thrownException = exception;
        }
        assert.notEqual(nPool, null);
    });
});

describe("queueWork() shall complete the callback with an exception object when a loaded file's module cannot be compiled.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/compileErrorModule.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed and performed callback with a valid exception object.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "compileErrorFunction",
            workParam: {
                testObject: { objectProp: 'test property ' }
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(typeof exceptionObject, "object");
                    assert.equal(workId, 1);
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

describe("queueWork() shall complete the callback with an exception object when a loaded file's module can't resolve another required() module.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/failedRequireModule.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed and performed callback with a valid exception object.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "failedFunction",
            workParam: {
                testObject: { objectProp: 'test property ' }
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(typeof exceptionObject, "object");
                    assert.equal(workId, 1);
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

describe("queueWork() shall complete the callback with an exception object when a sub-module performs logic resulting in an exception", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/exceptionParentModule.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed and performed callback with a valid exception object.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "exceptionParentFunction",
            workParam: {
                testObject: { objectProp: 'test property ' }
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(typeof exceptionObject, "object");
                    assert.equal(workId, 1);
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

describe("queueWork() shall complete the callback with an exception object when a the work function is not defined", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/helloWorld.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed and performed callback with a valid exception object.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "thisIsNotAValidFunction",
            workParam: {
                testString: '- queueWork() - Test In Progress'
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(typeof exceptionObject, "object");
                    assert.equal(workId, 1);
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