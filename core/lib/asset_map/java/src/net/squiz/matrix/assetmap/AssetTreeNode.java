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
* $Id: AssetTreeNode.java,v 1.1 2004/06/29 01:23:56 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import javax.swing.tree.*;
import java.awt.*;
import java.util.*;

/**
 * <code>AssetMapTreeNode</code> is a tree node that uniqually identifies 
 * an Asset in its specific location in the tree. An <code>AssetMapTreeNode</code>
 * is identified by an asset and a linkid
 * 
 * @author Marc McIntyre
 */
public class AssetTreeNode extends DefaultMutableTreeNode {
	
	/** The linkid that this node represents */
	private final String linkid;

	/** The preview URL */
	private String previewURL = "";
	
	/** The font used for noedes.*/
	public static final Font NODE_FONT = new Font("node_font", Font.PLAIN, 10);
	
	/** the nodes font color */
	public static final Color NODE_FONT_COLOR = new Color(0x342939); 
	
	/**
	 * Constructs a new Asset Tree Node and sets the user object to the
	 * specified asset
	 * 
	 * @param asset the asset that represents this node
	 * @param linkid the linkid of this node
	 */
	public AssetTreeNode(Asset asset, String linkid) {
		setUserObject(asset);
		this.linkid = linkid;
	}
	
	/**
	 * Returns the asset that represents this node
	 * 
	 * @return the asset
	 */
	public Asset getAsset() {
		return (Asset) getUserObject();
	}
	
	/**
	 * Returns the linkid to the parent asset of this node
	 * 
	 * @return the linkid
	 */
	public String getLinkId() {
		return linkid;
	}
	
	/**
	 * Returns TRUE if this node is a leaf
	 * 
	 * @return TRUE if this node is a leaf
	 */
	public boolean isLeaf() {
		return (getAsset().getChildCount() == 0);
	}
	
	/**
	 * Returns the preview URL of this node
	 * 
	 * @return the preview URL of this node
	 */
	public String getPreviewURL() {
		return previewURL;
	}
	
	/**
	 * Sets the preview URL of this node
	 * 
	 * @param previewURL the URL to set
	 */
	public void setPreviewURL(String previewURL) {
		this.previewURL = previewURL;
	}
	
	/**
	 * Returns TRUE if this node has a child with the
	 * specified linkid
	 * 
	 * @param linkid the linkid of the child to chech for
	 * @return TRUE if this node has a child with the specified linkid
	 */
	public boolean hasNodeWithLinkId(String linkid) {
		Enumeration e = children();
		while (e.hasMoreElements()) {
			Object node = e.nextElement();
			if (!(node instanceof AssetTreeNode))
				continue;
				
			if (((AssetTreeNode) node).getLinkId().equals(linkid))
			return true;
		}
		return false;
	}
	
	/**
	 * Returns the child of this node with the specified linkid
	 * 
	 * @param linkid the linkid of the wanted child
	 * @return the <code>AssetTreeNode</code> with the specifed linkid
	 * or null if it node not exist
	 */
	public AssetTreeNode getNodeWithLinkid(String linkid) {
		Enumeration e = children();
		while (e.hasMoreElements()) {
			AssetTreeNode node = (AssetTreeNode) e.nextElement();
			if (node.getLinkId().equals(linkid))
				return node;
		}
		return null;
	}
	
	/**
	 * returns a list of linkids that this node has for children
	 * 
	 * @return the list of linkids that this node has for children
	 */
	public ArrayList getChildrenLinkIds() {
		ArrayList linkids = new ArrayList();
		Enumeration e = children();
		while (e.hasMoreElements()) {
			AssetTreeNode node = (AssetTreeNode) e.nextElement();
			linkids.add(node.getLinkId());
		}
		return linkids;
	}
	
	/**
	 * Removes any nodes that this node has as children that are not
	 * in the specified list of linkids
	 * 
	 * @param linkids the list of linkids to do the diff on
	 */
	public void removeDiffChildLinks(ArrayList linkids) {
		ArrayList childLinks = getChildrenLinkIds();
		Iterator it = childLinks.iterator();
		ArrayList staleLinkids = new ArrayList();
		
		boolean found = false;
		while (it.hasNext()) {
			String oldLinkid = (String) it.next();
			Iterator linkIterator = linkids.iterator();
			found = false;
			while (linkIterator.hasNext()) {
				String currentLinkid = (String) linkIterator.next();
				if (oldLinkid.equals(currentLinkid)) 
					found = true;
			}
			if (!found)
				staleLinkids.add(oldLinkid);
		}
		if (!(staleLinkids.isEmpty())) {
			Iterator staleIterator = staleLinkids.iterator();
			while (staleIterator.hasNext()) {
				String staleid = (String) staleIterator.next();
				Object node = getNodeWithLinkid(staleid);
				int [] childIndices = new int [] { getIndex((MutableTreeNode) node)};
				Object[] removedNodes = new Object[] { node };
				remove((MutableTreeNode) node);
				AssetManager.INSTANCE.fireNodesWereRemoved(
						this, childIndices, removedNodes);
			}
		}
	}
}
