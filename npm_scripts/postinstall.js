var fsautil = require('fsautil');

console.log("relocating freshly-built native plugin...");
fsautil.mkdir_p("./lib",function(){
	fsautil.cp("./build/Release/p4nodeapi.node","./lib/p4nodeapi.node",function(){
		console.log("cleaning up build dir...");
		fsautil.rm_rf("./build",function(){
			console.log("cleaning up downloaded p4 c++ api files...");
			fsautil.rm_rf("./tmp",function(){
				console.log("all cleaned up, and ready to rock!");
				process.exit(0);
			});
		});
	});
});