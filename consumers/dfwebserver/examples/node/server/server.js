var express = require('express'),
    multer  = require('multer')

var path = require('path');
var fs = require('fs');

var docformats = require('../../../node/src/docformats');
docformats.options.binaryPath = path.resolve('../../../../../build/bin');


var app = express();

var dfwebserver = path.resolve(__dirname, '../../..');
var web = path.resolve(__dirname, '../../../../web');

app.get('/upload', function(req, res){
  res.send('<html><head></head><body>\
               <form method="POST" enctype="multipart/form-data">\
                <input type="file" name="filefield"><br />\
                <input type="submit">\
              </form>\
            </body></html>');
});

var multerOptions = { 
    dest: path.resolve(dfwebserver, 'uploads/'),
    rename: function (fieldname, filename) {
        return filename;
    }
};

app.post('/upload',[ multer(multerOptions), function(req, res){
    console.log(req.body) // form fields
    console.log(req.files) // form files
    //res.json(req.files).status(204).end()
    
  res.send('<html><head></head><body>\
               <a href="/get/' + req.files.filefield.name + '/">Navigate to this link to run dfconvert GET in the server\
                </a>\
            </body></html>');
}]);


function getInput(id) {
    return path.resolve(dfwebserver, 'uploads', id);
}

function getOutput(id) {
    var folder = path.resolve(dfwebserver, 'pool', id)
    var parsed = path.parse(folder);
    
    return path.resolve(folder, parsed.name + '.html');
}

function getDynamicResource(id, resource) {
    var folder = path.resolve(dfwebserver, 'pool', id)

    return path.resolve(folder, resource);
}

function getStaticResource(resource) {
    return path.resolve(web, 'client', resource);
}

function sendFile(res, filename) {
    if (fs.existsSync(filename)) {
        res.sendFile(filename);
    } else {
        res.status(404).end(); // Not Found
    }
}


app.get('/get/:id', function(req, res){
    var input = getInput(req.params.id);
    var abstract = getOutput(req.params.id);
    
    try {
        // This is required because DocFormat API throw an error
        // when abstract is present in the filesystem
        fs.unlinkSync(abstract); 
    } catch (ex) {
        // do nothing
    }
    docformats.get(input, abstract);
    sendFile(res, abstract);
});

app.get('/get/:id/*', function(req, res){
    var staticResource = getStaticResource(req.params[0]);
    var dynamicResource = getDynamicResource(req.params.id, req.params[0]);
    
    if (fs.existsSync(staticResource)) {
        res.sendFile(staticResource);
    } else {
        sendFile(res, dynamicResource);
    }
});



app.listen(process.env.PORT || 8080);



