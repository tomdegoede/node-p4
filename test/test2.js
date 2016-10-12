var cmd1 = "clients -m 4".split(" ");
var cmd2 = "jobs -m 6".split(" ");
var p4nodeapi = require('./build/Release/p4nodeapi');
hi = new p4nodeapi.P4NodeApi();
hi.run(cmd1,function(err,data){
  console.log(data);
  console.log(err);
});
hi2 = new p4nodeapi.P4NodeApi();
    hi2.run(cmd2,function(err,data){
      console.log(data);
      console.log(err);
    });

