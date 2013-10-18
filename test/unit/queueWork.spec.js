// load appropriate npool module
try {
    var nPool = require(__dirname + '/../../build/Release/npool');
}
catch (e) {
    var nPool = require(__dirname + '/../../build/Debug/npool');
}

describe("[ queueWork() - Unit Tests ]", function() {
  it("OK", function() {
    expect(nPool).not.toBe(undefined);
  });
});

describe("queueWork() shall throw an exception when passed an invalid argument.", function() {
  it("Exception thrown when there are no parameters.", function() {
    var thrownException = null;
    try {
        nPool.queueWork();
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown when there is more than 1 parameter.", function() {
    var thrownException = null;
    try {
        nPool.queueWork({}, 'extraParameter');
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });

  it("Exception thrown when the parameter is of the wrong type.", function() {
    var thrownException = null;
    try {
        nPool.queueWork('invalid type');
    }
    catch(exception) {
        thrownException = exception;
    }
    expect(thrownException).not.toBe(null);
  });
});

describe("queueWork() shall execute without throwing an exception when a single valid unit of work is queued.", function() {
  it("Executed without throwing an exception.", function() {
    var completeFlag = false;
    

    var thrownException = null;
    var resultObject = null;
    var resultId = null;

    runs(function() {
        nPool.loadFile(1, __dirname + '/../resources/helloWorld.js');
        nPool.createThreadPool(2);
    
        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "sayHelloWorld",
            workParam: {
                testString: '- queueWork() - Test In Progress'
            },

            callbackFunction: function(callbackObject, workId) {
                resultObject = callbackObject;
                resultId = workId;
                completeFlag = true;
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

    // wait for 1 second
    waitsFor(function() {
        return completeFlag;
    }, "Shouldn't have taken this long to complete successfully.", 2000);

    runs(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
        expect(thrownException).toBe(null);
        expect(resultObject.resultString).toEqual('Hello World - queueWork() - Test In Progress');
        expect(resultId).toEqual(1);
    });
  });
});

describe("queueWork() shall execute without throwing an exception when multiple valid units of work are queued.", function() {
  it("Executed without throwing an exception.", function() {
    var completeFlag = false;
    
    var totalExecutions = 0;
    var thrownException = null;
    var resultObject = null;
    var resultId = null;

    runs(function() {
        nPool.loadFile(1, __dirname + '/../resources/helloWorld.js');
        nPool.createThreadPool(2);

        for(var i = 0; i < 10; i++) {

            var unitOfWork = {
                workId: i + 1,
                fileKey: 1,
                workFunction: "sayHelloWorld",
                workParam: {
                    testString: '- queueWork() - Test In Progress'
                },

                callbackFunction: function(callbackObject, workId) {
                    resultObject = callbackObject;
                    totalExecutions++;
                    if(totalExecutions == 10) {
                        completeFlag = true;
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

    // wait for 1 second
    waitsFor(function() {
        return completeFlag;
    }, "Shouldn't have taken this long to complete successfully.", 2000);

    runs(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
        expect(thrownException).toBe(null);
        expect(resultObject.resultString).toEqual('Hello World - queueWork() - Test In Progress');
    });
  });
});