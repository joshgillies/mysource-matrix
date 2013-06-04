/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: js_asset_map.js,v 1.1.2.47 2013/06/04 06:55:05 lwright Exp $
*
*/

/**
 * JS_Asset_Map
 *
 * Purpose
 *    JavaScript version of the Asset Map.
 *
 *    Required browser versions:
 *    IE 8 or later, recent versions of Firefox, Chrome or Safari.
 *    Earlier versions of IE and other browsers should revert back to the
 *    Java asset map.
 *
 * @author  Luke Wright <lwright@squiz.net>
 * @version $Revision: 1.1.2.47 $
 * @package   MySource_Matrix
 * @subpackage __core__
 */


var JS_Asset_Map = new function() {

    /**
     * Enumerated list of statuses
     * @var {Object}
     */
    var Status = {
        Archived:          0x01,
        UnderConstruction: 0x02,
        PendingApproval:   0x04,
        Approved:          0x08,
        Live:              0x10,
        LiveApproval:      0x20, // Up For Review
        Editing:           0x40, // Safe Editing
        EditingApproval:   0x80, // Safe Edit Pending Approval
        EditingApproved:   0x100 // Safe Edit Approved
    }

    /**
     * Enumerated list of link types
     * @var {Object}
     */
    var LinkType = {
        Type1:  0x01,
        Type2:  0x02,
        Type3:  0x04,
        Notice: 0x08
    }

    /**
     * The target element where the asset map will be drawn
     * @var {String}
     */
    var targetElement = null;

    /**
     * Options passed to the asset map.
     * @var {Object}
     */
    var options = {};

    /**
     * The current user
     * @var {String}
     */
    var currentUser = '';

    /**
     * Asset type cache
     * @var {Object}
     */
    var assetTypeCache = {};

    /**
     * Asset category cache
     *
     * Empty category is category name '_EMPTY_'.
     *
     * @var {Object}
     */
    var assetCategories = {};

    /**
     * List of parents of asset types.
     */
    var assetTypeParents = {};

    /**
     * The display format of asset names, including keywords
     * @var {String}
     */
    var assetDisplayFormat = '';

    /**
     * The current tree ID (zero-based)
     * @var {Number}
     */
    var currentTreeid = 0;

    /**
     * Hash of timeouts/intervals.
     * @var {Object}
     */
    var timeouts = {};

    /**
     * Denotes the last created asset type, so the add menu can show it
     * @var {String}
     */
    var lastCreatedType = null;

    /**
     * List of trees. By default, this will be an array of no more than two
     * trees, although it is possible to support more.
     * @var {Array}
     */
    var trees = [];

    /**
     * Use me status.
     * @var {Object]
     */
    var useMeStatus = null;

    /**
     * Move me status.
     *
     * If the whole variable is null, move me status is disabled.
     * - source: {Array.<Node>} the asset lines that is/are being moved/cloned/linked.
     *   - This can be null, in this case we are placing a new asset created with
     *     the "Add Menu".
     * - callback: {Function} the callback when a drop location has been selected.
     *   - param {Node} Echoes the source of the move me action.
     *   - param {Node} When an asset line node, it's been dropped directly on an asset.
     *                  If it's a placeholder, it's been dropped in-between assets.
     *                  The placeholder node should have dataset attributes of assetid,
     *                  linkid of parent, and sort order of the following asset (sort
     *                  order is -1 if dropped after the last child).
     *
     * @var {Object}
     */
    var moveMeStatus = null;


    /**
     * List of assets to refresh, sent from elsewhere in the Matrix system.
     *
     * This will get processed every 2 seconds if it contains something, similar to the
     * Java asset map. It does this rather than doing it for each request because a HIPO
     * processing a large number of assets (for instance, something that cascades a
     * status change throughout a whole site) would make multiple requests to update a
     * single asset (enforced by Matrix's event system), particularly if a large threshold
     * for changes is set for the HIPO. This allows some form of batching to occur for
     * these updates.
     *
     * These refreshes should be for the asset itself, not their whole tree. If the
     * asset is shown at multiple places in the tree (whether or not it has been since
     * re-collapsed), it should be updated in all places, and all trees.
     *
     * @var {Array.<String>}
     */
    var refreshQueue = [];


//--        UTILITY FUNCTIONS        --//


    /**
     * Create an element with optional unselectable attribute set on (for IE<=9).
     *
     * @param {String}  tagName The name of the tag to create.
     * @param {Boolean} [selectable=false] Whether the text should be selectable.
     *
     * @param {Node}
     */
    var _createEl = function(tagName, selectable) {
        var el = targetElement.ownerDocument.createElement(tagName);

        if (selectable !== true) {
            el.setAttribute('unselectable', 'on');
        } else {
            dfx.addClass(el, 'usersel');
        }

        return el;
    };


    /**
     * Create a child container for an asset.
     *
     * Returns a div with the correct class, ID and parentid dataset attribute.
     * It's the responsibility of the caller to spot the container in the DOM
     * at the place it wants.
     *
     * @param {String} parentid The parent asset of this container.
     *
      * @returns {Node}
     */
    var _createChildContainer = function(parentid) {
        var containerId = 'child-indent-' + encodeURIComponent(parentid);
        var container   = _createEl('div');
        container.id    = containerId;
        container.setAttribute('data-parentid', parentid);
        dfx.addClass(container, 'childIndent');
        return container;
    };

    /**
     * Convert a single status number (1, 2, 4, ..., 256) into a status class name.
     *
     * If not a valid status ID, "Unknown" returned.
     *
     * @param {Number} statusNum The status number to convert.
     *
     * @returns {String}
     */
    var _statusClass = function(statusNum) {
        var retval = 'Unknown';
        for (var x in Status) {
            if (Status[x] === statusNum) {
                retval = x;
                break;
            }
        }

        return retval;
    };

    /**
     * Format an asset tree node.
     *
     * @param {Object} asset The asset (as returned from JSON) to format.
     */
    var _formatAsset = function(asset) {
        var assetid    = asset._attributes.assetid;
        var name       = asset._attributes.name;
        var typeCode   = asset._attributes.type_code;
        var status     = Number(asset._attributes.status);
        var sortOrder  = Number(asset._attributes.sort_order);
        var numKids    = Number(asset._attributes.num_kids);
        var linkid     = asset._attributes.linkid;
        var linkType   = Number(asset._attributes.link_type);
        var accessible = Number(asset._attributes.accessible);

        var assetLine = _createEl('div');
        dfx.addClass(assetLine, 'asset');
        //assetLine.id = 'asset-' + encodeURIComponent(assetid);
        assetLine.setAttribute('data-assetid', assetid);
        assetLine.setAttribute('data-asset-path', asset._attributes.asset_path);
        assetLine.setAttribute('data-linkid', linkid);
        assetLine.setAttribute('data-link-path', asset._attributes.link_path);
        assetLine.setAttribute('data-sort-order', sortOrder);
        assetLine.setAttribute('data-typecode', typeCode);

        if (assetTypeCache[typeCode]) {
            assetLine.setAttribute('title', assetTypeCache[typeCode].name + ' [' + assetid + ']');
        } else {
            assetLine.setAttribute('title', 'Unknown Asset Type [' + assetid + ']');
        }

        var leafSpan = _createEl('span');
        dfx.addClass(leafSpan, 'leaf');

        var iconSpan = _createEl('span');
        dfx.addClass(iconSpan, 'icon');
        iconSpan.setAttribute('style', 'background-image: url(../__data/asset_types/' + typeCode + '/icon.png)');

        if (accessible === 0) {
            var flagSpan = _createEl('span');
            dfx.addClass(flagSpan, 'not-accessible');
            assetLine.appendChild(flagSpan);
        } else if (linkType === LinkType.Type2) {
            var flagSpan = _createEl('span');
            dfx.addClass(flagSpan, 'type2-link');
            assetLine.appendChild(flagSpan);
        }

        if ((accessible !== 0) && (numKids !== 0)) {
            var expandSpan = _createEl('span');
            dfx.addClass(expandSpan, 'branch-status');
            assetLine.appendChild(expandSpan);
        }

        var nameSpan = _createEl('span');
        if (nameSpan.textContent !== undefined) {
            nameSpan.textContent = name;
        } else if (nameSpan.innerText !== undefined) {
            nameSpan.innerText = name;
        }

        var statusClass = _statusClass(status);
        dfx.addClass(nameSpan, 'assetName');
        dfx.addClass(nameSpan, 'status' + statusClass);

        assetLine.appendChild(leafSpan);
        assetLine.appendChild(iconSpan);
        assetLine.appendChild(nameSpan);

        return assetLine;
    };


    /**
     * Returns true if the type code passed is a parent type.
     *
     * Reserved - may be used in future to test .
     *
     * @param {String} typecode   The child type code.
     * @param {String} parentType The prospective parent type code.
     *
     * @returns {Boolean}
     */
    var _isAncestorType = function(typecode, parentType) {
        var ok = false;
        if (parentType === 'asset') {
            ok = true;
        } else {
            while (typecode) {
                if (typecode === parentType) {
                    ok = true;
                    break;
                }

                typecode = assetTypeParents[typecode];
            }//end while
        }//end if

        return ok;
    };


//--        INITIALISATION        --//

    /**
     * Start the asset map.
     *
     * @param {Object} options
     */
    this.start = function(startOptions) {
        var self = this;

        targetElement      = options.targetElement || dfx.getId('asset_map_container');
        assetDisplayFormat = options.displayFormat || '%asset_short_name%';
        options            = startOptions;

        var assetMap       = dfx.getId('asset_map_container');
        assetMap.style.height = (document.documentElement.clientHeight - 120) + 'px';

        this.drawToolbar();
        var containers = [
            this.drawTreeContainer(0),
            this.drawTreeContainer(1)
        ];
        this.drawStatusList();
        this.drawMessageLine();

        this.resizeTree();

        this.message('Initialising...', true);
        this.doRequest({
            _attributes: {
                action: 'initialise'
            }
        }, function(response) {
            // Cache all the asset types.
            var assetTypes = response['asset_types'][0]['type'];
            for (var i = 0; i < assetTypes.length; i++) {
                var typeinfo = assetTypes[i];
                var typecode = typeinfo['_attributes']['type_code'];

                if (((typeinfo['_attributes']['instantiable'] !== '0')) && (typeinfo['_attributes']['allowed_access'] !== 'system')) {
                    var category = typeinfo['_attributes']['flash_menu_path'];
                    if (category) {
                        assetCategories[category] = assetCategories[category] || [];
                        assetCategories[category].push(typecode);
                    }
                }

                var parentType = typeinfo['_attributes']['parent_type'];
                assetTypeCache[typecode] = typeinfo['_attributes'];
                assetTypeCache[typecode]['screens'] = {};

                if (parentType !== 'asset') {
                    assetTypeParents[typecode] = parentType;
                }

                for (var j = 0; j < typeinfo['screen'].length; j++) {
                    var screencode = typeinfo['screen'][j]['_attributes']['code_name'];
                    var screenname = typeinfo['screen'][j]['_content'];
                    assetTypeCache[typecode]['screens'][screencode] = screenname;
                }//end for
            }//end for

            var assets = response['assets'][0]['asset'];
            self.drawTree(assets[0], containers[0]);
            self.drawTree(assets[0], containers[1]);

            self.drawTreeList();
            self.selectTree(0);
            self.initEvents();
            self.message('Success!', false, 2000);
        });
    };

    /**
     * Start the simple asset map.
     *
     * @param {Object} options
     */
    this.startSimple = function(options) {
        targetElement = options.targetElement;
    };

    /**
     * Initialise events.
     *
     * @param {Object} options
     */
    this.initEvents = function() {
        var document = targetElement.ownerDocument;
        var assetMap = dfx.getId('asset_map_container');
        var trees    = dfx.getClass('tree', assetMap);
        var self     = this;

        timeouts.refreshQueue = setInterval(function() {
            if (refreshQueue.length > 0) {
                self.processRefreshQueue();
            }
        }, 2000);

        dfx.addEvent(this.getDefaultView(document), 'resize', function() {
            self.resizeTree();
        });

        var statusDivider = dfx.getId('asset_map_status_list_divider');
        dfx.addEvent(statusDivider, 'click', function() {
            dfx.toggleClass(statusDivider.parentNode, 'expanded');
            self.resizeTree();
        });

        dfx.addEvent(dfx.getId('asset_map_button_restore'), 'click', function() {
            // Teleport back to root.
            self.teleport(1, 1);
        });

        dfx.addEvent(dfx.getId('asset_map_button_statuses'), 'click', function() {
            var assetMap = dfx.getId('asset_map_container');
            dfx.toggleClass(assetMap, 'statuses-shown');
        });

        dfx.addEvent(dfx.getId('asset_map_button_collapse'), 'click', function() {
            var assetMap      = dfx.getId('asset_map_container');
            var childIndents  = dfx.getClass('childIndent', assetMap);
            var branchButtons = dfx.getClass('branch-status', assetMap);

            dfx.addClass(childIndents, 'collapsed');
            dfx.removeClass(branchButtons, 'expanded');
        });

        dfx.addEvent(targetElement, 'contextmenu', function(e) {
            e.preventDefault();
        });

        dfx.addEvent(trees, 'mousedown', function(e) {
            var branchTarget = null;
            var assetTarget  = null;

            var target = e.target;
            while (target && !branchTarget && !assetTarget) {
                if (dfx.hasClass(target, 'branch-status') === true) {
                    branchTarget = target;
                } else if ((dfx.hasClass(target, 'assetName') === true) || (dfx.hasClass(target, 'icon') === true)) {
                    if (dfx.hasClass(target.parentNode, 'asset') === true) {
                        assetTarget = target.parentNode;
                    }
                }
                target = target.parentNode;
            }

            if (assetTarget) {
                var tree   = dfx.getParents('.tree', assetTarget)[0];
                var treeid = tree.getAttribute('data-treeid');
                if (e.which === 1) {
                    // Left mouse button
                    self.clearMenus();
                    if ((e.ctrlKey === false) || (self.isInUseMeMode() === true)) {
                        // Normal click, or if in Use Me mode where multiple
                        // selection is not permitted.
                        dfx.removeClass(dfx.getClass('asset', assetMap), 'located selected');
                        dfx.addClass(assetTarget, 'selected');
                    } else {
                        // Ctrl+click. Toggle the selection of this asset, which
                        // could leave the map with multiple or zero selection.
                        dfx.removeClass(dfx.getClass('asset', assetMap), 'located');
                        dfx.toggleClass(assetTarget, 'selected');
                    }
                } else if (e.which === 3) {
                    // Right mouse button
                    if ((e.ctrlKey === false) || (self.isInUseMeMode() === true)) {
                        // Normal click, or if in Use Me mode where multiple
                        // selection is not permitted.
                        dfx.removeClass(dfx.getClass('asset', assetMap), 'selected located');
                    } else {
                        dfx.removeClass(dfx.getClass('asset', assetMap), 'located');
                    }

                    e.preventDefault();
                    if (dfx.hasClass(assetTarget, 'disabled') === false) {
                        dfx.addClass(assetTarget, 'selected');
                        var mousePos = dfx.getMouseEventPosition(e);

                        if (self.isInUseMeMode() === true) {
                            var menu = self.drawUseMeMenu(assetTarget);
                        } else {
                            var menu = self.drawScreensMenu(target);
                        }

                        self.topDocumentElement(target).appendChild(menu);

                        var elementHeight = self.topDocumentElement(targetElement).clientHeight;
                        var submenuHeight = dfx.getElementHeight(menu);
                        var targetRect = dfx.getBoundingRectangle(target);
                        dfx.setStyle(menu, 'left', (Math.max(10, mousePos.x) + 'px'));
                        dfx.setStyle(menu, 'top', (Math.min(elementHeight - submenuHeight - 10, mousePos.y) + 'px'));
                    } else {
                        self.clearMenus();
                    }//end if
                }//end if
            } else if (branchTarget) {
                // Set the target to the asset line.
                var target       = branchTarget.parentNode;
                var assetid      = target.getAttribute('data-assetid');
                var linkid       = target.getAttribute('data-linkid');
                var assetPath    = target.getAttribute('data-asset-path');
                var linkPath     = target.getAttribute('data-link-path');
                var rootIndentId = 'child-indent-' + encodeURIComponent(assetid);
                var container    = dfx.getId(rootIndentId);

                if (container) {
                    dfx.toggleClass(branchTarget, 'expanded');
                    dfx.toggleClass(container, 'collapsed');
                } else {
                    dfx.addClass(branchTarget, 'expanded');

                    var container = _createChildContainer(assetid);
                    dfx.addClass(container, 'loading');
                    container.innerHTML = 'Loading...';
                    target.parentNode.insertBefore(container, target.nextSibling);

                    // Loading.
                    self.message('Requesting children...', true);

                    self.doRequest({
                        _attributes: {
                            action: 'get assets',
                        },
                        asset: [
                            {
                                _attributes: {
                                    assetid: assetid,
                                    start: 0,
                                    limit: options.assetsPerPage,
                                    linkid: linkid
                                }
                            }
                        ]
                    }, function(response) {
                        dfx.removeClass(container, 'loading');
                        var assets = response['asset'][0];

                        if (!assets.asset) {
                            self.message('No children loaded', false, 2000);
                            dfx.remove(container);
                            dfx.remove(branchTarget);
                        } else {
                            container.innerHTML = '';
                            var assetCount = assets.asset.length;
                            assets._attributes.asset_path = assetPath;
                            assets._attributes.link_path  = linkPath;
                            self.drawTree(assets, container);

                            switch (assetCount) {
                                case 1:
                                    self.message('Loaded one child', false, 2000);
                                break;

                                default:
                                    self.message('Loaded ' + assetCount + ' children', false, 2000);
                                break;
                            }//end switch
                        }//end if
                    });
                }//end if

                return false;
            } else {
                // Deselect everything.
                dfx.removeClass(dfx.getClass('asset', trees), 'selected located');
                self.clearMenus();
            }//end if

        });
    };


//--        CORE ACTIONS        --//


    /**
     * Get the currently selected tree element.
     *
     * @return {Node|Null}
     */
    this.getCurrentTreeElement = function() {
        var trees = dfx.getClass('tree.selected', targetElement);

        if (trees.length > 0) {
            return trees[0];
        } else {
            return null;
        }
    };


    /**
     * Bring the selected tree to the foreground.
     *
     * @param {Number} treeid The tree ID (zero-indexed; use 0 for Tree One).
     */
    this.selectTree = function(treeid) {
        var trees = dfx.getClass('tree', targetElement);
        dfx.removeClass(trees, 'selected');
        dfx.addClass(trees[treeid], 'selected');

        var treeList = dfx.getClass('tree-list', targetElement)[0];
        var tabs     = dfx.getClass('tab', targetElement);
        dfx.removeClass(tabs, 'selected');
        dfx.addClass(tabs[treeid], 'selected');
    };


    /**
     * Return the asset nodes that have been selected on a specified tree.
     *
     * @param {Number} [treeid] Tree ID (zero-indexed, default = selected tree).
     *
     * @returns {Array.<Node>}
     */
    this.currentSelection = function(treeid) {
        if (treeid === undefined) {
            var tree = this.getCurrentTreeElement();
        } else {
            var tree = dfx.getClass('tree', targetElement)[treeid];
        }

        var assetMap = dfx.getId('asset_map_container');
        var trees    = dfx.getClass('tree', assetMap);
        var assets   = dfx.getClass('asset.selected', trees[treeid]);

        return assets;
    };


    /**
     * Teleport to a specific asset.
     *
     * Also use this to restore root, using assetid=1 and linkid=1.
     *
     * @param {String} assetid  The asset ID to teleport to.
     * @param {String} linkid   The link ID of the location being teleported to.
     * @param {Number} [treeid] Tree ID (zero-indexed, default = selected tree).
     */
    this.teleport = function(assetid, linkid, treeid) {
        var self = this;
        if (treeid === undefined) {
            var tree = this.getCurrentTreeElement();
        } else {
            var tree = dfx.getClass('tree', targetElement)[treeid];
        }

        if (tree) {
            self.doRequest({
                _attributes: {
                    action: 'get assets',
                },
                asset: [
                    {
                        _attributes: {
                            assetid: assetid,
                            start: 0,
                            limit: options.assetsPerPage,
                            linkid: linkid
                        }
                    }
                ]
            }, function(response) {
                // Cache all the asset types.
                //dfx.removeClass(tree, 'loading');
                var rootAsset = response['asset'][0];

                if (!rootAsset.asset) {
                    self.message('No children loaded', false, 2000);
                    dfx.remove(container);
                    dfx.remove(branchTarget);
                } else {
                    tree.innerHTML = '';

                    if (String(assetid) !== '1') {
                        var assetCount   = rootAsset.asset.length;
                        var rootIndentId = 'child-indent-' + encodeURIComponent(assetid);

                        rootAsset._attributes.name       = decodeURIComponent(rootAsset._attributes.name.replace(/\+/g, '%20'));
                        rootAsset._attributes.assetid    = decodeURIComponent(rootAsset._attributes.assetid.replace(/\+/g, '%20'));
                        rootAsset._attributes.type_code  = decodeURIComponent(rootAsset._attributes.type_code.replace(/\+/g, '%20'));

                        assetLine = _formatAsset(rootAsset);

                        dfx.addClass(assetLine, 'teleported');
                        tree.appendChild(assetLine);
                    }//end if

                    rootAsset._attributes.asset_path = rootAsset._attributes.assetid;
                    rootAsset._attributes.link_path  = rootAsset._attributes.linkid;
                    self.drawTree(rootAsset, tree);

                    switch (assetCount) {
                        case 1:
                            self.message('Loaded one child', false, 2000);
                        break;

                        default:
                            self.message('Loaded ' + assetCount + ' children', false, 2000);
                        break;
                    }//end switch
                }//end if
            });

        }
    };


    /**
     * Add a new asset.
     *
     * @param {String} typeCode      The type of asset being created.
     * @param {String} parentAssetid The parent asset.
     * @param {Number} [sortOrder]   New sort order (last child if omitted).
     */
    this.addAsset = function(typeCode, parentAssetid, sortOrder) {
        var self = this;
        if (sortOrder === undefined) {
            sortOrder = -1;
        }

        var command = {
            _attributes: {
                action: 'get url',
                cmd: 'add',
                parent_assetid: parentAssetid,
                pos: sortOrder,
                type_code: typeCode
            }
        };

        this.doRequest(command, function(response) {
            lastCreatedType = typeCode;
            if (response.url) {
                for (var i = 0; i < response.url.length; i++) {
                    var frame    = response.url[i]._attributes.frame;
                    var redirURL = response.url[i]._content;
                    self.frameRequest(redirURL, frame);
                }
            } else if (response._rootTag === 'error') {
                self.raiseError(response._content);
            }
        });
    };


    /**
     * Move an asset to a new parent.
     *
     * @param {String} assetid
     * @param {String} parentAssetid
     * @param {Number} [sortOrder] New sort order (last child if omitted)
     */
    this.moveAsset = function(assetid, newParentAssetid, sortOrder) {
        if (assetid === newParentAssetid) {
            // Changing the sort order only
        } else {
            // Moving to a new assetid.
        }
    };


    /**
     * Create a new link to an asset to a new parent.
     *
     * @param {String} assetid
     * @param {String} parentAssetid
     * @param {Number} [sortOrder] New sort order (last child if omitted)
     */
    this.createLink = function(assetid, newParentAssetid, sortOrder) {
        if (assetid === newParentAssetid) {
            // Shouldn't get here, but assets cannot be linked to itself.
            this.raiseError(js_translate('asset_map_error_cannot_link_to_itself'));
        }
    };


    /**
     * Clone an asset.
     *
     * @param {String} assetid
     * @param {String} parentAssetid
     * @param {Number} [sortOrder] New sort order (last child if omitted)
     */
    this.cloneAsset = function(assetid, newParentAssetid, sortOrder) {

    };

    /**
     * Resize the tree in response to height changes.
     *
     */
    this.resizeTree = function() {
        var document   = targetElement.ownerDocument;

        var assetMap = dfx.getId('asset_map_container');
        var toolbarDiv = dfx.getClass('toolbar')[0];
        var messageDiv = dfx.getClass('messageLine')[0];
        var statusList = dfx.getClass('statusList')[0];

        var treeDivs = dfx.getClass('tree');

        assetMap.style.height = (document.documentElement.clientHeight - 120) + 'px';
        for (var i = 0; i < treeDivs.length; i++) {
            treeDivs[i].style.height = (assetMap.clientHeight - toolbarDiv.clientHeight - messageDiv.clientHeight - statusList.clientHeight) + 'px';
        }
    };


    /**
     * Show a message on the bottom status bar of the asset map.
     *
     * If spinner is false, the spinner will appear but "idle".
     * If timeout not set, it will appear indefinitely until cleared.
     * Any timeout set for a previous message will be cleared.
     *
     * @param {String}  message   The message to print.
     * @param {Boolean} spinner   Whether to show a moving spinner.
     * @param {Number}  [timeout] Message timeout in milliseconds.
     */
    this.message = function(message, spinner, timeout) {
        var spinnerDiv       = dfx.getClass('spinner', dfx.getClass('messageLine', dfx.getId('asset_map_container')))[0];
        var messageDiv       = dfx.getId('asset_map_message');
        messageDiv.innerHTML = message;

        if (timeouts.message) {
            clearTimeout(timeouts.message);
            timeouts.message = null;
        }

        // The spinner is a sprite, so handle it using an interval.
        if ((spinner === false) && (timeouts.spinner)) {
            clearInterval(timeouts.spinner);
            dfx.setStyle(spinnerDiv, 'background-position', '0 0');
            timeouts.spinner = null;
        } else if ((spinner === true) && (!timeouts.spinner)) {
            dfx.setStyle(spinnerDiv, 'background-position', '-15px 0');
            timeouts.spinner = setInterval(function() {
                var bpPos   = dfx.getStyle(spinnerDiv, 'background-position').split(' ');
                var newLeft = ((parseInt(bpPos[0], 10) % 180) - 15);
                dfx.setStyle(spinnerDiv, 'background-position', newLeft + 'px 0px');
            }, 100);
        }

        if (timeout !== undefined) {
            timeouts.message = setTimeout(function() {
                messageDiv.innerHTML = '&nbsp;';
                msgTimeoutId = null;
            }, timeout);
        }

    }

    /**
     * Raise an error message.
     *
     * @param {String} message Message to display.
     */
    this.raiseError = function(message) {
        var codeRegexp = / \[([A-Z]{1,4}\d{4})\]$/;
        var title = js_translate('error');
        if (codeRegexp.test(message) === true) {
            var matches = codeRegexp.exec(message);
            title   = js_translate('asset_map_matrix_error', matches[1]);
            message = message.replace(codeRegexp, '');
        }

        var assetMap = dfx.getId('asset_map_container');

        var errorDiv = _createEl('div');
        dfx.addClass(errorDiv, 'errorPopup');

        var titleDiv = _createEl('div');
        dfx.addClass(titleDiv, 'errorTitle');
        titleDiv.innerHTML = title;

        // Body text should be selectable so it can be copy+pasted for
        // support purposes.
        var bodyDiv = _createEl('div', true);
        dfx.addClass(bodyDiv, 'errorBody');
        bodyDiv.innerHTML = message;

        var bottomDiv = _createEl('div');
        dfx.addClass(bottomDiv, 'errorBottom');

        var buttonDiv = _createEl('button');
        buttonDiv.innerHTML = js_translate('ok');

        bottomDiv.appendChild(buttonDiv);
        errorDiv.appendChild(titleDiv);
        errorDiv.appendChild(bodyDiv);
        errorDiv.appendChild(bottomDiv);
        assetMap.appendChild(errorDiv);

        dfx.addEvent(buttonDiv, 'click', function() {
            dfx.remove(errorDiv);
        });
    };

    /**
     * Get the default view/parent window of a document.
     *
     * @param {Document} document The document.
     *
     * @returns {Window}
     */
    this.getDefaultView = function(document) {
        if (document.ownerDocument) {
            document = document.ownerDocument;
        }

        if (document.defaultView) {
            return document.defaultView;
        } else if (document.parentWindow) {
            return document.parentWindow;
        }

        return null;
    };

    /**
     * Get the top window's document element.
     *
     * Used for placing menus.
     *
     * @param {Node} target The target element.
     *
     * @returns {Node}
     */
    this.topDocumentElement = function(target) {
        var topDoc = this.getDefaultView(target.ownerDocument).top.document.documentElement;
        return topDoc;
    }


//--        DRAWING METHODS        --//


    /**
     * Draw toolbar.
     *
     * @returns {Node}
     */
    this.drawToolbar = function() {
        var self = this;

        var container = _createEl('div');
        dfx.addClass(container, 'toolbar');
        targetElement.appendChild(container);

        var addButton = _createEl('div');
        dfx.addClass(addButton, 'addButton');
        container.appendChild(addButton);
        dfx.addEvent(addButton, 'click', function(e) {
            var assetMap = dfx.getId('asset_map_container');
            var target   = dfx.getMouseEventTarget(e);
            var mousePos = dfx.getMouseEventPosition(e);
            var menu     = self.drawAddMenu();
            self.topDocumentElement(target).appendChild(menu);
            dfx.setStyle(menu, 'left', (mousePos.x) + 'px');
            dfx.setStyle(menu, 'top', (mousePos.y) + 'px');
        });

        var tbButtons = _createEl('div');
        dfx.addClass(tbButtons, 'tbButtons');
        container.appendChild(tbButtons);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_refresh';
        dfx.addClass(tbButton, 'tbButton');
        dfx.addClass(tbButton, 'refresh');
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Refresh');
        tbButtons.appendChild(tbButton);
        dfx.addEvent(tbButton, 'click', function(e) {
            self.refreshTree();
        });

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_restore';
        dfx.addClass(tbButton, 'tbButton');
        dfx.addClass(tbButton, 'restore');
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Restore root');
        tbButtons.appendChild(tbButton);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_collapse';
        dfx.addClass(tbButton, 'tbButton');
        dfx.addClass(tbButton, 'collapse');
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Collapse all');
        tbButtons.appendChild(tbButton);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_statuses';
        dfx.addClass(tbButton, 'tbButton');
        dfx.addClass(tbButton, 'statuses');
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Show status');
        tbButtons.appendChild(tbButton);
    };

    /**
     * Draw the list of possible statuses and their colours.
     *
     * @returns {Node}
     */
    this.drawStatusList = function() {
        var container = _createEl('div');
        dfx.addClass(container, 'statusList');
        targetElement.appendChild(container);

        var divider = _createEl('div');
        divider.id        = 'asset_map_status_list_divider';
        dfx.addClass(divider, 'statusDivider');
        container.appendChild(divider);

        var dividerIcon = _createEl('div');
        dfx.addClass(dividerIcon, 'icon');
        divider.appendChild(dividerIcon);

        var dividerText = _createEl('span');
        dfx.addClass(dividerText, 'text');
        dividerText.innerHTML = 'Status colour key';
        divider.appendChild(dividerText);

        for (var x in Status) {
            var displayName = js_translate('status_' + x.replace(/([a-z])([A-Z])/g, '$1_$2').toLowerCase());

            var assetLine = _createEl('div');
            dfx.addClass(assetLine, 'asset');

            var iconSpan = _createEl('span');
            dfx.addClass(iconSpan, 'statusListIcon');
            dfx.addClass(iconSpan, 'status' + x);
            iconSpan.innerHTML = '&nbsp;';

            var nameSpan = _createEl('span');
            if (nameSpan.textContent !== undefined) {
                nameSpan.textContent = displayName;
            } else if (nameSpan.innerText !== undefined) {
                nameSpan.innerText = displayName;
            }

            dfx.addClass(nameSpan, 'assetName');
            assetLine.appendChild(iconSpan);
            assetLine.appendChild(nameSpan);
            container.appendChild(assetLine);
        }//end for
    };

    /**
     * Draw the message line on the asset map.
     *
     */
    this.drawMessageLine = function() {
        var container = _createEl('div');
        dfx.addClass(container, 'messageLine');
        targetElement.appendChild(container);

        var spinnerDiv = _createEl('div');
        dfx.addClass(spinnerDiv, 'spinner');
        container.appendChild(spinnerDiv);

        var messageDiv = _createEl('div');
        messageDiv.id        = 'asset_map_message';
        dfx.addClass(messageDiv, 'message');
        messageDiv.innerHTML = 'Loading...';
        container.appendChild(messageDiv);
    };


    /**
     * Draw the list of tree tabs.
     *
     */
    this.drawTreeList = function() {
        var self     = this;
        var treeList = _createEl('div');
        dfx.addClass(treeList, 'tree-list');

        var tree1 = _createEl('div');
        dfx.addClass(tree1, 'tab');
        tree1.innerHTML = 'Tree One';
        treeList.appendChild(tree1);
        dfx.addEvent(tree1, 'click', function() {
            self.selectTree(0);
        });

        var tree2 = _createEl('div');
        dfx.addClass(tree2, 'tab');
        tree2.innerHTML = 'Tree Two';
        treeList.appendChild(tree2);
        dfx.addEvent(tree2, 'click', function() {
            self.selectTree(1);
        });

        targetElement.appendChild(treeList);
    }

    /**
     * Draw a tree container.
     *
     * @returns {Node}
     */
    this.drawTreeContainer = function(treeid) {
        var container = _createEl('div');
        dfx.addClass(container, 'tree');
        container.setAttribute('data-treeid', treeid);
        targetElement.appendChild(container);

        return container;
    };

    /**
     * Draw a tree of child assets.
     *
     * The container can be the top of the tree (if the parent is the Root Folder)
     * or an indenting container if further down.
     *
     * @param {Object} rootAsset The root (or parent) asset of this tree branch.
     * @param {Node}   container The container to draw the tree into.
     */
    this.drawTree = function(rootAsset, container) {
        var assetLine = null;
        container.setAttribute('data-parentid', rootAsset._attributes.assetid);

        for (var i = 0; i < rootAsset.asset.length; i++) {
            var asset  = rootAsset.asset[i];
            asset._attributes.name      = decodeURIComponent(asset._attributes.name.replace(/\+/g, '%20'));
            asset._attributes.assetid   = decodeURIComponent(asset._attributes.assetid.replace(/\+/g, '%20'));
            asset._attributes.type_code = decodeURIComponent(asset._attributes.type_code.replace(/\+/g, '%20'));

            if (!rootAsset._attributes.asset_path) {
                asset._attributes.asset_path = asset._attributes.assetid;
            } else {
                asset._attributes.asset_path = rootAsset._attributes.asset_path + ',' + asset._attributes.assetid;
            }

            if (!rootAsset._attributes.link_path) {
                asset._attributes.link_path = asset._attributes.linkid;
            } else {
                asset._attributes.link_path  = rootAsset._attributes.link_path + ',' + asset._attributes.linkid;
            }

            assetLine = _formatAsset(asset);
            container.appendChild(assetLine);
        }//end for

        this.updateAssetsForUseMe(container);

        if (assetLine) {
            dfx.addClass(assetLine, 'last-child');
        }
    };

    this.addToRefreshQueue = function(assetids) {
        refreshQueue = refreshQueue.concat(assetids);
    };

    this.processRefreshQueue = function() {
        // Take a local copy of the refresh queue, and clear it.
        var processQueue = refreshQueue.concat([]);
        refreshQueue     = [];

        // Requests to be made. However, we are going to try and request zero children.
        var assetMap      = dfx.getId('asset_map_container');
        var assetRequests = [];

        for (var i = 0; i < processQueue.length; i++) {
            assetRequests.push({
                _attributes: {
                    assetid: processQueue[i],
                    linkid: null,
                    start: 0,
                    limit: 1 
                }
            });
        }//end for

        var processAssets = function(response) {
            for (var i = 0; i < response.asset.length; i++) {
                var thisAsset  = response.asset[i];
                thisAsset._attributes.name       = decodeURIComponent(thisAsset._attributes.name.replace(/\+/g, '%20'));
                thisAsset._attributes.assetid    = decodeURIComponent(thisAsset._attributes.assetid.replace(/\+/g, '%20'));
                thisAsset._attributes.type_code  = decodeURIComponent(thisAsset._attributes.type_code.replace(/\+/g, '%20'));

                var assetid    = thisAsset._attributes.assetid;
                var assetNodes = dfx.find(assetMap, 'div.asset[data-assetid=' + assetid  + ']');
                for (var j = 0; j < assetNodes.length; j++) {
                    var assetNode = assetNodes[j];
                    var newNode   = _formatAsset(thisAsset);

                    if (dfx.hasClass(assetNode, 'not-accessible') === true) {
                        dfx.addClass(newNode, 'not-accessible');
                    }

                    if (dfx.hasClass(assetNode, 'selected') === true) {
                        dfx.addClass(newNode, 'selected');
                    } else {
	                    dfx.addClass(newNode, 'located');
					}

                    newNode.setAttribute('data-linkid', assetNode.getAttribute('data-linkid'));
                    newNode.setAttribute('data-asset-path', assetNode.getAttribute('data-asset-path'));
                    newNode.setAttribute('data-link-path', assetNode.getAttribute('data-link-path'));

                    assetNode.parentNode.replaceChild(newNode, assetNode);
                }//end for
            }//end for

            self.message('Success!', false, 2000);
        };

        this.doRequest({
            _attributes: {
                action: 'get assets',
            },
            asset: assetRequests
        }, processAssets);

    };

    /**
     * Refresh the current tree.
     *
      * To refresh the full tree, pass no parameters. Otherwise, the root asset will
     * be used to refresh a partial tree (whether
     *
     * @param {String} [rootAsset] The root asset to refresh.
     */
    this.refreshTree = function(rootAsset) {
        var self     = this;
        var tree     = this.getCurrentTreeElement();
        if (rootAsset === undefined) {
            rootAsset = tree.getAttribute('data-parentid');
        }

        var assetids = [];
        if (String(rootAsset) === '1') {
            var rootNode = tree;
        } else {
            var rootNode = dfx.find(tree, 'div.childIndent[data-parentid="' + rootAsset + '"]')[0];
        }

        if (rootNode) {
            assetids.push(rootAsset);
            var children = dfx.getClass('childIndent', rootNode);
            for (var i = 0; i < children.length; i++) {
                assetids.push(children[i].getAttribute('data-parentid'));
            }
        }

        if (assetids.length > 0) {
            var assetRequests = [];
            while (assetids.length > 0) {
                var assetid    = assetids.shift();
                sortOrder      = 0;

                assetRequests.push({
                    _attributes: {
                        assetid: assetid,
                        linkid: null,
                        start: sortOrder,
                        limit: options.assetsPerPage
                    }
                });
            }//end while

            var processAssets = function(response) {
                var container = null;

                for (var i = 0; i < response.asset.length; i++) {
                    var thisAsset = response.asset[i];
                    var assetid   = thisAsset._attributes.assetid;

                    if (String(assetid) === '1') {
                        container = tree;
                    } else {
                        var assetNode = dfx.find(tree, 'div.asset[data-assetid=' + assetid  + ']')[0];
                        dfx.addClass(assetNode, 'expanded');

                        container = dfx.find(tree, 'div.childIndent[data-parentid=' + assetid  + ']')[0];
                        if (!container) {
                            if (dfx.hasClass(assetNode.nextSibling, 'childIndent') === false) {
                                container = _createChildContainer(assetid);
                                assetNode.parentNode.insertBefore(container, assetNode.nextSibling);
                            }//end if
                        }//end if
                    }//end if

                    container.innerHTML = '';
                    self.drawTree(thisAsset, container);
                }//end for

                self.message('Success!', false, 2000);
            };

            this.doRequest({
                _attributes: {
                    action: 'get assets',
                },
                asset: assetRequests
            }, processAssets);
        }//end if
    };


//--        LOCATE ASSET (BINOCULARS)        --//


    /**
     * Locate asset.
     */
    this.locateAsset = function(assetids, sortOrders) {
        var self         = this;
        var savedAssets  = assetids.concat([]);
        var tree         = this.getCurrentTreeElement();
        var container    = tree;

        dfx.removeClass(dfx.getClass('asset', tree), 'located selected');
        while (assetids.length > 0) {
            var assetid    = assetids.shift();
            var sortOrder  = sortOrders.shift();
            var assetLines = dfx.find(container, 'div[data-assetid=' + assetid + ']');

            if (assetLines.length === 0) {
                this.raiseError('Cannot locate asset.');
                return;
            } else {
                var assetLine = assetLines[0];
                if (assetids.length === 0) {
                    dfx.addClass(assetLine, 'selected');
                    assetLine.scrollIntoView(true);
                } else {
                    dfx.addClass(assetLine, 'located');
                    container = assetLine.nextSibling;
                    if (dfx.hasClass(container, 'childIndent') === false) {
                        assetids.unshift(assetid);
                        break;
                    } else {
                        var nextAsset = dfx.find(container, 'div[data-assetid=' + assetids[0] + ']');
                        if (nextAsset.length === 0) {
                            dfx.remove(container);
                            assetids.unshift(assetid);
                            break;
                        } else {
                            var branchTarget = dfx.getClass('branch-status', assetLine);
                            dfx.addClass(branchTarget, 'expanded');
                            dfx.removeClass(container, 'collapsed');
                        }//end if
                    }//end if
                }//end if
            }//end if
        }//end while

        if (assetids.length > 0) {
            var assetRequests = [];
            var allAssetids   = [].concat(assetids);
            allAssetids.shift();
            while (sortOrders.length > 0) {
                var assetid    = assetids.shift();
                var sortOrder  = sortOrders.shift();
                sortOrder      = Math.max(0, Math.floor(sortOrder / options.assetsPerPage) * options.assetsPerPage);

                assetRequests.push({
                    _attributes: {
                        assetid: assetid,
                        linkid: null,
                        start: sortOrder,
                        limit: options.assetsPerPage
                    }
                });
            }

            var processAssets = function(response) {
                for (var i = 0; i < response.asset.length; i++) {
                    var thisAsset = response.asset[i];
                    var container = _createChildContainer(thisAsset._attributes.assetid);
                    dfx.addClass(assetLine, 'expanded');
                    assetLine.parentNode.insertBefore(container, assetLine.nextSibling);
                    self.drawTree(thisAsset, container);

                    var nextAssetid = allAssetids[i];
                    assetLine       = dfx.find(container, 'div[data-assetid=' + nextAssetid + ']')[0];

                    if (i < (response.asset.length - 1)) {
                        dfx.addClass(assetLine, 'located');
                    } else {
                        dfx.addClass(assetLine, 'selected');
                        assetLine.scrollIntoView(true);
                    }
                }

                self.message('Success!', false, 2000);
            };

            this.doRequest({
                _attributes: {
                    action: 'get assets',
                },
                asset: assetRequests
            }, processAssets);
        }
    }


//--        MOVE ME MODE        --//


    /**
      * Set on "move me" mode.
     *
     * Move Me mode is the mode triggered when:
     * - An asset dragged for moving, linking or cloning.
      * - An asset is added using the toolbar's "Add" dropdown, to select a position.
     *
      * Source may be null. This is used for selecting a target for an asset added
     * using the toolbar's "Add" dropdown.
     *
     * @param {Node}     [source=null] The asset node being moved.
     * @param {Function} [callback]    The function to call after selecting target.
     */
    this.setMoveMeMode = function(source, callback) {
        source = source || null;

        var self     = this;
        var assetMap = dfx.getId('asset_map_container');
        dfx.addClass(assetMap, 'moveMeMode');
        moveMeStatus = {
            source: source,
            callback: callback,
            selection: null
        };

        var lineEl = _createEl('div');
        dfx.addClass(lineEl, 'selectLine');
        assetMap.appendChild(lineEl);
        moveMeStatus.selection = lineEl;

        dfx.addEvent(dfx.getClass('tree', assetMap), 'mousedown.moveMe', function(e) {
            if (moveMeStatus.selection) {
                callback(source, moveMeStatus.selection);
            }

            // if there's no valid target when they click, then that's too bad.
            self.cancelMoveMeMode();
        });

        dfx.addEvent(dfx.getClass('tree', assetMap), 'mousemove.moveMe', function(e) {
            dfx.removeClass(dfx.getClass('asset', assetMap), 'moveTarget');
            dfx.removeClass(lineEl, 'active');
            var target = dfx.getMouseEventTarget(e);
            while (target) {
                if (dfx.hasClass(target, 'asset') === true) {
                    break;
                }
                target = target.parentNode;
            }//end while

            if (target) {
                dfx.addClass(lineEl, 'active');
                var position     = dfx.getMouseEventPosition(e);

                // Find the next closest parent.
                var parentAsset  = dfx.getParents(target, '.childIndent')[0];
                if (parentAsset) {
                    parentAsset = parentAsset.previousSibling;
                }

                var assetMapCoords = dfx.getElementCoords(assetMap);
                var assetRect    = dfx.getBoundingRectangle(target);
                var fromTop      = position.y - assetRect.y1;
                var fromBottom   = assetRect.y2 - position.y + 1;

                var assetNameSpan = dfx.getClass('assetName', target)[0];
                var assetNameRect = dfx.getBoundingRectangle(assetNameSpan);

                moveMeStatus.selection = {
                    parentid: 1,
                    linkid: 1,
                    before: -1
                };

                if (fromTop <= 3) {
                    if (parentAsset) {
                        moveMeStatus.selection.parentid = parentAsset.getAttribute('data-assetid');
                        moveMeStatus.selection.linkid   = parentAsset.getAttribute('data-linkid');
                    }

                    moveMeStatus.selection.before = target.getAttribute('data-sort-order');
                    dfx.setCoords(lineEl, (assetNameRect.x1 - assetMapCoords.x), (assetRect.y1 - assetMapCoords.y));
                } else if (fromBottom <= 3) {
                    if (parentAsset) {
                        moveMeStatus.selection.parentid = parentAsset.getAttribute('data-assetid');
                        moveMeStatus.selection.linkid   = parentAsset.getAttribute('data-linkid');
                    }

                    var insertBefore = target.nextSibling;
                    if (insertBefore) {
                        moveMeStatus.selection.before = insertBefore.getAttribute('data-sort-order');
                    }

                    dfx.setCoords(lineEl, (assetNameRect.x1 - assetMapCoords.x), (assetRect.y2 - assetMapCoords.y));
                } else {
                    moveMeStatus.selection = {
                        parentid: target.getAttribute('data-assetid'),
                        linkid: target.getAttribute('data-linkid'),
                        before: -1
                    };

                    dfx.addClass(target, 'moveTarget');
                    dfx.setCoords(lineEl, (assetNameRect.x2 - assetMapCoords.x), (((assetRect.y1 + assetRect.y2) / 2) - assetMapCoords.y));
                }//end if
            } else {
                moveMeStatus.selection = null;
            }
        });

        dfx.addEvent(dfx.getClass('tree', assetMap), 'mouseout.moveMe', function(e) {

        });
    };


    /**
      * Cancel "move me" mode.
     *
     */
    this.cancelMoveMeMode = function() {
        var assetMap = dfx.getId('asset_map_container');
        dfx.removeClass(assetMap, 'moveMeMode');
        moveMeStatus = null;

        dfx.remove(dfx.getClass('selectLine', assetMap));
        dfx.removeEvent(dfx.getClass('tree', assetMap), 'mousedown.moveMe');
        dfx.removeEvent(dfx.getClass('tree', assetMap), 'mousemove.moveMe');
        dfx.removeEvent(dfx.getClass('tree', assetMap), 'mouseout.moveMe');
    };


//--        USE ME MODE        --//


    this.getUseMeFrame = function() {
        var win    = this.getDefaultView(targetElement);
        var retval = win;

        // We're inside a frame, so check for the main frame.
        if (win.frameElement) {
            retval = win.top.frames.sq_main;
            if (!retval) {
                // Main frame isn't there.
                retval = win;
            }
        }

        return retval;
    };


    /**
     * Enable/stop the "Use Me" mode.
     *
     * This allows Asset Finder widgets to select an asset from the asset map,
     * optionally filtered by type code.
     *
     * The type filter is either omitted (in which case all assets are selectable),
     * or a list of asset types.
     *
     *
     *
     * @param {Node}  element
     * @param {Array} [typeFilter] The type filter.
     *
     */
    this.setUseMeMode = function(name, safeName, typeFilter, doneCallback) {
        var self = this;

        if (this.isInUseMeMode() === true) {
            alert(js_translate('asset_finder_in_use'));
        } else {
            var sourceFrame = self.getUseMeFrame();
            var oldOnUnload = sourceFrame.onunload;
            dfx.addEvent(sourceFrame, 'unload', function() {
                self.cancelUseMeMode();
                if (dfx.isFn(oldOnUnload) === true) {
                    oldOnUnload.call(sourceFrame);
                }
            });

            var assetMap    = dfx.getId('asset_map_container');
            dfx.addClass(assetMap, 'useMeMode');
            useMeStatus = {
                namePrefix: name,
                idPrefix: safeName,
                typeFilter: typeFilter,
                doneCallback: doneCallback,
            };
            this.updateAssetsForUseMe();
        }//end if
    };


    /**
     * Cancel use me mode
     *
     */
    this.cancelUseMeMode = function() {
        var assetMap = dfx.getId('asset_map_container');
        dfx.removeClass(assetMap, 'useMeMode');
        useMeStatus = null;
        this.updateAssetsForUseMe();
    };


    /**
      * Update the enabled/disabled status for
     */
    this.updateAssetsForUseMe = function(rootTree) {
        if (rootTree === undefined) {
            rootTree = dfx.getClass('tree', dfx.getId('asset_map_container'));
        }

        var assets = dfx.getClass('asset', rootTree);

        if (useMeStatus === null) {
            // Not in use me mode.
            dfx.removeClass(assets, 'disabled');
        } else if (!useMeStatus.typeFilter || (useMeStatus.typeFilter.length === 0)) {
            // No type filter = enable all assets.
            dfx.removeClass(assets, 'disabled');
        } else {
            for (var i = 0; i < assets.length; i++) {
                var assetLine = assets[i];
                var typecode  = assetLine.getAttribute('data-typecode');
                if (useMeStatus.typeFilter.find(typecode) === -1) {
                    dfx.addClass(assetLine, 'disabled');
                } else {
                    dfx.removeClass(assetLine, 'disabled');
                }
            }//end for
        }//end if
    };


    /**
     * Draw the "Use Me" menu.
     *
     * @param {Node} assetNode
     *
     */
    this.drawUseMeMenu = function(assetNode) {
        var self    = this;
        var assetid = assetNode.getAttribute('data-assetid');
        this.clearMenus();

        var container = _createEl('div');
        dfx.addClass(container, 'assetMapMenu');
        dfx.addClass(container, 'useMeMenu');

        dfx.addEvent(container, 'contextmenu', function(e) {
            e.preventDefault();
        });

        var menuItem = this.drawMenuItem('Use Me');
        dfx.addEvent(menuItem, 'click', function(e) {
            var sourceFrame = self.getUseMeFrame().document;
            self.clearMenus();

            var assetNameLabel  = dfx.getId(useMeStatus.idPrefix + '_label', sourceFrame);
            var assetidBox      = dfx.getId(useMeStatus.idPrefix + '_assetid', sourceFrame);
            var assetidHidden   = dfx.getId(useMeStatus.namePrefix + '[assetid]', sourceFrame);
            var assetLinkHidden = dfx.getId(useMeStatus.namePrefix + '[linkid]', sourceFrame);
            var assetTypeHidden = dfx.getId(useMeStatus.namePrefix + '[type_code]', sourceFrame);
            var assetUrlHidden  = dfx.getId(useMeStatus.namePrefix + '[url]', sourceFrame);

            assetNameLabel.value  = dfx.getNodeTextContent(dfx.getClass('assetName', assetNode)[0]);
            assetidBox.value      = assetNode.getAttribute('data-assetid');
            assetidHidden.value   = assetNode.getAttribute('data-assetid');
            assetLinkHidden.value = assetNode.getAttribute('data-linkid');
            assetTypeHidden.value = assetNode.getAttribute('data-typecode');
            assetUrlHidden.value  = '';

            var changeButton = dfx.getId(useMeStatus.idPrefix + '_change_btn', sourceFrame);
            changeButton.value = js_translate('change');

            if (dfx.isFn(useMeStatus.doneCallback)) {
                useMeStatus.doneCallback(assetid);
            }

            self.cancelUseMeMode();
        });
        container.appendChild(menuItem);

        return container;
    };


    /**
     * Select the asset for "Use Me" mode.
     *
     * @param {Node} assetNode The asset being selected.
     *
     */
    this.selectAssetForUseMe = function(assetNode) {
    };


    /**
     * Return TRUE if the asset map is in "Use Me" mode.
     *
     * @returns {Boolean}
     */
    this.isInUseMeMode = function(excludePrefix) {
        var assetMap = dfx.getId('asset_map_container');
        var hasUseMe = dfx.hasClass(assetMap, 'useMeMode');

        if (hasUseMe === true) {
            if (excludePrefix === useMeStatus.namePrefix) {
                hasUseMe = false;
            }
        }

        return hasUseMe;

    };


//--        MENUS        --//


    /**
     * Draw list of screens menu (from right-clicking on an asset).
     *
     * @param {Node} assetNode The node that triggered the
     *
     * @returns {Node}
     */
    this.drawScreensMenu = function(assetNode) {
        var assetid   = assetNode.getAttribute('data-assetid');
        var assetPath = assetNode.getAttribute('data-asset-path');
        var linkid    = assetNode.getAttribute('data-linkid');
        var linkPath  = assetNode.getAttribute('data-link-path');
        var assetType = assetNode.getAttribute('data-typecode');

        this.clearMenus();
        var self = this;
        var container = _createEl('div');
        dfx.addClass(container, 'assetMapMenu');
        dfx.addClass(container, 'screens');
        dfx.addEvent(container, 'contextmenu', function(e) {
            e.preventDefault();
        });
        dfx.addEvent(container, 'mouseover', function(e) {
            if (!timeouts.addTypeSubmenu) {
                timeouts.addTypeSubmenu = setTimeout(function() {
                    self.clearMenus('addMenu');
                    self.clearMenus('subtype');
                    timeouts.addTypeSubmenu = null;
                }, 400);
            }
        });

        var screens = assetTypeCache[assetType]['screens'];
        for (var i in screens) {
            var menuItem = this.drawMenuItem(screens[i], null);
            menuItem.setAttribute('data-screen', i);
            dfx.addEvent(menuItem, 'click', function(e) {
                var target = e.currentTarget;
                self.clearMenus();

                var url = './?SQ_BACKEND_PAGE=main&backend_section=am&' +
                    'am_section=edit_asset&assetid=' + assetid +
                    '&sq_asset_path=' + assetPath + '&sq_link_path=' +
                    linkPath + '&asset_ei_screen=' + target.getAttribute('data-screen');
                self.frameRequest(url);
            });
            container.appendChild(menuItem);
        }

        var sep = this.drawMenuSeparator();
        container.appendChild(sep);

        var menuItem = this.drawMenuItem('Teleport', null);
        container.appendChild(menuItem);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.teleport(assetid, linkid);
        });

        var menuItem = this.drawMenuItem('Refresh', null);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.refreshTree(assetid);
        });
        container.appendChild(menuItem);

        // Don't show child options in the trash folder.
        // TODO: try to do this where no children are allowed for an asset type
        //       (needs additional handling in asset_map.inc).
        if (assetType !== 'trash_folder') {
            if (lastCreatedType === null) {
                var menuItem = this.drawMenuItem('No Previous Child', null);
                dfx.addClass(menuItem, 'disabled');
            } else {
                var menuItem = this.drawMenuItem('New ' + assetTypeCache[lastCreatedType].name, lastCreatedType);
                dfx.addEvent(menuItem, 'click', function(e) {
                    self.clearMenus();
                    self.addAsset(lastCreatedType, assetid, -1);
                });
            }
            container.appendChild(menuItem);

            var menuItem = this.drawMenuItem('New Child', null, true);
            container.appendChild(menuItem);

            dfx.addEvent(menuItem, 'mouseover', function(e) {
                if (timeouts.addTypeSubmenu) {
                    clearTimeout(timeouts.addTypeSubmenu);
                    timeouts.addTypeSubmenu = null;
                }
                e.stopPropagation();

                var assetMap = dfx.getId('asset_map_container');
                var target   = dfx.getMouseEventTarget(e);

                var existingMenu = dfx.getClass('assetMapMenu.addMenu', self.topDocumentElement(target));
                if (existingMenu.length === 0) {
                    var menu     = self.drawAddMenu(false, assetid);
                    self.topDocumentElement(target).appendChild(menu);
                    var elementHeight = self.topDocumentElement(targetElement).clientHeight;
                    var submenuHeight = dfx.getElementHeight(menu);
                    var targetRect = dfx.getBoundingRectangle(target);
                    dfx.setStyle(menu, 'left', (Math.max(10, targetRect.x2) + 'px'));
                    dfx.setStyle(menu, 'top', (Math.min(elementHeight - submenuHeight - 10, targetRect.y1) + 'px'));
                }
            });
        }

        return container;
    };


    /**
     * Draw "add child" main menu.
     *
     * Triggered by the "Add Child" option in the screens menu, or the "Add" button
     * in the toolbar.
     *
     * When the Add button in the toolbar is clicked, send no parameters to this
     * method. This will clear previous menus.
     *
     * If triggered from "Add Child" submenu, send clear=false and the asset ID
     * that the menu was triggered from. This will alert the subtype menu that
     * this is an "Add Child", so not to allow selection of where the new child
     * is placed, and not to hide the screens menu.
     *
     * @param {Boolean} [clear=true] Whether to clear existing menus first.
     * @param {String}  [parentid]   Parent asset ID.
     *
     * @returns {Node}
     */
    this.drawAddMenu = function(clear, parentid) {
        var self = this;
        if (clear !== false) {
            this.clearMenus();
        }

        var container = _createEl('div');
        dfx.addClass(container, 'assetMapMenu');
        dfx.addClass(container, 'addMenu');

        dfx.addEvent(container, 'contextmenu', function(e) {
            e.preventDefault();
        });
        dfx.addEvent(container, 'mouseover', function(e) {
            if (timeouts.addTypeSubmenu) {
                clearTimeout(timeouts.addTypeSubmenu);
                timeouts.addTypeSubmenu = null;
            }
            self.clearMenus('subtype');
        });

        for (var i in assetCategories) {
            var menuItem = this.drawMenuItem(i, null, true);
            menuItem.setAttribute('data-category', i);
            container.appendChild(menuItem);

            dfx.addEvent(menuItem, 'mouseover', function(e) {
                var target = e.currentTarget;
                e.stopPropagation();

                var existingMenu = dfx.getClass('assetMapMenu.subtype', self.topDocumentElement(target));

                if ((existingMenu.length === 0) || (existingMenu[0].getAttribute('data-category') !== target.getAttribute('data-category'))) {
                    dfx.remove(existingMenu);
                    var submenu = self.drawAssetTypeMenu(target.getAttribute('data-category'), parentid);
                    self.topDocumentElement(targetElement).appendChild(submenu);
                    var elementHeight = self.topDocumentElement(targetElement).clientHeight;
                    var submenuHeight = dfx.getElementHeight(submenu);
                    var targetRect = dfx.getBoundingRectangle(target);
                    dfx.setStyle(submenu, 'left', (Math.max(10, targetRect.x2) + 'px'));
                    dfx.setStyle(submenu, 'top', (Math.min(elementHeight - submenuHeight - 10, targetRect.y1) + 'px'));
                }
            });
        }

        var menuItem = this.drawMenuItem('Folder', 'folder');
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            if (parentid !== undefined) {
                self.addAsset('folder', parentid, -1);
            } else {
                self.setMoveMeMode(null, function(source, selection) {
                    self.addAsset('folder', selection.parentid, selection.before);
                });
            }
        });
        container.appendChild(menuItem);

        return container;
    };

    /**
     * Draw menu of asset types in a given category (or "flash menu path").
     *
     * If a parent ID was sent from the main add menu, send it here. This will
     * alert the menu that we have decided on the parent and we should add it
     * as the last child of that parent.
     *
     * @param {String} category
     *
     * @returns {Node}
     */
    this.drawAssetTypeMenu = function(category, parentid) {
        var self = this;
        this.clearMenus('subtype');
        var container = _createEl('div');
        dfx.addClass(container, 'assetMapMenu');
        dfx.addClass(container, 'subtype');
        container.setAttribute('data-category', category);

        dfx.addEvent(container, 'contextmenu', function(e) {
            e.preventDefault();
        });

        for (var i = 0; i < assetCategories[category].length; i++) {
            var typeCode = assetCategories[category][i];
            var type     = assetTypeCache[typeCode];

            var menuItem = this.drawMenuItem(type.name, typeCode);
            menuItem.setAttribute('data-typecode', typeCode);
            dfx.addEvent(menuItem, 'click', function(e) {
                self.clearMenus();
                var target   = e.currentTarget;
                var typeCode = target.getAttribute('data-typecode');

                if (parentid !== undefined) {
                    self.addAsset(typeCode, parentid, -1);
                } else {
                    self.setMoveMeMode(null, function(source, selection) {
                        self.addAsset(typeCode, selection.parentid, selection.before);
                    });
                }
            });
            container.appendChild(menuItem);
        }

        return container;
    };


    /**
     * Draw a normal menu item.
     *
     * @param {String}  text                The text for the menu item.
     * @param {String}  [assetType]         The asset type icon to paint, if any.
     * @param {Boolean} [hasChildren=FALSE] If TRUE a sub-menu arrow will be painted.
     *
     * @returns {Node}
     */
    this.drawMenuItem = function(text, assetType, hasChildren) {
        var menuItem = _createEl('div');
        dfx.addClass(menuItem, 'menuItem');

        if (assetType) {
            var icon = _createEl('div');
            dfx.addClass(icon, 'menuIcon');
            dfx.setStyle(icon, 'background-image', 'url(../__data/asset_types/' + assetType + '/icon.png)');
            menuItem.appendChild(icon);
        }

        var textSpan = _createEl('span');
        dfx.addClass(textSpan, 'menuText');
        textSpan.innerHTML = text;
        menuItem.appendChild(textSpan);

        if (hasChildren === true) {
            var icon = _createEl('div');
            dfx.addClass(icon, 'menuChild');
            menuItem.appendChild(icon);
        }

        return menuItem;
    };


    /**
     * Draw a separator menu item.
     *
     * @returns {Node}
     */
    this.drawMenuSeparator = function() {
        var sep = _createEl('div');
        dfx.addClass(sep, 'menuSep');
        return sep;
    };


    /**
     * Clear all menus or those of a certain type.
     *
     * @param {String} [type] The type of menu to clear (omit for all menus).
     */
    this.clearMenus = function(type) {
        if (type === undefined) {
            dfx.remove(dfx.getClass('assetMapMenu', this.topDocumentElement(targetElement)));
        } else {
            dfx.remove(dfx.getClass('assetMapMenu.' + type, this.topDocumentElement(targetElement)));
        }
    };


