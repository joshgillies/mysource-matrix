if (!window.dfx) {
    window.dfx = function() {};
    window.dfxjQuery = $.noConflict(true);
}

/*
 * The util package.
 */
var Util = {};

/**
 * A hash table indexed by an object.
 *
 * @constructor
 */
Util.ObjectHash = function()
{
    /*
     * The array of objects.
     *
     * @var array
     */
    this.objects = [];

    /*
     * The array of values.
     *
     * Each indice in this array corresponds the owning object for that value.
     *
     * @var array
     */
    this.values = [];

};


/*
 * Returns the index used to index this object.
 *
 * @param {Object} object The object to obtain the index for.
 *
 * @return integer
 */
Util.ObjectHash.prototype.getObjectIndex = function(object)
{
    var oln = this.objects.length;
    for (var i = 0; i < oln; i++) {
        if (this.objects[i] === object) {
           return i;
        }
    }

    return -1;

};


/*
 * Puts the specified value in the hash indexed by the specified object.
 *
 * If the object already exists in the hash, the value will be replaced with the
 * new value.
 *
 * @param {Object} object The object that represents the key.
 * @param {Mixed}  value  The value to store.
 *
 * @return void
 */
Util.ObjectHash.prototype.put = function(object, value)
{
    var index = this.getObjectIndex(object);
    if (index !== -1) {
        this.values[index] = value;
    } else {
        this.objects.push(object);
        this.values.push(value);
    }

};


/*
 * Returns the value for the specified object.
 *
 * @param {Object} object The object that represents the key.
 *
 * @return mixed or null if the object does not exist in the hash.
 */
Util.ObjectHash.prototype.get = function(object)
{
    var index = this.getObjectIndex(object);

    if (index !== -1) {
        return this.values[index];
    }

    return null;

};


/*
 * Returns TRUE if the specified object exists in the hash.
 *
 * @param {Object} object The object that represents the key.
 *
 * @return boolean
 */
Util.ObjectHash.prototype.containsKey = function(object)
{
    return (this.getObjectIndex(object) != -1);

};


/*
 * Replaces the value currently stored for the specified object.
 *
 * @param {Object} object The object that represents the key.
 * @param {Mixed}  value  The new value to store.
 *
 * @return void
 */
Util.ObjectHash.prototype.replace = function(object, value)
{
    var index = this.getObjectIndex(object);

    if (index !== -1) {
        this.values[index] = value;
    }

};

/**
 * Initialises the XML for parsing.
 *
 * @param string xml The XML contents.
 *
 * @return void
 */
Util.Xml = function(xml)
{
    // The xml argument is optional.
    this.xml = xml;

};

/*
 * Return an XML Object that is valid for the current browser.
 *
 * @return object
 */
Util.Xml.prototype.parse = function()
{
    var xml = this.xml;
    var doc = null;
    if (window.ActiveXObject) {
        doc       = new ActiveXObject("Microsoft.XMLDOM");
        doc.async = "false";
        doc.loadXML(xml);
    } else {
        var parser = new DOMParser();
        doc        = parser.parseFromString(xml,"text/xml");
    }

    return doc;

};

Util.Xml.getElementById = function(id, parent)
{
    if (parent) {
        if (parent.getElementById) {
            return parent.getElementById(id);
        }

        var pcln = parent.childNodes.length;
        for (var i = 0; i < pcln; i++) {
            if (parent.childNodes[i].getAttribute('id') === id) {
                return parent.childNodes[i];
            } else {
                if (parent.childNodes[i].childNodes.length > 0) {
                    var el = this.getElementById(id, parent.childNodes[i]);
                    if (el && el.getAttribute('id') === id) {
                        return el;
                    }
                }
            }
        }
    }

    return null;

};


/**
 * returns a left trimmed string.
 *
 * @param {String} value The string to trim.
 * @param {String} trimChars The different chars to trim off the left.
 *
 * @return {String}
 */
dfx.ltrim = function (str, trimChars)
{
    trimChars = trimChars || '\\s';
    return str.replace(new RegExp('^[' + trimChars + ']+', 'g'), '');

}

/**
 * returns a right trimmed string.
 *
 * @param {String} value The string to trim.
 * @param {String} trimChars The different chars to trim off the right.
 *
 * @return {String}
 */
dfx.rtrim = function (str, trimChars)
{
    trimChars = trimChars || '\\s';
    return str.replace(new RegExp('[' + trimChars + ']+$', 'g'), '');

}


/**
 * returns a trimmed string.
 *
 * @param {String} value The string to trim.
 * @param {String} trimChars The different chars to trim off both sides.
 *
 * @return {String}
 */
dfx.trim = function(value, trimChars)
{
    return dfx.ltrim(dfx.rtrim(value, trimChars), trimChars);

};

/**
 * returns true if specified string is blank.
 *
 * @param {String} value The string to test.
 *
 * @return {String}
 */
dfx.isBlank = function(value)
{
    if (!value || /^\s*$/.test(value)) {
        return true;
    }

    return false;

};

/**
 * returns an ellipsized string.
 *
 * @param {String} value The string to trim.
 *
 * @return {String}
 */
dfx.ellipsize = function(value, length)
{
    // Type validation.
    if (typeof value !== 'string' || typeof length !== 'number') {
        return '';
    }

    // Length needs to be at least zero.
    if (length < 0) {
        return '';
    }

    // If the string is not long enough, don't change it.
    if (value.length <= length) {
        return value;
    }

    value = value.substr(0, length);
    value = value.replace(/\s$/, '');

    // Figure out how many dots are on the end of the
    // string so we don't add too many.
    var end       = value.substr((length - 3), 3);
    var endNoDots = end.replace(/\.$/, '');
    var numDots   = (end.length - endNoDots.length);

    value = value + dfx.strRepeat('.', (3 - numDots));
    return value;

};


