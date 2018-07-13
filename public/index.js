// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getFiles',   //The server endpoint we are connecting to
        success: function (data) {
            addFiles(data.allFiles);
            populateTable(data.allFiles);

            if(data.allFiles.length == 0){
                document.getElementById("savedatabutton").disabled = true;
            }


        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#someform').submit(function(e){
        e.preventDefault();
        $.ajax({});
    });

    establishCon();

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/setuptables',   //The server endpoint we are connecting to
        success: function (data) {
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });


    document.getElementById("addindibutton").addEventListener("click", addIndividuals);
    document.getElementById("getdescbutton").addEventListener("click", getDescendants);
    document.getElementById("getanscbutton").addEventListener("click", getAncestors);
    document.getElementById("createGEDbutton").addEventListener("click", createGEDCOM);
    document.getElementById("clearbutton").addEventListener("click", clearstatus);

    document.getElementById("savedatabutton").addEventListener("click", saveFiles); 
    document.getElementById("cleardatabutton").addEventListener("click", clearFiles);
    document.getElementById("displaydbbutton").addEventListener("click", displayDB);   

    document.getElementById("helpbutton").addEventListener("click", help);  
    document.getElementById("cleardbbutton").addEventListener("click", cleardbbutton);   
    document.getElementById("dbdispindibutton").addEventListener("click", dbdispindibutton);    
    document.getElementById("dbindibutton").addEventListener("click", dispindifile);    

    document.getElementById("dbfilebutton").addEventListener("click", dbfilebutton);  

    document.getElementById("dbsubmbutton").addEventListener("click", dispfilebysub);    
    document.getElementById("dbsourcebutton").addEventListener("click", dispfilebysource); 
    
    document.getElementById("custbutton").addEventListener("click", custquery);   

    document.getElementById("viewFiledrop").onchange = function() {
        let filename = this.value;

        let table = document.getElementById("viewtable");
        table.innerHTML = "";
        var row = table.insertRow(0);     
        var cell = row.insertCell(0);
        var cell2 = row.insertCell(1);

        // Add some bold text in the new cell:
        cell.innerHTML = "<b>Given Name</b>";
        cell2.innerHTML = "<b>Surname<b/>";

        if(filename == "Select a file"){
            return;
        }


        $.ajax({
             type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/getInds',
            data: {
                'filename': filename
            },   //The server endpoint we are connecting to
            success: function (data) {
                let obj = data.Inds;
                for (var i = 0; i < obj.length; i++) {
                    let newrow = table.insertRow(i+1);     
                    var newcell = newrow.insertCell(0);
                    var newcell2 = newrow.insertCell(1);
                    newcell.innerHTML = obj[i].givenName;
                    newcell2.innerHTML = obj[i].surname;
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });



    };

});

