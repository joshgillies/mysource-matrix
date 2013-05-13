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
* $Id: js_asset_map.js,v 1.1.2.12 2013/05/13 23:27:17 lwright Exp $
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
 * @version $Revision: 1.1.2.12 $
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
     * @var {String}
     */
    var assetTypeCache = {};

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
     * @var {Number}
     */
    var msgTimeoutId = null;

    /**
     * @var {Node}
     */
    var selectedAsset = null;

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
        assetLine.className = 'asset';
        assetLine.id = 'asset-' + encodeURIComponent(assetid);
        assetLine.setAttribute('data-assetid', assetid);
        assetLine.setAttribute('data-linkid', linkid);

        if (assetTypeCache[typeCode]) {
            assetLine.setAttribute('title', assetTypeCache[typeCode].name + ' [' + assetid + ']');
        } else {
            assetLine.setAttribute('title', 'Unknown Asset Type [' + assetid + ']');
        }

        var leafSpan = _createEl('span');
        leafSpan.className = 'leaf';

        var iconSpan = _createEl('span');
        iconSpan.className = 'icon';
        iconSpan.setAttribute('style', 'background-image: url(../__data/asset_types/' + typeCode + '/icon.png)');

        if (accessible === 0) {
            var flagSpan = _createEl('span');
            flagSpan.className = 'not-accessible';
            assetLine.appendChild(flagSpan);
        } else if (linkType === LinkType.Type2) {
            var flagSpan = _createEl('span');
            flagSpan.className = 'type2-link';
            assetLine.appendChild(flagSpan);
        }

        if ((accessible !== 0) && (childCount !== 0)) {
            var expandSpan = _createEl('span');
            expandSpan.className = 'branch-status';
            assetLine.appendChild(expandSpan);
        }

        var nameSpan = _createEl('span');
        if (nameSpan.textContent !== undefined) {
            nameSpan.textContent = displayName;
        } else if (nameSpan.innerText !== undefined) {
            nameSpan.innerText = displayName;
        }

        var statusClass = _statusClass(status);
        nameSpan.className = 'assetName status' + statusClass;

        assetLine.appendChild(leafSpan);
        assetLine.appendChild(iconSpan);
        assetLine.appendChild(nameSpan);

        return assetLine;
    };

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
        var container = this.drawTreeContainer();
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
                assetTypeCache[typecode] = typeinfo['_attributes'];
                assetTypeCache[typecode]['screens'] = {};

                for (var j = 0; j < typeinfo['screen'].length; j++) {
                    var screencode = typeinfo['screen'][j]['_attributes']['code_name'];
                    var screenname = typeinfo['screen'][j]['_content'];
                    assetTypeCache[typecode]['screens'][screencode] = screenname;
                    self.message('Cache ' + i + '.' + j, true);
                }
            }

            var assets = response['assets'][0]['asset'];
            for (var i = 0; i < assets.length; i++) {
                if (assets[i]._attributes.type_code === 'root_folder') {
                    self.message('Draw tree', true);
                    self.drawTree(assets[i], container);
                }
            }

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

        dfx.addEvent(this.getDefaultView(document), 'resize', function() {
            var assetMap = dfx.getId('asset_map_container');
            assetMap.style.height = (document.documentElement.clientHeight - 120) + 'px';
        });

        var statusDivider = dfx.getId('asset_map_status_list_divider');
        dfx.addEvent(statusDivider, 'click', function() {
            dfx.toggleClass(statusDivider.parentNode, 'expanded');
            self.resizeTree();
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
            dfx.addClass(branchButtons, 'expanded');
        });

        dfx.addEvent(targetElement, 'contextmenu', function() {
            return false;
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
                if (e.which === 1) {
                    // Left mouse button
                    self.message('Select asset ' + assetTarget.getAttribute('data-assetid'), false);

                    if (e.ctrlKey === false) {
                        dfx.removeClass(dfx.getClass('asset', assetMap), 'selected');
                    }

                    dfx.addClass(assetTarget, 'selected');
                } else if (e.which === 3) {
                    // Right mouse button
                    self.message('Asset screens dropdown for asset ' + assetTarget.getAttribute('data-assetid'), false);

                    if (e.ctrlKey === false) {
                        dfx.removeClass(dfx.getClass('asset', assetMap), 'selected');
                    }

                    dfx.addClass(assetTarget, 'selected');
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
                    branchTarget.className += ' expanded';

                    var container       = _createEl('div');
                    container.className = 'childIndent loading';
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
                        container.className = container.className.replace(/ loading/, '');
                        var assets          = response['asset'][0];

                        if (!assets.asset) {
                            self.message('No children loaded', false, 2000);
                            container.parentNode.removeChild(container);
                            branchTarget.parentNode.removeChild(branchTarget);
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
            }//end if

        });
    }

    this.currentSelection = function(treeid) {
        if (treeid === undefined) {
            treeid = 0;
        }

        var assetMap = dfx.getId('asset_map_container');
        var trees    = dfx.getClass('tree', assetMap);
        var assets   = dfx.getClass(dfx.getClass('asset', trees[treeid]), 'selected');

        return assets;
    }

    this.raiseError = function(message) {
        var message = js_translate(message);

    };

    this.drawToolbar = function() {
        var container = _createEl('div');
        container.className = 'toolbar';
        targetElement.appendChild(container);

        var addButton = _createEl('div');
        addButton.className = 'addButton';
        container.appendChild(addButton);

        var tbButtons = _createEl('div');
        tbButtons.className = 'tbButtons';
        container.appendChild(tbButtons);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_refresh';
        tbButton.className = 'tbButton refresh';
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Refresh');
        tbButtons.appendChild(tbButton);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_restore';
        tbButton.className = 'tbButton restore';
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Restore root');
        tbButtons.appendChild(tbButton);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_collapse';
        tbButton.className = 'tbButton collapse';
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Collapse all');
        tbButtons.appendChild(tbButton);

        var tbButton = _createEl('div');
        tbButton.id        = 'asset_map_button_statuses';
        tbButton.className = 'tbButton statuses';
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Show status');
        tbButtons.appendChild(tbButton);
    };

    this.drawTreeContainer = function() {
        var container = _createEl('div');
        container.className = 'tree';
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
            assetLine.className += ' last-child';
        }
    };

    this.drawStatusList = function() {
        var container = _createEl('div');
        container.className = 'statusList';
        targetElement.appendChild(container);

        var divider = _createEl('div');
        divider.id        = 'asset_map_status_list_divider';
        divider.className = 'statusDivider';
        container.appendChild(divider);

        var dividerIcon = _createEl('div');
        dividerIcon.className = 'icon';
        divider.appendChild(dividerIcon);

        var dividerText = _createEl('span');
        dividerText.className = 'text';
        dividerText.innerHTML = 'Status colour key';
        divider.appendChild(dividerText);

        for (var x in Status) {
            var displayName = js_translate('status_' + x.replace(/([a-z])([A-Z])/g, '$1_$2').toLowerCase());

            var assetLine = _createEl('div');
            assetLine.className = 'asset';

            var iconSpan = _createEl('span');
            iconSpan.className = 'statusListIcon status' + x;
            iconSpan.innerHTML = '&nbsp;';

            var nameSpan = _createEl('span');
            if (nameSpan.textContent !== undefined) {
                nameSpan.textContent = displayName;
            } else if (nameSpan.innerText !== undefined) {
                nameSpan.innerText = displayName;
            }

            nameSpan.className = 'assetName';

            assetLine.appendChild(iconSpan);
            assetLine.appendChild(nameSpan);
            container.appendChild(assetLine);
        }//end for
    };

    this.drawMessageLine = function() {
        var container = _createEl('div');
        container.className = 'messageLine';
        targetElement.appendChild(container);

        var messageDiv = _createEl('div');
        messageDiv.id        = 'asset_map_message';
        messageDiv.className = 'message';
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
};