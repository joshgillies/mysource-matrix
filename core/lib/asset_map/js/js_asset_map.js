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
* $Id: js_asset_map.js,v 1.1.2.79 2013/07/17 07:01:45 lwright Exp $
*
*/

/**
 * JS_Asset_Map
 *
 * Purpose
 *    JavaScript version of the Asset Map.
 *    A more modern JavaScript version of the Asset Map, intended to compliment
 *    and eventually replace the original Java asset map.
 *
 *    Required browser versions:
 *    IE 8 or later, recent versions of Firefox, Chrome or Safari.
 *    Earlier versions of IE and other browsers should revert back to the
 *    Java asset map.
 *
 * @author  Luke Wright <lwright@squiz.net>
 * @version $Revision: 1.1.2.79 $
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

    var AssetActions = {
        GetUrl: 'get url',
        Move: 'move asset',
        NewLink: 'new link',
        Clone: 'clone'
    }

    var KeyCode = {
        Delete: 46,
        Escape: 27
    }

    /**
     * The target element where the asset map will be drawn
     * @var {Node}
     */
    var assetMapContainer = null;

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
     * The trash folder assetid.
     *
     * Save the trash folder ID so even if we are teleported out of view, we
     * know where to go when the DEL button is pressed.
     *
     * @var {String}
     */
    var trashFolder = '';

    /**
     * Asset type cache
     * @var {Object}
     */
    var assetTypeCache = {};

    /**
     * Asset category cache.
     *
     * Empty category is category name '_EMPTY_'.
     *
     * @var {Object}
     */
    var assetCategories = {};

    /**
     * List of parents of asset types, keyed by child type.
     * @var {Object}
     */
    var assetTypeParents = {};

    /**
     * The current tree ID (zero-based).
     * @var {Number}
     */
    var currentTreeid = 0;

    /**
     * Hash of timeouts/intervals.
     * @var {Object}
     */
    var timeouts = {};

    /**
     * Denotes the last created asset type, so the add menu can show it.
     * @var {String}
     */
    var lastCreatedType = null;

    /**
     * The last actually-clicked asset.
     *
     * Used primarily for block-selection using SHIFT+click.
     *
     * @var {Node}
     */
    var lastSelection = null;

    /**
     * List of trees.
     *
     * By default, this will be an array of no more than two
     * trees, although it is possible to support more.
     *
     * @var {Array}
     */
    var trees = [];

    /**
     * Tracks the status of "Use Me" (ie. asset finder) mode.
     *
     * If this is null, Use Me mode is not active.
     *
     * @var {Object}
     * @property {String}   namePrefix     The prefix for name attributes in the
     *                                     asset finder that activated Use Me mode.
     * @property {String}   idPrefix       The prefix for ID attributes in that
     *                                     asset finder.
     * @property {Boolean}  closeWhenDone  Set to true to close the asset finder when
     *                                     either cancelled or selected. Default when
     *                                     omitted is false. Enable this in Simple Edit
     *                                     mode or when initially hidden in Admin.
     * @property {Array}    [typeFilter]   The restriction on types that can be
     *                                     selected.
     * @property {Function} [doneCallback] When selection occurs, run this function.
     */
    var useMeStatus = null;

    /**
     * Tracks the status of a drag.
     *
     * There are two types of drag possible:
     * - If the drag starts upon an asset (either the name or the icon), then it is
     *   interpreted as dragging the current selection, which could be more than one
     *   asset. A drag will only be interpreted as one if the drop target becomes
     *   somewhere other than the initially dragged asset.
     *
     * - if the drag starts outside an asset, then it will be interpreted as a
     *   rectangular selection. Assets that are at least partially covered by the
     *   rectangle will be selected.
     *   NOTE: in the Java asset map, a rectangular selection, once selecting one or
     *   more assets, could not return to the situation of selecting zero assets (it
     *   would leave the last selected asset selected). This asset map will deselect
     *   all assets if nothing is selected.
     *
     * When no drag is active, this will be null.
     * When a drag is active, startPoint will be filled, and either "assetDrag" or
     * "selectionDrag" will be available.
     *
     * @property {Object} startPoint   The coordinates of the starting point.
     * @property {Number} startPoint.x The clientX of the starting point.
     * @property {Number} startPoint.y The clientY of the starting point.
     *
     * @property {Object} [assetDrag] Presence indicates asset drag is active.
     * @property {Node}   assetDrag.dragSource The asset actually dragged.
     * @property {Array.<Node>} assetDrag.selection The assets selected on
     *                                              drag start.
     *
     * @property {Object} [selectionDrag] Presence indicates selection drag active.
     * @property {Array.<Node>} selectionDrag.selection The assets currently selected.
     */
    var dragStatus = null;


    /**
     * List of assets to refresh, sent from elsewhere in the Matrix system.
     *
     * This will get processed every 2 seconds if it contains something, similar to
     * the Java asset map. It does this rather than doing it for each request because
     * a HIPO processing a large number of assets (for instance, one that cascades a
     * status change throughout a whole site) would make multiple requests to update
     * a single asset (enforced by Matrix's event system) - particularly if a large
     * HIPO threshold is set. This allows some form of batching.
     *
     * These refreshes should be for the asset itself, not their whole tree. If the
     * asset is shown at multiple places in the tree (whether or not it has been
     * re-collapsed), it should be updated in all places, and all trees.
     *
     * @var {Array.<String>}
     */
    var refreshQueue = [];


