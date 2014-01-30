if (!window.dfx) {
    window.dfx = function() {};
    window.dfxjQuery = $.noConflict(true);
}

/*
 * Addon element for array that allows checking for existence of an element.
 *
 * @param {mixed} value The value to search for in the array.
 *
 * @return TRUE if the array contains the passed value.
 * @type   boolean
 */
Array.prototype.inArray = function(value)
{
    if (Array.prototype.indexOf) {
        if (this.indexOf(value) >= 0) {
            return true;
        } else {
            return false;
        }
    }

    // Value is not a string, so the array must be iterated through and each
    // element checked against the search string.
    var len = this.length;
    for (var i = 0; i < len; i++) {
        if (this[i] === value) {
            return true;
        }
    }

    return false;

};//end Array.prototype.inArray()

/*
 * Searches the array for the given item.
 *
 * Returns the item index if found, else false.
 *
 * @param array array The array.
 * @param mixed item  Item to search in the specified array.
 *
 * @return int
 */
Array.prototype.find = function(item)
{
    /*
        TODO: This function could probably be got rid of and just use
        dfx.arraySearch instead.  Doing this would require updating all
        products.
    */
    var length = this.length;
    for (var i = 0; i < length; i++) {
        if (this[i] === item) {
            return i;
        }
    }

    return -1;

};//end Array.prototype.arraySearch()

/*
 * Merge an HTMLCollection into this array.
 *
 * Some objects have the properties of an array, but cannot be 'concat()'ed to
 * normal arrays. This fixes that functionality.
 *
 * @param HTMLCollection collection The collection to be merged. This is usually
 *                                  returned from getElementsByTagname().
 *
 * @return void
 * @type   void
 */
Array.prototype.mergeCollection = function(collection)
{
    if (!collection) {
        return;
    }

    var len = collection.length;
    for (var i = 0; i < len; i++) {
        this.push(collection[i]);
    }

};//end Array.prototype.mergeCollection()

Array.prototype.unique = function()
{
    var a = [];
    var l = this.length;
    for (var i = 0; i < l; i++) {
        if (a.find(this[i]) < 0) {
            a.push(this[i]);
        }
    }

    return a;

};

/**
 * Moves all array elements after specified index up by 1.
 *
 * @param {array} array Array to modify
 * @param {int}   index Index that will be removed.
 *
 * @return {array}
 */
function shiftArrayElements(array, index)
{
    // Shift all elements after index up by 1.
    var len = array.length;
    for (var i = parseInt(index); i < (len - 1); i++) {
        var n    = i + 1;
        array[i] = array[n];
    }

    array.pop();
    return array;

};//end shiftArrayElements()


/**
 * Loop through object or array.
 */
dfx.foreach = function(value, cb)
{
    if (value instanceof Array
        || (
            value !== null
            && typeof value === 'object'
            && typeof value.jquery === 'string'
        )
    ) {
        var len = value.length;
        for (var i = 0; i < len; i++) {
            var res = cb.call(this, i);
            if (res === false) {
                break;
            }
        }
    } else {
        for (var id in value) {
            if (value.hasOwnProperty(id) === true) {
                var res = cb.call(this, id);
                if (res === false) {
                    break;
                }
            }
        }
    }

};//end foreach()

dfx.isEmpty = function(value)
{
    if (value) {
        if (value instanceof Array) {
            if (value.length > 0) {
                return false;
            }
        } else {
            for (var id in value) {
                if (value.hasOwnProperty(id) === true) {
                    return false;
                }
            }
        }
    }

    return true;

};

dfx.isArray = function(v)
{
    return dfxjQuery.isArray(v);

};

 /**
 * Return TRUE if the value exists in an array..
 *
 * @param {String}  needle        The item you are looking for.
 * @param {array}   haystack      The array to look through.
 * @param {boolean} typeSensitive Type sensitive comparison - default is true.
 *
 * @return {String}
 */
dfx.inArray = function(needle, haystack, typeSensitive)
{
    if (dfx.isset(typeSensitive) === false) {
        typeSensitive = true;
    }

    var hln = haystack.length;
    for (var i = 0; i < hln; i++) {
        if ((typeSensitive === true && needle === haystack[i]) ||
            (typeSensitive === false && needle == haystack[i])
        ) {
            return true;
        }
    }

    return false;

};

/**
 * Computes the difference of two arrays.
 * If firstOnly is set to TRUE then only the elements that are in first array
 * but not in the second array will be returned.
 */
