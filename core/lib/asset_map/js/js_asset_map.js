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
* $Id: js_asset_map.js,v 1.1.2.8 2013/05/09 04:44:37 lwright Exp $
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
 * @version $Revision: 1.1.2.8 $
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
     * List of trees. By default, this will be an array of no more than two
     * trees, although it is possible to support more.
     * @var {Array}
     */
    var trees = [];

    /*var Asset = new function(options) {
        this.assetid    = options.assetid;
        this.name       = options.name;
        this.shortName  = options.shortName;
        this.typeCode   = options.typeCode;
        this.status     = options.status;
        this.childCount = options.childCount;
    };*/

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
        var assetLine = targetElement.ownerDocument.createElement('div');
        assetLine.className = 'asset';
        assetLine.id = 'asset-' + encodeURIComponent(assetid);
        assetLine.setAttribute('data-assetid', assetid);
        assetLine.setAttribute('data-linkid', linkid);

        if (assetTypeCache[typeCode]) {
            assetLine.setAttribute('title', assetTypeCache[typeCode].name + ' [' + assetid + ']');
        } else {
            assetLine.setAttribute('title', 'Unknown Asset Type [' + assetid + ']');
        }

        var leafSpan = targetElement.ownerDocument.createElement('span');
        leafSpan.className = 'leaf';

        var iconSpan = targetElement.ownerDocument.createElement('span');
        iconSpan.className = 'icon';
        iconSpan.setAttribute('style', 'background-image: url(../__data/asset_types/' + typeCode + '/icon.png)');

        if (accessible === 0) {
            var flagSpan = document.createElement('span');
            flagSpan.className = 'not-accessible';
            assetLine.appendChild(flagSpan);
        } else if (linkType === LinkType.Type2) {
            var flagSpan = document.createElement('span');
            flagSpan.className = 'type2-link';
            assetLine.appendChild(flagSpan);
        }

        if ((accessible !== 0) && (childCount !== 0)) {
            var expandSpan = targetElement.ownerDocument.createElement('span');
            expandSpan.className = 'branch-status';
            assetLine.appendChild(expandSpan);
        }

        var nameSpan = targetElement.ownerDocument.createElement('span');
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
        var readyStateCb = function() {
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

        targetElement      = options.targetElement || document.getElementById('asset_map_container');
        assetDisplayFormat = options.displayFormat || '%asset_short_name%';
        //var document       = targetElement.ownerDocument;

        var assetMap = document.getElementById('asset_map_container');
        assetMap.style.height = (document.documentElement.clientHeight - 120) + 'px';

        this.drawToolbar();
        var container = this.drawTreeContainer();
        this.drawStatusList();
        this.drawMessageLine();

        var treeDiv    = document.getElementsByClassName('tree')[0];
        var toolbarDiv = document.getElementsByClassName('toolbar')[0];
        var messageDiv = document.getElementsByClassName('messageLine')[0];
        var statusList = document.getElementsByClassName('statusList')[0];
        treeDiv.style.height = (assetMap.clientHeight - toolbarDiv.clientHeight - messageDiv.clientHeight - statusList.clientHeight) + 'px';

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
                }
            }

            var assets = response['assets'][0]['asset'];
            for (var i = 0; i < assets.length; i++) {
                if (assets[i]._attributes.type_code === 'root_folder') {
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
        var self     = this;

        targetElement.onclick = function(e) {
            if (e === undefined) {
                e = event;
            }

            var target = e.target;
            while (target && (/branch-status/.test(target.className) === false)) {
                target = target.parentNode;
            }

            if (target) {
                var branchTarget  = target;

                // Set the target to the asset line.
                var target       = target.parentNode;
                var assetid      = target.getAttribute('data-assetid');
                var linkid       = target.getAttribute('data-linkid');
                var rootIndentId = 'child-indent-' + encodeURIComponent(assetid);
                var container    = document.getElementById(rootIndentId);

                if (container) {
                    if (/collapsed/.test(container.className) === true) {
                        branchTarget.className += ' expanded';
                        container.className = container.className.replace(/ collapsed/, '');
                    } else {
                        container.className += ' collapsed';
                        branchTarget.className = branchTarget.className.replace(/ expanded/, '');
                    }
                } else {
                    branchTarget.className += ' expanded';

                    var container       = document.createElement('div');
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
            }//end if
        };//end onclick
    }

    this.raiseError = function(message) {
        var message = js_translate(message);
    };

    this.drawToolbar = function() {
        var container = targetElement.ownerDocument.createElement('div');
        container.className = 'toolbar';
        targetElement.appendChild(container);

        var addButton = targetElement.ownerDocument.createElement('div');
        addButton.className = 'addButton';
        container.appendChild(addButton);

        var tbButtons = targetElement.ownerDocument.createElement('div');
        tbButtons.className = 'tbButtons';
        container.appendChild(tbButtons);

        var tbButton = targetElement.ownerDocument.createElement('div');
        tbButton.className = 'tbButton refresh';
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Refresh');
        tbButtons.appendChild(tbButton);

        var tbButton = targetElement.ownerDocument.createElement('div');
        tbButton.className = 'tbButton restore';
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Restore root');
        tbButtons.appendChild(tbButton);

        var tbButton = targetElement.ownerDocument.createElement('div');
        tbButton.className = 'tbButton collapse';
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Collapse all');
        tbButtons.appendChild(tbButton);

        var tbButton = targetElement.ownerDocument.createElement('div');
        tbButton.className = 'tbButton statuses';
        tbButton.innerHTML = '&nbsp;';
        tbButton.setAttribute('title', 'Show status');
        tbButtons.appendChild(tbButton);
    };

    this.drawTreeContainer = function() {
        var container = targetElement.ownerDocument.createElement('div');
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
        var container = targetElement.ownerDocument.createElement('div');
        container.className = 'statusList';
        targetElement.appendChild(container);

        var divider = targetElement.ownerDocument.createElement('div');
        divider.className = 'statusDivider';
        container.appendChild(divider);

        for (var x in Status) {
            var displayName = js_translate('status_' + x.replace(/([a-z])([A-Z])/g, '$1_$2').toLowerCase());

            var assetLine = targetElement.ownerDocument.createElement('div');
            assetLine.className = 'asset';

            var iconSpan = targetElement.ownerDocument.createElement('span');
            iconSpan.className = 'statusListIcon status' + x;
            iconSpan.innerHTML = '&nbsp;';

            var nameSpan = targetElement.ownerDocument.createElement('span');
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
        var container = targetElement.ownerDocument.createElement('div');
        container.className = 'messageLine';
        targetElement.appendChild(container);

        var messageDiv = targetElement.ownerDocument.createElement('div');
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

};
