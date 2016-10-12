var cmd1 = "files //depot/dev/ggibbons/src/p4nodeapi/...".split(" ");
var p4nodeapi = require('./build/Release/p4nodeapi');
hi = new p4nodeapi.P4NodeApi();
hi.run(cmd1,function(err,data){
  console.log(data);
  console.log(err);
});

