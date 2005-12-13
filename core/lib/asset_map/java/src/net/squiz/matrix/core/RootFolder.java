
package net.squiz.matrix.core;

import org.w3c.dom.*;
import net.squiz.matrix.matrixtree.*;

public class RootFolder extends Asset {

	private MatrixTreeNode rootNode;

	public RootFolder(Element assetElement) {
		super(assetElement.getAttribute("assetid"));
		String linkid = processAssetXML(assetElement, true);
		rootNode = new MatrixTreeNode(this, linkid, getLinkType(assetElement), null, null, null);
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