//--        UTILITY FUNCTIONS        --//

    /**
     * Unique IDs for certain DIVs, so we can individually reference them.
     *
     * We can't use asset IDs or link IDs for this purpose, as they are not unique.
     * Better that we can target an individual ID for testing purposes.
     *
     * @var {Object}
     */
    var _uniqueElIds = {
        asset: 0,
        childIndent: 0
    }

    /**
     * Create an element with optional unselectable attribute set on (for IE<=9).
     *
     * This is required because unselectable="on" does not cascade, it must be set
     * on all elements that require it (unlike user-select in CSS).
     *
     * On those browsers that support CSS, user-select: none is the default for the
     * asset map, so if something is selectable we *add* the ability to select it
     * using a class.
     *
     * @param {String}  tagName            The name of the tag to create.
     * @param {Boolean} [selectable=false] Whether the text should be selectable.
     *
     * @param {Node}
     */
    var _createEl = function(tagName, selectable) {
        var el = assetMapContainer.ownerDocument.createElement(tagName);

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
        _uniqueElIds.childIndent++;
        var container = _createEl('div');
        container.id  = 'child-indent-uid-' + encodeURIComponent(_uniqueElIds.childIndent);
        container.setAttribute('data-parentid', parentid);
        dfx.addClass(container, 'for-asset-' + encodeURIComponent(parentid));
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
        var retval = js_translate('unknown');
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
     * Returns a div representing a single asset node in the tree, including branch
     * and expand/collapse element, based on what is set in the passed asset JSON.
     * It's the responsibility of the caller to spot the asset node in the DOM
     * at the place it wants.
     *
     * Pass the _attributes subelement of an asset to this function.
     *
     * @param {Object} assetAttrs The asset attributes, as returned from JSON.
     */
    var _formatAsset = function(assetAttrs) {
        var assetid    = assetAttrs.assetid;
        var name       = assetAttrs.name;
        var typeCode   = assetAttrs.type_code;
        var status     = Number(assetAttrs.status);

        if (assetAttrs.sort_order === undefined) {
            assetAttrs.sort_order = -1;
        }
        var sortOrder  = Number(assetAttrs.sort_order);

        if (assetAttrs.num_kids === undefined) {
            assetAttrs.num_kids = 0;
        }
        var numKids    = Number(assetAttrs.num_kids);

        if (assetAttrs.linkid === undefined) {
            assetAttrs.linkid = '';
            assetAttrs.asset_path = '';
            assetAttrs.link_path = '';
        }
        var linkid     = assetAttrs.linkid;

        if (assetAttrs.link_type === undefined) {
            assetAttrs.link_type = LinkType.Type1;
        }
        var linkType   = Number(assetAttrs.link_type);

        if (assetAttrs.accessible === undefined) {
            assetAttrs.accessible = 1;
        }
        var accessible = Number(assetAttrs.accessible);

        var assetLine = _createEl('div');

        _uniqueElIds.asset++;
        assetLine.id = 'asset-uid-' + encodeURIComponent(_uniqueElIds.asset);

        dfx.addClass(assetLine, 'asset');
        assetLine.setAttribute('data-assetid', assetid);
        assetLine.setAttribute('data-asset-path', assetAttrs.asset_path);
        assetLine.setAttribute('data-linkid', linkid);
        assetLine.setAttribute('data-link-path', assetAttrs.link_path);
        assetLine.setAttribute('data-sort-order', sortOrder);
        assetLine.setAttribute('data-typecode', typeCode);

        if (assetTypeCache[typeCode]) {
            assetLine.setAttribute('title', js_translate('asset_type_id', assetTypeCache[typeCode].name, assetid));
        } else {
            assetLine.setAttribute('title', js_translate('asset_type_id', js_translate('unknown_asset_type'), assetid));
        }

        var leafSpan = _createEl('span');
        dfx.addClass(leafSpan, 'leaf');

        var iconSpan = _createEl('span');
        dfx.addClass(iconSpan, 'icon');
        if (typeCode !== '') {
            iconSpan.setAttribute('style', 'background-image: url(' + options.assetIconPath + '/' + typeCode + '/icon.png)');
        }

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
     * Reserved - may be used in future to test cases where knowing ancestor types
     * is useful (for instance in disallowing assets to link to each other).
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
     * @param {Object} startOptions
     */
    this.start = function(startOptions) {
        var self = this;

        options              = startOptions;
        assetMapContainer    = options.targetElement || dfx.getId('asset_map_container');
        options.teleportRoot = options.teleportRoot  || '1';
        options.simple       = false;

        if (options.initialSelection !== '') {
            var selParts = options.initialSelection.split('~');
            options.initialSelection = {
                assetids: selParts[0].split('|'),
                sortOrders: selParts[1].split('|')
            };
        } else {
            options.initialSelection = null;
        }

        // Draw two trees only.
        this.drawToolbar();
        var containers = [
            this.drawTreeContainer(0),
            this.drawTreeContainer(1)
        ];
        this.drawStatusList();
        this.drawMessageLine();

        this.resizeTree();

        this.message(js_translate('asset_map_status_bar_init'), true);
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

                if (((typeinfo['_attributes']['instantiable'] !== '0')) &&
                    (typeinfo['_attributes']['allowed_access'] !== 'system')) {
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

            // Initialising gives us the root folder's immediate children.
            var assets    = response['assets'][0]['asset'];
            for (var i = 0; i < assets[0]['asset'].length; i++) {
                var asset = assets[0]['asset'][i];
                if (asset._attributes.type_code === 'trash_folder') {
                    trashFolder = asset._attributes.assetid;
                }
            }

            self.teleport(options.teleportRoot, null, 0, function() {
                // If an initial selection is passed, try to locate the current
                // asset from the URL.
                if (options.initialSelection) {
                    self.locateAsset(
                        options.initialSelection.assetids,
                        options.initialSelection.sortOrders
                    );
                }
            });
            self.teleport(options.teleportRoot, null, 1);

            self.drawTreeList();
            self.selectTree(0);
            self.initEvents();

            self.message(js_translate('asset_map_status_bar_success'), false, 2000);
        });
    };

    /**
     * Start the simple asset map.
     *
     * @param {Object} options
     */
    this.startSimple = function(startOptions) {
        var self = this;

        options              = startOptions;
        assetMapContainer    = options.targetElement || dfx.getId('asset_map_container');
        options.teleportRoot = options.teleportRoot  || '1';
        options.simple       = true;
        dfx.addClass(assetMapContainer, 'simple');

        // Simple asset map is one tree only, and the toolbar has no add menu.
        this.drawToolbar(false);
        this.drawTreeContainer(0);
        this.drawMessageLine();

        this.resizeTree();

        this.message(js_translate('asset_map_status_bar_init'), true);
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

                if (((typeinfo['_attributes']['instantiable'] !== '0')) &&
                    (typeinfo['_attributes']['allowed_access'] !== 'system')) {
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

            // Initialising gives us the root folder's immediate children.
            var assets    = response['assets'][0]['asset'];
            for (var i = 0; i < assets[0]['asset'].length; i++) {
                var asset = assets[0]['asset'][i];
                if (asset._attributes.type_code === 'trash_folder') {
                    trashFolder = asset._attributes.assetid;
                }
            }

            self.teleport(options.teleportRoot, null, 0);
            self.selectTree(0);
            self.initEvents();

            self.message(js_translate('asset_map_status_bar_success'), false, 2000);
        });
    };

    /**
     * Initialise events.
     *
     * @param {Object} options
     */
    this.initEvents = function() {
        var document = assetMapContainer.ownerDocument;
        var trees    = dfx.getClass('tree', assetMapContainer);
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
            self.teleport(options.teleportRoot, null);
        });

        dfx.addEvent(dfx.getId('asset_map_button_statuses'), 'click', function() {
            dfx.toggleClass(assetMapContainer, 'statuses-shown');
        });

        dfx.addEvent(dfx.getId('asset_map_button_collapse'), 'click', function() {
            var childIndents  = dfx.getClass('childIndent', assetMapContainer);
            var branchButtons = dfx.getClass('branch-status', assetMapContainer);

            dfx.addClass(childIndents, 'collapsed');
            dfx.removeClass(branchButtons, 'expanded');
        });

        dfx.addEvent(assetMapContainer, 'contextmenu', function(e) {
            e.preventDefault();
        });

        // Set this to keyup - Webkit does not emit keypress on non-printable keys,
        // similar IE.
        dfx.addEvent(assetMapContainer.ownerDocument.body, 'keyup', function(e) {
            var code = e.keyCode ? e.keyCode : e.which;
            switch (code) {
                case KeyCode.Delete:
                    if (options.simple === false) {
                        var msg       = '';
                        var title     = '';
                        var selection = self.currentSelection();

                        if (selection.length !== 0) {
                            if (selection.length > 1) {
                                msg   = js_translate('asset_map_confirm_move_children', selection.length);
                                title = js_translate('trash_assets');
                            } else if (selection.length === 1) {
                                msg   = js_translate('asset_map_confirm_move_child', dfx.getNodeTextContent(dfx.getClass('assetName', selection[0])[0]));
                                title = js_translate('trash_asset');
                            }

                            self.confirmPopup(msg, title, function() {
                                // Moving to trash.
                                self.moveAsset(AssetActions.Move, selection, trashFolder, -1);
                            });
                        }//end if
                    }
                break;

                case KeyCode.Escape:
                    dfx.remove(dfx.getClass('dragAsset', assetMapContainer));
                    dragStatus = null;
                    self.moveMe.cancel();
                break;
            }//end switch
        });

        dfx.addEvent(assetMapContainer, 'mousedown', function(e) {
            self.clearMenus();
        });

        for (var i = 0; i < trees.length; i++) {
            this.initTreeEvents(trees[i]);
        }
    };

    this.initTreeEvents = function(tree) {
        var self     = this;

        dfx.addEvent(tree, 'mousedown', function(e) {
            if (dragStatus) {
                // Don't handle overlapping button clicks.
                return true;
            }

            self.clearLocatedAssets();

            var which       = e.which;
            var allButtons  = e.buttons;
            if (!allButtons) {
                // IE<9 uses non-W3C method of detecting held buttons.
                allButtons = e.button;
            }

            if ((which === 3) && ((allButtons & 2) === 0)) {
                // RMB registered due to a ctrl-click (in eg. Firefox/Mac).
                // Change to a left click and consider it like a ctrl-click
                // in other platforms.
                which = 1;
            }

            dragStatus = {
                button: which,
                startPoint: {
                    x: e.clientX,
                    y: e.clientY
                }
            };

            var target       = e.target;
            var assetTarget  = null;
            var branchTarget = null;
            while (target && !assetTarget && !branchTarget) {
                if ((dfx.hasClass(target, 'assetName') === true) ||
                       (dfx.hasClass(target, 'icon') === true)) {
                    if (dfx.hasClass(target.parentNode, 'asset') === true) {
                        assetTarget = target.parentNode;
                    }
                } else if (dfx.hasClass(target, 'branch-status') === true) {
                    if (dfx.hasClass(target.parentNode, 'asset') === true) {
                        branchTarget = target.parentNode;
                    }
                }
                target = target.parentNode;
            }

            if (assetTarget) {
                e.stopImmediatePropagation();
                var assetTargetCoords = dfx.getElementCoords(assetTarget);
                if (self.isInUseMeMode() === true) {
                    // Use me mode. No multi-select, no drag.
                    // Either mouse button should be a single asset selection.
                    if ((which === 1) || (which === 3)) {
                        self.clearSelection();
                        dfx.toggleClass(assetTarget, 'selected');

                        if (which === 3) {
                            var menu = self.drawUseMeMenu(assetTarget);
                            self.positionMenu(menu, dragStatus.startPoint);
                        }
                        e.stopImmediatePropagation();
                    }
                } else {
                    if (which === 3) {
                        // Right mouse button. No drag, no multi-select, but
                        // preserve existing selections.
                        if (dfx.hasClass(assetTarget, 'selected') === false) {
                            self.clearSelection();
                            dfx.toggleClass(assetTarget, 'selected');
                            lastSelection = assetTarget;
                        }

                        if (options.simple === false) {
                            var selection = self.currentSelection();
                            if (selection.length > 1) {
                                // Multiple selection. Show move/clone/new-link menu.
                                var menu = self.drawMultiSelectMenu(selection);
                            } else {
                                // Single selection. Show screens menu.
                                var menu = self.drawScreensMenu(assetTarget);
                            }//end if (multiple selection)

                            self.positionMenu(menu, dragStatus.startPoint);
                        }
                    } else if (which === 1) {
                        if ((e.shiftKey === true) && (options.simple === false)) {
                            var selection = self.currentSelection();
                            if (selection.length > 0) {
                                // dfx.getElementsBetween best works in a forward
                                // direction. Work out which way that is.
                                if (lastSelection.compareDocumentPosition) {
                                    // IE9+ and other browsers.
                                    // A bit field of 0x04 indicates that the argument
                                    // follows the reference node.
                                    var docPos = lastSelection.compareDocumentPosition(assetTarget);
                                    var forwardSelection = (docPos & 0x04) > 0;
                                } else {
                                    // Make IE8 happy using sourceIndex.
                                    var forwardSelection = (assetTarget.sourceIndex > lastSelection.sourceIndex);
                                }

                                var between = [];
                                if (forwardSelection) {
                                    between = dfx.getElementsBetween(lastSelection, assetTarget);
                                } else {
                                    between = dfx.getElementsBetween(assetTarget, lastSelection);
                                }

                                console.info(between);
                                between.push(assetTarget);
                                for (var i = 0; i < between.length; i++) {
                                    if (dfx.hasClass(between[i], 'asset') === true) {
                                        if (e.ctrlKey === true) {
                                            dfx.toggleClass(between[i], 'selected');
                                        } else {
                                            dfx.addClass(between[i], 'selected');
                                        }
                                    }//end if
                                }//end for
                                e.preventDefault();
                            } else {
                                dfx.addClass(assetTarget, 'selected');
                            }//end if
                        } else if ((e.ctrlKey === true) && (options.simple === false)) {
                            // Control-left click. No drag, toggle selection of clicked asset.
                            dfx.toggleClass(assetTarget, 'selected');
                        } else {
                            // Left click. Possible drag. If clicked asset is already selected,
                            // maintain current selection, otherwise deselect all previous
                            // selection and select this one.
                            if (dfx.hasClass(assetTarget, 'selected') === false) {
                                self.clearSelection();
                                dfx.addClass(assetTarget, 'selected');
                            }

                            if (options.simple === false) {
                                dragStatus.assetDrag = {
                                    initialAsset: assetTarget,
                                    selection: self.currentSelection(),
                                    offset: {
                                        x: assetTargetCoords.x - dragStatus.startPoint.x,
                                        y: assetTargetCoords.y - dragStatus.startPoint.y
                                    }
                                }
                            }
                        }//end if (ctrl-click)

                        lastSelection = assetTarget;
                    }//end if (use me mode)
                }//end if (asset target)
            } else if (branchTarget) {
                // Clicked the expand/collapse button.
                self.expandAsset(branchTarget);
                e.stopImmediatePropagation();
            } else {
                if (options.simple === false) {
                    if (which === 1) {
                        dragStatus.selectionDrag = {
                            selection: []
                        }
                    } else if (which === 3) {
                        var menu = self.drawAddMenu();
                        self.positionMenu(menu, {x: e.clientX, y: e.clientY});
                        e.stopImmediatePropagation();
                    }
                }
            }//end if (type of target)
        });

        dfx.addEvent(assetMapContainer, 'mousemove', function(e) {
            var target     = dfx.getMouseEventTarget(e);
            var insideTree = null;
            if (dfx.hasClass(target, 'tree') === true) {
                insideTree = target;
            } else {
                insideTree = dfx.getParents(target, '.tree')[0];
            }

            if (dragStatus) {
                var assetMapCoords = dfx.getElementCoords(assetMapContainer);
                var mousePos       = dfx.getMouseEventPosition(e);
                if (dragStatus.selectionDrag) {
                    if (insideTree) {
                        if (!timeouts.selectionDrag) {
                            timeouts.selectionDrag = setInterval(function() {
                                var rectDims = dfx.getBoundingRectangle(selectionRect);
                                var assets   = dfx.getClass('asset', tree);
                                for (var i = 0; i < assets.length; i++) {
                                    var asset    = assets[i];
                                    var iconDims = dfx.getBoundingRectangle(dfx.getClass('icon', asset)[0]);
                                    var nameDims = dfx.getBoundingRectangle(dfx.getClass('assetName', asset)[0]);

                                    // Work out if this asset is being touched by the selection.
                                    if ((rectDims.x2 < iconDims.x1) || (rectDims.x1 > nameDims.x2)) {
                                        dfx.removeClass(asset, 'selected');
                                    } else if ((rectDims.y2 < iconDims.y1) || (rectDims.y1 > iconDims.y2)) {
                                        dfx.removeClass(asset, 'selected');
                                    } else {
                                        dfx.addClass(asset, 'selected');
                                    }
                                }
                            }, 40);
                        }

                        var selectionRect = dfx.getClass('selectionRect', assetMapContainer)[0];
                        if (!selectionRect) {
                            var selectionRect = _createEl('div');
                            dfx.addClass(selectionRect, 'selectionRect');
                            assetMapContainer.appendChild(selectionRect);
                        }

                        dfx.setCoords(
                            selectionRect,
                            Math.min(e.clientX, dragStatus.startPoint.x) - assetMapCoords.x,
                            Math.min(e.clientY, dragStatus.startPoint.y) - assetMapCoords.y
                        );
                        dfx.setStyle(selectionRect, 'height', (Math.abs(e.clientY - dragStatus.startPoint.y) + 1) + 'px');
                        dfx.setStyle(selectionRect, 'width', (Math.abs(e.clientX - dragStatus.startPoint.x) + 1) + 'px');

                        dfx.addEvent(selectionRect, 'mousemove', function(e) {
                            dfx.setCoords(
                                selectionRect,
                                Math.min(e.clientX, dragStatus.startPoint.x) - assetMapCoords.x,
                                Math.min(e.clientY, dragStatus.startPoint.y) - assetMapCoords.y
                            );
                            dfx.setStyle(selectionRect, 'height', (Math.abs(e.clientY - dragStatus.startPoint.y) + 1) + 'px');
                            dfx.setStyle(selectionRect, 'width', (Math.abs(e.clientX - dragStatus.startPoint.x) + 1) + 'px');
                        });
                    }//end if
                } else if (dragStatus.assetDrag) {
                    dragStatus.currentPoint = {
                        x: mousePos.x - assetMapCoords.x + dragStatus.assetDrag.offset.x,
                        y: mousePos.y - assetMapCoords.y + dragStatus.assetDrag.offset.y,
                    }

                    if (self.isInUseMeMode() === false) {
                        var selection = self.currentSelection();

                        if (self.moveMe.isActive() === false) {
                            self.moveMe.enable(selection);
                        }

                        var dragAsset = dfx.getClass('dragAsset', assetMapContainer)[0];
                        if (!dragAsset) {
                            var dragAsset = _createEl('div');
                            dfx.addClass(dragAsset, 'dragAsset');
                            assetMapContainer.appendChild(dragAsset);

                            if (selection.length > 1) {
                                // _formatAsset with base asset icon and "2 assets", eg.
                                var assetAttrs = {
                                    assetid: 0,
                                    name: js_translate('%s_assets', selection.length),
                                    type_code: '',
                                    status: 0

                                }
                                dfx.addClass(dragAsset, 'multiple');
                            } else {
                                // _formatAsset with asset details
                                var assetAttrs = {
                                    assetid: selection[0].getAttribute('data-assetid'),
                                    name: dfx.getNodeTextContent(dfx.getClass('assetName', selection[0])[0]),
                                    type_code: selection[0].getAttribute('data-typecode'),
                                    status: 0
                                }
                            }//end if

                            var formattedAsset = _formatAsset(assetAttrs);
                            dragAsset.appendChild(formattedAsset);
                            formattedAsset.removeAttribute('title');
                            dfx.remove(dfx.getClass('leaf', formattedAsset));
                            dfx.remove(dfx.getClass('branch-status', formattedAsset));

                            dfx.addEvent(dragAsset, 'mousemove', function(e) {
                                // We moved but not enough to move off the draggable.
                                var mousePos = dfx.getMouseEventPosition(e);
                                dragStatus.currentPoint.x = mousePos.x - assetMapCoords.x + dragStatus.assetDrag.offset.x;
                                dragStatus.currentPoint.y = mousePos.y - assetMapCoords.y + dragStatus.assetDrag.offset.y;

                                dfx.setStyle(dragAsset, 'left', (dragStatus.currentPoint.x) + 'px');
                                dfx.setStyle(dragAsset, 'top', (dragStatus.currentPoint.y) + 'px');

                                // We need to determine what's underneath the
                                // draggable, too, to set the Move Me mode's pointer.
                                dfx.setStyle(dragAsset, 'display', 'none');
                                var underlyingEl = assetMapContainer.ownerDocument.elementFromPoint(mousePos.x, mousePos.y);
                                if (dfx.hasClass(underlyingEl, 'tab') === true) {
                                    var hoverTreeid = underlyingEl.getAttribute('data-treeid');
                                    self.setHoverTab(hoverTreeid, function(treeid) {
                                        self.selectTree(treeid);
                                    });
                                } else {
                                    self.clearHoverTab();
                                }

                                if (dfx.hasClass(underlyingEl, 'asset') === false) {
                                    underlyingEl = dfx.getParents(underlyingEl, '.asset')[0];
                                }
                                if (underlyingEl && (dfx.getClass('branch-status', underlyingEl).length > 0) &&
                                    (dfx.getClass('expanded', underlyingEl).length === 0)) {
                                    var hoverAssetid = underlyingEl.getAttribute('data-assetid');
                                    self.setHoverAsset(hoverAssetid, function(assetid) {
                                        self.expandAsset(underlyingEl);
                                    });
                                } else {
                                    self.clearHoverAsset();
                                }

                                self.moveMe.updatePosition(underlyingEl, mousePos);
                                dfx.setStyle(dragAsset, 'display', 'block');
                                e.stopImmediatePropagation();
                            });
                        }//end if (draggable exists)

                        // We moved far enough between events that we're not on the
                        // draggable anymore.
                        var underlyingEl = assetMapContainer.ownerDocument.elementFromPoint(mousePos.x, mousePos.y);
                        dfx.setStyle(dragAsset, 'left', dragStatus.currentPoint.x + 'px');
                        dfx.setStyle(dragAsset, 'top', dragStatus.currentPoint.y + 'px');

                        if (dfx.hasClass(underlyingEl, 'asset') === false) {
                            underlyingEl = dfx.getParents(underlyingEl, '.asset')[0];
                        }
                        if (underlyingEl && (dfx.getClass('branch-status', underlyingEl).length > 0) &&
                            (dfx.getClass('expanded', underlyingEl).length === 0)) {
                            var hoverAssetid = underlyingEl.getAttribute('data-assetid');
                            self.setHoverAsset(hoverAssetid, function(assetid) {
                                self.expandAsset(underlyingEl);
                            });
                        } else {
                            self.clearHoverAsset();
                        }
                    }
                }//end if
            }//end if
        });

        dfx.addEvent(dfx.getClass('tab', assetMapContainer), 'mouseenter', function(e) {
            if (dragStatus && dragStatus.assetDrag) {
                var assetMapCoords = dfx.getElementCoords(assetMapContainer);
                var target         = dfx.getMouseEventTarget(e);
                var hoverTreeid    = target.getAttribute('data-treeid');
                self.setHoverTab(hoverTreeid, function(treeid) {
                    self.selectTree(treeid);
                });

                var dragAsset = dfx.getClass('dragAsset', assetMapContainer)[0];
                var mousePos = dfx.getMouseEventPosition(e);
                dragStatus.currentPoint = {
                    x: mousePos.x - assetMapCoords.x + dragStatus.assetDrag.offset.x,
                    y: mousePos.y - assetMapCoords.y + dragStatus.assetDrag.offset.y,
                }
                dfx.setStyle(dragAsset, 'left', dragStatus.currentPoint.x + 'px');
                dfx.setStyle(dragAsset, 'top', dragStatus.currentPoint.y + 'px');
            }
        });

        dfx.addEvent(dfx.getClass('tab', assetMapContainer), 'mouseleave', function(e) {
            self.clearHoverTab();
        });

        dfx.addEvent(assetMapContainer, 'mouseup', function(e) {
            var mousePos = dfx.getMouseEventPosition(e);
            var menu     = null;

            dfx.remove(dfx.getClass('dragAsset', assetMapContainer));
            if (dragStatus) {
                if (dragStatus.selectionDrag) {
/*
                    dfx.removeEvent(dfx.getClass('asset', assetMapContainer), 'mouseover.rectSelection');
                    dfx.removeEvent(dfx.getClass('asset', assetMapContainer), 'mouseout.rectSelection');
*/
                    var selectionRect = dfx.getClass('selectionRect', assetMapContainer);
                    if (selectionRect) {
                        dfx.remove(selectionRect);
                    }

                    var dragAsset = dfx.getClass('dragAsset', assetMapContainer);
                    if (dragAsset) {
                        dfx.remove(dragAsset);
                    }
                    if (Math.abs(e.clientX - dragStatus.startPoint.x) < 2 &&
                        Math.abs(e.clientY - dragStatus.startPoint.y) < 2) {
                        // Treat as a click.
                        self.clearSelection();
                    }

                    clearInterval(timeouts.selectionDrag);
                    timeouts.selectionDrag = null;
                } else if (dragStatus.assetDrag) {
                    self.clearHoverAsset();
                    timeouts.assetDrag = null;

                    // If the draggable was moved two pixels or less in both
                    // directions, do not treat as a drag.
                    if (Math.abs(e.clientX - dragStatus.startPoint.x) > 2 ||
                        Math.abs(e.clientY - dragStatus.startPoint.y) > 2) {
                        // Work out our selection and show the dropdown menu.
                        if (self.moveMe.isActive()) {
                            if (self.moveMe.selection) {
                                var moveTarget = {
                                    source: self.moveMe.source,
                                    selection: self.moveMe.selection
                                };
                                var menu = self.drawMoveTargetMenu(moveTarget);
                                self.positionMenu(menu, mousePos);
                            }
                            self.moveMe.cancel();
                        }//end if (move me active)
                    }//end if (dragged by enough)

                    e.stopImmediatePropagation();
                }//end if
            }//end if

            dragStatus = null;
        });
    };


    this.expandAsset = function(branchTarget) {
        var self         = this;
        var assetid      = branchTarget.getAttribute('data-assetid');
        var linkid       = branchTarget.getAttribute('data-linkid');
        var assetPath    = branchTarget.getAttribute('data-asset-path');
        var linkPath     = branchTarget.getAttribute('data-link-path');
        var rootIndentId = 'childIndent.for-asset-' + encodeURIComponent(assetid);
        var container    = dfx.getClass(rootIndentId)[0];

        if (container) {
            dfx.toggleClass(dfx.getClass('branch-status', branchTarget), 'expanded');
            dfx.toggleClass(container, 'collapsed');
        } else {
            dfx.addClass(dfx.getClass('branch-status', branchTarget), 'expanded');

            var container = _createChildContainer(assetid);
            dfx.addClass(container, 'loading');
            container.innerHTML = js_translate('asset_map_loading_node');
            branchTarget.parentNode.insertBefore(container, branchTarget.nextSibling);

            // Loading.
            this.message(js_translate('asset_map_status_bar_requesting'), true);

            this.doRequest({
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
                    self.message(js_translate('asset_map_status_bar_loaded_children', 0), false, 2000);
                    dfx.remove(container);
                    dfx.remove(dfx.getClass('branch-status', branchTarget));
                } else {
                    container.innerHTML = '';
                    var assetCount = assets.asset.length;
                    assets._attributes.asset_path = assetPath;
                    assets._attributes.link_path  = linkPath;
                    self.drawTree(assets, container);

                    switch (assetCount) {
                        case 1:
                            self.message(js_translate('asset_map_status_bar_loaded_child'), false, 2000);
                        break;

                        default:
                            self.message(
                                js_translate('asset_map_status_bar_loaded_children', assetCount),
                                false,
                                2000
                            );
                        break;
                    }//end switch
                }//end if
            });
        }//end if (container exists)

    };

    this.setHoverTab = function(treeid, callback) {
        // Check whether we already have a timeout for this tree.
        if (timeouts.hoverTab) {
            if (timeouts.hoverTab.treeid !== treeid) {
                // A timeout exists for a different tree.
                this.clearHoverTab();
            }
        }

        // If we are now clear, then create the new timeout.
        if (!timeouts.hoverTab) {
            var timeout = setTimeout(function() {
                callback(treeid);
            }, 1000);
            timeouts.hoverTab = {
                timeout: timeout,
                treeid: treeid
            };
        }
    }


    this.clearHoverTab = function() {
        if (timeouts.hoverTab) {
            clearTimeout(timeouts.hoverTab.timeout);
        }
        timeouts.hoverTab = null;
    }


    this.setHoverAsset = function(assetid, callback) {
        // Check whether we already have a timeout for this asset.
        if (timeouts.hoverAsset) {
            if (timeouts.hoverAsset.assetid !== assetid) {
                // A timeout exists for a different asset.
                this.clearHoverAsset();
            }
        }

        // If we are now clear, then create the new timeout.
        if (!timeouts.hoverAsset) {
            var timeout = setTimeout(function() {
                callback(assetid);
            }, 1000);
            timeouts.hoverAsset = {
                timeout: timeout,
                assetid: assetid
            };
        }
    }


    this.clearHoverAsset = function() {
        if (timeouts.hoverAsset) {
            clearTimeout(timeouts.hoverAsset.timeout);
        }
        timeouts.hoverAsset = null;
    }


