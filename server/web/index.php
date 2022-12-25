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



        var _app = {
            "loadAuthorities": function()
            {
                var auth = $("#authorities");

                $.get("fpdata.php?m=authority", function(data){
                    for(i = 0; i < data.length; i++)
                        auth.append('<div><a href="Javascript:app().loadAreas(\'' + q(data[i]) + '\');">' + data[i] + '</a>' +
                            '<div id="areas_' + q(data[i]).replace(/ /g, '_') + '" class="indent"></div></div>');
                }, "json");
            },

            "loadAreas": function(id)
            {
                var d = $("#areas_" + id.replace(/ /g, '_'));
                if(d.html() == "")
                    $.get("fpdata.php?m=area&a=" + encodeURI(id), function(data){
                        for(i = 0; i < data.length; i++)
                        {
                            d.append('<div><a href="JavaScript:app().loadStops(\'' + q(data[i]) + '\');">' + data[i] + '</a>' +
                                '<div id="stops_' + q(data[i]).replace(/ /g, '_') + '" class="indent"></div>' +
                                '</div>');
                        }
                    }, "json");
                else
                    d.html("");
            },

            "loadStops": function(id)
            {
                var dx = $("#stops_" + id.replace(/ /g, '_'));
                // console.log("#stops_" + id.replace(/ /g, '_'));
                if(dx.html() == "")
                    $.get("fpdata.php?m=stop&a=" + encodeURI(id), function(data){
                        for(i = 0; i < data.length; i++)
                        {
                            var d = data[i];
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