//--        BACKGROUND REQUESTS        --//


    /**
     * Do a request to the asset map PHP code.
     *
     * @param {Object}   command  The command (and params) to request.
     * @param {Function} callback The callback function.
     */
    this.doRequest = function(command, callback) {
        url = '.?SQ_BACKEND_PAGE=asset_map_request&json=1';
        var xhr = new XMLHttpRequest();
        var str = JSON.stringify(command);
        var self = this;
        var readyStateCb = function() {
            self.message('Requesting...', true);
            if (xhr.readyState === 4) {
                var response = xhr.responseText;
                if (response !== null) {
                    try {
                        response = JSON.parse(response);
                        self.message('', false);
                    } catch (ex) {
                        // self.raiseError(ex.message);
                        // That we made it here means it couldn't be handled.
                        self.message('Failed!', false, 2000);
                        self.raiseError(ex.message);
                        return;
                    }

                    callback(response);
                }
            }
        }

        xhr.open(
           'POST',
           url
        );

        self.message('Requesting...', true);

        xhr.setRequestHeader('Content-type', 'application/json');
        xhr.onreadystatechange = readyStateCb;
        xhr.send(str);
    };


    /**
     * Request a URL in a specified frame (or main frame if no frame specified).
     *
     * @param {String} url               The URL to request.
     * @param {String} [frame='sq_main'] The frame to request into.
     */
    this.frameRequest = function(url, frame) {
        if (!frame) {
            frame = 'sq_main';
        }

        var top = this.getDefaultView(targetElement.ownerDocument).top;
        top.frames[frame].location.href = url;
    };


};


