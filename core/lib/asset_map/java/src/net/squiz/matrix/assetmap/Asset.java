/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: Asset.java,v 1.3 2004/06/29 03:39:30 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.util.*;
import java.awt.Color;
import javax.swing.tree.MutableTreeNode;


/**
 * An <code>Asset</code> mearly contains information about 
 * an <code>Asset</code> from the MySource Matrix system. 
 * An <code>Asset</code> has no relation to the to structure 
 * of the tree nodes. Instead it serves as a lookup when information
 * about an asset is needed. An asset does, however, contain a
 * collection of nodes that represent the various locations where
 * this asset exists in the Asset Tree. 
 * 
 * @author Marc McIntyre
 */
public class Asset {
	
	/** A collection of tre nodes for this asset */
	private Map nodes = new HashMap();
	
	/** Asset ID */
	private final String id;

	/** Asset name  */
	private String name = "";

	/** Asset type */
	private AssetType type;

	/** Asset status */
	private int status;

	/**  Whether this asset is accessible to the current user. */
	private boolean accessible;

	/** The URL for this asset */
	private String url = "";

	/** The web path that belong to this asset  */
	private String webPath = "";

	/** the number of children this asset has */
	private int childCount = 0;
	
	/** TRUE if this children have been loaded */
	private boolean childrenLoaded = false;
	
	/** The link type to parent*/
	private int linkType;

	/** constant for link type 1 */
	public static final int SQ_LINK_TYPE_1 = 1;
	
	/** constant for link type 2 */
	public static final int SQ_LINK_TYPE_2 = 2;
	
	
	/* STATUSES */


	/** Asset Archival status. */
	public static final int ARCHIVED = 1;

	/** Asset Under Construction status. */
	public static final int UNDER_CONSTRUCTION = 2;

	/** Asset Workflow Pending Approval status. */
	public static final int PENDING_APPROVAL = 4;

	/** Asset Workflow Approved status. */
	public static final int APPROVED = 8;

	/** Asset Live status. */
	public static final int LIVE = 16;

	/** Asset Live Under Review status. */
	public static final int LIVE_APPROVAL = 32;

	/** Asset Safe Editing status */
	public static final int EDITING = 64;

	/** Asset Workflow Approval Safe Editing status. */
	public static final int EDITING_APPROVAL = 128;

	/** Asset Workflow Approved Safe Editing status. */
	public static final int EDITING_APPROVED = 256;


	/* STATUS COLOURS */


	/** The Acrhived colour */
	public static final Color ARCHIVED_COLOUR = new Color(0x655240);

	/** The Under Construction colour */
	public static final Color UNDER_CONSTRUCTION_COLOUR = new Color(0xBCE2F5);

	/** The Pending Approval colour */
	public static final Color PENDING_APPROVAL_COLOUR = new Color(0xDCD2E6);

	/** The Approved colour */
	public static final Color APPROVED_COLOUR = new Color(0xF4D425);

	/** The Live colour */
	public static final Color LIVE_COLOUR = new Color(0xDBF18A);

	/** The Live Approval colour */
	public static final Color LIVE_APPROVAL_COLOUR = new Color(0xDCD2E6);

	/** The Editing Colour */
	public static final Color EDITING_COLOUR = new Color(0xF25C86);

	/** The Editing Approval Colour */
	public static final Color EDITING_APPROVAL_COLOUR = new Color(0xCCCCCC);

	/** The Editing Approved Colour */
	public static final Color EDITING_APPROVED_COLOUR = new Color(0xFF9A00);
	
	/** A colour that will be used if the status colout is unknown */
	public static final Color UNKNOWN_STATUS_COLOUR = new Color(0xFF0000);
	
	
	/**
	 * Creates an <code>Asset</code> and sets the fields for an <code>Asset</code>.
	 * 
	 * @param id the assetid
	 * @param name the asset name
	 * @param type the asset type
	 * @param status the asset status
	 * @param accessible 
	 * @param url
	 * @param webPaths
	 */
	public Asset(String id, 
			String name, 
			AssetType type,
			int linkType,
			int status, 
			boolean accessible,
			String url,
			String webPath) {
		
		if (id == null)
			throw new IllegalArgumentException("Assetid is null");
		if (type == null)
			throw new IllegalArgumentException("Type is null");
		
		this.id         = id;
		this.name       = name;
		this.type       = type;
		this.linkType   = linkType;
		this.status     = status;
		this.accessible = accessible;
		this.url        = url;
		this.webPath    = webPath;
	
	}
	
