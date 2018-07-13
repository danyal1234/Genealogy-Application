'use strict'

// C library API
const ffi = require('ffi');
const mysql = require('mysql');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  let uploadFile = req.files.uploadFile;


  if(uploadFile == undefined || !req.files) {
    res.redirect('/');
    return res.status(400).send('No files were uploaded.');
  }
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');

  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 
 
app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

let cLibrary = ffi.Library('./sharedLib', {
  'saveGEDCOM': [ 'void' , [ 'string', 'string', 'string' ] ],
  'createIndJSON': [ 'string', [ 'string' ] ],
  'JSONaddindi' : ['void' , [ 'string', 'string', 'string' ] ],
  'filterfiles': [ 'string', [ 'string' ] ],
  //'JSONdescendants': ['string', ['string', 'string', 'string', 'int']],
  //'JSONancestors': ['string', ['string', 'string', 'string', 'int']]
  'GEDCOMtoJSON': [ 'string', [ 'string' ] ]
});

app.get('/getFiles', function(req , res){

  let files = fs.readdirSync('./uploads/');
  let filterdFiles = filterFiles(files);

  res.send({
    allFiles: filterdFiles
  });

});

app.get('/createGEDCOM', function(req, res){
  cLibrary.saveGEDCOM('uploads/' + req.query.filename, req.query.submname, req.query.submadr);
});

/*app.get('/getdesc', function(req, res){

  let glistJSON = cLibrary.JSONdescendants('uploads/' + req.query.filename, req.query.fname, req.query.lname, req.query.num);

  let obj = JSON.parse(glistJSON);

  res.send({
    object: obj
  });

});

app.get('/getansc', function(req, res){

  let glistJSON = cLibrary.JSONancestors('uploads/' + req.query.filename, req.query.fname, req.query.lname, req.query.num);

  let obj = JSON.parse(glistJSON);

  res.send({
    object: obj
  });

});*/

app.get('/addIndi', function(req , res){

  let file = req.query.filename;
  let firstname = req.query.firstname;
  let secondname = req.query.secondname;

  cLibrary.JSONaddindi('uploads/' + file, firstname, secondname);

});

app.get('/getInds', function(req , res){

  let file = req.query.filename;

  res.send({
    Inds: JSON.parse(cLibrary.createIndJSON('uploads/' + file))
  });

});


app.get('/getGEDCOMS', function(req , res){

  let file = req.query.filenames;

  res.send({
    GEDCOMS: JSON.parse(cLibrary.GEDCOMtoJSON('uploads/' + file))
  });

});


function filterFiles(files) {

  let filterFiles = [];

  for (var i = 0; i < files.length; i++) {

    let extension = files[i].split(".");
    if(extension[extension.length-1] == "ged" && cLibrary.filterfiles('uploads/' + files[i]) == "OK"){
      filterFiles.push(files[i]);
    }

  }


    return filterFiles;
}

var connection;


app.get('/getconnection', function(req , res){
  let input = req.query.input;
  let tokens = input.split(",");

  connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : tokens[0],
    password : tokens[1],
    database : tokens[2]
  });

  connection.connect();


  connection.query("SELECT * FROM INFORMATION_SCHEMA.TABLES", function (err, rows, fields) {
      if (err){ 
        console.log("Something went wrong. "+err);
        res.send({
          toPrint: 1
        });
      }
      else{
        res.send({
          toPrint: 0
        });
      }
  });

});


app.get('/setuptables', function(req , res){ 

  let filetable = "CREATE TABLE FILE (file_id INT AUTO_INCREMENT PRIMARY KEY,  file_Name VARCHAR(60) NOT NULL,  source VARCHAR(250) NOT NULL, version VARCHAR(10) NOT NULL, encoding VARCHAR(10) NOT NULL, sub_name VARCHAR(62) NOT NULL, sub_addr VARCHAR(256), num_individials INT, num_families INT)";
  connection.query(filetable, function (err, rows, fields) {
      //if (err) console.log("Something went wrong. "+err);
  });

  let inditable = "CREATE TABLE INDIVIDUAL (ind_id INT AUTO_INCREMENT PRIMARY KEY,  surname VARCHAR(256) NOT NULL,  given_name VARCHAR(256) NOT NULL, sex VARCHAR(1), fam_size INT, source_file INT, FOREIGN KEY (source_file) REFERENCES FILE (file_id) ON DELETE CASCADE)";
  connection.query(inditable, function (err, rows, fields) {
      //if (err) console.log("Something went wrong. "+err);
  });


});


