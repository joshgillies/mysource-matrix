if (!window.dfx) {
    window.dfx = function() {};
    window.dfxjQuery = $.noConflict(true);
}

dfx.get = function(url, data, callBack)
{
    url = dfx.cleanAjaxRequestUrl(url);
    dfxjQuery.get(url, data, callBack);

};


dfx.post = function(url, data, successCallback, errorCallback, timeout, extraParams)
{
    url = dfx.cleanAjaxRequestUrl(url);
    timeout = timeout || 20;
    var params = {
        url: url,
        type: 'POST',
        data: data,
        success: successCallback,
        error: errorCallback,
        timeout: (timeout * 1000)
    };

    if (extraParams) {
        dfx.foreach(extraParams, function(key) {
            params[key] = extraParams[key];
            return true;
        });
    }

    dfxjQuery.ajax(params);

};

/**
 * Retrieves JSON encoded data from the URL.
 *
 * Note: make sure you have parenthesis around your json string else
 * JS will throw "invalid label" error.
 */
dfx.getJSON = function(url, data, callBack)
{
    url = dfx.cleanAjaxRequestUrl(url);
    dfxjQuery.getJSON(url, data, callBack);

};

/**
 * Returns a cleaned URL to make an ajax request with.
 */
dfx.cleanAjaxRequestUrl = function(url)
{
    if (typeof url !== 'string') {
        // Do the jquery URL default to current location on undefined
        // URL earlier so we can catch the IE8 hash tag bug
        var url = location.href;
    }

    // No ajax requests should have an anchor part in the URL.
    // Also IE8 bug - Anchor becomes part of last query string value.
    url = dfx.noAnchorPartUrl(url);

    return url;
};