	/**
	 * Constructs an Asset
	 * @param id the assetid of this asset
	 * @param name the name of this asset
	 * @param type the type of this asset
	 */
	public Asset(String id, String name, AssetType type) {
		this.id = id;
		this.name = name;
		this.type = type;
	}
	
	/**
	 * Refreshes an assets internals.
	 *
	 * @param name The name of the asset
	 * @param status the status of the asset
	 * @param linkType the link type of the asset
	 * @param accessible if TRUE the asset is accessible
	 * @param url the url for the asset
	 * @param webPath the web path for this asset
	 */
	public void refresh(String name, 
			int status, 
			int linkType, 
			boolean accessible, 
			String url, 
			String webPath) {
		
		this.name       = name;
		this.status     = status;
		this.linkType   = linkType;
		this.accessible = accessible;
		this.url        = url;
		this.webPath   = webPath;
	}
	
	/**
	 * Returns TRUE if this is a TYPE_2 link
	 * @return
	 */
	public boolean isType2Link() {
		return (linkType == SQ_LINK_TYPE_2);
	}
	
	/**
	 * Returns the assetid of this <code>Asset</code>.
	 * 
	 * @return the assetid of this <code>Asset</code>
	 */
	public String getId() {
		return id;
	}
	
	/**
	 * Returns the name of this <code>Asset</code>.
	 * 
	 * @return the name of this <code>Asset</code>
	 */
	public String getName() {
		return name;
	}
	
	/**
	 * Returns the <code>AssetType</code> of this asset.
	 * 
	 * @return the <code>AssetType</code> of this asset
	 */
	public AssetType getType() {
		return type;
	}
	
	/**
	 * Returns the status of this <code>Asset</code>
	 * 
	 * @return the status of this <code>Asset</code>
	 */
	public int getStatus() {
		return status;
	}
	
	/**
	 * Returns the URL of this <code>Asset</code>.
	 * 
	 * @return the URL of this <code>Asset</code>
	 */
	public String getURL() {
		return url;
	}
	
	/**
	 * Returns the web paths of this <code>Asset</code>.
	 * 
	 * @return the web paths of this <code>Asset</code>
	 */
	public String getWebPath() {
		return webPath;
	}
	
	/**
	 * Returns TRUE if this asset is Accessible
	 * 
	 * @return TRUE if this asset is accessible
	 */
	public boolean isAccessible() {
		return accessible;
	}
	
	/**
	 * Sets the childrenLoaded property
	 * 
	 * @param childrenLoaded TRUE if this assets children have been loaded
	 */
	public void setChildrenLoaded(boolean childrenLoaded) {
		this.childrenLoaded = childrenLoaded;
	}
	
	/**
	 * Returns TRUE if this asset has its children loaded
	 * 
	 * @return the children loaded property
	 */
	public boolean childrenLoaded() {
		return childrenLoaded;
	}
	
	/**
	 * Sets the URL
	 * 
	 * @param url the new url to set
	 */
	public void setURL(String url) {
		this.url = url;
	}
	
	/**
	 * Returns the number of children that this <code>Asset</code> has
	 * 
	 * @return the number of children that this <code>Asset</code> has
	 */
	public int getChildCount(){
		return childCount;
	}
	
	/**
	 * Sets the number of children that this <code>Asset</code> has
	 * 
	 * @param childCount the number of children that this <code>Asset</code> has
	 */
	public void setChildCount(int childCount) {
		this.childCount = childCount;
	}
	
	/**
	 * Returns an <code>Iterator</code> of the <code>TreeNodes</code> 
	 * That this <code>Asset</code> is currently representing
	 * 
	 * @return an <code>Iterator<code> of the nodes of this <code>Asset</code>.
	 */
	public Iterator getTreeNodes() {
		
		ArrayList list = new ArrayList();
		Iterator it = nodes.values().iterator();
		
		while (it.hasNext()) {
			List nextList = (List) it.next();
			Iterator nextIt = nextList.iterator();
			while (nextIt.hasNext()) {
				AssetTreeNode node = (AssetTreeNode) nextIt.next();
				list.add(node);
			}
		}
		return list.iterator();
	}
	
