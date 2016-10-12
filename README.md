Installation
============
To install:  
> `npm install`

To change the version of the p4 c++ api:  
> `npm config set p4:apiVersion [version (eg: 10.2, 12.2, etc)]`

(Default is currently 12.2)

To run the test:   
> `npm test`

    ***NOTE: by default, the test tries to connect to a perforce depot at "perforce:1666"
        to change ports, run `npm config set p4:testPort [desired P4PORT]`
        (if you set it to "", it will use your P4PORT env var)***
