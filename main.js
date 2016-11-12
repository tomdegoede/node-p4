const os = require('os'); // x64
const NativeP4NodeApi = require(["./lib/p4nodeapi", os.platform(), os.arch()].join('_')).P4NodeApi;

module.exports.P4Connection = function(_config) {
	var m_config = {
		"user":"",
		"password":"",
		"port":"",
		"client":"",
		"json":false
	};

	for (var key in m_config) {
		if (_config.hasOwnProperty(key))
			m_config[key] = _config[key];
	}

    var _con = new NativeP4NodeApi({
        'user':m_config.user,
        'port':m_config.port,
        'client':m_config.client,
        'json':m_config.json
    });
	
	this.run = function() {
		var args = arguments[0];
		if (typeof(args)=="string")
			args = args.split(" ");

		var formString 	= "";
		var callback	= false;

		if (typeof(arguments[1])=="string")
			formString = arguments[1];
		else if (typeof(arguments[1])=="function")
			callback = arguments[1];

		if (!callback) {
            callback = arguments[2];
		}

		if (typeof(callback)!="function")
			throw Error("must provide callback to P4Connection.run");

		_con.run(args,formString,callback);
	};

    this.close = function () {
        _con.close();
    };

	return this;
};