	/**
	 * Returns true if this asset has a node with the specified linkid
	 * 
	 * @param linkid the linkid of the node
	 * @return TRUE if this asset has the node with the specified linkid
	 */
	public boolean hasNode(String linkid) {
		return nodes.containsKey(linkid);
	}
	
	/**
	 * Adds a <code>TreeNode</code> that represents this <code>Asset</code>
	 * In a particular location in the tree
	 * 
	 * @param node the Unique <code>TreeNode</code>
	 * @param linkid the id of the link
	 */
	public AssetTreeNode createNode(String linkid) {
		if (linkid == null)
			throw new IllegalArgumentException("linkid is null");
		
		List list = null;
		if (nodes.containsKey(linkid))
			list = (List) nodes.get(linkid);
		else
			list = new ArrayList();
		
		AssetTreeNode node = new AssetTreeNode(this, linkid);
		list.add(node);
		nodes.put(linkid, list);
		
		return node;
	}
	
	/**
	 * Propagates a node down all the nodes in the node list of the asset. If
	 * any of this assets nodes do not have the specified assets nodes, they
	 * are inserted into their appropriate positions 
	 * @param childAsset
	 * @param linkid
	 * @param index
	 */
	public void propogateNode(Asset childAsset, String linkid, int index) {
		
		Iterator iterator = getTreeNodes();
		while (iterator.hasNext()) {
			AssetTreeNode node = (AssetTreeNode) iterator.next();
			if (!(node.hasNodeWithLinkId(linkid))) {
				MutableTreeNode newNode = childAsset.createNode(linkid);
				node.insert(newNode, index);
				AssetManager.INSTANCE.fireNodesWereInserted(node, new int[] { index } );
			}
		}
	}
	
	/** 
	 * Removes any nodes that this asset has that are not in the specified
	 * list of linkids
	 */
	public void cleanNodes(ArrayList linkids) {
		
		Iterator iterator = getTreeNodes();

		ArrayList staleLinkids = null;
		while (iterator.hasNext()) {
			((AssetTreeNode) iterator.next()).removeDiffChildLinks(linkids);
		}
	}
	
	/**
	 * Returns a list of <code>TreeNodes</code> identified by the specified linkid
	 * 
	 * @param linkid the linkid of the <code>TreeNodes</code>
	 * @return the <code>TreeNodes</code> with the specified linkid
	 */
	public List getNodes(String linkid) {
		return (List) nodes.get(linkid);
	}

	/**
	 * Returns the string representation of this <code>Asset</code>
	 * 
	 * @return the <code>String</code> representation of this <code>Asset</code>
	 */
	public String toString() {
		return ("[" + id + "] " + name + " (" + type.getName() + ")");
	}
	
	/**
	 * Returns a unique int that identifies this <code>Asset</code>
	 * 
	 * @return the int identifying this <code>Object</code>
	 */
	public int hashCode() {
		return id.hashCode();
	}
	
	/**
	 * Returns TRUE if the specified <code>Object</code> is equal to this <code>Object</code>
	 * 
	 * @return TRUE if the spcified <code>Object</code> is equal to this <code>Object</code>
	 */
	public boolean equals(Object obj) {
		if (!(obj instanceof Asset))
			return false;
		return (((Asset) obj).id.equals(id));
	}
	
	/**
	 * Returns a status colour based on the status of this
	 * <code>Asset</code>
	 * 
	 * @return the status colour based on the status of this
	 * <code>Asset</code>. If the status unknown, <code>Color.RED</code>
	 *  will be returned
	 */
	public Color getStatusColour() {
		switch (status) {
			case ARCHIVED:
				return ARCHIVED_COLOUR;
			case UNDER_CONSTRUCTION:
				return UNDER_CONSTRUCTION_COLOUR;
			case LIVE:
				return LIVE_COLOUR;
			case LIVE_APPROVAL:
				return LIVE_APPROVAL_COLOUR;
			case PENDING_APPROVAL:
				return PENDING_APPROVAL_COLOUR;
			case APPROVED:
				return APPROVED_COLOUR;
			case EDITING:
				return EDITING_COLOUR;
			case EDITING_APPROVAL:
				return EDITING_APPROVAL_COLOUR;
			case EDITING_APPROVED:
				return EDITING_APPROVED_COLOUR;
			default:
				System.err.println("Unknown status :" + status);
				return UNKNOWN_STATUS_COLOUR;
		}
	}

}