//--        LEGACY FUNCTIONS FOR ASSET FINDER        --//


/*
 * NOTE:
 * These legacy functions exist mainly to allow custom Simple Edit interfaces
 * that expect the old-style functions (in the global space) to still work.
 *
 * Only the functions that are accessed from the asset finder are implemented.
 * Other functions existed that were called from the Java applet which are not
 * replicated here. Functions should perform a minimum of their own processing
 * and defer to JS_Asset_Map for as much as possible.
 */


/**
 * Handler for clicking of the Change/Cancel button of an asset finder.
 *
 * Sets or cancels Use Me mode as appropriate, or fires an alert message if
 * the Change button is clicked on an Asset Finder while another already has
 * a claim on the asset map.
 *
 * The type codes are pipe separated (eg. 'page_standard|news_item') and a
 * single type code restriction has a trailing pipe (eg. 'page_standard|').
 * Type code restrictions do not match ancestors - descendants can be specified
 * on the PHP side but they are converted to individual types for JS/Java.
 * If no type code restriction is set, all types are allowed.
 *
 * @param {String}   name           The prefix for asset finder name attributes.
 * @param {String}   safeName       The prefix for asset finder ID attributes.
 * @param {String}   [typeCodes]    Pipe-separated list of restricted type codes.
 * @param {Function} [doneCallback] Callback to call once a selection is made.
 */
