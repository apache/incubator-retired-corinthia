'use strict';

var path = require('path');
var exec = require('child_process').execFileSync;
var mkdirp = require("mkdirp").sync;

var df = exports;

df.options = {binaryPath : null};

df.get = function (input, abstract) {
    try {
        var cmd = null;
        if (df.options.binaryPath === undefined || df.options.binaryPath === null) {
            cmd = 'dfconvert';
        } else {
            cmd = path.join(df.options.binaryPath, 'dfconvert');
        }
        console.log(cmd + " get " + input + " " + abstract + "");
        mkdirp(path.dirname(abstract));
        exec(cmd, ['get', input, abstract]);
    } catch (ex) {
        console.log(ex);
        return false;
    }
    return true;
};