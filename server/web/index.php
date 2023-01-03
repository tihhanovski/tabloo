<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <title>Tabloo</title>
    <script src="/inc/jquery-3.5.1.min.js"></script>
    <style>
        .indent{padding-left: 30px;}
    </style>
</head>
<body>
    <div id="authorities">
    </div>
    <script language="JavaScript">

        const apiUrl = '/tablooapi/';


        var _app = {
            "loadAuthorities": function()
            {
                var auth = $("#authorities");

                // const url = "fpdata.php?m=authority"
                const url = apiUrl + 'authorities';

                $.get(url, function(data){
                    data.data.forEach(i => auth.append('<div><a href="Javascript:app().loadAreas(\'' + q(i) + '\');">' + i + '</a>' +
                            '<div id="areas_' + q(i).replace(/ /g, '_') + '" class="indent"></div></div>'))
                }, "json");
            },

            "loadAreas": function(id)
            {
                var d = $("#areas_" + id.replace(/ /g, '_'));
                //"fpdata.php?m=area&a=" + encodeURI(id)
                const url = apiUrl + 'areas/' + encodeURIComponent(id)
                console.log(url);
                if(d.html() == "")
                    $.get(url, function(data){
                        data.data.forEach(i => d.append('<div>' +
                                '<a href="JavaScript:app().loadStops(\'' + q(i) + '\');">' + i + '</a>' +
                                '<div id="stops_' + q(i).replace(/ /g, '_') + '" class="indent"></div>' +
                                '</div>'));
                    }, "json");
                else
                    d.html("");
            },

            "loadStops": function(id)
            {
                var dx = $("#stops_" + id.replace(/ /g, '_'));
                //"fpdata.php?m=stop&a=" + encodeURI(id)
                const stopsUrl = apiUrl + 'stops/' + encodeURIComponent(id)

                if(dx.html() == "")
                    $.get(stopsUrl, function(data){
                        for(i = 0; i < data.data.length; i++)
                        {
                            var d = data.data[i];
                            var url = 'ask/?c=' + encodeURI(d.code);
                            dx.append('<div><a href="' + url + '" target="_blank">' + d.code + '</a> '
                                + '<a href="' + url + '&h=1" target="_blank">' + d.name 
                                + (d.memo !== '' ? ' (' + d.memo + ')' : '') + '</a>' +
                                '</div>');
                        }
                    }, "json");
                else
                    dx.html("");
            }
        };

        function q(s)
        {
            return s.replace(/"/g, '\\x22').replace(/'/g, '\\x27');
        }

        function app()
        {
            return _app;
        }

        $(function(){
            app().loadAuthorities();
        });

    </script>
</body>
</html>