function asset_finder_change_btn_press(name, safeName, typeCodes, doneCallback)
{
    if (typeCodes === '') {
        typeCodes = undefined;
    } else {
        // Split piped type codes into an array, but if there's only one
        // type code then there's a trailing pipe at the end.
        var typeCodes = typeCodes.split('|');
        if ((typeCodes.length === 2) && (typeCodes[1] === '')) {
            typeCodes.pop();
        }
    }//end if

    var mainWin      = JS_Asset_Map.getUseMeFrame();
    var changeButton = dfx.getId(safeName + '_change_btn', mainWin.document);
    if (JS_Asset_Map.isInUseMeMode(name) === true) {
        alert(js_translate('asset_finder_in_use'));
    } else if (JS_Asset_Map.isInUseMeMode() === true) {
        changeButton.setAttribute('value', js_translate('change'));
        JS_Asset_Map.cancelUseMeMode();
    } else {
        changeButton.setAttribute('value', js_translate('cancel'));
        JS_Asset_Map.setUseMeMode(name, safeName, typeCodes, doneCallback);
    }

}//asset_finder_change_btn_press()


/**
 * Handler for clicking of the Clear button of an asset finder.
 *
 * @param {String} name     The prefix for asset finder name attributes.
 * @param {String} safeName The prefix for asset finder ID attributes.
 */
