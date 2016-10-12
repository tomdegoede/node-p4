var os 	 	= require('os');
var fs 	 	= require("fs");
var http 	= require("http");
var url  	= require("url");
var exec 	= require("child_process").exec;
var fsautil;

function startPrebuild() {
	fsautil.rm_rf("./build",function(){
		console.log("cleaning up previous builds");
		fsautil.rm_rf("./tmp",function(){
			fsautil.rm_rf("./lib",function(){
				fs.mkdir("./tmp",function(){
					var architecture;
					if (os.platform()=="darwin")
						architecture = "darwin90x86";
					else if (os.platform()=="linux")
						architecture = "linux26x86";

					if (os.arch()=="x64")
						architecture+="_64";

					var version = (process.env.npm_package_config_apiVersion || "12.2");
					var arch = (process.env.npm_package_config_arch || architecture);

					console.log("GET: p4 c++ api, version:",version,", for:",arch);
					var downloadUrl = "http://ftp.perforce.com/perforce/r"+version+"/bin."+arch+"/p4api.tgz";
					console.log("downloading from: ",downloadUrl);
					var parsedUrl = url.parse(downloadUrl);
					
					var file = fs.createWriteStream("./tmp/p4api.tgz");
					http.get({
						host:parsedUrl.host,
						port:80,
						path:parsedUrl.path
					},function(res){
						res.on("data",function(data){
							file.write(data);
						});
						res.on("end",function(){
							if (res.statusCode==200) {
								console.log("download finished successfully!");
								file.end();
								exec("tar -xf ./p4api.tgz",{cwd:process.cwd()+"/tmp"},function(){
									console.log("extracting tar...");
									fs.unlink("./tmp/p4api.tgz",function(){
										console.log("removing tar...");
										fs.readdir("./tmp",function(er,items){
											console.log("relocating tar contents...");
											fsautil.cp_r("./tmp/"+items[0],"./tmp/p4api",function(){
												fsautil.rm_rf("./tmp/"+items[0],function(){
													console.log("ready to build native plugin!");
													process.exit(0);
												});
											});
										});
									});
								});
							}
							else {
								console.log("download request failed w/ status code: ",res.statusCode);
								fsautil.rm_rf("./tmp",function(){
									process.exit(1);
								});
							}
						});
					});
				});
			
			});
		});
	});
}

try {
	fsautil = require("fsautil");
	startPrebuild();
}
catch(e) {
	exec("npm install fsautil",function(){
		fsautil = require("fsautil");
		startPrebuild();
	});
}