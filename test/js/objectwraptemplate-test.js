/*********************************************************************
 * NAN - Native Abstractions for Node.js
 *
 * Copyright (c) 2015 NAN contributors
 *
 * MIT License <https://github.com/nodejs/nan/blob/master/LICENSE.md>
 ********************************************************************/

const test     = require('tap').test
    , testRoot = require('path').resolve(__dirname, '..')
    , bindings = require('bindings')({ module_root: testRoot, bindings: 'objectwraptemplate' });

test('objectwraptemplate', function (t) {
  t.plan(10);

  t.type(bindings.MyObject, 'function');

  var obj1 = new bindings.MyObject(10);

  t.type(obj1.getHandle, 'function');
  t.type(obj1.getValue, 'function');
  t.type(obj1.getHandle(), 'object');
  t.type(obj1.getValue(), 'number');

  var obj2 = bindings.newFactoryObjectInstance(123);

  t.type(obj2.getValue, 'function');
  t.type(obj2.getValue(), 'number');
  t.equal(obj2.getValue(), 123);
  obj2.increment();
  t.equal(obj2.getValue(), 124);

  var another = obj2.makeAnotherFactoryObject(456);
  t.equal(another.getValue(), 456);
});
