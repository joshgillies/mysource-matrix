if (!window.dfx) {
    window.dfx = function() {};
    window.dfxjQuery = $.noConflict(true);
}

/**
 * General functions that must be included on every execution.
 */


/**
 * Implements inheritance for two classes.
 *
 * @param {funcPtr} child  The class that is inheriting the parent methods.
 * @param {funcPtr} parent The parent that is being implemented.
 *
 * @return void
 * @type   void
 */
dfx.inherits = function(child, parent)
{
    dfx.noInclusionInherits(child, parent);

};//end inherits()

/**
 * Implements inheritance for two classes.
 *
 * The main difference with inherits() function is that
 * this does not include parent widget type before inheritance operation.
 *
 * @param {funcPtr} child  The class that is inheriting the parent methods.
 * @param {funcPtr} parent The parent that is being implemented.
 *
 * @return void
 * @type   void
 */
dfx.inherited           = {};
dfx.noInclusionInherits = function(child, parent)
{
    if (dfx.inherited[child + parent]) {
        return;
    }

    dfx.inherited[child + parent] = true;

    if (parent instanceof String || typeof parent === 'string') {
        parent = window[parent];

    }

    if (child instanceof String || typeof child === 'string') {
        child = window[child];
    }

    var above = function(){};
    if (dfx.isset(parent) === true) {
        for (value in parent.prototype) {
            // If the child method already exists, move this method to the parent
            // so it can be called using super.method().
            if (child.prototype[value]) {
                above.prototype[value] = parent.prototype[value];
                continue;
            }

            child.prototype[value] = parent.prototype[value];
        }
    }

    if (child.prototype) {
        above.prototype.constructor = parent;
        child.prototype['super']    = new above();
    }

};//end noInclusionInherits()
