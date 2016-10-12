//grab reference to the p4 connection object constructor
var P4NodeApi = require('../build/Release/p4nodeapi').P4NodeApi;

//construct a p4 connection object (one per command - since they'll be running simultaneously)
var p4 = new P4NodeApi();
var p5 = new P4NodeApi();
var p6 = new P4NodeApi();

//if you pass nothing to the constructor it uses your env vars, but
//you can pass an object to specify port/user/client
var p7 = new P4NodeApi({
    'user':     'jking',
    'port':     'perforce:1666',
    'client':   'jplay'
});
//if you provide an object that is missing any of the values, then the
//missing values revert to environemntal defaults
var p8 = new P4NodeApi({
    'port':     'play:1999'
});

console.log("test.js: \n");

//set up some commands (each must be an array!) to run
var cmd1 = ["info"];
var cmd2 = "clients sdfsdf -m 6".split(" "); //this one will return an error
var cmd3 = "users -m 1".split(" ");

//run the commands

p4.run(cmd1,function(err, data){
    console.log("    pointer: p4");
    console.log("    command: "+cmd1);
    console.log("    output:");
    console.log("       err: \n"+err);
    console.log("       data: \n"+data);
});

p5.run(cmd2,function(err, data){
    console.log("    pointer: p5");
    console.log("    command: "+cmd2);
    console.log("    output:");
    console.log("       err: \n"+err);
    console.log("       data: \n"+data);
});

p6.run(cmd3,function(err, data){
    console.log("    pointer: p6 (first command/callback)");
    console.log("    command: "+cmd3);
    console.log("    output:");
    console.log("       err: \n"+err);
    console.log("       data: \n"+data);

    //now that this is done running, you can run another command with the same pointer, if you'd like
    p6.run(cmd1,function(err, data){
        console.log("    pointer: p6 (second command/callback run via callback nesting)");
        console.log("    command: "+cmd1);
        console.log("    output:");
        console.log("       err: \n"+err);
        console.log("       data: \n"+data);
    });
});

// p7.run(["info"],function(err, data){
//     console.log("    pointer: p7");
//     console.log("    command: info");
//     console.log("    output:");
//     console.log("       err: \n"+err);
//     console.log("       data: \n"+data);
// });


// p8.run(["info"],function(err, data){
//     console.log("    pointer: p8");
//     console.log("    command: info");
//     console.log("    output:");
//     console.log("       err: \n"+err);
//     console.log("       data: \n"+data);
// });
