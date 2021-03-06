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
        'password':m_config.password,
        'port':m_config.port,
        'client':m_config.client,
        'json':m_config.json
    });

	var queue = null;
	
	this.run = function(args, input) {
		if (typeof(args)=="string")
			args = args.split(" ");

        return queue = (queue || Promise.resolve()).catch(function() {
            return true;
        }).then(function () {
            return new Promise(function (resolve, reject) {
                _con.run(args, input || "", function (err, ress) {
                    if(err) {
                        reject(err);
                    } else {
                        resolve(ress);
                    }
                });
            });
        });
	};

    this.close = function () {
        _con.close();
    };

	return this;
};
