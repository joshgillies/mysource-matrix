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
* $Id: js_asset_map.js,v 1.1.2.23 2013/05/17 04:59:39 lwright Exp $
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
 * @version $Revision: 1.1.2.23 $
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
     * setTimeout() return value for the status bar
     * @var {Number}
     */
    var msgTimeoutId = null;

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

    var _createEl = function(tagName) {
        var el = targetElement.ownerDocument.createElement(tagName);
        el.setAttribute('unselectable', 'on');
        return el;
    }

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

    var _formatAsset = function(assetid, displayName, typeCode, status, childCount, linkid, linkType, accessible) {
        var assetLine = _createEl('div');
        dfx.addClass(assetLine, 'asset');
        assetLine.id = 'asset-' + encodeURIComponent(assetid);
        assetLine.setAttribute('data-assetid', assetid);
        assetLine.setAttribute('data-linkid', linkid);
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

        if ((accessible !== 0) && (childCount !== 0)) {
            var expandSpan = _createEl('span');
            dfx.addClass(expandSpan, 'branch-status');
            assetLine.appendChild(expandSpan);
        }

        var nameSpan = _createEl('span');
        if (nameSpan.textContent !== undefined) {
            nameSpan.textContent = displayName;
        } else if (nameSpan.innerText !== undefined) {
            nameSpan.innerText = displayName;
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
     * Start the asset map.
     *
     * @param {Object} options
     */
    this.start = function(options) {
        var self = this;

        targetElement      = options.targetElement || dfx.getId('asset_map_container');
        assetDisplayFormat = options.displayFormat || '%asset_short_name%';
        //var document       = targetElement.ownerDocument;

        var assetMap = dfx.getId('asset_map_container');
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

                assetTypeCache[typecode] = typeinfo['_attributes'];
                assetTypeCache[typecode]['screens'] = {};

                for (var j = 0; j < typeinfo['screen'].length; j++) {
                    var screencode = typeinfo['screen'][j]['_attributes']['code_name'];
                    var screenname = typeinfo['screen'][j]['_content'];
                    assetTypeCache[typecode]['screens'][screencode] = screenname;
                    self.message('Cache ' + i + '.' + j, true);
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

    this.teleport = function(assetid, linkid, treeid) {
        var self = this;
        if (treeid === undefined) {
            var tree = dfx.getClass('tree.selected', targetElement)[0];
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
                            limit: 50,  // replace with set limit
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

                        assetLine = _formatAsset(
                            rootAsset._attributes.assetid,
                            rootAsset._attributes.name,
                            rootAsset._attributes.type_code,
                            Number(rootAsset._attributes.status),
                            Number(rootAsset._attributes.num_kids),
                            rootAsset._attributes.linkid,
                            Number(rootAsset._attributes.link_type),
                            Number(rootAsset._attributes.accessible)
                        );

                        dfx.addClass(assetLine, 'teleported');
                        tree.appendChild(assetLine);
                    }//end if

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
    }

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

    this.selectTree = function(treeid) {
        var trees = dfx.getClass('tree', targetElement);
        dfx.removeClass(trees, 'selected');
        dfx.addClass(trees[treeid], 'selected');

        var treeList = dfx.getClass('tree-list', targetElement)[0];
        var tabs     = dfx.getClass('tab', targetElement);
        dfx.removeClass(tabs, 'selected');
        dfx.addClass(tabs[treeid], 'selected');
    }

    this.doRequest = function(command, callback) {
        url = '.?SQ_BACKEND_PAGE=asset_map_request&json=1';
        var xhr = new XMLHttpRequest();
        var str = JSON.stringify(command);
        var self = this;
        var readyStateCb = function() {
            self.message('Requesting...' + xhr.readyState, true);
            if (xhr.readyState === 4) {
                var response = xhr.responseText;
                if (response !== null) {
                    response = JSON.parse(response);
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

        dfx.addEvent(this.getDefaultView(document), 'resize', function() {
            var assetMap = dfx.getId('asset_map_container');
            assetMap.style.height = (document.documentElement.clientHeight - 120) + 'px';
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
                    if (e.ctrlKey === false) {
                        // Normal click.
                        dfx.removeClass(dfx.getClass('asset', assetMap), 'selected');
                        dfx.addClass(assetTarget, 'selected');
                    } else {
                        // Ctrl+click. Toggle the selection of this asset, which
                        // could leave the map with multiple or zero selection.
                        dfx.toggleClass(assetTarget, 'selected');
                    }

                    var assets = self.currentSelection();
                    switch (assets.length) {
                        case 1:
                            self.message('Asset ' + assets[0].getAttribute('data-assetid') + ' selected', false);
                        break;
                        case 0:
                            self.message('No assets selected', false);
                        break;
                        default:
                            self.message(assets.length + ' assets selected', false);
                        break;
                    }
                } else if (e.which === 3) {
                    // Right mouse button
                    self.message('Asset screens dropdown for asset ' + assetTarget.getAttribute('data-assetid'), false);

                    if (e.ctrlKey === false) {
                        dfx.removeClass(dfx.getClass('asset', assetMap), 'selected');
                    }

                    dfx.addClass(assetTarget, 'selected');

                    e.preventDefault();
                    var mousePos = dfx.getMouseEventPosition(e);
                    var menu     = self.drawScreensMenu(
                        target.getAttribute('data-assetid'),
                        target.getAttribute('data-linkid'),
                        target.getAttribute('data-typecode')
                    );
                    self.topDocumentElement(target).appendChild(menu);

                    var mapRect = dfx.getBoundingRectangle(assetMap);
                    dfx.setStyle(menu, 'left', (mousePos.x) + 'px');
                    dfx.setStyle(menu, 'top', (mousePos.y) + 'px');
                }
            } else if (branchTarget) {
                // Set the target to the asset line.
                var target       = branchTarget.parentNode;
                var assetid      = target.getAttribute('data-assetid');
                var linkid       = target.getAttribute('data-linkid');
                var rootIndentId = 'child-indent-' + encodeURIComponent(assetid);
                var container    = dfx.getId(rootIndentId);

                if (container) {
                    dfx.toggleClass(branchTarget, 'expanded');
                    dfx.toggleClass(container, 'collapsed');
                } else {
                    dfx.addClass(branchTarget, 'expanded');

                    var container = _createEl('div');
                    dfx.addClass(container, 'childIndent');
                    dfx.addClass(container, 'loading');
                    container.id        = rootIndentId;
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
                                    limit: 50,  // replace with set limit
                                    linkid: linkid
                                }
                            }
                        ]
                    }, function(response) {
                        // Cache all the asset types.
                        dfx.removeClass(container, 'loading');
                        var assets = response['asset'][0];

                        if (!assets.asset) {
                            self.message('No children loaded', false, 2000);
                            dfx.remove(container);
                            dfx.remove(branchTarget);
                        } else {
                            container.innerHTML = '';
                            var assetCount = assets.asset.length;
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
                self.message('Deselect', false);
                dfx.removeClass(dfx.getClass('asset', trees), 'selected');
                self.clearMenus();
            }//end if

        });
    }

    this.currentSelection = function(treeid) {
        if (treeid === undefined) {
            treeid = 0;
        }

        var assetMap = dfx.getId('asset_map_container');
        var trees    = dfx.getClass('tree', assetMap);
        var assets   = dfx.getClass('asset.selected', trees[treeid]);

        return assets;
    }

    this.raiseError = function(message) {
        var message = js_translate(message);

    };

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

    this.drawScreensMenu = function(assetid, linkid, assetType) {
        this.clearMenus();
        var self = this;
        var container = _createEl('div');
        dfx.addClass(container, 'assetMapMenu');
        dfx.addClass(container, 'screens');
        dfx.addEvent(container, 'contextmenu', function(e) {
            e.preventDefault();
        });

        var screens = assetTypeCache[assetType]['screens'];
        for (var i in screens) {
            var menuItem = this.drawMenuItem(screens[i], null);
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
        container.appendChild(menuItem);

        if (assetType !== 'trash_folder') {
            var menuItem = this.drawMenuItem('No Previous Child', null);
            dfx.addClass(menuItem, 'disabled');
            container.appendChild(menuItem);

            var menuItem = this.drawMenuItem('Add Child', null, true);
            container.appendChild(menuItem);

            dfx.addEvent(menuItem, 'mouseover', function(e) {
                var assetMap = dfx.getId('asset_map_container');
                var target   = dfx.getMouseEventTarget(e);

                var existingMenu = dfx.getClass('assetMapMenu.addMenu', self.topDocumentElement(target));
                if (existingMenu.length === 0) {
                    var menu     = self.drawAddMenu(false);
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
    }

    this.drawAddMenu = function(clear) {
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

        for (var i in assetCategories) {
            var menuItem = this.drawMenuItem(i, null, true);
            menuItem.setAttribute('data-category', i);
            container.appendChild(menuItem);

            dfx.addEvent(menuItem, 'mouseover', function(e) {
                var target = e.currentTarget;

                var existingMenu = dfx.getClass('assetMapMenu.subtype', self.topDocumentElement(target));

                if ((existingMenu.length === 0) || (existingMenu[0].getAttribute('data-category') !== target.getAttribute('data-category'))) {
                    dfx.remove(existingMenu);
                    var submenu = self.drawAssetTypeMenu(target.getAttribute('data-category'));
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
        container.appendChild(menuItem);

        return container;
    }

    this.drawAssetTypeMenu = function(category) {
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
            container.appendChild(menuItem);
        }

        return container;
    }

    this.drawMenuSeparator = function() {
        var sep = _createEl('div');
        dfx.addClass(sep, 'menuSep');
        return sep;
    }

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
    }

    this.drawTreeContainer = function(treeid) {
        var container = _createEl('div');
        dfx.addClass(container, 'tree');
        container.setAttribute('data-treeid', treeid);
        targetElement.appendChild(container);

        return container;
    };

    this.drawTree = function(rootAsset, container) {
        var assetLine = null;

        for (var i = 0; i < rootAsset.asset.length; i++) {
            var asset  = rootAsset.asset[i];
            asset._attributes.name       = decodeURIComponent(asset._attributes.name.replace(/\+/g, '%20'));
            asset._attributes.assetid    = decodeURIComponent(asset._attributes.assetid.replace(/\+/g, '%20'));
            asset._attributes.type_code  = decodeURIComponent(asset._attributes.type_code.replace(/\+/g, '%20'));

            assetLine = _formatAsset(
                asset._attributes.assetid,
                asset._attributes.name,
                asset._attributes.type_code,
                Number(asset._attributes.status),
                Number(asset._attributes.num_kids),
                asset._attributes.linkid,
                Number(asset._attributes.link_type),
                Number(asset._attributes.accessible)
            );

            container.appendChild(assetLine);
        }

        if (assetLine) {
            dfx.addClass(assetLine, 'last-child');
        }
    };

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

    this.drawMessageLine = function() {
        var container = _createEl('div');
        dfx.addClass(container, 'messageLine');
        targetElement.appendChild(container);

        var messageDiv = _createEl('div');
        messageDiv.id        = 'asset_map_message';
        dfx.addClass(messageDiv, 'message');
        messageDiv.innerHTML = 'Loading...';
        container.appendChild(messageDiv);
    }

    this.message = function(message, spinner, timeout) {
        var messageDiv       = targetElement.ownerDocument.getElementById('asset_map_message');
        messageDiv.innerHTML = message;

        if (msgTimeoutId) {
            clearTimeout(msgTimeoutId);
        }

        if (timeout !== undefined) {
            msgTimeoutId = setTimeout(function() {
                messageDiv.innerHTML = '&nbsp;';
                msgTimeoutId = null;
            }, timeout);
        }

    }

    this.addAsset = function(assetid, parentAssetid, linkType) {

    };


    this.refreshTree = function(treeNum) {

    };


    this.moveAsset = function(assetid, newParentAssetid, sortOrder) {
        if (assetid === newParentAssetid) {
            // Changing the sort order only
        } else {
            // Moving to a new assetid.
        }
    };


    this.createLink = function(assetid, newParentAssetid, sortOrder) {
        if (assetid === newParentAssetid) {
            // Shouldn't get here, but assets cannot be multiply linked.
            this.raiseError(js_translate('asset_map_error_multiply_linked'));
        }
    };


    this.cloneAsset = function(assetid, newParentAssetid, sortOrder) {

    };


    this.getUrl = function(assetid, screen) {

    };

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
    }

    this.getDefaultView = function(document) {
        if (document.defaultView) {
            return document.defaultView;
        } else if (document.parentWindow) {
            return document.parentWindow;
        }

        return null;
    };

    this.topDocumentElement = function(target) {
        var topDoc = this.getDefaultView(target.ownerDocument).top.document.documentElement;
        return topDoc;
    }

    this.clearMenus = function(type) {
        if (type === undefined) {
            dfx.remove(dfx.getClass('assetMapMenu', this.topDocumentElement(targetElement)));
        } else {
            dfx.remove(dfx.getClass('assetMapMenu.' + type, this.topDocumentElement(targetElement)));
        }
    }
};
