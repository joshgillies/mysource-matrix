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
* $Id: MatrixTreeNode.java,v 1.2.2.2 2006/01/18 02:58:10 sdanis Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;
import javax.swing.tree.*;
import java.awt.*;
import java.util.*;
import java.awt.datatransfer.*;
import java.io.*;

/**
 * <code>MatrixTreeNode</code> is a tree node that uniqually identifies
 * an Asset in its specific location in the tree. An <code>MatrixTreeNode</code>
 * is identified by an asset and a linkid
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixTreeNode extends DefaultMutableTreeNode
	implements Serializable {


	/* The linkid that this node represents */
	private final String linkid;

	/**
	 * The type of link to the parent
	 * @see MatrixConstants
	 */
	private int linkType;

	/* The URL including paths to this node */
	private String url;
	private String webPath;

	/**
	 * Constructs a new Asset Tree Node and sets the user object to the
	 * specified asset
	 *
	 * @param asset the asset that represents this node
	 * @param linkid the linkid of this node
	 */
	public MatrixTreeNode(Asset asset, String linkid, int linkType, String url, String webPath) {
		setUserObject(asset);
		this.linkid = linkid;
		this.linkType = linkType;
		this.url = url;
		this.webPath = webPath;
	}

	public String toString() {
		return getAsset().getName() + " Linkid : " + linkid;
	}

	/**
	 * Returns the asset that represents this node
	 *
	 * @return the asset
	 */
	public Asset getAsset() {
		return (Asset) getUserObject();
	}

	public int getLinkType() {
		return linkType;
	}

	public void setLinkType(int linkType) {
		this.linkType = linkType;
	}

	/**
	 * Returns the linkid to the parent asset of this node
	 *
	 * @return the linkid
	 */
	public String getLinkid() {
		return linkid;
	}

	public boolean isShadowAsset() {
		return (linkid.equals("0") && linkid.split(":").length > 1);
	}

	/**
	 * Returns TRUE if this node is a leaf
	 *
	 * @return TRUE if this node is a leaf
	 */
	public boolean isLeaf() {
		// if the asset is not root and user has no access then make this asset a leaf node
		if (!getAsset().getId().equals("1") && !getAsset().isAccessible()) {
			return true;
		}

		return (getAsset().getNumKids() == 0);
	}

	/**
	 * Returns the preview URL of this node
	 *
	 * @return the preview URL of this node
	 */
	public String getURL() {
		if (url == null) {
			return "";
		} else if (webPath == null) {
			return url;
		}
		return url + "/" + webPath;
	}

	/**
	 * Returns TRUE if this node has a child with the
	 * specified linkid. It is not possible for a MatrixTreeNode to have more
	 * than 1 child with the same linkid
	 *
	 * @param linkid the linkid of the child to chech for
	 * @return TRUE if this node has a child with the specified linkid
	 */
	public boolean hasChildWithLinkid(String linkid) {
		return (getChildWithLinkid(linkid) == null) ? false : true;
	}

	public MatrixTreeNode getChildWithLinkid(String linkid) {
		Enumeration children = children();
		while (children.hasMoreElements()) {
			MatrixTreeNode node = (MatrixTreeNode) children.nextElement();
			if (node.getLinkid().equals(linkid))
				return (MatrixTreeNode) node;
		}
		return null;
	}


	public void propagateUrl(String url) {
		this.url = url;
		Enumeration children = children();
		while (children.hasMoreElements()) {
			MatrixTreeNode node = (MatrixTreeNode) children.nextElement();
			node.propagateUrl(getURL());
		}
	}

	public void propagateWebPath(String webPath) {
		this.webPath = webPath;
		Enumeration children = children();
		while (children.hasMoreElements()) {
			MatrixTreeNode node = (MatrixTreeNode) children.nextElement();
			// from here on in, all the nodes under this particular node
			// only have changed their urls as this node's url + "/" + webPath
			// is its children's url
			node.propagateUrl(getURL());
		}
	}

	/**
	 * Returns a comma separated list of assetids from the root node where this
	 * node is the last assetid in the list
	 * @param node the node of the wanted asset path
	 * @return the command separated asset path
	 * @see #getLinkPath()
	 */
	public String getAssetPath() {
		Object[] path = getPath();
		StringBuffer assetPath = new StringBuffer();
		for (int i = 0; i < path.length; i++) {
			assetPath.append(",").append(((MatrixTreeNode) path[i]).getAsset().getId());
		}
		return assetPath.toString();
	}

	/**
	 * Returns a comma separated list of linkids from the root node where this
	 * node is the last linkid in the list
	 * @param node the tree node of the wanted link path
	 * @return the link path for the specifed tree node
	 * @see #getAssetPath()
	 */
	public String getLinkPath() {
		Object[] path = getPath();
		StringBuffer linkPath = new StringBuffer();
		for (int i = 0; i < path.length; i++) {
			linkPath.append(",").append(((MatrixTreeNode) path[i]).getLinkid());
		}
		return linkPath.toString();
	}
}
