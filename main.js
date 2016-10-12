// var NativeP4NodeApi;

// (function() {
//     var os = require('os');
//     var architecture;

//     if (os.platform()=="darwin")
//     	architecture = "darwin90x86";
//     else if (os.platform()=="linux")
//     	architecture = "linux26x86";

//     if (os.arch()=="x64")
//     	architecture+="_64";

//     NativeP4NodeApi = require("./lib."+architecture+"/p4nodeapi.node").P4NodeApi;
//  })()

NativeP4NodeApi = require("./lib/p4nodeapi").P4NodeApi;

module.exports.P4Connection = function(_config) {
	var that = this;
	var m_config = {
		"user":"",
		"password":"",
		"port":"",
		"client":"",
		"json":true
	}

	for (var key in m_config) {
		if (_config.hasOwnProperty(key))
			m_config[key] = _config[key];
	}

	this.clone = function(overwriteParams) {
		var n_config = {};
		for (var key in m_config)
			if (overwriteParams.hasOwnProperty(key))
				n_config[key] = overwriteParams[key];
			else
				n_config[key] = m_config[key];

		return new module.exports.P4Connection(n_config);
	}

	this.updateForm = function(outputArgs,updates,inputArgs,callback) {
		if (typeof(outputArgs)=="string")
			outputArgs = outputArgs.split(" ");

		if (outputArgs.indexOf("-o")<0)
			outputArgs.push("-o");

		that.run(outputArgs,"",true,function(e,d){
			that.joinFields(d[0],function(err,joinedFields){
				var inputString = "";
				for (var field in joinedFields) {
					if (joinedFields.hasOwnProperty(field)) {
						var fieldVal;
						if (updates.hasOwnProperty(field))
							fieldVal = updates[field];
						else
							fieldVal = joinedFields[field];

						if (fieldVal!=null) {
							if (fieldVal instanceof Array)
								fieldVal = "\n"+fieldVal.join("\n");
							
							inputString+= field+": " + fieldVal + "\n";
						}
					}
				}
				that.run(inputArgs,inputString,callback);
			});
		});
	}

	this.joinFields = function(formData,callback){
		try {
			var formFields = Object.keys(formData);
			var joinedFields = {};

			for (var i=0; i<formFields.length; i++) 
			{
				var field = formFields[i];
				var numMatch = field.match(/\d+$/);
				if (numMatch == null)
					joinedFields[field] = formData[field];
				else 
				{
					var _field = field.replace(numMatch[0],"");
					var index = parseInt(numMatch[0]);

					if (joinedFields.hasOwnProperty(_field))
						joinedFields[_field].push(formData[field]);
					else
						joinedFields[_field] = [formData[field]];
				}
			}
			callback(null,joinedFields);
		}
		catch(err) {
			console.log("p4con - joinfields caught: ",err);
			callback(err);
		}
	}

	this.validate = function(callback) {
		that.run(["files","-m","1","//..."],function(e){
			callback((e.length==0));
		});
	}
	
	this.run = function() {
		var args = arguments[0];
		if (typeof(args)=="string")
			args = args.split(" ");

		var formString 	= "";
		var json 		= true;
		var callback	= false;

		if (typeof(arguments[1])=="string")
			var formString = arguments[1];
		else if (typeof(arguments[1])=="boolean")
			json = arguments[1];
		else if (typeof(arguments[1])=="function")
			callback = arguments[1];

		if (!callback) {
			if (typeof(arguments[2])=="boolean") {
				json = arguments[2];
				callback = arguments[3];
			}
			else
				callback = arguments[2];
		}

		if (typeof(callback)!="function")
			throw Error("must provide callback to P4Connection.run");

		var _con = new NativeP4NodeApi({
			'user':m_config.user,
			'port':m_config.port,
			'client':m_config.client,
			'json':json
		});

		function _callback(err,data) {
			if (json) {
				try {
					data = JSON.parse(data);
				}
				catch(e) { 
					// err += "& JSON.parse Error."; 
				}
			}
			delete _con;
			callback(err,data);
		}
		_con.run(args,formString,_callback);
	}

	return this;
}
