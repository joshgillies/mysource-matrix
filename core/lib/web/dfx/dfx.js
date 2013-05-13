var dfxLoadedCallback = null;

// Defines which files will be included. These must be in the same directory as
(function() {
    var jQueryFilesString = 'jquery.js|jquery.ui.js';
    var dfxLibraryString  = 'event.js|dom.js|css.js|general.js|arrays.js|effects.js|ajax.js|util.js|json.js|date.js|xpath.js';
    var dfxScripts        = document.getElementsByTagName('script');
    var path              = null;

    // Loop through all the script tags that exist in the document and find the one
    // that has included this file.
    var dfxScriptsLen = dfxScripts.length;
    for (var i = 0; i < dfxScriptsLen; i++) {
        if (dfxScripts[i].src) {
            if (dfxScripts[i].src.match(/dfx\.js/)) {
                // We have found our appropriate <script> tag that includes the
                // DfxJSLib library, so we can extract the path and include the rest.
                path = dfxScripts[i].src.replace(/dfx\.js/,'');
                break;
            }
        }
    }

    var jQueryFiles        = jQueryFilesString.split('|');
    var jQueryFilesLen     = jQueryFiles.length;
    var dfxLibraryFiles    = dfxLibraryString.split('|');
    var dfxLibraryFilesLen = dfxLibraryFiles.length;

    var _loadScript = function(scriptName, callback) {
        var script = document.createElement('script');

        if (navigator.appName == 'Microsoft Internet Explorer') {
            var rv = -1;
            var re = new RegExp("MSIE ([0-9]{1,}[\.0-9]{0,})");
            if (re.exec(navigator.userAgent) != null) {
                rv = parseFloat(RegExp.$1);
            }

            if (rv <= 8.0) {
                script.onreadystatechange = function() {
                    if (/^(loaded|complete)$/.test(this.readyState) === true) {
                        callback.call(window);
                    }
                };
            }
        }//end if

        script.onload = function() {
            callback.call(window);
        };

        script.src = path + scriptName;

        if (document.head) {
            document.head.appendChild(script);
        } else {
            document.getElementsByTagName('head')[0].appendChild(script);
        }

    };

    var _loadScripts = function(scripts, callback) {
        if (scripts.length === 0) {
            callback.call(window);
            return;
        }

        var script = scripts.shift();
        _loadScript(script, function() {
            _loadScripts(scripts, callback);
        });
    };

    _loadScripts(jQueryFiles, function() {
       // Load DfxJSLib files.
       _loadScripts(dfxLibraryFiles, function() {
           if (dfxLoadedCallback) {
               dfxLoadedCallback.call(window);
           } else {
                var maxTry   = 10;
                var interval = setInterval(function() {
                    maxTry--;
                    if (dfxLoadedCallback) {
                        dfxLoadedCallback.call(window);
                        clearInterval(interval);
                    } else if (maxTry === 0) {
                        clearInterval(interval);
                    }
                }, 500);
            }
       });
    });

})();
