# nPool

A platform independent thread pool [add-on for Node.js](http://nodejs.org/api/addons.html).

**nPool's primary features and benefits include:**

 * Linux, Mac and Windows support
 * Efficient and straightforward interface
 * Transparent marshalling of javascript objects in and out of the thread pool
 * User defined context of the callback function executed after task completion
 * Use of object types to complete units of work
 * Support for UTF-8 strings

## Table of Contents

* [The Implementation](#the-implementation)
* [Building From Source](#buiding-from-source)
* [API Documentation](#api-documentation)
* [License](#license)

## The Implementation

nPool is written entirely in C/C++.  The thread pool and synchronization frameworks are written in C and the add-on interface is written in C++.  The library has no third-party dependencies other than [Node.js](http://nodejs.org/) and [V8](https://code.google.com/p/v8/).

The cross-platform threading component utilizes [`pthreads`](https://computing.llnl.gov/tutorials/pthreads/) for Mac and Unix.  On Windows, native threads ([`CreateThread`](http://msdn.microsoft.com/en-us/library/windows/desktop/ms682453)) and [`CRITICAL_SECTIONS`](http://msdn.microsoft.com/en-us/library/windows/desktop/ms682530) are used.  Task based units of work are performed via a FIFO queue that is processed by the thread pool.  Each thread within the thread pool utilizes a distinct [`v8::Isolate`](http://izs.me/v8-docs/classv8_1_1Isolate.html) to execute javascript parallely.  Callbacks to the main Node.js thread are coordinated via [libuv’s](http://nikhilm.github.io/uvbook/introduction.html) [`uv_async`](http://nikhilm.github.io/uvbook/threads.html#inter-thread-communication) inter-thread communication mechanism.

One thing to note, [`unordered_maps`](http://en.cppreference.com/w/cpp/container/unordered_map) are used within the add-on interface, therefore, it is necessary that the platform of choice provides [C++11](http://en.wikipedia.org/wiki/C%2B%2B11) (Windows and Linux) or [TR1](http://en.wikipedia.org/wiki/C%2B%2B_Technical_Report_1) (Apple) implementations of the standard library.

## Building From Source

nPool can be easily compiled on Mac, Linux and Windows using [`node-gyp`](https://github.com/TooTallNate/node-gyp).

Simply run the following command within a console or terminal `node-gyp clean configure build`.

This will automatically configure the environment and produce the add-on module.

**Requirements:**

* Node.js 0.10.* or later
* Standard C and C++ libraries
 * Windows and Unix: C++11
 * Mac: TR1

*Windows requires a Visual Studio Express installation that provides the native C++ Windows SDK environment.*

## API Documentation

nPool provides a very simple and efficient interface.  Currently, there are a total of five functions:

1. [`createThreadPool`] (#createthreadpool)
2. [`destroyThreadPool`] (#destroythreadpool)
3. [`loadFile`] (#loadfile)
4. [`removeFile`] (#removefile)
5. [`queueWork`] (#queuework)

**Example:**
```js
// load nPool module
var nPool = require('npool');

// work complete callback from thread pool 
var callbackFunction = function (callbackObject, workId) { ... }

// load files defining object types
nPool.loadFile(1, './objectType.js');

// create thread pool with two threads
nPool.createThreadPool(2);

// create the unit of work object
var unitOfWork = {
    workId: 9124,
    fileKey: 1,
    workFunction: "objectMethod",
    workParam: {
        aProperty: [ 134532, "xysaf" ]
    },

    callbackFunction: callbackFunction,
    callbackContext: this
}

// queue the unit of work
nPool.queueWork(unitOfWork);
```

---

### createThreadPool

```js
createThreadPool(numThreads)
```
 
This function creates the thread pool.  At this time, the module only supports one thread pool per Node.js process.  Therefore, this function should only be called once, prior to `queueWork` or `destroyThreadPool`.

The function takes one parameter:

 * `numThreads` *uint32* - number of threads to create within the thread pool

**Example:**

```js
// create thread pool with two threads
nPool.createThreadPool(2);
```

---

### destroyThreadPool

```js
destroyThreadPool()
```

This function destroys the thread pool.  This function should only be called once and only when there will be no subsequent calls to the `queueWork` function.  This method can be called safely even if there are tasks still in progress.  At a lower level, this actually signals all threads to exit, but causes the main thread to block until all threads finish their currently executing in-progress units of work.  This does block the main Node.js thread, so this should only be executed when the process is terminating.

This function takes no parameters.

**Example:**

```js
// destroy the thread pool
nPool.destroyThreadPool();
```

---

### loadFile

```js
loadFile(fileKey, filePath)
```

This function serializes a javascript file that contains a constructor function for an object type.  The file buffer will be cached on the Node.js main thread.

This function can be called at any time.  It should be noted that this is a synchronous call, so the serialization of the file will occur on the main thread of the Node.js process.  That being said, it would be prudent to load all necessary files at process startup, especially since they will be cached in memory.

Each thread, on first execution with a unit of work which requires the file referenced by `fileKey`, will de-serialize and compile the contents into a V8 function. The function is used to instantiate a new persistent V8 object instance of the object type.  The persistent object instance is then cached per thread.  Every subsequent unit of work referencing the `fileKey` will retrieve the already cached object instance.

This function takes two parameters:

 * `fileKey` *uint32* - uniquely identifies a file
 * `filePath` *string* - path to javascript file to be cached

Each file must have a unique key.

**Example:**

```js
// load files defining object types
nPool.loadFile(1, './fileA.js');
nPool.loadFile(2, './fileB.js');
```

---

### removeFile

```js
removeFile(fileKey)
```

This function removes a javascript file from the file cache.  This function can be called at any time, with one caveat.  The user should take care to not remove files that are currently referenced in pending units of work that have yet to be processed by the thread pool.

This function takes one parameter:

 * `fileKey` *uint32* - unique key for a loaded file

There must exist a file for the given key.

**Example:**

```js
// remove file associated with fileKey: 1
nPool.removeFile(1);
```

---

### queueWork

```js
queueWork(unitOfWorkObject)
```

This function queues a unit of work for execution on the thread pool.  This function should be called after `createThreadPool` and prior to `destroyThreadPool`.

The function takes one parameter, a unit of work object which contains specific and required properties.

A `unitOfWorkObject` contains the following named properties:

 * `workId` *uint32* - This parameter is a unique integer that identifies a unit of work.  This integer is later passed as a parameter to the work complete callback function.

 * `fileKey` *uint32* - This parameter is an integer that is used as a key to reference a file that was previously cached via the `loadFile` function.  At this time there is no run-time logic to handle the a case when a file key does not reference a previously loaded file.  Therefore, ensure that a file exists for the given key.

 * `workFunction` *string* - This parameter is a string that declares the name of a function.  This function name will be used in conjunction with the `fileKey` in order to reference a specific object instance method.   The function name must match a method on the object type that is defined by the file associated with the given `fileKey`.  The method will be called from within a background thread to process the unit of work object passed to `queueWork`.  This function should ultimately return an object which is passed to the `callbackFunction`.

 * `workParam` *object* - This is user defined object that is the input for the task.  The object will be passed as the only parameter to the object instance method that is executed in the thread pool.  Any function properties on the object will not be available when it is used in the thread pool because serialization does not support packing functions.

 * `callbackFunction` *function* - This property specifies the work complete callback function.  The function is executed on the main Node.js thread.
The work complete callback function takes two parameters:
  * `callbackObject` *object* - the object that is returned by the `workFunction`
  * `workId` *uint32* -  the unique identifier, `workId`, that was passed with the unit of work when it was queued

 * `callbackContext` *context* - This property specifies the context (`this`) of the `callbackFunction` when it is called.

**Example:**

```js
// create the unit of work object
var unitOfWork = {

		// unique identifer of unit of work
		workId: 34290,

		// object type file
		fileKey: 1,

		// object instance function to perform unit of work
		workFunction: "objectMethodName",

		// object to be passed to work function
		workParam: {
			arrayProperty: [ ... ],
			objectProperty: { ... },
			valueProperty: 123,
			stringProperty: "abcd"
		},

		// function that will be called on main Node.js when the task is complete
		callbackFunction: fibonacciCallbackFunction,

		// context that the unit of work function will be called in
		callbackContext: someOtherObject
};
```
```js
// queue the unit of work
nPool.queueWork(unitOfWork);
```

## License

```
Copyright (c) 2013, Ivan Hall <ivan.hall@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of nPool nor the names of its contributors may be used 
      to endorse or promote products derived from this software without specific 
      prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL IVAN HALL BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
