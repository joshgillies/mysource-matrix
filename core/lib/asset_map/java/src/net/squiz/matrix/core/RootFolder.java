/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: RootFolder.java,v 1.5 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

package net.squiz.matrix.core;

import org.w3c.dom.*;
import net.squiz.matrix.matrixtree.*;

public class RootFolder extends Asset {

	private MatrixTreeNode rootNode;

	public RootFolder(Element assetElement) {
		super(assetElement.getAttribute("assetid"));
		String linkid = processAssetXML(assetElement, true);
		rootNode = new MatrixTreeNode(this, linkid, getLinkType(assetElement), null, null, null, 0);
		addNode(rootNode, linkid);
	}

	/**
	 * Processes the xml for the asset, and returns the MatrixTreeNode
	 * with the corresponding linkid from the xml
	 */
	public MatrixTreeNode processAssetXML(
		Element assetElement,
		MatrixTreeNode parent,
		int index) {
			String linkid = processAssetXML(assetElement, false);
			return rootNode;
	}

	public MatrixTreeNode getRootNode() {
		return rootNode;
	}
}
