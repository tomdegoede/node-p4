var P4NodeApi = require('./build/Release/p4nodeapi').P4NodeApi;

var p4 = new P4NodeApi({
                       "user":"jking",
                       "port":"play:1999",
                       "json":false
                       });

p4.run(['user','-o'],function(e,d){

    console.log("Error: \n",e);
    console.log("Data: \n",d);
    var specSet = d.split("\n\n")
                                                 
    var specString = "";
        
    for (var i=0;i<specSet.length;i++) {
        if (specSet[i].indexOf("Email:")==0)
        {   
            if (specSet[i].indexOf("jking@gmail.com")>0)
                specSet[i] = "Email: jking@perforce.com";
            else
                specSet[i] = "Email: jking@gmail.com";
        }
       
        if (typeof(specSet[i][0])=="string")
            if (specSet[i][0] == specSet[i][0].toUpperCase())
                specString += specSet[i] + "\n\n";
    }
       
    p4.run(['user','-i'],specString,function(e2,d2){
               console.log("Error:    ",e2);
               console.log("Data:    ",d2);
            });
});

var p5 = new P4NodeApi({
                       "user":"jking",
                       "port":"play:1999"
                       });

p5.run(['user','-o'],function(e,d){
       
       console.log("Error: \n",e);
       console.log("Data: \n",d);
       var specSet = d.split("\n\n")
       try {
            var dObj = JSON.parse(d);
        
            if (dObj[0].FullName=="Jaimen King")
                dObj[0].FullName = "J-man King";
            else
                dObj[0].FullName = "Jaimen King";
        
            var specSet = JSON.stringify(dObj).replace("[","")
                                            .replace("]","")
                                            .replace("{","")
                                            .replace("}","")
                                            .replace(/:/g,":\ ")
                                            .replace(/\"/g,"")
                                            .split(",");
       
           var specString = "";
       
           for (var i=0;i<specSet.length;i++) {
               if (specSet[i][0] == specSet[i][0].toUpperCase())
                   specString += specSet[i] + "\n\n";
           }
       
           p5.run(['user','-i'],specString,function(e2,d2){
                  console.log("Error:    ",e2);
                  console.log("Data:     ",d2);
           });
       }
       catch(e3) {
           console.log("ERROR:",e3);
       }
});