dfx.arrayDiff = function(array1, array2, firstOnly)
{
    var al  = array1.length;
    var res = [];
    for (var i = 0; i < al; i++) {
        if (dfx.inArray(array1[i], array2) === false) {
            res.push(array1[i]);
        }
    }

    if (firstOnly !== true) {
        al = array2.length;
        for (var i = 0; i < al; i++) {
            if (dfx.inArray(array2[i], array1) === false) {
                res.push(array2[i]);
            }
        }
    }

    return res;

};

/**
 * Returns the keys of an array or keys of properties of an object as an indexed array.
 */
dfx.arrayImplode  = function(glue, pieces)
{
    var ret          = '';
    var finalTrimReq = false;
    dfx.foreach(pieces, function(key) {
        ret += pieces[key] + glue;
        finalTrimReq = true;
    });

    if (finalTrimReq) {
        var trimLn = (ret.length - glue.length);
        ret        = ret.substr(0, trimLn);
    }

    return ret;

};

/**
 * Returns the keys of an array or keys of properties of an object as an indexed array.
 */
dfx.arrayKeys  = function(array)
{
    var ret = new Array();
    var i   = 0;
    dfx.foreach(array, function(key) {
        ret[i] = key;
        i++;
    });

    return ret;

};

/**
 * Merges an array of any type similiar to PHP.
 *
 * If array1 is a JS array the elements will simply be added to the end of array1.
 * If array1 is an object the arrays will be merged maintaining keys and if a key
 * for an element exists in array2 which is the same as array 1 the value in array2
 * will overwrite in array1.
 *
 * @param {array|object} array1 First array to merge into.
 * @param {array|object} array2 Second array to merge in.
 *
 * @return {array|object}
 */
dfx.arrayMerge = function (array1, array2)
{
    // We won't maintain the index if array1 is a JS array because if it tries to
    // merge with a string index it would fail.
    if (array1 instanceof Array) {
        var maintainIndex = false;
    } else {
        var maintainIndex = true;
    }

    // Do the merging.
    dfx.foreach(array2, function(idx) {
        var value = array2[idx];
        if (maintainIndex === true) {
            array1[idx] = value;
        } else {
            array1.push(value);
        }
    });

    return array1;

};

dfx.removeArrayIndex = function(array, index)
{
    if (!array || dfx.isset(array[index]) === false) {
        return null;
    }

    return array.splice(index, 1);

};

/**
 * Searches object or normal array for a value. Returns false if not found otherwise
 * returns the index it was found at.
 */
dfx.arraySearch = function(needle, haystack)
{
    var foundAtIndex = false;
    if (needle instanceof String) {
        needle = needle.toString();
    }

    if ((typeof needle === 'string' && typeof needle === 'boolean') || (haystack instanceof Array === false && haystack instanceof Object === false)) {
        return foundAtIndex;
    }

    dfx.foreach(haystack, function(i) {
        var value = haystack[i];
        if (value === needle) {
            foundAtIndex = i;
            return false;
        }
    });

    return foundAtIndex;

};

/**
 * Creates an array for you filled with what you pass.  By default it will be an
 * object array unless you request it and your starting index is zero.
 */
dfx.arrayFill = function(startIndex, num, value, nonObjectArray)
{
    if (nonObjectArray === true && startIndex === 0) {
        var retAr = [];
    } else {
        var retAr = {};
    }

    for (var i = startIndex; i < num; i++) {
        retAr[i] = value;
    }

    return retAr;

};

dfx.arrayIntersect = function(array1, array2)
{
    var tmp    = {};
    var unique = [];
    var count  = array2.length;

    for (var i = 0; i < count; i++) {
        tmp[array2[i]] = array2[i];
    }

    count = array1.length;
    for (var i = count; i >= 0; i--) {
        if (dfx.isset(tmp[array1[i]]) === false) {
            dfx.unset(array1, i);
        }
    }

    return array1;

};

/**
 * Returns the number of elements in either a JS array or object array.
 *
 * @param {array|object} anyArray Any type of array to count the elements of.
 *
 * @return int
 */
dfx.count = function(anyArray)
{
    if (anyArray instanceof Array) {
        return anyArray.length;
    } else {
        var counter = 0;
        dfx.foreach(anyArray, function(key) {
            counter++;
        });

        return counter;
    }

};

/**
 * Equivalent of PHP unset($array[$idx]) for arrays.
 *
 * Using the avoids getting undefined value left in normal JS array when you use
 * delete $array[$idx].  You also don't need to worry about what type of array you
 * were given.
 *
 * @param {array|object} anyArray Any type of array to count the elements of.
 * @param {int|string}   index    The index of the item to delete.
 *
 * @return void
 */
dfx.unset = function(anyArray, index)
{
    if (anyArray instanceof Array) {
        anyArray.splice(index, 1);
    } else {
        delete anyArray[index];
    }

};
