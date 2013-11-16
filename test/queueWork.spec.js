var assert = require("assert")

// load appropriate npool module
try {
    var nPool = require(__dirname + '/../build/Release/npool');
}
catch (e) {
    var nPool = require(__dirname + '/../build/Debug/npool');
}

describe("[ queueWork() - Tests ]", function() {
  it("OK", function() {
    assert.notEqual(nPool, undefined);
  });
});

describe("queueWork() shall throw an exception when passed an invalid argument.", function() {

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

describe("queueWork() shall execute without throwing an exception when a single valid unit of work is queued.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/helloWorld.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed without throwing an exception and returned valid results.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "sayHelloWorld",
            workParam: {
                testString: '- queueWork() - Test In Progress'
            },

            callbackFunction: function(callbackObject, workId) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(callbackObject.resultString, 'Hello World - queueWork() - Test In Progress');
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

describe("queueWork() shall execute without throwing an exception when a single valid unit of work is queued.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/fibonacciModule.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Executed all units of work without throwing an exception and returned valid results.", function(done) {
        var totalExecutions = 0;
        var thrownException = null;
        var resultObject = null;
        var resultId = null;
        var assertionException = null;

        // test for total execution count
        var executionIterations = 10;

        // make sure test ends within 5 sec
        this.timeout(5);

        // execute the tests
        for(var i = 0; i < executionIterations; i++) {

            var unitOfWork = {
                workId: 1,
                fileKey: 1,
                workFunction: "calcFibonacciNumber",
                workParam: {
                    fibNumber: 10
                },

                callbackFunction: function(callbackObject, workId) {
                    try {
                        assert.equal(thrownException, null);
                        assert.equal(callbackObject.fibCalcResult, 55);
                        assert.equal(workId, 1);
                    }
                    catch(exception) {
                        assertionException = exception;
                    }

                    // wait until all executions occur
                    if(++totalExecutions == 10) {
                        done(assertionException);
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
        }
    });
});