function dispfilebysource(){
    document.getElementById("dbstatusarea").value = "";
    let query = document.getElementById("dbsource").value;

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getsourcefiles',
        data: {
            'query': query
        },   //The server endpoint we are connecting to
        success: function (data) {
            if(data.toPrint.length == 0){
                document.getElementById("dbstatusarea").value = "No files";
            }

            for (var i = 0; i < data.toPrint.length; i++) {
                document.getElementById("dbstatusarea").value += data.toPrint[i].file_Name+ "\n";
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

} 

function establishCon(){
    let input = prompt("Enter your username,password and database name in the format (username,password,databasename)?");

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getconnection',
        data: {
            'input': input
        },   //The server endpoint we are connecting to
        success: function (data) {
            if(data.toPrint == 1){
                alert("Unable to establish connection please re-enter information.")
                establishCon();
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function dbfilebutton(){
    document.getElementById("dbstatusarea").value = "";

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/orderedfiles',  //The server endpoint we are connecting to
        success: function (data) {
            if(data.toPrint.length == 0){
                document.getElementById("dbstatusarea").value = "No files";
            }
            else{
                for (var i = 0; i < data.toPrint.length; i++) {
                    document.getElementById("dbstatusarea").value += data.toPrint[i].file_Name + " individual count: " + data.toPrint[i].num_individials + "\n";
                }
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function dispfilebysub(){
    document.getElementById("dbstatusarea").value = "";
    let query = document.getElementById("dbsubname").value;

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getsubmname',
        data: {
            'query': query
        },   //The server endpoint we are connecting to
        success: function (data) {
            if(data.toPrint.length == 0){
                document.getElementById("dbstatusarea").value = "No files";
            }

            for (var i = 0; i < data.toPrint.length; i++) {
                document.getElementById("dbstatusarea").value += data.toPrint[i].file_Name + "\n";
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function custquery(){
    document.getElementById("dbstatusarea").value = "";
    let query = document.getElementById("dbcust").value;

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getcustq',
        data: {
            'query': query
        },   //The server endpoint we are connecting to
        success: function (data) {
            if(data.toPrint == "Invalid query"){
                document.getElementById("dbstatusarea").value = "Invalid query";
            }
            else{
                for (var i = 0; i < data.toPrint.length; i++) {
                    document.getElementById("dbstatusarea").value += data.toPrint[i];
                }
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function cleardbbutton(){
    document.getElementById("dbstatusarea").value = "";
}

function clearstatus(){
    document.getElementById("statusarea").value = "";
}

function createGEDCOM(){

    let filename = document.getElementById("getfilename").value;
    let submname = document.getElementById("getsubmname").value;
    let submaddr = document.getElementById("getsubmadress").value;

    if(filename.length == 0){
        document.getElementById("statusarea").value = "Must provide a filename";
    }

    let extension = filename.split(".");
    if(extension[extension.length-1] != "ged" || extension.length == 1){
      document.getElementById("statusarea").value = "File extension must be .ged";
    }

    if(submname.length == 0){
        document.getElementById("statusarea").value = "Must provide a submitter name";
        return;
    }

    let gedc = 5.5;
    let source = "genealogy";
    let encoding = "ANSEL";

    let table = document.getElementById("filetable");

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/createGEDCOM',
        data: {
            'filename': filename,
            'submname': submname,
            'submadr':submaddr
        },   //The server endpoint we are connecting to
        success: function (data) {

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    var row = table.insertRow(table.rows.length);

    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    var cell3 = row.insertCell(2);
    var cell4 = row.insertCell(3);
    var cell5 = row.insertCell(4);
    var cell6 = row.insertCell(5);
    var cell7 = row.insertCell(6);
    var cell8 = row.insertCell(7);

    cell1.innerHTML=  '<a href="'+ "/uploads/" + filename+'">'+filename+'</a>';
    cell2.innerHTML = source;
    cell3.innerHTML = gedc;
    cell4.innerHTML = encoding;
    cell5.innerHTML = submname;
    cell6.innerHTML = submaddr;
    cell7.innerHTML = "0";
    cell8.innerHTML = "0";


    let select = document.getElementById('viewFiledrop');
    var opt = document.createElement('option');
    opt.value = filename;
    opt.innerHTML = filename;
    select.appendChild(opt);

    select = document.getElementById('getanscFiledrop');
    var opt2 = document.createElement('option');
    opt2.value = filename;
    opt2.innerHTML = filename;
    select.appendChild(opt2);

    select = document.getElementById('getdescFiledrop');
    var opt3 = document.createElement('option');
    opt3.value = filename;
    opt3.innerHTML = filename;
    select.appendChild(opt3);

    select = document.getElementById('addindiFiledrop');
    var opt1 = document.createElement('option');
    opt1.value = filename;
    opt1.innerHTML = filename;
    select.appendChild(opt1);


    document.getElementById("statusarea").value = "Saved a new file: " + filename;


    document.getElementById("getfilename").value = "";
    document.getElementById("getsubmname").value = "";
    document.getElementById("getsubmadress").value = "";

}

function getDescendants(){
	let filename = document.getElementById("getdescFiledrop").value;
    let fname = document.getElementById("descfname").value;
    let lname = document.getElementById("desclname").value;
    let num = parseInt(document.getElementById("descnum").value);

    if(filename == "Select a file"){
        document.getElementById("statusarea").value = "Please select a file";
        return;
    }
    else if(num < 0){
        document.getElementById("statusarea").value = "Number of Generations must be greater than or equal to 0";
        return;
    }
    else if(fname.length == 0){
        document.getElementById("statusarea").value = "Must provide First Name";
        return;
    }
    else if(lname.length == 0){
        document.getElementById("statusarea").value = "Must provide Last Name";
        return;
    }

    /*$.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/getdesc',
            data: {
                'filename': filename,
                'fname' : fname,
                'lname': lname,
                'num' : num
            },   //The server endpoint we are connecting to
            success: function (data) {
                let obj = data.obj;
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
    });*/

    var table = document.getElementById("desctable");
    table.innerHTML = "";

    for (var j = 0; j < num; j++) {

    var row = table.insertRow(j);
    let generation = row.insertCell(0);

    var cell1 = row.insertCell(1);

    generation.innerHTML = "Genaration " + (j+1); 
    cell1.innerHTML = "person" + Math.floor(Math.random() * 11)  +", person" + Math.floor(Math.random() * 11);

    }

    document.getElementById("statusarea").value = "";

}

function getAncestors(){
	let filename = document.getElementById("getanscFiledrop").value;
    let fname = document.getElementById("anscfname").value;
    let lname = document.getElementById("ansclname").value;
    let num = parseInt(document.getElementById("anscnum").value);

    if(filename == "Select a file"){
        document.getElementById("statusarea").value = "Please select a file";
        return;
    }
    else if(num == null || num < 0){
        document.getElementById("statusarea").value = "Number of Generations must be greater than or equal to 0";
        return;
    }
    else if(fname.length == 0){
        document.getElementById("statusarea").value = "Must provide First Name";
        return;
    }
    else if(lname.length == 0){
        document.getElementById("statusarea").value = "Must provide Last Name";
        return;
    }

    var table = document.getElementById("ansctable");
    table.innerHTML = "";

    for (var j = 0; j < num; j++) {

    var row = table.insertRow(j);
    let generation = row.insertCell(0);

    var cell1 = row.insertCell(1);

    generation.innerHTML = "Genaration " + (j+1); 
    cell1.innerHTML = "person" + Math.floor(Math.random() * 11)  +", person" + Math.floor(Math.random() * 11);

    }

    document.getElementById("statusarea").value = "";

}

function populateTable(files){
    let table = document.getElementById("filetable");

    // Create an empty <tr> element and add it to the 1st position of the table:
    for (var i = 0; i < files.length; i++) {
    var row = table.insertRow(i+1);

    // Insert new cells (<td> elements) at the 1st and 2nd position of the "new" <tr> element:
    let cell1 = row.insertCell(0);
    let cell2 = row.insertCell(1);
    let cell3 = row.insertCell(2);
    let cell4 = row.insertCell(3);
    let cell5 = row.insertCell(4);
    let cell6 = row.insertCell(5);
    let cell7 = row.insertCell(6);
    let cell8 = row.insertCell(7);

    cell1.innerHTML=  '<a href="'+ "/uploads/" + files[i]+'">'+files[i]+'</a>';


    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getGEDCOMS',
        data: {
            'filenames': files[i]
        },   //The server endpoint we are connecting to
        success: function (data) {
            let obj = data.GEDCOMS;
            cell2.innerHTML = obj.source;
            cell3.innerHTML = obj.version;
            cell4.innerHTML = obj.encoding;
            cell5.innerHTML = obj.name;
            cell6.innerHTML = obj.adress;
            cell7.innerHTML = obj.indi;
            cell8.innerHTML = obj.fam;
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    // Add some text to the new cells

    }
}

function addIndividuals(){
    let filename = document.getElementById("addindiFiledrop").value;
    let firstname = document.getElementById("indilastname").value;
    let lastname = document.getElementById("indifirstname").value;

    if(filename == "Select a file"){
        document.getElementById("statusarea").value = "Please select a file";
        return;
    }
    if(firstname.length == 0){
        document.getElementById("statusarea").value = "Please enter a first name";
        return;
    }
    if(lastname.length == 0){
        document.getElementById("statusarea").value = "Please enter a last name";
        return;
    }

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/addIndi',
        data: {
            'filename': filename,
            'firstname': firstname,
            'secondname': lastname
        },   //The server endpoint we are connecting to
        success: function (data) {
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    document.getElementById("statusarea").value = "added a new individual " + firstname + " " + lastname + " " +  "to file " + filename;
    
    document.getElementById("indifirstname").value = "";
    document.getElementById("indilastname").value = "";
    document.getElementById("addindiFiledrop").value = "Select a file";
}

function addFiles(files) {
    let select = document.getElementById('viewFiledrop');

    for (var i = 0; i < files.length; i++){
        var opt = document.createElement('option');
        opt.value = files[i];
        opt.innerHTML = files[i];
        select.appendChild(opt);
    }

    select = document.getElementById('getanscFiledrop');

    for (var i = 0; i < files.length; i++){
        var opt = document.createElement('option');
        opt.value = files[i];
        opt.innerHTML = files[i];
        select.appendChild(opt);
    }

    select = document.getElementById('getdescFiledrop');

    for (var i = 0; i < files.length; i++){
        var opt = document.createElement('option');
        opt.value = files[i];
        opt.innerHTML = files[i];
        select.appendChild(opt);
    }

    select = document.getElementById('addindiFiledrop');

    for (var i = 0; i < files.length; i++){
        var opt = document.createElement('option');
        opt.value = files[i];
        opt.innerHTML = files[i];
        select.appendChild(opt);
    }

    select = document.getElementById('dbFiledrop');

    for (var i = 0; i < files.length; i++){
        var opt = document.createElement('option');
        opt.value = files[i];
        opt.innerHTML = files[i];
        select.appendChild(opt);
    }


}

function saveFiles(){

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/deletedb',
        success: function (data) {
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/saveFiles',
        success: function (data) {
            document.getElementById("statusarea").value = data.toPrint;
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function help(){

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/help',
        success: function (data) {
            for (var i = 0; i < data.toPrint.length; i++) {
                document.getElementById("dbstatusarea").value += toString(data.toPrint[i]);
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function dbdispindibutton(){

    document.getElementById("dbstatusarea").value = "";

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getindilname',
        success: function (data) {
            if(data.toPrint.length == 0){
                document.getElementById("dbstatusarea").value = "No individuals";
                return;
            }
            for (var i = 0; i < data.toPrint.length; i++) {
                document.getElementById("dbstatusarea").value += data.toPrint[i].given_name + " " +data.toPrint[i].surname + "\n";
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function clearFiles(){

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/deletedb',
        success: function (data) {
            document.getElementById("statusarea").value = data.toPrint;
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function dispindifile(){
    document.getElementById("dbstatusarea").value = "";

    let file = document.getElementById("dbFiledrop").value;

    if(file == "Select a file"){
        document.getElementById("dbstatusarea").value = "Please select a file";
        return;
    }

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getfileindis',
        data: {
            'filename': file
        },
        success: function (data) {
            if(data.toPrint.length == 0){
                document.getElementById("dbstatusarea").value = "No individuals";
                return;
            }
            for (var i = 0; i < data.toPrint.length; i++) {
                document.getElementById("dbstatusarea").value += data.toPrint[i].given_name + " " +data.toPrint[i].surname + "\n";
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function displayDB(){

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getdbstatus',
        success: function (data) {
            document.getElementById("statusarea").value = data.toPrint;
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

}

function openTab(evt, name) {
    var i, tabcontent, tablinks;

    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }

    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }

    document.getElementById(name).style.display = "block";
    evt.currentTarget.className += " active";
}

