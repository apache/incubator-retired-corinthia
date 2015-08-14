var docformats = require('../../node/src/docformats');
var path = require("path");

docformats.options.binaryPath = path.resolve('../../../../build/bin');

var input = path.resolve('../../assets/input.docx');
var abstract = path.resolve('../../pool/input.docx/input.html');

var status = docformats.get(input, abstract);
console.log(status);