dfx.ellipsizeDom = function(elem, length)
{
    var browserInfo = dfx.browser();
    if (browserInfo.type === 'msie') {
        // Handle ellipsis with CSS style, text-align: ellipsis;.
        dfx.setStyle(elem, 'text-overflow', 'ellipsis');
        dfx.setStyle(elem, 'white-space', 'nowrap');
        dfx.setStyle(elem, 'width', length + 'px');
    } else {
        // We have to manually handle FF browsers to correctly
        // ellipsize based on the width.
        dfx.setStyle(elem, 'visibility', 'hidden');
        var currWidth = dfx.getStyle(elem, 'width');
        currWidth     = parseInt(currWidth.substr(0, (currWidth.length - 2)), 10);
        if (currWidth > length) {
            var oriName = dfx.getHtml(elem);
            var tmpName = oriName;
            while (currWidth > length) {
                tmpName = tmpName.substring(0, (tmpName.length - 1));
                dfx.setHtml(elem, tmpName);
                currWidth = dfx.getStyle(elem, 'width');
                currWidth = parseInt(currWidth.substr(0, (currWidth.length - 2)), 10);
            }

            var ellipsisLen = tmpName.length + 1;
            oriName = dfx.ellipsize(oriName, (ellipsisLen - 4));
            dfx.setHtml(elem, oriName);
        }

        dfx.setStyle(elem, 'visibility', 'visible');
    }//end if

};

/**
 * Changes the first character to uppercase.
 */
dfx.ucFirst = function(str)
{
    return str.substr(0,1).toUpperCase() + str.substr(1, str.length);

};

dfx.ucWords = function(str)
{
    return str.toLowerCase().replace(/\w+/g,function(s){
          return s.charAt(0).toUpperCase() + s.substr(1);
    });

};

/**
 * Returns true if specified var is a function
 */
dfx.isFn = function(f)
{
    if (typeof f === 'function') {
        return true;
    }

    return false;

};

dfx.isObj = function(v)
{
    if (v !== null && typeof v === 'object') {
        return true;
    }

    return false;

};

dfx.isset = function(v)
{
    if (typeof v !== 'undefined' && v !== null) {
        return true;
    }

    return false;

};

/**
 * Returns true if the specified string contains numbers only.
 */
dfx.isNumeric = function(str)
{
    var result = str.match(/^[-+]?[ ]?\d+\.?\d*$/);

    if (result !== null) {
        return true;
    }

    return false;

};

dfx.clone = function(value, shallow)
{
    if (typeof value !== 'object') {
        return value;
    }

    if (value === null) {
        var valueClone = null;
    } else {
        var valueClone = new value.constructor();
        for (var property in value) {
            if (shallow) {
                valueClone[property] = value[property];
            }

            if (value[property] === null) {
                valueClone[property] = null;
            } else if (typeof value[property] === 'object') {
                valueClone[property] = dfx.clone(value[property], shallow);
            } else {
                valueClone[property] = value[property];
            }
        }
    }

    return valueClone;

};

// Return TRUE if two objects are NOT same.
dfx.objDiff = function(obj1, obj2)
{
    var count1 = 0;
    var count2 = 0;
    for (var p in obj1) {
        count1++;
    }

    for (var q in obj2) {
        count2++;
    }

    if (count1 !== count2) {
        return true;
    }

    for (var p in obj1) {
        if (obj2.hasOwnProperty(p) === false) {
            return true;
        }

        if (typeof obj1[p] === 'object') {
            if (dfx.objDiff(obj1[p], obj2[p])) {
                return true;
            }
        } else {
            if (obj1[p] !== obj2[p]) {
                return true;
            }
        }
    }

    return false;

};

/**
 * Strips a protcol from a URL.
 *
 * @param {string} url The URL to strip the protocol from.
 *
 * @return string
 */