//--        CORE ACTIONS        --//


    /**
     * Get the currently selected tree element.
     *
     * @return {Node|Null}
     */
    this.getCurrentTreeElement = function() {
        var trees = dfx.getClass('tree.selected', assetMapContainer);

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
        var trees = dfx.getClass('tree', assetMapContainer);
        dfx.removeClass(trees, 'selected');
        dfx.addClass(trees[treeid], 'selected');

        var treeList = dfx.getClass('tree-list', assetMapContainer)[0];
        var tabs     = dfx.getClass('tab', assetMapContainer);
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
            var tree = dfx.getClass('tree', assetMapContainer)[treeid];
        }

        var assets = dfx.getClass('asset.selected', tree);
        return assets;
    };


    this.clearSelection = function(treeid) {
        if (treeid === undefined) {
            var tree = this.getCurrentTreeElement();
        } else {
            var tree = dfx.getClass('tree', assetMapContainer)[treeid];
        }

        dfx.removeClass(
            dfx.getClass('asset', tree),
            'selected located'
        );
        lastSelection = null;
    }
    this.clearLocatedAssets = function(treeid) {
        if (treeid === undefined) {
            var tree = this.getCurrentTreeElement();
        } else {
            var tree = dfx.getClass('tree', assetMapContainer)[treeid];
        }

        dfx.removeClass(
            dfx.getClass('asset', tree),
            'located'
        );
    }

    this.positionMenu = function(menu, mousePos) {
        var topDoc = this.topDocumentElement(assetMapContainer);

        topDoc.appendChild(menu);
        var elementHeight = topDoc.clientHeight;
        var submenuHeight = dfx.getElementHeight(menu);
        dfx.setStyle(
            menu,
            'left',
            (Math.max(10, mousePos.x) + 'px')
        );
        dfx.setStyle(
            menu,
            'top',
            (Math.min(
                elementHeight - submenuHeight - 10,
                mousePos.y
            ) + 'px')
        );
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
    this.teleport = function(assetid, linkid, treeid, callback) {
        var self = this;
        if (treeid === undefined) {
            var tree = this.getCurrentTreeElement();
        } else {
            var tree = dfx.getClass('tree', assetMapContainer)[treeid];
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
                    self.message(js_translate('asset_map_status_bar_loaded_children', 0), false, 2000);
                    dfx.remove(container);
                    dfx.remove(branchTarget);
                } else {
                    tree.innerHTML = '';
                    var assetCount = rootAsset.asset.length;
                    if (String(assetid) !== '1') {
                        rootAsset._attributes.name      = decodeURIComponent(
                            rootAsset._attributes.name.replace(/\+/g, '%20')
                        );
                        rootAsset._attributes.assetid   = decodeURIComponent(
                            rootAsset._attributes.assetid.replace(/\+/g, '%20')
                        );
                        rootAsset._attributes.type_code = decodeURIComponent(
                            rootAsset._attributes.type_code.replace(/\+/g, '%20')
                        );

                        assetLine = _formatAsset(rootAsset._attributes);

                        dfx.addClass(assetLine, 'teleported');
                        tree.appendChild(assetLine);
                    }//end if

                    rootAsset._attributes.asset_path = rootAsset._attributes.assetid;
                    rootAsset._attributes.link_path  = rootAsset._attributes.linkid;
                    self.drawTree(rootAsset, tree);

                    if (dfx.isFn(callback) === true) {
                        callback();
                    }

                    switch (assetCount) {
                        case 1:
                            self.message(js_translate('asset_map_status_bar_loaded_child'), false, 2000);
                        break;

                        default:
                            self.message(
                                js_translate('asset_map_status_bar_loaded_children', assetCount),
                                false,
                                2000
                            );
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
                action: AssetActions.GetUrl,
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
     * Move an asset(s) to a new parent.
     *
     * @param {String} assetid
     * @param {String} parentAssetid
     * @param {Number} [sortOrder] New sort order (last child if omitted)
     */
    this.moveAsset = function(action, assetNodes, newParentAssetid, sortOrder) {
        var self   = this;
        var assets = [];
        for (var i = 0; i < assetNodes.length; i++) {
            var parentid = assetNodes[i].parentNode.getAttribute('data-parentid');
            assets.push({
                _attributes: {
                    assetid: assetNodes[i].getAttribute('data-assetid'),
                    linkid: assetNodes[i].getAttribute('data-linkid'),
                    parentid: parentid
                }
            });
        }

        var command = {
            _attributes: {
                action: action,
                to_parent_assetid: newParentAssetid,
                to_parent_pos: sortOrder,
            },
            asset: assets
        };

        this.doRequest(command, function(response) {
            if (response.url) {
                for (var i = 0; i < response.url.length; i++) {
                    var redirURL = response.url[i]._content;
                    self.openHipoWindow(redirURL);
                }
            } else if (response._rootTag === 'error') {
                self.raiseError(response._content);
            } else {
                self.refreshTree();
            }
        });
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
        var document   = assetMapContainer.ownerDocument;

        var toolbarDiv = dfx.getClass('toolbar')[0];
        var messageDiv = dfx.getClass('messageLine')[0];
        var statusList = dfx.getClass('statusList')[0];

        // Only add the status height if it exists (ie. not in simple mode).
        var statusHeight = 0;
        if (statusList) {
            statusHeight = statusList.clientHeight;
        }

        var treeDivs = dfx.getClass('tree');
        assetMapContainer.style.height = (document.documentElement.clientHeight - 100) + 'px';
        for (var i = 0; i < treeDivs.length; i++) {
            treeDivs[i].style.height = (assetMapContainer.clientHeight - toolbarDiv.clientHeight - messageDiv.clientHeight - statusHeight) + 'px';
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
        var spinnerDiv       = dfx.getClass(
            'spinner',
            dfx.getClass(
                'messageLine',
                assetMapContainer
            )
        )[0];
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
    this.confirmPopup = function(message, title, yesCallback, noCallback) {
        var confirmDiv = _createEl('div');
        dfx.addClass(confirmDiv, 'confirmPopup');

        var titleDiv = _createEl('div');
        dfx.addClass(titleDiv, 'confirmTitle');
        titleDiv.innerHTML = title;

        // Body text should be selectable so it can be copy+pasted for
        // support purposes.
        var bodyDiv = _createEl('div', true);
        dfx.addClass(bodyDiv, 'confirmBody');
        bodyDiv.innerHTML = message;

        var bottomDiv = _createEl('div');
        dfx.addClass(bottomDiv, 'confirmBottom');

        var buttonYesDiv = _createEl('button');
        buttonYesDiv.innerHTML = js_translate('yes');

        var buttonNoDiv = _createEl('button');
        buttonNoDiv.innerHTML = js_translate('no');

        bottomDiv.appendChild(buttonYesDiv);
        bottomDiv.appendChild(buttonNoDiv);
        confirmDiv.appendChild(titleDiv);
        confirmDiv.appendChild(bodyDiv);
        confirmDiv.appendChild(bottomDiv);
        assetMapContainer.appendChild(confirmDiv);

        dfx.addEvent(buttonYesDiv, 'click', function() {
            dfx.remove(confirmDiv);
            if (dfx.isFn(yesCallback) === true) {
                yesCallback();
            }
        });

        dfx.addEvent(buttonNoDiv, 'click', function() {
            dfx.remove(confirmDiv);
            if (dfx.isFn(noCallback) === true) {
                noCallback();
            }
        });
    };
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
        assetMapContainer.appendChild(errorDiv);

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
        var defaultView = this.getDefaultView(target.ownerDocument);

        if ((!defaultView.frameElement) || (defaultView.frameElement.name === 'sq_sidenav')) {
            var topDoc = defaultView.top.document.documentElement;
        } else {
            var topDoc = target.ownerDocument.documentElement;
        }
        return topDoc;
    }


//--        DRAWING METHODS        --//


    /**
     * Draw toolbar.
     *
     * The add button is disabled in simple asset map mode - the other items in
     * the toolbar are still drawn.
     *
     * @param {Boolean} [drawAddButton=true] Draw the Add button.
     *
     * @returns {Node}
     */
    this.drawToolbar = function(drawAddButton) {
        var self = this;

        var container = _createEl('div');
        dfx.addClass(container, 'toolbar');
        assetMapContainer.appendChild(container);

        if (drawAddButton !== false) {
            var addButton = _createEl('div');
            dfx.addClass(addButton, 'addButton');
            container.appendChild(addButton);
            dfx.addEvent(addButton, 'click', function(e) {
                var target   = dfx.getMouseEventTarget(e);
                var mousePos = dfx.getMouseEventPosition(e);
                var menu     = self.drawAddMenu();
                self.topDocumentElement(target).appendChild(menu);
                dfx.setStyle(menu, 'left', (mousePos.x) + 'px');
                dfx.setStyle(menu, 'top', (mousePos.y) + 'px');
            });
        }

        var tbButtons = _createEl('div');
        dfx.addClass(tbButtons, 'tbButtons');
        container.appendChild(tbButtons);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_refresh';
        dfx.addClass(tbButton, 'tbButton');
        dfx.addClass(tbButton, 'refresh');
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', js_translate('asset_map_tooltip_refresh_all'));
        tbButtons.appendChild(tbButton);
        dfx.addEvent(tbButton, 'click', function(e) {
            self.refreshTree();
        });

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_restore';
        dfx.addClass(tbButton, 'tbButton');
        dfx.addClass(tbButton, 'restore');
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', js_translate('asset_map_tooltip_restore_root'));
        tbButtons.appendChild(tbButton);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_collapse';
        dfx.addClass(tbButton, 'tbButton');
        dfx.addClass(tbButton, 'collapse');
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', js_translate('asset_map_tooltip_collapse_all'));
        tbButtons.appendChild(tbButton);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_statuses';
        dfx.addClass(tbButton, 'tbButton');
        dfx.addClass(tbButton, 'statuses');
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', js_translate('asset_map_tooltip_toggle_status'));
        tbButtons.appendChild(tbButton);
    };


    /**
     * Draw a pagination tool.
     *
     * @param {String} direction     Direction of pagination ("up" or "down").
     * @param {Number} offset        Offset number of assets to start from.
     * @param {Number} [totalAssets] Total number of assets. Omit or "-1" for bridges.
     *
     * @returns {Node}
     */
    this.drawPaginationTool = function(direction, offset, totalAssets) {
        var pageDiv = _createEl('div');
        pageDiv.id  = 'asset_map_pagination_tool';

        if (direction === 'up') {
            var tb1Button = _createEl('div');
            dfx.addClass(tb1Button, 'page-button previous-page');

            var tb2Button = _createEl('div');
            dfx.addClass(tb2Button, 'page-button first-page');
        } else {
            var tb1Button = _createEl('div');
            dfx.addClass(tb1Button, 'page-button next-page');

            var tb2Button = _createEl('div');
            dfx.addClass(tb2Button, 'page-button last-page');
        }

        pageDiv.appendChild(tb1Button);
        pageDiv.appendChild(tb2Button);

        return pageDiv;
    }


    /**
     * Draw the list of possible statuses and their colours.
     *
     * @returns {Node}
     */
    this.drawStatusList = function() {
        var container = _createEl('div');
        dfx.addClass(container, 'statusList');
        assetMapContainer.appendChild(container);

        var divider = _createEl('div');
        divider.id        = 'asset_map_status_list_divider';
        dfx.addClass(divider, 'statusDivider');
        container.appendChild(divider);

        var dividerIcon = _createEl('div');
        dfx.addClass(dividerIcon, 'icon');
        divider.appendChild(dividerIcon);

        var dividerText = _createEl('span');
        dfx.addClass(dividerText, 'text');
        dividerText.innerHTML = js_translate('asset_map_status_colour_key');
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
        assetMapContainer.appendChild(container);

        var spinnerDiv = _createEl('div');
        dfx.addClass(spinnerDiv, 'spinner');
        container.appendChild(spinnerDiv);

        var messageDiv = _createEl('div');
        messageDiv.id        = 'asset_map_message';
        dfx.addClass(messageDiv, 'message');
        messageDiv.innerHTML = '';
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
        tree1.setAttribute('data-treeid', 0);
        tree1.innerHTML = js_translate('asset_map_tree1_name');
        treeList.appendChild(tree1);
        dfx.addEvent(tree1, 'click', function() {
            self.selectTree(0);
        });

        var tree2 = _createEl('div');
        dfx.addClass(tree2, 'tab');
        tree2.setAttribute('data-treeid', 1);
        tree2.innerHTML = js_translate('asset_map_tree2_name');
        treeList.appendChild(tree2);
        dfx.addEvent(tree2, 'click', function() {
            self.selectTree(1);
        });

        assetMapContainer.appendChild(treeList);
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
        assetMapContainer.appendChild(container);

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

            assetLine = _formatAsset(asset._attributes);
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

    /**
     * Replace occurrences of an asset in a tree with updated information.
     *
     * @param {Object} newAsset The new asset info.
     * @param {String} assetid  The asset ID to replace.
     * @param {Number} [linkid] Only nodes with this linkid will be updated.
     *                          (Omit to update all nodes with passed assetid.)
     */
    this.updateAsset = function(newAsset, assetid, linkid) {
    };

    this.processRefreshQueue = function() {
        var self = this;

        // Take a local copy of the refresh queue, and clear it.
        var processQueue = refreshQueue.concat([]);
        refreshQueue     = [];

        // Requests to be made. However, we are going to try and request zero children.
        var assetRequests = [];
        var treeRefresh   = [];

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
                var assetNodes = dfx.find(assetMapContainer, 'div.asset[data-assetid=' + assetid  + ']');
                for (var j = 0; j < assetNodes.length; j++) {
                    var assetNode     = assetNodes[j];
                    var newNode       = _formatAsset(thisAsset._attributes);
                    newNode.className = assetNode.className;

                    newNode.setAttribute('data-linkid', assetNode.getAttribute('data-linkid'));
                    newNode.setAttribute('data-asset-path', assetNode.getAttribute('data-asset-path'));
                    newNode.setAttribute('data-link-path', assetNode.getAttribute('data-link-path'));

                    assetNode.parentNode.replaceChild(newNode, assetNode);
                }//end for

                var expansions = dfx.find(assetMapContainer, '.childIndent[data-parentid="' + assetid + '"]');
                if (expansions.length > 0) {
                    treeRefresh.push(assetid);
                    for (var j = 0; j < expansions.length; j++) {
                        var parentid = expansions[j].getAttribute('data-parentid');
                        if (treeRefresh.inArray(parentid) === false) {
                            treeRefresh.push(parentid);
                        }
                    }//end for
                }//end if
            }//end for

            if (treeRefresh.length > 0) {
                for (var j = 0; j < treeRefresh.length; j++) {
                    self.refreshTree(treeRefresh[j]);
                }
            }
            self.message(js_translate('asset_map_status_bar_success'), false, 2000);
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
                if (dfx.hasClass(children[i], 'collapsed') === true) {
                    dfx.remove(children[i]);
                } else {
                    assetids.push(children[i].getAttribute('data-parentid'));
                }
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
                        var assetNode    = dfx.find(tree, 'div.asset[data-assetid=' + assetid  + ']')[0];
                        var branchButton = dfx.getClass('branch-status', assetNode);
                        dfx.addClass(branchButton, 'expanded');

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

                self.message(js_translate('asset_map_status_bar_success'), false, 2000);
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
                this.raiseError(js_translate('asset_map_error_locate_asset', savedAssets.pop()));
                return;
            } else {
                var assetLine = assetLines[0];
                if (assetids.length === 0) {
                    dfx.addClass(assetLine, 'selected');
                    lastSelection = assetLine;
                    assetLine.scrollIntoView(true);
                    this.getDefaultView(assetLine).top.scrollTo(0, 0);
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
                        lastSelection = assetLine;
                        assetLine.scrollIntoView(true);
                        this.getDefaultView(assetLine).top.scrollTo(0, 0);
                    }
                }

                self.message(js_translate('asset_map_status_bar_success'), false, 2000);
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


    this.moveMe = new function() {
        /**
         * Source of the move.
         *
         * May remain null if there is no asset source. This occurs when placing a
         * new asset created through the "Add" menu.
         *
         * @var {Node}
         */
        this.source = null;

        /**
         * Current selection.
         * @var {Node}
         */
        this.selection = null;


        /**
         * Callback to call after selecting a target.
         * @var {Function}
         */
        this.doneCallback = null;


        /**
         * The node that is used to mark the current selection of the Move Me mode.
         *
         * Null when inactive.
         *
         * @private
         * @var {Node}
         */
        var _lineEl = null;

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
        this.enable = function(source, callback) {
            source = source || null;

            var self = this;
            dfx.addClass(assetMapContainer, 'moveMeMode');
            this.source       = source;
            this.doneCallback = callback;
            this.selection    = null;

            _lineEl = _createEl('div');
            dfx.addClass(_lineEl, 'selectLine');
            assetMapContainer.appendChild(_lineEl);

            dfx.addEvent(dfx.getClass('tree', assetMapContainer), 'mousedown.moveMe', function(e) {
                if (self.selection) {
                    self.doneCallback.call(self, self.source, self.selection);
                }

                // if there's no valid target when they click, then that's too bad.
                self.cancel();
            });

            dfx.addEvent(dfx.getClass('tree', assetMapContainer), 'mousemove.moveMe', function(e) {
                var target = dfx.getMouseEventTarget(e);
                while (target) {
                    if (dfx.hasClass(target, 'asset') === true) {
                        break;
                    }
                    target = target.parentNode;
                }//end while

                if (target) {
                    var position = dfx.getMouseEventPosition(e);
                    self.updatePosition.call(self, target, position);
                } else {
                    dfx.removeClass(dfx.getClass('asset', assetMapContainer), 'moveTarget');
                    dfx.removeClass(_lineEl, 'active');
                    self.selection = null;
                }
            });
        };


        this.isActive = function() {
            var hasClass = dfx.hasClass(assetMapContainer, 'moveMeMode');
            return hasClass;
        }

        /**
         * Update the position of the selection line.
         *
         * The mouse position is required because we need it to determine whether
         * to highlight the asset or the "in-between" zone between assets. We treat
         * witihin 3 pixels of the top or bottom of an asset as selecting the space
         * between an asset and its sibling.
         *
         * @param {Node}      target     The asset being targeted by the mouse.
         * @param {Object}    mousePos   Mouse position.
         * @property {Number} mousePos.x X position of the mouse.
         * @property {Number} mousePos.y Y position of the mouse.
         */
        this.updatePosition = function(target, mousePos) {
            // Find the next closest parent.
            dfx.removeClass(dfx.getClass('asset', assetMapContainer), 'moveTarget');
            while (target) {
                if (dfx.hasClass(target, 'asset') === true) {
                    break;
                }
                target = target.parentNode;
            }//end while

            if (!target) {
                dfx.removeClass(_lineEl, 'active');
                this.selection = null;
                return;
            }

            dfx.addClass(_lineEl, 'active');
            var parentAsset  = dfx.getParents(target, '.childIndent')[0];
            if (parentAsset) {
                parentAsset = parentAsset.previousSibling;
            }

            var assetMapCoords = dfx.getElementCoords(assetMapContainer);
            var assetRect    = dfx.getBoundingRectangle(target);
            var fromTop      = mousePos.y - assetRect.y1;
            var fromBottom   = assetRect.y2 - mousePos.y + 1;

            var assetNameSpan = dfx.getClass('assetName', target)[0];
            var assetNameRect = dfx.getBoundingRectangle(assetNameSpan);

            this.selection = {
                parentid: 1,
                linkid: 1,
                before: -1
            };

            if (fromTop <= 3) {
                if (parentAsset) {
                    this.selection.parentid = parentAsset.getAttribute('data-assetid');
                    this.selection.linkid   = parentAsset.getAttribute('data-linkid');
                }

                this.selection.before = target.getAttribute('data-sort-order');
                dfx.setCoords(_lineEl, (assetNameRect.x1 - assetMapCoords.x), (assetRect.y1 - assetMapCoords.y));
            } else if (fromBottom <= 3) {
                if (parentAsset) {
                    this.selection.parentid = parentAsset.getAttribute('data-assetid');
                    this.selection.linkid   = parentAsset.getAttribute('data-linkid');
                }

                var insertBefore = target.nextSibling;
                if (insertBefore) {
                    this.selection.before = insertBefore.getAttribute('data-sort-order');
                }

                dfx.setCoords(_lineEl, (assetNameRect.x1 - assetMapCoords.x), (assetRect.y2 - assetMapCoords.y));
            } else {
                this.selection = {
                    parentid: target.getAttribute('data-assetid'),
                    linkid: target.getAttribute('data-linkid'),
                    before: -1
                };

                dfx.addClass(target, 'moveTarget');
                dfx.setCoords(_lineEl, (assetNameRect.x2 - assetMapCoords.x), (((assetRect.y1 + assetRect.y2) / 2) - assetMapCoords.y));
            }//end if
        };


        /**
         * Cancel "move me" mode.
         *
         */
        this.cancel = function() {
            dfx.removeClass(assetMapContainer, 'moveMeMode');
            dfx.remove(_lineEl);

            _lineEl           = null;
            this.source       = null;
            this.selection    = null;
            this.doneCallback = null;

            dfx.removeEvent(dfx.getClass('tree', assetMapContainer), 'mousedown.moveMe');
            dfx.removeEvent(dfx.getClass('tree', assetMapContainer), 'mousemove.moveMe');
        };
    };


//--        USE ME MODE        --//


    this.getUseMeFrame = function() {
        var win    = this.getDefaultView(assetMapContainer);
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

            dfx.addClass(assetMapContainer, 'useMeMode');
            useMeStatus = {
                namePrefix: name,
                idPrefix: safeName,
                typeFilter: typeFilter,
                doneCallback: doneCallback,
            };

            // toggle frame
            var thisFrame    = this.getDefaultView(assetMapContainer.ownerDocument).top.frames['sq_sidenav'];
            var resizerFrame = this.getDefaultView(assetMapContainer.ownerDocument).top.frames['sq_resizer'];
            if (thisFrame.frameElement.parentNode.style.display === 'none') {
                resizerFrame.toggleFrame();
                useMeStatus.closeWhenDone = true;
            }
            this.updateAssetsForUseMe();
        }//end if
    };


    /**
     * Cancel use me mode
     *
     */
    this.cancelUseMeMode = function() {
        if (useMeStatus && (useMeStatus.closeWhenDone === true)) {
            var resizerFrame = this.getDefaultView(assetMapContainer.ownerDocument).top.frames['sq_resizer'];
            resizerFrame.toggleFrame();
        }
        dfx.removeClass(assetMapContainer, 'useMeMode');
        useMeStatus = null;
        this.updateAssetsForUseMe();
    };


    /**
     * Update the enabled/disabled status for Use Me mode.
     */
    this.updateAssetsForUseMe = function(rootTree) {
        if (rootTree === undefined) {
            rootTree = dfx.getClass('tree', assetMapContainer);
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

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_useme'));
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
        var hasUseMe = dfx.hasClass(assetMapContainer, 'useMeMode');

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

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_teleport'), null);
        container.appendChild(menuItem);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.teleport(assetid, linkid);
        });

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_refresh'), null);
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
                var menuItem = this.drawMenuItem(js_translate('asset_map_menu_no_previous_child'), null);
                dfx.addClass(menuItem, 'disabled');
            } else {
                var menuItem = this.drawMenuItem(js_translate('asset_map_menu_new_previous', assetTypeCache[lastCreatedType].name), lastCreatedType);
                dfx.addEvent(menuItem, 'click', function(e) {
                    self.clearMenus();
                    self.addAsset(lastCreatedType, assetid, -1);
                });
            }
            container.appendChild(menuItem);

            var menuItem = this.drawMenuItem(js_translate('asset_map_menu_new_child'), null, true);
            container.appendChild(menuItem);

            dfx.addEvent(menuItem, 'mouseover', function(e) {
                if (timeouts.addTypeSubmenu) {
                    clearTimeout(timeouts.addTypeSubmenu);
                    timeouts.addTypeSubmenu = null;
                }
                e.stopPropagation();

                var target   = dfx.getMouseEventTarget(e);

                var existingMenu = dfx.getClass('assetMapMenu.addMenu', self.topDocumentElement(target));
                if (existingMenu.length === 0) {
                    var menu     = self.drawAddMenu(false, assetid);
                    self.topDocumentElement(target).appendChild(menu);
                    var elementHeight = self.topDocumentElement(assetMapContainer).clientHeight;
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
     * Draw move target menu.
     *
     * The move target menu pops up after an asset is dragged. It will allow
     * moving, re-linking or cloning, or cancelling.
     *
     * It should always be passed an array of nodes because of the possibility of
     * multi-asset drag.
     *
     * @param {Array.<Node>} assetNodes The node(s) that triggered the selection.
     *
     * @returns {Node}
     */
    this.drawMoveTargetMenu = function(moveTarget) {
        this.clearMenus();
        var self = this;
        var container = _createEl('div');
        dfx.addClass(container, 'assetMapMenu');
        dfx.addClass(container, 'multiMove');
        dfx.addEvent(container, 'contextmenu', function(e) {
            e.preventDefault();
        });

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_move_here'), null);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.moveAsset(AssetActions.Move, moveTarget.source, moveTarget.selection.parentid, moveTarget.selection.before);
        });
        container.appendChild(menuItem);

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_link_here'), null);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.moveAsset(AssetActions.NewLink, moveTarget.source, moveTarget.selection.parentid, moveTarget.selection.before);
        });
        container.appendChild(menuItem);

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_clone_here'), null);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.moveAsset(AssetActions.Clone, moveTarget.source, moveTarget.selection.parentid, moveTarget.selection.before);
        });
        container.appendChild(menuItem);

        var sep = this.drawMenuSeparator();
        container.appendChild(sep);

        var menuItem = this.drawMenuItem(js_translate('cancel'), null);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
        });
        container.appendChild(menuItem);

        return container;
    };

    /**
     * Draw multi-select menu.
     *
     * The multi-select menu gives users the option to move, re-link or clone
     * the selected assets.
     *
     * @param {Array.<Node>} assetNodes The nodes that triggered the selection.
     *
     * @returns {Node}
     */
    this.drawMultiSelectMenu = function(assetNodes) {
        this.clearMenus();
        var self = this;
        var container = _createEl('div');
        dfx.addClass(container, 'assetMapMenu');
        dfx.addClass(container, 'multiSelect');
        dfx.addEvent(container, 'contextmenu', function(e) {
            e.preventDefault();
        });

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_move'), null);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.moveMe.enable(assetNodes, function(source, selection) {
                self.moveAsset(AssetActions.Move, assetNodes, selection.parentid, selection.before);
            });
        });
        container.appendChild(menuItem);

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_link'), null);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.moveMe.enable(assetNodes, function(source, selection) {
                self.moveAsset(AssetActions.NewLink, assetNodes, selection.parentid, selection.before);
            });
        });
        container.appendChild(menuItem);

        var menuItem = this.drawMenuItem(js_translate('asset_map_menu_clone'), null);
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            self.moveMe.enable(assetNodes, function(source, selection) {
                self.moveAsset(AssetActions.Clone, assetNodes, selection.parentid, selection.before);
            });
        });
        container.appendChild(menuItem);

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

        // Load up the asset category names so we can sort them.
        var assetCatSort = [];
        for (i in assetCategories) {
            assetCatSort.push(i);
        }

        assetCatSort.sort();

        for (var i = 0; i < assetCatSort.length; i++) {
            var catid    = assetCatSort[i];
            var menuItem = this.drawMenuItem(catid, null, true);
            menuItem.setAttribute('data-category', catid);
            container.appendChild(menuItem);

            dfx.addEvent(menuItem, 'mouseover', function(e) {
                var target = e.currentTarget;
                e.stopPropagation();

                var existingMenu = dfx.getClass('assetMapMenu.subtype', self.topDocumentElement(target));

                if ((existingMenu.length === 0) || (existingMenu[0].getAttribute('data-category') !== target.getAttribute('data-category'))) {
                    dfx.remove(existingMenu);
                    var submenu = self.drawAssetTypeMenu(target.getAttribute('data-category'), parentid);
                    self.topDocumentElement(assetMapContainer).appendChild(submenu);
                    var elementHeight = self.topDocumentElement(assetMapContainer).clientHeight;
                    var submenuHeight = dfx.getElementHeight(submenu);
                    var targetRect = dfx.getBoundingRectangle(target);
                    dfx.setStyle(submenu, 'left', (Math.max(10, targetRect.x2) + 'px'));
                    dfx.setStyle(submenu, 'top', (Math.min(elementHeight - submenuHeight - 10, targetRect.y1) + 'px'));
                }
            });
        }

        // Folder always sits at the bottom.
        var menuItem = this.drawMenuItem('Folder', 'folder');
        dfx.addEvent(menuItem, 'click', function(e) {
            self.clearMenus();
            if (parentid !== undefined) {
                self.addAsset('folder', parentid, -1);
            } else {
                self.moveMe.enable(null, function(source, selection) {
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

        // Load up the asset type names so we can sort them.
        var assetTypeSort = [];
        for (var i = 0; i < assetCategories[category].length; i++) {
            var typeCode = assetCategories[category][i];
            var type     = assetTypeCache[typeCode];
            assetTypeSort.push(type);
        }

        assetTypeSort.sort(function(a, b) {
            if (a.name > b.name) {
                return 1;
            } else if (a.name < b.name) {
                return -1;
            } else {
                return 0;
            }
        });

        for (var i = 0; i < assetTypeSort.length; i++) {
            var type     = assetTypeSort[i];
            var typeCode = type.type_code;

            var menuItem = this.drawMenuItem(type.name, typeCode);
            menuItem.setAttribute('data-typecode', typeCode);
            dfx.addEvent(menuItem, 'click', function(e) {
                self.clearMenus();
                var target   = e.currentTarget;
                var typeCode = target.getAttribute('data-typecode');

                if (parentid !== undefined) {
                    self.addAsset(typeCode, parentid, -1);
                } else {
                    self.moveMe.enable(null, function(source, selection) {
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
            dfx.remove(dfx.getClass('assetMapMenu', this.topDocumentElement(assetMapContainer)));
        } else {
            dfx.remove(dfx.getClass('assetMapMenu.' + type, this.topDocumentElement(assetMapContainer)));
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
        url = options.rootEditUrl + '/?SQ_BACKEND_PAGE=asset_map_request&json=1';
        //url = '.' + '?SQ_BACKEND_PAGE=asset_map_request&json=1';
        var xhr = new XMLHttpRequest();
        var str = JSON.stringify(command);
        var self = this;
        var readyStateCb = function() {
            self.message(js_translate('asset_map_status_bar_requesting'), true);
            if (xhr.readyState === 4) {
                var response = xhr.responseText;
                if (response !== null) {
                    try {
                        response = JSON.parse(response);
                        self.message('', false);
                    } catch (ex) {
                        // self.raiseError(ex.message);
                        // That we made it here means it couldn't be handled.
                        self.message(js_translate('asset_map_status_bar_error_requesting'), false, 2000);
                        self.raiseError(ex.message);
                        console.info(response);
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

        self.message(js_translate('asset_map_status_bar_requesting'), true);

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

        var top = this.getDefaultView(assetMapContainer.ownerDocument).top;
        top.frames[frame].location.href = url;
    };


    this.openHipoWindow = function(url) {
        var window = this.getDefaultView(assetMapContainer).top;
        window.focus();
        var popup = window.open(url, 'hipo_job', 'width=650,height=400,scrollbars=1,toolbar=0,menubar=0,location=0,resizable=1');
        popup.focus();

    };

};//end JS_Asset_Map


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
