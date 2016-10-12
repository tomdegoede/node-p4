try {var P4Con = require('../main').P4Connection;}
catch(e){
    console.log("caught error:",e);
    if (e.toString().indexOf("Cannot find module '../lib/p4nodeapi'")>=0)
        console.log("run `npm install` to build required native module.");

    process.exit(1);
}

var port = process.env.npm_package_config_testPort;

console.log("testing using port:",port);

if (port == "env")
    port = "";

var p4 = new P4Con({
    'port':port,
    'json':true
});

console.log("running 'p4 info'");
p4.run(["info"],function(e,d){
    finishedTest("info",(e==''));
});

console.log("running 'p4 depots'");
p4.run(["depots"],function(e,d){
    finishedTest("depots",(e==''));
});


var infoFinished = false;
var infoPassed = false;
var depotsFinished = false;
var depotsPassed = false;

function finishedTest(cmd,passed) {
    if (cmd=="info"){
        infoFinished = true;
        infoPassed = passed;
    }
    else if (cmd=="depots") {
        depotsFinished = true;
        depotsPassed = passed;
    }

    if (infoFinished && depotsFinished) {
        var code = 1;

        if (infoPassed && depotsPassed)
            code = 0;

        console.log("Commands succeeded? - p4 info:",infoPassed,", p4 depots: ",depotsPassed);

        process.exit(code);
    }
}