dfx.stripUrlProtcol = function(url)
{
    var pStartIdx = url.search(/:\/\//);
    if (pStartIdx === -1) {
        return url;
    } else {
        // Add three so get rid of all of the ://.
        pStartIdx += 3;
        var protocolStrippedUrl = url.substr(pStartIdx);
        return protocolStrippedUrl;
    }

};

dfx.baseUrl = function(fullUrl)
{
    var qStartIdx = fullUrl.search(/\?|#/);
    if (qStartIdx === -1) {
        return fullUrl;
    } else {
        var baseUrl = fullUrl.substr(0, qStartIdx);
        return baseUrl;
    }

};

/**
 * Returns the given URL's path.
 *
 * @param {string} fullUrl The URL to get the path from.
 *
 * @return string
 */
dfx.getUrlPath = function(fullUrl)
{
    var protocolStrippedUrl = dfx.stripUrlProtcol(fullUrl);
    var protocolFreeBaseUrl = dfx.baseUrl(protocolStrippedUrl);

    var pStartIdx = protocolFreeBaseUrl.search(/\//);
    if (pStartIdx === -1) {
        return '';
    } else {
        // Get rid of the first slash.
        pStartIdx += 1;
        var path   = protocolFreeBaseUrl.substr(pStartIdx);
        return path;
    }

};

/**
 * Return key value pairs from the given query string.
 */
dfx.queryString = function(url)
{
    var result    = {};
    var qStartIdx = url.search(/\?/);
    if (qStartIdx === -1) {
        return result;
    } else {
        var aStartIdx = url.search(/\#/);
        if (aStartIdx === -1) {
            var anchorPartAdj = 0;
        } else {
            var anchorPartAdj = (url.length - aStartIdx + 1);
        }

        // QryStr part is between ? and # in the URL.
        var queryStr = url.substr((qStartIdx + 1), (url.length - qStartIdx - anchorPartAdj));
        if (queryStr.length > 0) {
            var pairs = queryStr.split('&');
            var len   = pairs.length;
            var pair  = [];
            for (var i = 0; i < len; i++) {
                // Is it a valid key value pair?
                if (pairs[i].search('=') !== -1) {
                    pair            = pairs[i].split('=');
                    result[pair[0]] = pair[1];
                }
            }

            return result;
        } else {
            return result;
        }
    }//end if

};

/**
 * Returns the anchor part of the URL.  Blank if no # or
 * hash followed by the actual anchor name
 */
dfx.anchorPart = function(url)
{
    if (typeof url === 'string') {
        var aStartIdx = url.search(/\#/);
        if (aStartIdx === -1) {
            url = '';
        } else {
            url = url.substr(aStartIdx, (url.length - aStartIdx));
        }
    }

    return url;

};

/**
 * Returns given URL without the anchor part if it was given.
 */
dfx.noAnchorPartUrl = function(url)
{
    if (typeof url === 'string') {
        var aStartIdx = url.search(/\#/);
        if (aStartIdx !== -1) {
            var url = url.substr(0, aStartIdx);
        }
    }

    return url;

};

/**
 * Adds given (var => value) list to the given URLs query string.
 */
dfx.addToQueryString = function(url, addQueries)
{
    var mergedUrl        = '';
    var baseUrl          = dfx.baseUrl(url);
    var queryStringArray = dfx.queryString(url);
    mergedQry = dfx.objectMerge(queryStringArray, addQueries);

    var queryStr = '?';
    dfx.foreach(mergedQry, function(key) {
            queryStr = queryStr + key + '=' + mergedQry[key] + '&';
        });

    // More than just a ? to add to the URL?
    if (queryStr.length > 1) {
        // Put the URL together with qry str and take off the trailing &.
        mergedUrl = baseUrl + queryStr.substr(0, (queryStr.length - 1));
    } else {
        mergedUrl = url;
    }

    var anchorPartURL = dfx.anchorPart(url);
    if (anchorPartURL.length > 0) {
        mergedUrl = mergedUrl + anchorPartURL;
    }

    return mergedUrl;

};

dfx.removeFromQueryString = function (url, idenifier)
{
    if (url == undefined) {
        url = '';
    }

    if (idenifier == undefined) {
        idenifier = '';
    }

    // Remove the index we are after.
    var trimmedUrl = url.replace(new RegExp('&*' + idenifier + '=[^&\\s\#]*', 'g'), '');

    // Remove any ? then nothing.
    trimmedUrl = trimmedUrl.replace(/^[?&]+|[?&]+$/g, '');

    // Replace any leftover ?& with ? .
    trimmedUrl = trimmedUrl.replace(/\?&/g, '?');

    // Replace any leftover ?# with # .
    trimmedUrl = trimmedUrl.replace(/\?\#/g, '\#');

    return trimmedUrl;

};

/**
 * Adds given url path eg. about/dept to the given URL.
 * Handles urls with query strings and/or anchors fine.
 */
dfx.addToPath = function(url, addPath)
{
    addPath = dfx.trim(addPath, '/');
    if (addPath.length > 0) {
        var mergedUrl        = '';
        var baseUrl          = dfx.baseUrl(url);
        var queryStringArray = dfx.queryString(url);
        var anchorPartURL    = dfx.anchorPart(url);

        baseUrl = dfx.rtrim(baseUrl, '/');

        mergedUrl = baseUrl + '/' + addPath;

        if (!dfx.isEmpty(queryStringArray)) {
            mergedUrl += '?';
            dfx.foreach(queryStringArray, function(key) {
                mergedUrl += key + '=' + queryStringArray[key] + '&';
            });

            mergedUrl = mergedUrl.substr(0, (mergedUrl.length - 1));
        }

        if (anchorPartURL.length > 0) {
            mergedUrl += anchorPartURL;
        }
    } else {
        var mergedUrl = url;
    }//end if

    return mergedUrl;

};

/**
 * Return the filename without the path.
 */
dfx.getFileInputName = function(fileFieldValue)
{
    var filename = '';
    if (fileFieldValue.indexOf('\\') > -1) {
      filename = fileFieldValue.substring(fileFieldValue.lastIndexOf('\\') + 1, fileFieldValue.length);
    }

    if (fileFieldValue.indexOf('/') > -1) {
      filename = fileFieldValue.substring(fileFieldValue.lastIndexOf('/') + 1, fileFieldValue.length);
    }

    if (filename === '') {
        return fileFieldValue;
    }

    return filename;

};


/**
 * Return a unique id.
 */
dfx.getUniqueId = function()
{
    var id  = Math.ceil((1 + Math.random()) * 100000).toString();
    id     += Math.ceil((1 + Math.random()) * 100000).toString();
    return id;

};

/**
 * Merges two objects together
 */
dfx.objectMerge = function (ob1, ob2)
{
    dfx.foreach(ob2, function(key) {
        ob1[key] = ob2[key];
        return true;
    });

    return ob1;

};

/**
 * Converts the multiple spaces, tabs, new lines to hard-spaces (&nbsp;)
 * NOT COMPLETE
 */
dfx.convertSpaces = function(elem, options)
{
    options = options || {};
    if (dfx.isset(options.newLines) === false) {
        options.newLines = true;
    }

    if (dfx.isset(options.tabs) === false) {
        options.tabs = true;
    }

    // Traverse the elem and replace the spaces.
    var count   = elem.childNodes.length;
    var c       = String.fromCharCode(160);
    var content = null;
    for (var i = 0; i < count; i++) {
        var child = elem.childNodes[i];
        content   = null;
        if (child.nodeType === dfx.TEXT_NODE) {
            content = child.data;
        }

        if (content !== null) {
            // Change \r\n to \n.
            var rep = '';
            content = content.replace(/\r/g, rep);

            // Convert all new lines to <br /> tags.
            if (options.newLines === false) {
                rep = '';
            } else {
                rep = '<br />';
            }

            content = content.replace(/\n/g, rep);

            // Convert tabs to 4 spaces.
            if (options.tabs === false) {
                rep = '';
            } else {
                rep = c + c + c + c;
            }

            content = content.replace(/\t/g, rep);

            if (child.data) {
                // Update child content.
                child.data = content;
            }
        }//end if

        if (child.childNodes && child.childNodes.length > 0) {
            dfx.convertSpaces(child, options);
        }
    }//end for

};

dfx.stripTags = function(content, allowedTags)
{
    var match;
    var re      = new RegExp(/<\/?(\w+)((\s+\w+(\s*=\s*(?:".*?"|'.*?'|[^'">\s]+))?)+\s*|\s*)\/?>/gim);
    var resCont = content;
    while ((match = re.exec(content)) != null) {
        if (dfx.isset(allowedTags) === false || dfx.inArray(match[1], allowedTags) !== true) {
            resCont = resCont.replace(match[0], '');
        }
    }

    return resCont;

};

dfx.getImage = function(url, callback)
{
    var img    = new Image();
    img.onload = function() {
        callback.call(this, img);
    };

    img.onerror = function() {
        callback.call(this, false);
    };

    img.src = url;

};


dfx.resizeImage = function(img, size, sizesOnly)
{
    var h = dfx.attr(img, 'height');
    var w = dfx.attr(img, 'width');

    var max = null;
    if ((size instanceof Object) === true) {
        // Rectangle, i.e. max h and max w constraint can be different.
        max = dfx.clone(size);
    } else {
        // Square.
        max = {
            height: size,
            width: size
        };
    }

    if (h === w) {
        // Square, use the smaller one for both.
        var min = Math.min(max.width, max.height);
        h       = min;
        w       = min;
    } else {
        if (w >= max.width || h >= max.height) {
            // Shrink.
            if (w >= max.width) {
                h = (h * (max.width / w));
                w = max.width;
            }

            if (h >= max.height) {
                // Height is still over max, resize again.
                w = (w * (max.height / h));
                h = max.height;
            }
        } else {
            // Enlarge.
            if (w > h) {
                h = (h * (max.width / w));
                w = max.width;
            } else if (h > w) {
                w = (w * (max.height / h));
                h = max.height;
            }
        }//end if
    }//end if

    h = Math.round(h);
    w = Math.round(w);

    if (sizesOnly === true) {
        var result = {
            height: h,
            width: w
        };
        return result;
    } else {
        dfx.attr(img, 'height', h);
        dfx.attr(img, 'width', w);
        return img;
    }

};

dfx.strRepeat = function(str, multiplier)
{
    var rstr = '';
    for (var i = 0; i < multiplier; i++) {
        rstr += str;
    }

    return rstr;

};

dfx.browser = function()
{
    var result     = {};
    result.version = dfxjQuery.browser.version;
    if (dfxjQuery.browser.mozilla === true) {
        result.type = 'mozilla';
    } else if (dfxjQuery.browser.msie === true) {
        result.type = 'msie';
    } else if (dfxjQuery.browser.opera === true) {
        result.type = 'opera';
    } else if (dfxjQuery.browser.safari === true) {
        result.type = 'safari';
    } else if (dfxjQuery.browser.chrome === true) {
        result.type = 'chrome';
    }

    return result;

};

dfx.getElemPositionStyles = function(elem, orientation)
{
    var h       = dfx.getElementHeight(elem);
    var w       = dfx.getElementWidth(elem);
    var res     = {};
    orientation = orientation || Widget.CENTER;

    switch (orientation) {
        case Widget.CENTER:
            res = {
                'margin-top': ((h / 2) * (-1)) + 'px',
                'top': '50%',
                'margin-left': ((w / 2) * (-1)) + 'px',
                'left': '50%'
            };
        break;

        case Widget.TOP:
            res = {
                'margin-top': (h * (-1)) + 'px',
                'top': '0px',
                'margin-left': ((w / 2) * (-1)) + 'px',
                'left': '50%'
            };
        break;

        case Widget.BOTTOM:
            res = {
                'margin-top': (h * (-1)) + 'px',
                'top': '100%',
                'margin-left': ((w / 2) * (-1)) + 'px',
                'left': '50%'
            };
        break;

        case Widget.LEFT:
            res = {
                'margin-top': ((h / 2) * (-1)) + 'px',
                'top': '50%',
                'left': '0px'
            };
        break;

        case Widget.RIGHT:
            res = {
                'margin-top': ((h / 2) * (-1)) + 'px',
                'top': '50%',
                'margin-left': (w * (-1)) + 'px',
                'left': '100%'
            };
        break;

        default:
            // Do nothing.
        break;
    }//end switch

    return res;

};

dfx.htmlspecialchars = function(str, noQuotes)
{
    str = str.replace(/&/g, '&amp;'); // First &amp.
    if (noQuotes !== true) {
        str = str.replace(/"/g, '&quot;');
        str = str.replace(/'/g, '&#039;');
    }

    str = str.replace(/</g, '&lt;');
    str = str.replace(/>/g, '&gt;');
    return str;

};

dfx.htmlspecialcharsDecode = function(str)
{
    str = str.replace(/&gt;/g, '>');
    str = str.replace(/&lt;/g, '<');
    str = str.replace(/&#039;/g, '\'');
    str = str.replace(/&quot;/g, '"');
    str = str.replace(/&amp;/g, '&'); // Last &amp;

    return str;

};

dfx.readableSize = function(size, unit)
{
    var units = ['B',
                 'kB',
                 'MB',
                 'GB'];

    var maxUnit = (units.length - 1);

    // Accept units as a parameter, maybe...
    if (unit) {
        var index = units.find(unit);
        if (index < 0) {
            unit = null;
        }
    }

    if (unit < 0) {
        unit = 2;
    }

    var factor = 0;
    while (unit !== factor && size >= 1000 && factor < maxUnit) {
        size = (size / 1000);
        factor++;
    }

    var readable = size.toFixed(2) + units[factor];
    return readable;

};

// TODO: Once CMS conversion finishes, remove ThumbView related functions from util.js.
dfx.displayThumbViewer = function(thumb, evt, targetWidget)
{
    var intervalid  = null;
    var thumbViewer = dfx.getId('SplashScreenThumbViewer');
    if (thumbViewer === null) {
        thumbViewer = dfx.createThumbViewer(thumb, evt);
        dfx.hideElement(thumbViewer);
        document.body.appendChild(thumbViewer);
    }

    var oriImage = dfx.getMouseEventTarget(evt);
    dfx.getId('SplashScreenThumbViewer-img').setAttribute('src', oriImage.getAttribute('src'));
    dfx.setThumbViewerText(thumb, evt, function() {
        /*
            TODO: Once IE 8 releases stable version, thumbviewer without
            this code. For some reason, I have to recreate event mask div
            everytime to make it work with IE at the moment.
            21/Nov/2008.
        */
        /*dfx.remove(dfx.getId('SplashScreenThumbViewer-eventMask'));
        var eventMask       = document.createElement('div');
        eventMask.id        = 'SplashScreenThumbViewer-eventMask';
        eventMask.className = 'SplashScreenThumbViewerEventMask';
        eventMask.innerHTML = ' &nbsp;';

        dfx.getId('SplashScreenThumbViewer').appendChild(eventMask);*/

        var thumbWidth   = 54;
        var thumbHeight  = 79;
        var viewerWidth  = 236;
        var viewerHeight = 193;

        var target = dfx.getMouseEventTarget(evt);
        var coords = dfx.getElementCoords(target);
        var left   = (coords.x - (viewerWidth - thumbHeight));
        var top    = (coords.y - (viewerHeight - thumbWidth));

        var scrollY = dfx.getScrollCoords().y;
        if (scrollY > 0) {
            top -= scrollY;
        }

        dfx.setStyle(thumbViewer, 'left', left);
        dfx.setStyle(thumbViewer, 'top', top);

        dfx.showElement(thumbViewer);
        targetWidget.thumbDisplayed = true;

        var setMousePos = function(e) {
            var scrY  = dfx.getScrollCoords().y;
            var pageX = e.pageX;
            var pageY = (e.pageY - scrY);

            if ((pageX < left || pageX > (left + viewerWidth)) || (pageY < top || pageY > (top + viewerHeight))) {
                dfx.hideElement(dfx.getId('SplashScreenThumbViewer'));
                clearInterval(intervalid);
                dfx.stopMousePositionTrack(setMousePos);
            }
        };

        dfx.startMousePositionTrack(setMousePos);
    });

};


dfx.createThumbViewer = function(thumb, evt)
{
    var thumbWrapper       = document.createElement('div');
    thumbWrapper.id        = 'SplashScreenThumbViewer';
    thumbWrapper.className = 'SplashScreenThumbViewerWrapper';

    var imageHolder       = document.createElement('div');
    imageHolder.className = 'SplashScreenThumbViewerImageHolder';

    var description       = document.createElement('div');
    description.id        = 'SplashScreenThumbViewer-desc';
    description.className = 'SplashScreenThumbViewerDescription';

    var oriImage = dfx.getMouseEventTarget(evt);
    var image    = document.createElement('img');
    image.id     = 'SplashScreenThumbViewer-img';
    image.src    = oriImage.getAttribute('src');

    imageHolder.appendChild(image);
    thumbWrapper.appendChild(imageHolder);
    thumbWrapper.appendChild(description);
    return thumbWrapper;

};

dfx.setThumbViewerText = function(thumb, evt, callback)
{
    var oriImage = dfx.getMouseEventTarget(evt);
    var assetid  = oriImage.getAttribute('assetid');
    var version  = oriImage.getAttribute('version');

    if (version === '0') {
        AssetManager.getAsset(assetid, function(asset) {
            dfx.getId('SplashScreenThumbViewer-desc').innerHTML = asset.name;
            callback();
        }, true, {attributes: ['name'],
            type: false,
            typeIcon: false,
            linking: [],
            urls: false}
        );
    } else {
        dfx.getId('SplashScreenThumbViewer-desc').innerHTML = 'Version ' + version;
        callback();
    }

};

dfx.preloadStylesheetImages = function(prefix, defaultBaseUrl)
{
    prefix         = prefix || [];
    defaultBaseUrl = defaultBaseUrl || '/';

    var styleSheets = document.styleSheets;
    var sln         = styleSheets.length;

    for (var i = 0; i < sln; i++) {
        var baseUrl  = '';
        var contents = '';
        if (styleSheets[i].href) {
            // Get the base URL.
            baseUrl = styleSheets[i].href.substring(0, styleSheets[i].href.lastIndexOf('/'));
        }

        if (baseUrl !== '') {
            baseUrl += '/';
        } else {
            baseUrl = defaultBaseUrl;
        }

        if (styleSheets[i].cssRules) {
             var sheetRules = styleSheets[i].cssRules;
             var rln        = sheetRules.length;
             for (var j = 0; j < rln; j++) {
                 contents += sheetRules[j].cssText;
             }
        } else {
             contents += styleSheets[i].cssText;
        }

        var re      = '(' + prefix.join('|') + ')[^\(]+\.(gif|jpg|png)';
        var regExp  = new RegExp(re, 'g');
        var imgUrls = contents.match(regExp);
        if (imgUrls !== null && imgUrls.length > 0) {
            dfx.foreach(imgUrls, function(key) {
                var img = new Image();
                img.src = baseUrl + imgUrls[key];
            });
        }
    }//end for

};

dfx.getFileExtension = function(filename)
{
    var parts = filename.split('.');
    if (parts.length === 1) {
        return '';
    }

    var ext = parts[(parts.length - 1)].toLowerCase();
    return ext;

};

dfx.commonEntitiesArray = {
    160: '&nbsp;',     // space
    168: '&uml;',      //  ¨
    169: '&copy;',     //  ©
    170: '&ordf;',     //  ª
    171: '&laquo;',    //  «
    172: '&not;',      //  ¬
    173: '&shy;',      //  ­
    174: '&reg;',      //  ®
    175: '&macr;',     //  ¯
    176: '&deg;',      //  °
    177: '&plusmn;',   //  ±
    178: '&sup2;',     //  ²
    179: '&sup3;',     //  ³
    180: '&acute;',    //  ´
    181: '&micro;',    //  µ
    182: '&para;',     //  ¶
    183: '&middot;',   //  ·
    184: '&cedil;',    //  ¸
    185: '&sup1;',     //  ¹
    186: '&ordm;',     //  º
    187: '&raquo;',    //  »
    188: '&frac14;',   //  ¼
    189: '&frac12;',   //  ½
    190: '&frac34;',   //  ¾
    191: '&iquest;',   //  ¿
    215: '&times;',    //  ×
    247: '&divide;',   //  ÷
    977: '&thetasym;', //  ϑ
    978: '&upsih;',    //  ϒ
    982: '&piv;',      //  ϖ
    8226: '&bull;',    //  *
    8230: '&hellip;',  //  …
    8242: '&prime;',   //  ′
    8243: '&Prime;',   //  ″
    8254: '&oline;',   //  ‾
    8260: '&frasl;',   //  ⁄
    8472: '&weierp;',  //  ℘
    8465: '&image;',   //  ℑ
    8476: '&real;',    //  ℜ
    8482: '&trade;',   //  ™
    8501: '&alefsym;', //  ℵ
    8592: '&larr;',    //  ←
    8593: '&uarr;',    //  ↑
    8594: '&rarr;',    //  →
    8595: '&darr;',    //  ↓
    8596: '&harr;',    //  ↔
    8629: '&crarr;',   //  ↵
    8656: '&lArr;',    //  ⇐
    8657: '&uArr;',    //  ⇑
    8658: '&rArr;',    //  ⇒
    8659: '&dArr;',    //  ⇓
    8660: '&hArr;',    //  ⇔
    8704: '&forall;',  //  ∀
    8706: '&part;',    //  ∂
    8707: '&exist;',   //  ∃
    8709: '&empty;',   //  ∅
    8711: '&nabla;',   //  ∇
    8712: '&isin;',    //  ∈
    8713: '&notin;',   //  ∉
    8715: '&ni;',      //  ∋
    8719: '&prod;',    //  ∏
    8721: '&sum;',     //  ∑
    8722: '&minus;',   //  −
    8727: '&lowast;',  //  ∗
    8730: '&radic;',   //  √
    8733: '&prop;',    //  ∝
    8734: '&infin;',   //  ∞
    8736: '&ang;',     //  ∠
    8743: '&and;',     //  ∧
    8744: '&or;',      //  ∨
    8745: '&cap;',     //  ∩
    8746: '&cup;',     //  ∪
    8747: '&int;',     //  ∫
    8756: '&there4;',  //  ∴
    8764: '&sim;',     //  ∼
    8773: '&cong;',    //  ≅
    8776: '&asymp;',   //  ≈
    8800: '&ne;',      //  ≠
    8801: '&equiv;',   //  ≡
    8804: '&le;',      //  ≤
    8805: '&ge;',      //  ≥
    8834: '&sub;',     //  ⊂
    8835: '&sup;',     //  ⊃
    8836: '&nsub;',    //  ⊄
    8838: '&sube;',    //  ⊆
    8839: '&supe;',    //  ⊇
    8853: '&oplus;',   //  ⊕
    8855: '&otimes;',  //  ⊗
    8869: '&perp;',    //  ⊥
    8901: '&sdot;',    //  ⋅
    8968: '&lceil;',   //  ⌈
    8969: '&rceil;',   //  ⌉
    8970: '&lfloor;',  //  ⌊
    8971: '&rfloor;',  //  ⌋
    9001: '&lang;',    //  ⟨
    9002: '&rang;',    //  ⟩
    9674: '&loz;',     //  ◊
    9824: '&spades;',  //  ♠
    9827: '&clubs;',   //  ♣
    9829: '&hearts;',  //  ♥
    9830: '&diams;'    //  ♦
};

dfx.alphabetEntitiesArray = {
    161: '&iexcl;',    //  ¡
    162: '&cent;',     //  ¢
    163: '&pound;',    //  £
    164: '&curren;',   //  ¤
    165: '&yen;',      //  ¥
    166: '&brvbar;',   //  ¦
    167: '&sect;',     //  §
    192: '&Agrave;',   //  À
    193: '&Aacute;',   //  Á
    194: '&Acirc;',    //  Â
    195: '&Atilde;',   //  Ã
    196: '&Auml;',     //  Ä
    197: '&Aring;',    //  Å
    198: '&AElig;',    //  Æ
    199: '&Ccedil;',   //  Ç
    200: '&Egrave;',   //  È
    201: '&Eacute;',   //  É
    202: '&Ecirc;',    //  Ê
    203: '&Euml;',     //  Ë
    204: '&Igrave;',   //  Ì
    205: '&Iacute;',   //  Í
    206: '&Icirc;',    //  Î
    207: '&Iuml;',     //  Ï
    208: '&ETH;',      //  Ð
    209: '&Ntilde;',   //  Ñ
    210: '&Ograve;',   //  Ò
    211: '&Oacute;',   //  Ó
    212: '&Ocirc;',    //  Ô
    213: '&Otilde;',   //  Õ
    214: '&Ouml;',     //  Ö
    216: '&Oslash;',   //  Ø
    217: '&Ugrave;',   //  Ù
    218: '&Uacute;',   //  Ú
    219: '&Ucirc;',    //  Û
    220: '&Uuml;',     //  Ü
    221: '&Yacute;',   //  Ý
    222: '&THORN;',    //  Þ
    223: '&szlig;',    //  ß
    224: '&agrave;',   //  à
    225: '&aacute;',   //  á
    226: '&acirc;',    //  â
    227: '&atilde;',   //  ã
    228: '&auml;',     //  ä
    229: '&aring;',    //  å
    230: '&aelig;',    //  æ
    231: '&ccedil;',   //  ç
    232: '&egrave;',   //  è
    233: '&eacute;',   //  é
    234: '&ecirc;',    //  ê
    235: '&euml;',     //  ë
    236: '&igrave;',   //  ì
    237: '&iacute;',   //  í
    238: '&icirc;',    //  î
    239: '&iuml;',     //  ï
    240: '&eth;',      //  ð
    241: '&ntilde;',   //  ñ
    242: '&ograve;',   //  ò
    243: '&oacute;',   //  ó
    244: '&ocirc;',    //  ô
    245: '&otilde;',   //  õ
    246: '&ouml;',     //  ö
    248: '&oslash;',   //  ø
    249: '&ugrave;',   //  ù
    250: '&uacute;',   //  ú
    251: '&ucirc;',    //  û
    252: '&uuml;',     //  ü
    253: '&yacute;',   //  ý
    254: '&thorn;',    //  þ
    255: '&yuml;',     //  ÿ
    402: '&fnof;',     //  ƒ
    913: '&Alpha;',    //  Α
    914: '&Beta;',     //  Β
    915: '&Gamma;',    //  Γ
    916: '&Delta;',    //  Δ
    917: '&Epsilon;',  //  Ε
    918: '&Zeta;',     //  Ζ
    919: '&Eta;',      //  Η
    920: '&Theta;',    //  Θ
    921: '&Iota;',     //  Ι
    922: '&Kappa;',    //  Κ
    923: '&Lambda;',   //  Λ
    924: '&Mu;',       //  Μ
    925: '&Nu;',       //  Ν
    926: '&Xi;',       //  Ξ
    927: '&Omicron;',  //  Ο
    928: '&Pi;',       //  Π
    929: '&Rho;',      //  Ρ
    931: '&Sigma;',    //  Σ
    932: '&Tau;',      //  Τ
    933: '&Upsilon;',  //  Υ
    934: '&Phi;',      //  Φ
    935: '&Chi;',      //  Χ
    936: '&Psi;',      //  Ψ
    937: '&Omega;',    //  Ω
    945: '&alpha;',    //  α
    946: '&beta;',     //  β
    947: '&gamma;',    //  γ
    948: '&delta;',    //  δ
    949: '&epsilon;',  //  ε
    950: '&zeta;',     //  ζ
    951: '&eta;',      //  η
    952: '&theta;',    //  θ
    953: '&iota;',     //  ι
    954: '&kappa;',    //  κ
    955: '&lambda;',   //  λ
    956: '&mu;',       //  μ
    957: '&nu;',       //  ν
    958: '&xi;',       //  ξ
    959: '&omicron;',  //  ο
    960: '&pi;',       //  π
    961: '&rho;',      //  ρ
    962: '&sigmaf;',   //  ς
    963: '&sigma;',    //  σ
    964: '&tau;',      //  τ
    965: '&upsilon;',  //  υ
    966: '&phi;',      //  φ
    967: '&chi;',      //  χ
    968: '&psi;',      //  ψ
    969: '&omega;'    //  ω
};

dfx.replaceNamedEntities = function(html)
{
    var newHtml = '';
    var ln      = html.length;
    for (i = 0; i < ln; i++) {
        code = html.charCodeAt(i);
        if (code > 127) {
            var entity = dfx.commonEntitiesArray[code];
            if (!entity) {
                entity = dfx.alphabetEntitiesArray[code];
            }

            if (entity) {
                newHtml += entity;
            } else {
                newHtml += html.charAt(i);
            }
        } else {
            newHtml += html.charAt(i);
        }
    }

    return newHtml;

};

/**
 * Does not replace currency symbols, and alphabet symbols.
 */
dfx.replaceCommonNamedEntities = function(html)
{
    var newHtml = '';
    var ln      = html.length;
    for (i = 0; i < ln; i++) {
        code = html.charCodeAt(i);
        if (code > 127) {
            var entity = dfx.commonEntitiesArray[code];
            if (entity) {
                newHtml += entity;
            } else {
                newHtml += html.charAt(i);
            }
        } else {
            newHtml += html.charAt(i);
        }
    }

    return newHtml;

};


/**
 * Validates a URL.
 *
 * Currently only validates not including query string or hash tag.
 *
 * @param {string}  url
 * @param {boolean} requireScheme
 * @param {object}  allowedSchemes
 * @param {boolean} allowLeadingPathUnderscores If leading path underscores should be allowed.
 * @param {boolean} allowUpperCasePath          If uppercase characters should allowed in the path.
 *
 * @return boolean
 */
dfx.validateUrl = function(url, requireScheme, allowedSchemes, allowLeadingPathUnderscores, allowUpperCasePath)
{
    // Initialise option to be true if not passed.
    if (allowLeadingPathUnderscores !== false) {
        allowLeadingPathUnderscores = true;
    }

    // Initialise option to be true if not passed.
    if (allowUpperCasePath !== false) {
        allowUpperCasePath = true;
    }

    if (requireScheme === true) {
        if (dfx.isEmpty(allowedSchemes) === true) {
            // If a scheme is required but no valid schemes are given
            // we must always say the URL is invalid.
            return false;
        } else {
            var schemeMatch = '(' + dfx.arrayImplode('|', allowedSchemes) + '):\/\/';
        }
    } else {
        if (dfx.isEmpty(allowedSchemes) === true) {
            var schemeMatch = '';
        } else {
            var schemeMatch = '((' + dfx.arrayImplode('|', allowedSchemes) + '):\/\/)?';
        }
    }//end if

    // Scheme which may be optional.
    var regExStr = '^' + schemeMatch;

    // Domain starts with one or more letters or numbers first.
    var domainMatch = '[a-z0-9]+';

    // Zero or more occurances of 1 dash or 1 dot with one or more letters/numbers again.
    // There may be 2 or more letters to follow after a dot.
    domainMatch += '(([\\-\\.]{1}[a-z0-9]+)*\\.[a-z]{2,})?';

    // IP match is three sets of 1-3 numbers with dots following them followed by 1
    // more set of 1-3 numbers.
    var ipMatch = '(?:\\d{1,3}\\.){3}\\d{1,3}';

    // Domain or IP match is ok.
    regExStr += '(' + domainMatch + '|' + ipMatch + ')';

    // Optional port match which is a colon followed by 1-5 numbers.
    regExStr += '(:[0-9]{1,5})?';

    // Up till end of string or start of query string ? or start of path /.  The rest
    // will be checked later.
    regExStr += '(\\?.*|\/.*|#.*)*$';

    // Case insensitive matching.
    var regExp = new RegExp(regExStr, 'i');

    // Finally run the regular expression.
    var matches = url.match(regExp);
    if (matches === null) {
        return false;
    }

    // Validate the path part of a URL.
    var urlValid = true;
    var urlPath  = dfx.getUrlPath(url);

    if (urlPath !== '') {
        // Start of string should not start with a slash because that indicates the
        // domain ended with a slash and path starts with a different slash.  Also,
        // just a plain double slash is not allowed.
        var doubleSlashExp = '^\/|\/\/';
        var regExp = new RegExp(doubleSlashExp, 'i');
        var doubleSlashIdx = urlPath.search(regExp);
        if (doubleSlashIdx !== -1) {
            // A double slash has been found in the path.
            return false;
        }

        // Now check each individual piece of the path.
        var individualPaths = urlPath.split('/');
        dfx.foreach(individualPaths, function(idx) {
            var pathValid = dfx.validateSingleUrlPath(individualPaths[idx], allowLeadingPathUnderscores, allowUpperCasePath);
            if (pathValid === false) {
                urlValid = false;
                return false;
            }
        });
    }//end if

    return urlValid;

};

/**
 * Validates a single URL path component.
 *
 * eg. If the entire path was /home/about-us only home or about-us should be
 * provided.
 *
 * @param string  path                    The path to test.
 * @param boolean allowLeadingUnderscores FALSE if you don't want to allow leading
 *                                        underscores in a path. Default is allow.
 * @param boolean allowUpperCasePath      FALSE if you don't want to allow uppercase
 *                                        characters in a path. Default is allow.
 *
 * @return boolean
 */
dfx.validateSingleUrlPath = function(singlePath, allowLeadingUnderscores, allowUpperCasePath)
{
    // Initialise option to be true if not passed.
    if (allowLeadingUnderscores !== false) {
        allowLeadingUnderscores = true;
    }

    // Initialise option to be true if not passed.
    if (allowUpperCasePath !== false) {
        allowUpperCasePath = true;
    }

    if (allowUpperCasePath === false && singlePath.toLowerCase() !== singlePath) {
        // Upper case in the path isn't allowed and we found one.
        return false;
    }

    var regExStr = '';
    if (allowLeadingUnderscores === false) {
        regExStr += '^_|';
    }

    regExStr += '[^a-z0-9\-$_@.!*~(),]';

    var regExp = new RegExp(regExStr, 'i');

    // Finally run the regular expression.
    var matches = singlePath.match(regExp);
    if (matches === null) {
        var urlValid = true;
    } else {
        var urlValid = false;
    }

    return urlValid;

};


/**
 * Validates an email.
 *
 * Chose not to use a domain white list given .anything is on the way.  A feature
 * this regex currently does not support is <Name Part> of an email so add if
 * needed.  This expression is based on Arluison Guillaume http://www.mi-ange.net/
 * email regex.
 *
 * @return boolean
 */
dfx.validateEmail = function(email)
{
    var regExStr = '^[-a-z0-9~!$%^&*_=+}{\'?]+(\.[-a-z0-9~!$%^&*_=+}{\'?]+)*@([a-z0-9_][-a-z0-9_]*(\.[-a-z0-9_]+)*\.([a-z][a-z]+)|([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}))(:[0-9]{1,5})?$';

    var regExp = new RegExp(regExStr, 'i');

    // Finally run the regular expression.
    var matches = email.match(regExp);
    if (matches === null) {
        var emailValid = false;
    } else {
        var emailValid = true;
    }

    return emailValid;

};


/**
 * Get selected html.
 *
 * @param DomNode parent The element to restrict selection under.
 *
 * @return string
 */
dfx.getSelectedHtml = function(parent)
{
    var html = '';
    if (document.selection && document.selection.createRange) {
        var range = document.selection.createRange();
        if (range.parentElement) {
            if (range.parentElement() === parent
                || dfx.isChildOf(range.parentElement(), parent, document.body) === true
            ) {
                html = range.htmlText;
            }
        }
    } else if (window.getSelection) {
        var selection = window.getSelection();
        if (selection.rangeCount > 0) {
            var range = selection.getRangeAt(0);
            if (range.commonAncestorContainer === parent
                || dfx.isChildOf(range.commonAncestorContainer, parent, document.body) === true
            ) {
                var clonedSelection = range.cloneContents();
                var div             = document.createElement('div');
                div.appendChild(clonedSelection);
                html = dfx.getHtml(div);
            }
        }
    }

    return html;
};


/**
 * Very basic implementation of sprintf.
 *
 * Currently only supports replacement of %s in the specified string.
 * Usage: dfx.sprintf('Very %s implementation of %s.', 'basic', 'sprintf');
 */
dfx.sprintf = function(str)
{
    var c = arguments.length;
    if (c <= 1) {
        return str;
    }

    for (var i = 1; i < c; i++) {
        str = str.replace(/%s/, arguments[i]);
    }

    return str;

};


/* @codingStandardsIgnoreStart */
if (!window.console) {
    window.console = {};
    window.console.log = function() {};
    window.console.info = function() {};
}
/* @codingStandardsIgnoreEnd */
