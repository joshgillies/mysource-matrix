if (!window.dfx) {
    window.dfx = function() {};
    window.dfxjQuery = $.noConflict(true);
}

/**
 * Moves given elements.
 *
 * Note: If +/- specified in front of the toX or toY then animation will be relative.
 */
dfx.move = function(elements, left, top, duration, callback, easing)
{
    var opts = {};
    if (left !== null) {
        opts.left = left;
    }

    if (top !== null) {
        opts.top = top;
    }

    dfxjQuery(elements).animate(opts, duration, easing, callback);

};

/**
 * Allows custom animations.
 */
dfx.animate = function(elements, params, duration, callback, easing)
{
    dfxjQuery(elements).animate(params, duration, easing, callback);

};

dfx.fadeIn = function(elements, speed, callback)
{
    dfxjQuery(elements).fadeIn(speed, callback);

};

dfx.fadeOut = function(elements, speed, callback)
{
    dfxjQuery(elements).fadeOut(speed, callback);

};

dfx.blindDown = function(elements, speed, callback)
{
    dfxjQuery(elements).slideDown(speed, callback);

};

dfx.blindUp = function(elements, speed, callback)
{
    dfxjQuery(elements).slideUp(speed, callback);

};

dfx.blindToggle = function(elements, speed, callback)
{
    dfxjQuery(elements).slideToggle(speed, callback);

};

dfx.bounce = function(element, times, height, callback)
{
    if (times > 0) {
        var coords = dfx.getElementCoords(element);
        dfx.move(element, null, (coords.y - height), 400, function() {
            dfx.move(element, null, coords.y, 400, function() {
                if (times > 0) {
                    dfx.bounce(element, (times - 1), height, callback);
                } else if (callback) {
                    callback.call(this);
                }
            });
        });
    } else if (callback) {
        callback.call(this);
    }

};

dfx.stop = function(elements)
{
    dfxjQuery(elements).stop();

};
