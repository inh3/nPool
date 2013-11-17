var assert = require("assert");

// load appropriate npool module
var nPool = null;
try {
    nPool = require(__dirname + '/../build/Release/npool');
}
catch (e) {
    nPool = require(__dirname + '/../build/Debug/npool');
}

describe("[ Module Context - Tests ]", function() {
    it("OK", function() {
        assert.notEqual(nPool, undefined);
    });
});

describe("When a work module is loaded it should have properly defined context.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/moduleContextBase.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Module context is properly defined.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "checkModuleContext",
            workParam: {
                arrayProperty: [ 1, 2, 3 ],
                objectProperty: { objectProp: 'test property' },
                numberProperty: 99,
                stringProperty: 'This is a string property.'
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(workId, 1);
                    assert.equal(exceptionObject, null);
                    assert.equal(typeof callbackObject.workParam, "object");
                    assert.equal(typeof callbackObject.workParam.arrayProperty, "object");
                    assert.equal(typeof callbackObject.workParam.objectProperty, "object");
                    assert.equal(typeof callbackObject.workParam.numberProperty, "number");
                    assert.equal(typeof callbackObject.workParam.stringProperty, "string");
                    assert.equal(typeof callbackObject.__dirname, "string");
                    assert.equal(typeof callbackObject.__filename, "string");
                    assert.equal(callbackObject["console.log"], true);
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

describe("When a sub-module is required by a work module it should have properly defined context.", function() {

    before(function() {
        nPool.loadFile(1, __dirname + '/resources/moduleContextBase.js');
        nPool.createThreadPool(2);
    });

    after(function() {
        nPool.destroyThreadPool();
        nPool.removeFile(1);
    });

    it("Module context is properly defined.", function(done) {
        var thrownException = null;
        var resultObject = null;
        var resultId = null;

        var unitOfWork = {
            workId: 1,
            fileKey: 1,
            workFunction: "checkSubModuleContext",
            workParam: {
                arrayProperty: [ 1, 2, 3 ],
                objectProperty: { objectProp: 'test property' },
                numberProperty: 99,
                stringProperty: 'This is a string property.'
            },

            callbackFunction: function(callbackObject, workId, exceptionObject) {
                try {
                    assert.equal(thrownException, null);
                    assert.equal(workId, 1);
                    assert.equal(exceptionObject, null);
                    assert.equal(typeof callbackObject.workParam, "object");
                    assert.equal(typeof callbackObject.workParam.arrayProperty, "object");
                    assert.equal(typeof callbackObject.workParam.objectProperty, "object");
                    assert.equal(typeof callbackObject.workParam.numberProperty, "number");
                    assert.equal(typeof callbackObject.workParam.stringProperty, "string");
                    assert.equal(typeof callbackObject.__dirname, "string");
                    assert.equal(typeof callbackObject.__filename, "string");
                    assert.equal(callbackObject.globalWasSetBySubModule, true);
                    assert.equal(callbackObject["console.log"], true);
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