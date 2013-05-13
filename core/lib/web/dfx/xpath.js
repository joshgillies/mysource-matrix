if (!window.dfx) {
    window.dfx = function() {};
    window.dfxjQuery = $.noConflict(true);
}

/**
 * Returns a unique path for the given node.
 *
 * @param DOMNode node The node to retrieve the path for.
 *
 * @return string the path.
 */
dfx.getPath = function(node)
{
    var path = '';

    while (node && node.parentNode) {
        if (node.nodeType === dfx.TEXT_NODE) {
            var sibling = node.previousSibling;
            var pos     = 1;
            while (sibling) {
                pos++;
                sibling = sibling.previousSibling;
            }

            if (pos <= 1) {
                path = '/node()';
            } else {
                path = '/node()[' + pos + ']';
            }
        } else {
            var nodeName = node.nodeName.toLowerCase();
            var sibling = node.previousSibling;
            var pos     = 1;
            while (sibling) {
                if (sibling.nodeType === dfx.ELEMENT_NODE &&
                    nodeName === sibling.nodeName.toLowerCase()
                ) {
                    pos++;
                }

                sibling = sibling.previousSibling;
            }

            if (pos <= 1) {
                path = '/' + nodeName + path;
            } else {
                path = '/' + nodeName + '[' + pos + ']' + path;
            }
        }//end if

        node = node.parentNode;
    }//end while

    return path;

}

/**
 * Returns the node within the document for the specified path.
 *
 * @param string path The path for the wanted node.
 *
 * @return DOMNode
 */
dfx.getNode = function(path)
{
    if (document.evaluate) {
        var node = document.evaluate(path, document, null, XPathResult.FIRST_ORDERED_NODE_TYPE, null);
        return node.singleNodeValue;
    } else {
        return dfx.getNodeFromPath(path);
    }

};

/**
 * Provides an domtree traversal method for retrieving the node for the
 * specified path.
 *
 * @param string path The path for the wanted node.
 *
 * @return DOMNode
 */
dfx.getNodeFromPath = function(path)
{
    var paths  = path.split('/');
    var parent = document;
    var pln    = paths.length;
    for (var i = 0; i < pln; i++) {
        if (dfx.trim(paths[i]) === '') {
            continue;
        }

        parent = dfx.getNodeFromPathSegment(parent, paths[i]);
    }

    return parent;

};

/**
 * Returns the node for the specified path segment under the specified parent.
 *
 * @param DOMElement parent The parent to retreive the child for.
 * @param string     path   The path segment that identifies the child.
 *
 * @return DOMNode
 */
dfx.getNodeFromPathSegment = function(parent, path)
{
    var pos = path.match(/\[(\d+)\]/);

    if (!pos) {
        pos = 1;
    } else {
        pos = parseInt(pos[1]);
        if (!pos) {
            pos = 1;
        }
    }

    var brPos = path.indexOf('[') || path.length;
    var type  = path.substr(0, brPos);

    if (!type) {
        type = path;
    }

    var node, found = 1;
    var cln         = parent.childNodes.length;
    for (var i = 0; i < cln; i++) {
        node = parent.childNodes[i];

        if (node.nodeType === dfx.DOCUMENT_TYPE_NODE) {
            continue;
        }

        if (type === 'node()') {
            if (found === pos) {
                return node;
            }

            found++;
        } else if (node.nodeName && type === node.nodeName.toLowerCase()) {
            if (found === pos) {
                return node;
            }

            found++;
        }
    }

    throw Error('XPath: node could not be found');

};

/**
 * Returns the node previous to the node at the specified path.
 *
 * If the last path segment of the specified path contains a node type, the
 * next previous node of that type will be returned.
 *
 * @param string path The path to the node next to the wanted node.
 *
 * @return DOMNode
 */
dfx.getPreviousNode = function(path)
{
    var paths    = path.split('/');
    var lastStep = paths.pop();
    var pos      = lastStep.match(/\[(\d+)\]/)[1];
    lastStep     = lastStep.replace(/\[(\d+)\]/, '[' + (parseInt(pos) - 1) + ']');

    path = paths.join('/') + '/' + lastStep;

    return dfx.getNode(path);

};