app.get('/saveFiles', function(req , res){

  let temp = fs.readdirSync('./uploads/');
  let files = filterFiles(temp);


  let filecount = 0;
  let indcount = 0;

  for (var i = 0; i < files.length; i++) {
    filecount++;
    let file = files[i];
    let obj = JSON.parse(cLibrary.GEDCOMtoJSON('uploads/' + file));



        let insert = "INSERT INTO FILE (file_Name,  source, version, encoding, sub_name, sub_addr, num_individials, num_families) VALUES (";
        insert += "'" + file + "','" + obj.source + "','" + obj.version + "','" + obj.encoding + "','" + obj.name + "','" + obj.adress + "','" + obj.indi  + "','" + obj.fam + "'";
        insert += ")";
        connection.query(insert, function (err, result) {
          if (err){
            console.log("Something went wrong. "+err);
          }
        });

        let indi = JSON.parse(cLibrary.createIndJSON('uploads/' + file));

        for (var j = 0; j < indi.length; j++) {
          indcount++;
          let insertindi = "INSERT INTO INDIVIDUAL (surname,  given_name, sex, fam_size, source_file) VALUES (";
          insertindi += "'" + indi[j].surname + "','" + indi[j].givenName + "','" +"NULL','0','" + filecount + "')";

          connection.query(insertindi, function (err, result) {
            if (err) {
              console.log("Something went wrong. "+err);
            }
          });
      }
  }


  res.send({
    toPrint: "Database has " + filecount + " files and " + indcount + " individuals"
  });

});

app.get('/getfileindis', function(req , res){ 

  let filename = req.query.filename;

  let request = "SELECT * FROM FILE WHERE file_Name = '" + filename +"'";

  connection.query(request, function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);

    let num = rows[0].file_id;

    connection.query("SELECT * FROM INDIVIDUAL WHERE source_file = '" + num + "'" , function (err, rows, fields) {
      if (err) console.log("Something went wrong. "+err);
      res.send({
        toPrint: rows
      });
    });  

    
  });  

});

app.get('/getcustq', function(req , res){ 

  let query = req.query.query;

  connection.query("SELECT " + query, function (err, rows, fields) {
    if (err){
      res.send({
        toPrint: "Invalid query"
      });
    }
    else{
      res.send({
        toPrint: rows
      });
    } 
  });  

});

app.get('/orderedfiles', function(req , res){ 

  connection.query("SELECT * FROM FILE ORDER BY num_individials DESC", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
    res.send({
      toPrint: rows
    });
  });  

});

app.get('/getsourcefiles', function(req , res){ 

  connection.query("SELECT * FROM FILE WHERE source = '" + req.query.query + "'", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
    res.send({
      toPrint: rows
    });
  });  

});

app.get('/getsubmname', function(req , res){ 

  connection.query("SELECT * FROM FILE WHERE sub_name = '" + req.query.query + "'", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
    res.send({
      toPrint: rows
    });
  });  

});

app.get('/getindilname', function(req , res){ 

  connection.query("SELECT * FROM INDIVIDUAL ORDER BY surname ASC", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
    res.send({
      toPrint: rows
    });
  });  

});

app.get('/deletedb', function(req , res){ 

  connection.query("DELETE FROM FILE", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
  });  

  connection.query("ALTER TABLE FILE AUTO_INCREMENT = 1", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
  });  

  connection.query("ALTER TABLE INDIVIDUAL AUTO_INCREMENT = 1", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
  });

  res.send({
    toPrint: "Database has 0 files and 0 individuals"
  });
});

app.get('/help', function(req , res){ 

  connection.query("DESCRIBE FILE", function (err, result) {
    if (err) console.log("Something went wrong. "+err);

  connection.query("DESCRIBE INDIVIDUAL", function (err, result2) {
    if (err) console.log("Something went wrong. "+err);

    let toPrint = result;
    toPrint += result2;

    res.send({
      toPrint: toPrint
    });

  });  

  });  


});

app.get('/getdbstatus', function(req , res){

  let indcount;
  let filecount;

  connection.query("SELECT * FROM FILE", function (err, rws, filds) {
    if (err) console.log("Something went wrong. "+err);
    let filecount = rws.length;

  connection.query("SELECT * FROM INDIVIDUAL", function (err, rows, filds) {
    if (err) console.log("Something went wrong. "+err);
    let indcount = rows.length;

  res.send({
    toPrint: "Database has "+ filecount +" files and " + indcount + " individuals"
  }); 

  }); 

  });

});

/*connection.query("drop table FILE", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
});*/

//connection.end();