function asset_finder_clear_btn_press(name, safeName)
{
    var sourceFrame = JS_Asset_Map.getUseMeFrame().document;
    dfx.getId(name + '[assetid]', sourceFrame).value    = '0';
    dfx.getId(name + '[url]', sourceFrame).value        = '';
    dfx.getId(name + '[linkid]', sourceFrame).value     = '';
    dfx.getId(name + '[type_code]', sourceFrame).value  = '';
    dfx.getId(safeName + '_label', sourceFrame).value   = '';
    dfx.getId(safeName + '_assetid', sourceFrame).value = '';

}//end asset_finder_clear_btn_press()


/**
 * Handler for clicking of the Reset button of an asset finder.
 *
 * @param {String} name     The prefix for asset finder name attributes.
 * @param {String} safeName The prefix for asset finder ID attributes.
 * @param {String} assetid  The asset ID the asset finder is to be reset to.
 * @param {String} label    The asset name label used for the reset.
 */
function asset_finder_reset_btn_press(name, safeName, assetid, label)
{
    var sourceFrame = JS_Asset_Map.getUseMeFrame().document;
    dfx.getId(name + '[assetid]', sourceFrame).value    = assetid;
    dfx.getId(name + '[url]', sourceFrame).value        = '';
    dfx.getId(name + '[linkid]', sourceFrame).value     = '';
    dfx.getId(name + '[type_code]', sourceFrame).value  = '';
    dfx.getId(safeName + '_label', sourceFrame).value   = label;
    dfx.getId(safeName + '_assetid', sourceFrame).value = assetid;

}//end asset_finder_clear_btn_press()


/**
 * Handler for changing the assetid textbox of an asset finder.
 *
 * @param {String}   name         The prefix for asset finder name attributes.
 * @param {String}   safeName     The prefix for asset finder ID attributes.
 * @param {String}   typeCodes    Asset type restriction. Currently unused.
 * @param {Function} doneCallback Callback to be fired after the change.
 * @param {String}   assetid      The entered asset ID.
 */
function asset_finder_assetid_changed(name, safeName, typeCodes, doneCallback, assetid)
{
    var sourceFrame = JS_Asset_Map.getUseMeFrame().document;
    var assetidBox = dfx.getId(name + '[assetid]', sourceFrame);
    assetidBox.value = assetid;

    if (dfx.isFn(doneCallback) === true) {
        doneCallback.call(assetidBox, assetid);
    }

}//end asset_finder_assetid_changed()


/**
 * Reload assets as requested by other parts of Matrix.
 *
 * Replaces the polling in the old Java asset map.
 */
function reload_assets(assetids)
{
    if (dfx.isArray(assetids) === false) {
        assetids = assetids.split('|');
    }

    JS_Asset_Map.addToRefreshQueue(assetids);

}//end reload_assets()
