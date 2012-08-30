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
* $Id: Asset.java,v 1.17 2012/08/30 01:09:20 ewang Exp $
*
*/

/**
 * :tabSize=4:indentSize=4:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 */
package net.squiz.matrix.core;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.debug.*;
import javax.swing.tree.*;
import org.w3c.dom.*;
import java.beans.*;
import java.util.*;
import java.awt.Color;
import java.io.*;

/**
 * Assets are created automatically by the system, and therefore a provided as
 * a lookup device for the assets properties and its representing child nodes.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class Asset implements MatrixConstants, Serializable {

	protected final String id;
	protected String initName = "";
	private AssetType type;
	private int status;
	private boolean accessible;
	private String url = "";
	private String webPath = "";
	private boolean childrenLoaded = false;
	private int numKids = 0;
	private int totalKidsLoaded = 0;
	protected Map nodes = null;
	private int sort_order = 0;

	//{{{ Public Methods

	public Asset(String id) {
		this.id = id;
		//nodes = new ArrayList();
		nodes = new HashMap();
	}

	/**
	 * Constructs an asset.
	 * The asset is constructed from the specified XML element and 'attaches'
	 * it to the specified parent TreeNode, at the specified index
	 * @param assetElement the xml element that represents this asset
	 * @param parent the parent to attach this asset to
	 * @param index the index where to create the asset under the parent
	 * @return the Asset
	 */
	public Asset(Element assetElement, MatrixTreeNode parent, int index) {
		this(MatrixToolkit.rawUrlDecode(assetElement.getAttribute("assetid")));
		String linkid = processAssetXML(assetElement, true);
		createNode(linkid, getLinkType(assetElement), parent, index);
		// we need to tell the new node what its link type is
		setLinkType(assetElement, linkid);
	}

	public void setTotalKidsLoaded(int kidsLoaded) {
		this.totalKidsLoaded = kidsLoaded;
	}

	public int getTotalKidsLoaded() {
		return totalKidsLoaded;
	}

	public String toString() {
		return "[" + id + "]";
	}

	/**
	 * Returns the assetid of this Asset
	 * @return the assetid of this Asset
	 */
	public String getId() {
		return id;
	}

	/**
	 * Returns the name of this Asset.
	 * @return the name of this Asset
	 */
	public String getName(String linkid) {
		MatrixTreeNode node = getNode(linkid);
		if (node == null) {
			return "";
		}
		return node.getName();
	}

	public boolean setName(String name, String linkid) {
		if (linkid.equals("")) {
			Iterator nodeIterator = getTreeNodes();
			while (nodeIterator.hasNext()) {
				MatrixTreeNode node = (MatrixTreeNode) nodeIterator.next();
				if (node != null) {
					node.setName(name);
					return true;
				}
			}
		}

		MatrixTreeNode node = getNode(linkid);
		if (node != null) {
			String nodeName = node.getName();
			if (nodeName != null) {
				if (nodeName.equals(name)) {
					return false;
				}
			}
			node.setName(name);
		} else {
			return false;
		}
		return true;
	}

	public void addNode(MatrixTreeNode node, String linkid) {
		nodes.put(linkid, node);
	}

	public MatrixTreeNode getNode(String linkid) {
		if (nodes.containsKey(linkid)) {
			return (MatrixTreeNode)nodes.get(linkid);
		}
		return null;
	}

	/**
	* Returns Links for this Asset. Each asset can have more than one node assign to it.
	* @return String[]
	*/
	public String[] getLinkIds() {
		String[] linkIds = new String[nodes.size()];
		Iterator nodeIterator = getTreeNodes();
		int i = 0;
		while (nodeIterator.hasNext()) {
			MatrixTreeNode node = (MatrixTreeNode) nodeIterator.next();
			if (node != null) {
				linkIds[i] = node.getLinkid();
				i++;
			}
		}
		return linkIds;
	}

	public Iterator getTreeNodes() {
		return nodes.values().iterator();
	}

	/**
	 * Returns the AssetType of this asset.
	 * @return the AssetType of this asset
	 */
	public AssetType getType() {
		return type;
	}

	/**
	 * Returns the status of this Asset
	 * @return the status of this Asset
	 */
	public int getStatus() {
		return status;
	}

	/**
	 * Returns the URL of this Asset.
	 * @return the URL of this Asset
	 */
	public String getURL() {
		return url;
	}

	/**
	 * Returns the web paths of this Asset.
	 * @return the web paths of this Asset
	 */
	public String getWebPath() {
		return webPath;
	}

	/**
	 * Returns TRUE if this asset is Accessible
	 * @return TRUE if this asset is accessible
	 */
	public boolean isAccessible() {
		return accessible;
	}

	/**
	 * Returns the number of children this asset has
	 * @return the number of children this asset has
	 */
	public int getNumKids() {
		return numKids;
	}

	public void update(Element assetElement) {
		processAssetXML(assetElement, false);
	}

	public String processAssetXML(Element assetElement) {
		String linkid = processAssetXML(assetElement, false);
		return linkid;
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
			if (!parent.hasChildWithLinkid(linkid))
				return createNode(linkid, getLinkType(assetElement), parent, index);
			return parent.getChildWithLinkid(linkid);
	}


	public boolean childrenLoaded() {
		return childrenLoaded;
	}

	public void setChildrenLoaded(boolean loaded) {
		childrenLoaded = loaded;
	}

	/**
	 * Returns an Iterator of the TreeNodes
	 * That this Asset is currently representing
	 * @return an Iterator of the nodes of this Asset.
	 */
	//public Iterator getTreeNodes() {
	//	return nodes.iterator();
	//}

	/**
	 * Returns the tree nodes with the specified linkid. If is possible for an
	 * asset to have more than one linkid. This will occur when it has the same
	 * parent, but a different treeid in the tree.
	 * @param linkid the linkid of the wanted nodes
	 * @return the tree nodes with the specified linkid
	 * @see getTreeNodes()
	 */
	public MatrixTreeNode[] getNodesWithLinkid(String linkid) {
		Iterator nodes = getTreeNodes();
		List wantedNodes = null;
		while (nodes.hasNext()) {
			MatrixTreeNode node = (MatrixTreeNode) nodes.next();
			if (node.getLinkid().equals(linkid)) {
				// lazily create
				if (wantedNodes == null)
					wantedNodes = new ArrayList();
				wantedNodes.add(node);
			}
		}
		if (wantedNodes == null) {
			return new MatrixTreeNode[0];
		} else {
			return (MatrixTreeNode[]) wantedNodes.toArray(
				new MatrixTreeNode[wantedNodes.size()]);
		}
	}

	public void propagateNode(Asset childAsset, String linkid, int linkType, int index) {
		Iterator nodeIterator = getTreeNodes();

		DefaultTreeModel[] components = MatrixTreeModelBus.getBusComponents();

		while (nodeIterator.hasNext()) {
			MatrixTreeNode node = (MatrixTreeNode) nodeIterator.next();

			// We need a way of checking if the shadow asset already exists, because all the
			// linkids are the same. So loop over all the children of the parent and check
			// if the names are the same. Unfortunately this is the only check...
			Enumeration children = node.children();
			boolean childExists = false;
			while (children.hasMoreElements()) {
				MatrixTreeNode fellowChild = (MatrixTreeNode) children.nextElement();
				if (fellowChild.getName() == childAsset.getName(linkid))
					childExists = true;
			}

			MatrixTreeNode child = node.getChildWithLinkid(linkid);

			if (child == null || (AssetManager.isShadowAsset(childAsset) && !childExists)) {
				//childAsset.createNode(linkid, linkType, node, index);
			} else if (!AssetManager.isShadowAsset(childAsset)) {
				if (node.hasPreviousNode()) {
					index++;
				}
				if (node.getIndex(child) != index) {
					MatrixTreeModelBus.moveNode(child, node, index);
				}
			}
		}
	}

	public void propagateChildren(MatrixTreeNode node) {
		Iterator nodes = getTreeNodes();
		while (nodes.hasNext()) {
			MatrixTreeNode nextNode = (MatrixTreeNode) nodes.next();

			// find the first node that has children and propogate its
			// children to this node
			if (nextNode.getChildCount() > 0) {
				Enumeration children = nextNode.children();
				int index = 0;
				while (children.hasMoreElements()) {
					MatrixTreeNode childNode
						= (MatrixTreeNode) children.nextElement();
					MatrixTreeNode newChild =
						childNode.getAsset().createNode(
							childNode.getLinkid(),
							childNode.getLinkType(),
							node
						);

					// we only want to insert the nodes into the current tree
					// that we are accessing to save on memory consumption
					MatrixTree tree = MatrixTreeBus.getLastExpandedTree();
					((DefaultTreeModel) tree.getModel()).insertNodeInto(newChild, node, index);
					index++;
				}
				return;
			}
		}
	}

	/**
	 * Returns a status colour based on the status of this
	 * Asset
	 *
	 * @return the status colour based on the status of this
	 * Asset. If the status unknown, Color.RED
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
				//System.err.println("Unknown status :" + status);
				return UNKNOWN_STATUS_COLOUR;
		}
	}


	private boolean setNodeSortOrder(String linkid, int sort_order) {
		MatrixTreeNode[] nodes = getNodesWithLinkid(linkid);
		boolean changed = false;
		for (int i = 0; i < nodes.length; i++) {
			if (nodes[i].getSortOrder() != sort_order) {
				nodes[i].setSortOrder(sort_order);
				changed = true;
			}
		}
		return changed;
	}

	//}}}
	//{{{ Protected Methods

	protected String processAssetXML(Element assetElement, boolean create) {

		String linkid = "", name = "", typeCode = "";
		boolean accessible = false;
		int linkType = 0, status = 0, numKids = 0;
		boolean hasLinkType = false, hasStatus = false, hasName = false,
		hasAccessible = false, hasUrl = false, hasWebPath = false, hasSortOrder = false,
		hasNumKids = false, hasTypeCode = false;
		AssetType type = this.type;

		if (assetElement.hasAttribute("linkid"))
			linkid = MatrixToolkit.rawUrlDecode(assetElement.getAttribute("linkid"));



		// the following attributes can be modified after creation
		try {
			if (assetElement.hasAttribute("type_code")) {
				typeCode = assetElement.getAttribute("type_code");
				type = AssetManager.getAssetType(typeCode);
				hasTypeCode = true;
			}
			if (assetElement.hasAttribute("name")) {
				name = MatrixToolkit.rawUrlDecode(assetElement.getAttribute("name"));
				hasName = true;
			}

			if (assetElement.hasAttribute("link_type")) {
				linkType = Integer.parseInt(assetElement.getAttribute("link_type"));
				hasLinkType = true;
			}
			if (assetElement.hasAttribute("status")) {
				status = Integer.parseInt(assetElement.getAttribute("status"));
				hasStatus = true;
			}
			if (assetElement.hasAttribute("num_kids")) {
				numKids = Integer.parseInt(assetElement.getAttribute("num_kids"));
				hasNumKids = true;
			}
			if (assetElement.hasAttribute("accessible")) {
				accessible = assetElement.getAttribute("accessible").equals("1");
				hasAccessible = true;
			}
			if (assetElement.hasAttribute("url")) {
				String url = assetElement.getAttribute("url");
				hasUrl = true;
			}
			if (assetElement.hasAttribute("web_path")) {
				String webPath = assetElement.getAttribute("web_path");
				hasWebPath = true;
			}
			if (assetElement.hasAttribute("sort_order")) {
				this.sort_order	= Integer.parseInt(assetElement.getAttribute("sort_order"));
				hasSortOrder = true;
			}

		} catch (NumberFormatException exp) {
			System.out.println(exp.getMessage());
		}

		if (create) {
			this.initName	= name;
			this.accessible = accessible;
			this.status     = status;
			this.url        = url;
			this.webPath    = webPath;
			this.numKids    = numKids;
			this.type       = AssetManager.getAssetType(typeCode);
		} else {
			this.initName	= name;
			boolean refresh = false;

			// TODO: Change this back to what it was
			boolean linkTypeChanged = setLinkType(linkType, linkid);

			if (linkTypeChanged)
				Log.log("Link Type changed for asset "  + id, Asset.class);

			// the following properties require that an event
			// is fired to notify that the nodes of this asset
			// have visually changed
			refresh |= hasName       && setName(name, linkid);
			refresh |= hasStatus     && setStatus(status);
			refresh |= hasLinkType   && linkTypeChanged;
			refresh |= hasAccessible && setAccessible(accessible);
			refresh |= hasNumKids    && setNumKids(numKids);
			refresh |= hasTypeCode   && setTypeCode(type);
			refresh |= hasSortOrder	 && setNodeSortOrder(linkid, this.sort_order);

			if (refresh)
				nodesChanged();
			if (hasUrl && setUrl(url))
				updateUrls();
			if (hasWebPath && setWebPath(webPath))
				updateWebPaths();
		}
		return linkid;
	}

	protected MatrixTreeNode createNode(
		String linkid,
		int linkType,
		MatrixTreeNode parent,
		int index) {
			MatrixTreeNode node = createNode(linkid, linkType, parent);
			if (parent != null) {
				MatrixTreeModelBus.insertNodeInto(node, parent, index);
			}
			return node;
	}

	/**
	 * Creates a node, but does not add it to the model bus or to the parent:
	 * it simply returns it. The parent it used to generate a url for this node
	 *
	 * @param linkid the linkid of this node
	 * @param parent the parent of this node
	 * @param index the index where this node
	 */
	protected MatrixTreeNode createNode(String linkid, int linkType, MatrixTreeNode parent) {
		MatrixTreeNode node = new MatrixTreeNode(
			this,
			linkid,
			linkType,
			getNodeURL(parent),
			webPath,
			this.initName,
			this.sort_order
		);
		addNode(node, linkid);

		return node;
	}



	//}}}
	//{{{ Package Private Methods

	/**
	 * Performs a diff between the specified linkids and the linkids of the child
	 * nodes of all this assets nodes, and removes any nodes that
	 * are not specified by the linkids
	 *
	 * @param linkids the linkids to diff, removing any not within this list
	 */
	void removeDiffChildNodes(String[] linkids) {
		Iterator nodeIterator = getTreeNodes();
		List staleNodes = null;
		boolean found = false;

		// foreach of the nodes that this asset prepresents...
		while (nodeIterator.hasNext()) {
			MatrixTreeNode node = (MatrixTreeNode) nodeIterator.next();
			Enumeration children = node.children();
			// loop over all the child nodes of this asset
			// and see if any of nodes are not in the specfied linkids.
			// If they are not, we need to remove them.
			while (children.hasMoreElements()) {
				MatrixTreeNode childNode = (MatrixTreeNode) children.nextElement();

				if (childNode instanceof ExpandingNextNode && (node.getChildCount() >= AssetManager.getLimit())) {
					continue;
				} else if (childNode instanceof ExpandingPreviousNode && (getTotalKidsLoaded() > 0 )) {
					continue;
				}

				String linkid = childNode.getLinkid();
				found = false;
				for (int i = 0; i < linkids.length; i++) {
					if (linkids[i].equals(linkid)) {
						found = true;
						break;
					}
				}
				if (!found) {
					// lazily create
					if (staleNodes == null)
						staleNodes = new ArrayList();
					staleNodes.add(childNode);
				}

			}
		}

		if (staleNodes != null) {
			Iterator staleIterator = staleNodes.iterator();
			DefaultTreeModel[] components = MatrixTreeModelBus.getBusComponents();
			while (staleIterator.hasNext()) {
				MatrixTreeNode node = (MatrixTreeNode) staleIterator.next();
				// do not want to remove next, and previous nodes
				if (node instanceof ExpandingPreviousNode) {
					// we have to set the previous node as the first child
					MatrixTreeNode parent = (MatrixTreeNode)node.getParent();
					parent.insert(node,0);
					for (int i = 0; i < components.length; i++) {
						DefaultTreeModel model = components[i];
						model.nodeStructureChanged(parent);
					}
				} else {
					Log.log("removing " + node + " in Asset", Asset.class);
					MatrixTreeModelBus.removeNodeFromParent(node);
				}
			}
		}
	}

	void updateChildren() {}

	//}}}
	//{{{ Private Methods

	private void nodesChanged() {
		Iterator nodeIterator = getTreeNodes();
		while (nodeIterator.hasNext()) {
			MatrixTreeNode node = (MatrixTreeNode) nodeIterator.next();

			// boolean to control the refresh of the ExpandingNodes
			boolean removedNext = false;
			boolean removedPrev = false;

			if (!node.hasNextNode() && node.getChildCount() >= AssetManager.getLimit() && getNumKids() > AssetManager.getLimit() && node.getParent() != null) {
				// BUG1666-1, added condition to check number of kids
				// getChildCount have not been updated here, i.e. the count is from the previous run
				MatrixTreeNode nextNode = (MatrixTreeNode) new ExpandingNextNode(getNumKids(), node.getChildCount(), getTotalKidsLoaded());
				MatrixTreeModelBus.insertNodeInto((MatrixTreeNode) nextNode, node, node.getChildCount());
			} else if (node.hasNextNode() && (getNumKids() <= AssetManager.getLimit())) {
				// TODO:
				// when the last asset in the current set is removed, the nextNode should be removed
				// and the assetmap should show the previous set automatically
				MatrixTreeNode nextNode = (MatrixTreeNode) node.getChildAt(node.getChildCount()-1);
				MatrixTreeModelBus.removeNodeFromParent(nextNode);
				removedNext = true;
			}

			if (node.hasPreviousNode() && (getNumKids() == 0)) {
				MatrixTreeNode prevNode = (MatrixTreeNode) node.getChildAt(0);
				MatrixTreeModelBus.removeNodeFromParent(prevNode);
				removedPrev = true;
			}

			// update the count and the name, then notify the bus for refresh, BUG 1538
			if (node.hasPreviousNode() && !removedPrev) {
				ExpandingNode prevNode = (ExpandingNode) node.getChildAt(0);
				prevNode.setParentTotalAssets(getNumKids());
				prevNode.setName(prevNode.getName());
				MatrixTreeModelBus.nodeChanged(prevNode);
			}
			if (node.hasNextNode() && !removedNext) {
				ExpandingNode nextNode = (ExpandingNode) node.getChildAt(node.getChildCount()-1);
				nextNode.setParentTotalAssets(getNumKids());
				nextNode.setName(nextNode.getName());
				MatrixTreeModelBus.nodeChanged(nextNode);
			}

			MatrixTreeModelBus.nodeChanged(node);
		}
	}

	private void updateUrls() {
		Iterator nodeIterator = getTreeNodes();
		while (nodeIterator.hasNext()) {
			MatrixTreeNode node = (MatrixTreeNode) nodeIterator.next();
			node.propagateUrl(url);
		}
	}

	private void updateWebPaths() {
		Iterator nodeIterator = getTreeNodes();
		while (nodeIterator.hasNext()) {
			MatrixTreeNode node = (MatrixTreeNode) nodeIterator.next();
			node.propagateWebPath(webPath);
		}
	}

	/**
	 * Generates a url for a node based on the parent node
	 * @param parent the parent node of the node
	 */
	private String getNodeURL(MatrixTreeNode parent) {
		// if we have a url then we are an entity like a site
		// so set the url to our url, otherwise use the parent's url
		String nodeUrl = "";
		if (url != null && (!url.equals("")))
			nodeUrl = url;
		else if (parent != null)
			nodeUrl = parent.getURL();
		return nodeUrl;
	}

	private boolean setNumKids(int count) {
		if (numKids == count)
			return false;
		numKids = count;

		// reset node set count if this node no longer has any kids
		if (numKids == 0) {
			setTotalKidsLoaded(0);
		}

		return true;
	}

	private boolean setStatus(int newStatus) {
		if (status == newStatus)
			return false;
		status = newStatus;
		return true;
	}

	private boolean setTypeCode(AssetType newTypeCode) {
		if (type.equals(newTypeCode))
			return false;
		type = newTypeCode;
		return true;
	}

	protected int getLinkType(Element assetElement) {
		return Integer.parseInt(assetElement.getAttribute("link_type"));
	}

	private boolean setLinkType(Element assetElement, String linkid) {
		return setLinkType(getLinkType(assetElement), linkid);
	}

	private boolean setLinkType(int newLinkType, String linkid) {
		MatrixTreeNode[] nodes = getNodesWithLinkid(linkid);
		boolean changed = false;
		for (int i = 0; i < nodes.length; i++) {
			if (nodes[i].getLinkType() != newLinkType) {
				nodes[i].setLinkType(newLinkType);
				changed = true;
			}
		}
		return changed;
	}

	private boolean setAccessible(boolean isAccessible) {
		if (accessible == isAccessible)
			return false;
		accessible = isAccessible;
		return true;
	}

	private boolean setUrl(String newUrl) {
		if (url.equals(newUrl))
			return false;
		url = newUrl;
		return true;
	}

	private boolean setWebPath(String newWebPath) {
		if (webPath.equals(newWebPath))
			return false;
		webPath = newWebPath;
		return true;
	}
}

