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
* $Id: AssetManager.java,v 1.2 2004/06/29 03:39:30 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;


import java.util.*;
import org.w3c.dom.*;
import java.net.*;
import java.io.IOException;
import javax.swing.tree.*;
import javax.swing.*;

/**
 * The AssetManager handles all requests for processing assets that
 * belong in the MySource Matrix system. This includes initialising the assets
 * and init time, refreshing assets, moving, new linking cloning removing.
 * The AssetManager also handles the asset types, and receiving child assets
 * when an asset is expanded.
 *  
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetManager implements ReloadAssetListener {
	
	/** The singleton instance of the asset manager */
	public static final AssetManager INSTANCE = new AssetManager();
	
	/** The map of assets (assetid => asset) */
	private HashMap assets;
	
	/** The map of asset types (type_code => assetType) */
	private HashMap assetTypes;
	
	/** the assetid of the current logged in user */
	private String currentUserid;
	
	/** the type code of the current logged in user */
	private String currentUserType;
	
	/** The current user's workspace id */
	private String workspaceId;
	
	/** 
	 * The root node of the folder, which is not always the root folder.
	 * During teleportation, this node reflects the the node that was 
	 * teleported
	 */
	private MutableTreeNode rootNode;
	
	/** The new link command contant */
	public static final String LINK_TYPE_NEW_LINK = "new link";
	
	/** the move asset command constant */
	public static final String LINK_TYPE_MOVE = "move asset";
	
	/**The clone command constant */
	public static final String LINK_TYPE_CLONE = "clone";
	
	/** The loading text */
	public static final String LOADING_TEXT = "Loading...";
	
	/** The popup parameters used to popup windows during create link */
	public static String POPUP_PARAMS = "toolbar=0,menubar=0,location=0" +
			",status=0,scrollbars=1,resizable=1,width=650,height=400";
	
	/** The tree */
	private JTree tree;
	
	/** the placeholder node that is reused when loading child nodes */
	private MutableTreeNode loadingNode = new DefaultMutableTreeNode();
	
	/**
	 * Constructs a new Asset Manager
	 */
	private AssetManager() {
		loadingNode.setUserObject(LOADING_TEXT);
		// There is no public constructor, because we want
		// to ensure that the is only one instance of this object
	}
	
	/**
	 * Sets the tree
	 * 
	 * @param tree the Asset Tree
	 */
	public void setTree(JTree tree) {
		this.tree = tree;
	}
	
	/**
	 * Returns the AssetTree in use
	 * 
	 * @return the AssetTree
	 */
	public JTree getTree() {
		return tree;
	}
	
	/**
	 * fires an Event to notify the tree model that nodes where inserted
	 * 
	 * @param node the node that was inserted
	 * @param childIndices the indicies where it was inserted
	 */
	public void fireNodesWereInserted(TreeNode node, int[] childIndices) {
		if (tree == null)
			throw new IllegalStateException("The tree was not initialised!");
		((DefaultTreeModel) tree.getModel()).nodesWereInserted(node, childIndices);
	}
	
	/**
	 * Notifies the tree model that nodes were removed
	 * 
	 * @param node the parent node where the node was removed
	 * @param childIndices the indidicies where the node was removed
	 * @param removedNodes the removed nodes
	 */
	public void fireNodesWereRemoved(
			TreeNode node,
			int[] childIndices, 
			Object[] removedNodes) {
		
		if (tree == null)
			throw new IllegalStateException("The tree was not initialised!");
		((DefaultTreeModel) 
			tree.getModel()).nodesWereRemoved(node, childIndices, removedNodes);
	}
	
	/**
	 * Initialises the AssetMap by creating a store of Assets, Asset types, 
	 * any initialising the current user.
	 * 
	 * @throws IOException
	 */
	public void initialise() throws IOException {
		
		assets = new HashMap();
		assetTypes = new HashMap();
		JsEventManager.sharedInstance().addJsListener("reload_assets", this);
		
		Document response = doAssetRequest("<command action=\"initialise\" />");
		NodeList children = response.getDocumentElement().getChildNodes();
		
		for (int i = 0; i < children.getLength(); i++) {
			if (!(children.item(i) instanceof Element))
				continue;
			
			Element childElement = (Element) children.item(i);

			if (childElement.getTagName().equals("asset_types")) {
				NodeList xmlNodes = childElement.getChildNodes();
				processAssetTypesXML(xmlNodes);
			} else if (childElement.getTagName().equals("assets")) {
				processAssetsXML(childElement, null, false);
			} else if (childElement.getTagName().equals("current_user")) {
				processCurrentUserXML(childElement);
			} else if (childElement.getTagName().equals("workspace")) {
				processWorkspaceXML(childElement);
			}
		}
	}
	
	/**
	 * Reloads assets when requested from the javascript
	 * 
	 * @param e the Javascript event
	 */
	public void assetsReloaded(JsEvent e) {

		try {
			HashMap params = (HashMap) e.getParams();
			String[] assetids 
				= MatrixToolkit.split((String) params.get("assetids"), ",");
			
			for (int i = 0; i < assetids.length; i++) {
				System.out.println("reloading " + assetids[i]);
				reloadAsset(assetids[i]);
			}
			
		} catch (Exception ioe) {
			System.out.println("Exception when reloading assets");
			ioe.printStackTrace();
		}
	}
	
	/**
	 * processes the AssetTypes XML element at init time and constructs
	 * a store of asset types
	 * 
	 * @param xmlNodes the xml nodes that represent the asset types
	 */
	private void processAssetTypesXML(NodeList xmlNodes) {

		for (int i = 0; i < xmlNodes.getLength(); i++) {
			if (!(xmlNodes.item(i) instanceof Element))
				continue;
			Element assetTypeElement = (Element) xmlNodes.item(i);

			String typeCode       = assetTypeElement.getAttribute("type_code");
			String name           = assetTypeElement.getAttribute("name");
			boolean instantiable  = assetTypeElement.getAttribute("instantiable").equals("1");
			String version        = assetTypeElement.getAttribute("version");
			String allowedAccess  = assetTypeElement.getAttribute("allowed_access");
			String parentTypeCode = assetTypeElement.getAttribute("parent_type");
			String menuPath       = assetTypeElement.getAttribute("flash_menu_path");
			
			String[] menuPathArray = null;
			
			if (!menuPath.trim().equals(""))
				menuPathArray = MatrixToolkit.split(menuPath, "/\\/");
			else
				menuPathArray = new String[0];

			AssetType type = null;
			AssetType parentType = null;
			
			if (!assetTypes.containsKey(typeCode)) {
				type = new AssetType(typeCode);
				assetTypes.put(typeCode, type);
			} else {
				type = getAssetType(typeCode);
			}
			if (!(assetTypes.containsKey(parentTypeCode))) {
				parentType = new AssetType(parentTypeCode);
				assetTypes.put(parentTypeCode, parentType);
			} else {
				parentType = getAssetType(parentTypeCode);
			}
		
			type.setInfo(name, instantiable, version, allowedAccess, menuPathArray);
			type.setParentType(parentType);
			
			NodeList screenNodes = assetTypeElement.getChildNodes();
			
			for (int j = 0; j < screenNodes.getLength(); j++) {
				if (!(screenNodes.item(j) instanceof Element))
					continue;
				Element screenElement = (Element) screenNodes.item(j);
				String codeName = screenElement.getAttribute("code_name");
				String screenName = screenElement.getFirstChild().getNodeValue();
				type.addScreen(codeName, screenName);
			}
		}
	}
	
	/**
	 * Creates the root folder asset at init time
	 * 
	 * @param childElement the childElement that represents the root asset
	 * @return the Root AssetNode
	 */
	private AssetTreeNode createRootAsset(Element childElement) {
		
		String assetid  = MatrixToolkit.rawUrlDecode(childElement.getAttribute("assetid"));
		String name     = MatrixToolkit.rawUrlDecode(childElement.getAttribute("name"));
		String typeCode = childElement.getAttribute("type_code");
		
		AssetType type = getAssetType(typeCode);
		Asset rootFolder = new Asset(assetid, name, type);
		assets.put(assetid, rootFolder);
		MutableTreeNode node = createAssetNode(rootFolder, null, "", 0);
		rootNode = node;
		
		return (AssetTreeNode) node;
	}
	
	/**
	 * Processes the Asset XML element and creates/updates a store of assets
	 * 
	 * @param childElement the XML element representing the Assets
	 * @param parent the parent where these asset belong under
	 * @param refresh if TRUE frefresh current assets in the system
	 */
	private void processAssetsXML(
			Element childElement, 
			AssetTreeNode parent, 
			boolean refresh) {
		
		String parentAssetid = childElement.getAttribute("assetid");
	
		if ((rootNode == null) && !(assets.containsKey(parentAssetid)))
			parent = createRootAsset(childElement);
		NodeList xmlNodes = (NodeList) childElement.getChildNodes();

		int count = 0;
		for (int i = 0; i < xmlNodes.getLength(); i++) {
			
			if (!(xmlNodes.item(i) instanceof Element))
				continue;
			
			Element assetElement = (Element) xmlNodes.item(i);
			if (!(assetElement.getTagName().equals("asset")))
				continue;
			
			String linkid = MatrixToolkit.rawUrlDecode(
					assetElement.getAttribute("linkid"));
			Asset asset = processAssetXML(assetElement, refresh);
			
			// if this is us (the parent) then continue
			if (asset.getId().equals(parent.getAsset().getId()))
				continue;
			
			if (!asset.hasNode(linkid)) {
				createAssetNode(asset, parent, linkid, 0);
				
			// Sometimes, we load an asset that has already had some, but not
			// all of its assets loaded previously. The asset knows about its
			// child, but the parent does not yet have the node with this linkid
			// so we need to check this
				
			} else if (!parent.hasNodeWithLinkId(linkid)) {
				AssetTreeNode newNode = asset.createNode(linkid);
				parent.insert(newNode, 0);
			}
			count++;
		}
		
		((AssetTreeNode) parent).getAsset().setChildCount(count);
		((AssetTreeNode) parent).getAsset().setChildrenLoaded(true);
	}
	
	/**
	 * Processes the XML element for an individual asset
	 * 
	 * @param assetElement the XML element that represents this asset
	 * @param refresh if TRUE refresh the asset if it exists in the system
	 * @return the asset
	 */
	private Asset processAssetXML(Element assetElement, boolean refresh) {
		Asset asset = null;
		
		String assetid     = MatrixToolkit.rawUrlDecode(assetElement.getAttribute("assetid"));
		String name        = MatrixToolkit.rawUrlDecode(assetElement.getAttribute("name"));
		int childCount     = Integer.parseInt(assetElement.getAttribute("child_count"));
		String typeCode    = assetElement.getAttribute("type_code");
		int linkType	   = Integer.parseInt(assetElement.getAttribute("link_type"));
		boolean accessible = assetElement.getAttribute("accessible").equals("1");
		int status         = Integer.parseInt(assetElement.getAttribute("status"));
		String url         = assetElement.getAttribute("url");
		String webPath     = assetElement.getAttribute("web_path");
		//int sortOrder    = Integer.parseInt(assetElement.getAttribute("sort_order"));
		
		if (!assets.containsKey(assetid)) {
			AssetType type = getAssetType(typeCode);
			asset = createAsset(assetid, name, type, linkType, status, accessible, url, webPath);
		} else {			
			asset = getAsset(assetid);
			
			// make sure that if the url has changed in this asset 
			// that we propagate the url down the tree
			if (!(asset.getURL().equals(url)))
				propagateURL(asset, url);
			
			if (refresh) {
				asset.refresh(name, status, linkType, accessible, url, webPath);
				Iterator iterator = asset.getTreeNodes();
				while (iterator.hasNext()) {
					AssetTreeNode node = (AssetTreeNode) iterator.next();
					((DefaultTreeModel) tree.getModel()).nodeChanged(node);
				}
			}
		}
		
		asset.setChildCount(childCount);
		
		return asset;
	}
	
	/**
	 * Updates an assets structure in the tree. 
	 * 
	 * @param childElement the XML elements that represents the asset
	 * @param parent the parent asset of the asset to update
	 */
	public void updateAsset(Element childElement, Asset parent) {
		NodeList childNodes = (NodeList) childElement.getChildNodes();

		int index = 0;
		ArrayList linkids = new ArrayList();
		
		for (int i = 0; i < childNodes.getLength(); i++) {
			if (!(childNodes.item(i) instanceof Element))
				continue;
			Element assetElement = (Element) childNodes.item(i);
			Asset asset = processAssetXML(assetElement, true);

			// continue if this is us anyway
			if (parent.getId().equals(asset.getId()))
				continue;
			
			String linkid = MatrixToolkit.rawUrlDecode(assetElement.getAttribute("linkid"));
			linkids.add(linkid);
			int sortOrder = Integer.parseInt(assetElement.getAttribute("sort_order"));
			
			if (parent.childrenLoaded())
				parent.propogateNode(asset, linkid, sortOrder);
			index++;
		}
		parent.cleanNodes(linkids);
		parent.setChildCount(index);
	}
	
	/**
	 * Propagates a url of an asset down to all its children (not only)
	 * immediate children
	 * 
	 * @param parent the parent to traverse from
	 * @param url the url to propagate
	 */
	private void propagateURL(Asset parent, String url) {
		
		parent.setURL(url);
		
		Iterator childNodes = parent.getTreeNodes();
		while (childNodes.hasNext()) {
			AssetTreeNode node = (AssetTreeNode) childNodes.next();
			propagateURL(node, url);
		}
	}
	
	/**
	 * Traverses asset tree nodes that are only under the particular parent
	 * and propagates their preview url. It is important that we only consider
	 * nodes under a particular branch, as the asset may exist in some other part
	 * of the tree which may have a totally different url on the site level.
	 * 
	 * @param parent the parent to propagate from
	 * @param url the url to propagate
	 */
	private void propagateURL(AssetTreeNode parent, String url) {
		if (url.equals("") || url == null)
			return;
		
		// if we have a url then we want to set this as the preview
		// url of the node
		if (parent.getAsset().getURL().equals("")) {
			if (!parent.getAsset().getWebPath().equals(""))
				parent.setPreviewURL(url + "/" + parent.getAsset().getWebPath());
		} else {
			parent.setPreviewURL(parent.getAsset().getURL());
		}
		Enumeration children = parent.children();
		while (children.hasMoreElements()) {
			AssetTreeNode node = (AssetTreeNode) children.nextElement();
			propagateURL(node, url);
		}
	}
	
	/**
	 * Creates a <code>TreeNode</code> for an <code>Asset</code> that is identified
	 * by the specifed linkid. A null parent is permitted, if the 
	 * <code>TreeNode</code> is the root node.
	 * 
	 * @param asset the <code>Asset</code> that this node represents
	 * @param parent the parent <code>TreeNode</code> of the specified node
	 * @param linkid the linkid of the link between the new node and the parent node
	 * @param index the index of the child position where this node exists
	 * @return the newly created node
	 */
	public MutableTreeNode createAssetNode(
			Asset asset,
			AssetTreeNode parent,
			String linkid,
			int index) {
	
		AssetTreeNode node = (AssetTreeNode) asset.createNode(linkid);
		
		if (parent != null) {
			if (!(asset.getURL().equals(""))) {
				node.setPreviewURL(asset.getURL());
			} else {
				String parentURL = parent.getPreviewURL();
				if ((!parentURL.equals("")) && (!asset.getWebPath().equals("")))
					node.setPreviewURL(parentURL + "/" + asset.getWebPath());
				
			}
			parent.insert(node, index);
		}
		
		return node;
	}
	
	/**
	 * Returns the root node at this current time.
	 * 
	 * @return the root node
	 */
	public MutableTreeNode getRootNode() {
		return rootNode;
	}
	
	/**
	 * Deletes the objects in the system. This is necessary so that 
	 * when reloading the asset map in the same browser session, it does a
	 * full refresh of the assets, asset types etc
	 */
	public void deleteCachedObjects() {
		System.out.println("clearing cache");
		assets = null;
		assetTypes = null;
		rootNode = null;
	}
	
	/**
	 * Processes the current user XML element at init time
	 * 
	 * @param xmlNodes the xmlNodes that represent the current user
	 */
	private void processCurrentUserXML(Element xmlNodes) {
		currentUserid = xmlNodes.getAttribute("assetid");
		currentUserType = xmlNodes.getAttribute("type_code");
		// String name = xmlNodes.getAttribute("name");
	}
	
	/**
	 * Processes the workspace XML element at init time
	 * 
	 * @param xmlNodes the XML Element that represents the workspace
	 */
	private void processWorkspaceXML(Element xmlNodes) {
		String assetid = xmlNodes.getAttribute("assetid");
		String typeCode = xmlNodes.getAttribute("type_code");
		String name = xmlNodes.getAttribute("name");
		String linkid = xmlNodes.getAttribute("linkid");
		int status = Integer.parseInt(xmlNodes.getAttribute("status"));
		
		workspaceId = assetid;
		
		Asset workspace = createAsset(assetid, name, 
				getAssetType(typeCode), 0, status, true, "", "");
		
		assets.put(assetid, workspace);
		
		workspace.setChildCount(1);
		workspace.createNode(linkid);
	}
	
	/**
	 * Returns the workspace id of the workspace of the current user
	 * 
	 * @return the workspace id
	 */
	public String getWorkspaceId() {
		return workspaceId;
	}
	
	public String getCurrentUserTypeCode() {
		return currentUserType;
	}
	
	/**
	 * Creates an asset and adds it to the asset store
	 * 
	 * @param assetid the asset id 
	 * @param name the asset name
	 * @param type the type of the asset
	 * @param linkType the linktype to the parent
	 * @param status the status of this asset
	 * @param accessible if this asset is accessible
	 * @param url the url of this asset
	 * @param webPath the webpath of this asset
	 * @return the newly created asset
	 */
	public Asset createAsset(
			String assetid,
			String name, 
			AssetType type, 
			int linkType,
			int status, 
			boolean accessible, 
			String url, 
			String webPath) {
				
		if (assets.containsKey(assetid)) 
			throw new IllegalArgumentException("Asset " + assetid + 
					" already exists");
		
		Asset asset = new Asset(
				assetid, 
				name, 
				type,
				linkType, 
				status, 
				accessible, 
				url, 
				webPath);
		
		assets.put(assetid, asset);
		
		return asset;
	}
	
	/**
	 * Makes a request to the Matrix system for the child nodes of 
	 * the specifed node. The nodes will be automagically appended 
	 * to the root node, and any assets that are not currently apart
	 * of the <code>Asset</code> map will be loaded, and their 
	 * appropriate nodes will be added to the <code>Asset's</code> node 
	 * list. A placeholder loading node will be appended to the parent
	 * node during the loading process, and will be removed once the 
	 * loading of the child nodes has completed.
	 * 
	 * @param node The node whos children are to be loaded
	 * @throws IOException If the request could not be successfully made
	 */
	public void loadChildAssets(final AssetTreeNode node) throws IOException {

		insertLoadingNode(node);
		
		Runnable worker = new Runnable() {
			public void run() {
				StringBuffer xml = new StringBuffer("<command action=\"get assets\">");
				addAssetToXML(xml, node.getAsset());
				xml.append("</command>");
		
				Document response = doAssetRequest(xml.toString());
				
				// also refresh if the assets exist. If the user does a full
				// refresh of the asset map and this node is not expanded, but
				// has had its children loaded once before, the refresh method 
				// will mark them as not loaded, so they may require refreshing
				
				processAssetsXML(response.getDocumentElement(), node, true);
				
				// fire that the node structure has changed so that the loading node
				// is replaced with the actual nodes
				
				((DefaultTreeModel) tree.getModel()).nodeStructureChanged(node);
				
				// remove the placeholder node once all the nodes
				// have been appended to the parent node
				
				removeLoadingNode(node);
			}
		};
		SwingUtilities.invokeLater(worker);
	}
	
	/**
	 * Propagates child nodes of a node. When an asset has had its children
	 * loaded in one part of the tree, a request is no longer needed to acquire
	 * the assets but because this node is different, it needs to have the
	 * children of the other node appended to this node
	 *  
	 * @param node the node to propagate the children on
	 */
	public void propagateChildren(AssetTreeNode node) {
		Asset asset = node.getAsset();
		Iterator nodes = asset.getTreeNodes();
		
		while (nodes.hasNext()) {
			AssetTreeNode nextNode = (AssetTreeNode) nodes.next();
			
			// find the first node that has children and propogate its 
			// children to this node
			if (nextNode.getChildCount() > 0) {
				Enumeration children = nextNode.children();
				int index = 0;
				while(children.hasMoreElements()) {
					AssetTreeNode childNode 
						= (AssetTreeNode) children.nextElement();
					AssetTreeNode newChild 
						= childNode.getAsset().createNode(childNode.getLinkId());
					node.insert(newChild, index);
					index++;
				}
				return;
			}
		}
	}
	
	/**
	 * Inserts a placeholder loading node during the loading of 
	 * child assets of a particular asset.
	 *  
	 * @param parentNode the parent node to add the placeholder node
	 */
	public void insertLoadingNode(AssetTreeNode parentNode) {
		parentNode.insert(loadingNode, 0);
		fireNodesWereInserted(parentNode, new int [] { 
				parentNode.getIndex(loadingNode) });
	}
	
	/**
	 * Removes the placeholder loading node from the specified parent 
	 * 
	 * @param parentNode the node to remove the placeholding node from
	 */
	public void removeLoadingNode(AssetTreeNode parentNode) {
		int index = parentNode.getIndex(loadingNode);
		if (index == -1)
			return;
		
		Object [] removedNodes = new Object [] { loadingNode };
		int [] childIndices = new int [] { index };
		parentNode.remove(index);
		fireNodesWereRemoved(parentNode, childIndices, removedNodes);
	}
	
	/**
	 * Returns the <code>Asset</code> with the specifed assetid.
	 * 
	 * @param assetid the asset of the wanted asset
	 * @return the <code>Asset</code>
	 */
	public Asset getAsset(String assetid) {
		return (Asset) assets.get(assetid);
	}
	
	/**
	 * Creates a link between two assets. The link is either a new link, a move
	 * or a clone. 
	 * @param linkType the type of link to create. This can be
	 *  <pre>
	 *     AssetManager.LINK_TYPE_MOVE
	 *     AssetManager.LINK_TYPE_NEW_LINK
	 *     AssetManager.LINK_TYPE_CLONE
	 *  </pre>
	 * @param mover the asset that is being moved/new linked/cloned
	 * @param parent the parent where the new link is created under
	 * @param index the index to create the new link
	 */
	public void createLink(
			String linkType, 
			AssetTreeNode mover, 
			AssetTreeNode parent, 
			int index) {
		
		String fromParentId 
			= ((AssetTreeNode) mover.getParent()).getAsset().getId();
		String toParentId = ((AssetTreeNode) parent).getAsset().getId();
		String linkid = ((AssetTreeNode) mover).getLinkId();	
		
		fromParentId = MatrixToolkit.rawUrlEncode(fromParentId, true);
		toParentId = MatrixToolkit.rawUrlEncode(toParentId, true);

		String xml = generateCreateLinkXML(
				linkType, 
				fromParentId, 
				toParentId, 
				linkid, 
				index);
		
		Document response = doAssetRequest(xml);
		if (response == null)
			return;
		
		NodeList children = response.getDocumentElement().getChildNodes();
		
		String url = getUrlFromResponse(children);
		
		if (url == null) {
			moveNodeOnBranch(parent, mover, index);
			System.out.println("URL IS NULL");
		} else {
			String basePath = MySource.INSTANCE.getBaseURL().toString();
			AssetMap.INSTANCE.openWindow(url, "Moving Asset", POPUP_PARAMS);
		}
	}
	
	/**
	 * Moves a node on the same branch that it exists
	 * 
	 * @param parent the parent branch where the node exists
	 * @param mover the node to move
	 * @param index the new index to where the node will exists
	 */
	public void moveNodeOnBranch(
			AssetTreeNode parent,
			AssetTreeNode mover,
			int index) {
		
		int oldIndex = parent.getIndex(mover);
		
		if (oldIndex == index)
			return;
		
		parent.remove(oldIndex);
		
		// we need to do this because we remove the node before inserting
		// it in the other position. When this happens, the index becomes out
		// by 1 
		
		if (oldIndex < index)
			index -= 1;
		
		parent.insert(mover, index);
		fireNodesWereRemoved(
				parent, new int [] { oldIndex }, new Object [] { mover });
		fireNodesWereInserted(parent, new int [] { index });
	}
	
	/**
	 * Adds an asset to the Mysource Matrix system. A request is made to the
	 * mysource matrix system with information about the new asset, The response
	 * is processes and the asset is added accordingly.
	 * 
	 * @param parent the parent where the asset will exist
	 * @param typeCode the typeCode of the new asset
	 * @param index the index of the new asset
	 */
	public void addAsset(AssetTreeNode parent, String typeCode, int index) {
		
		String parentAssetid = MatrixToolkit.rawUrlEncode(
				parent.getAsset().getId(), true);
		String xml = "<command action=\"get url\" cmd=\"add\" " 
			+ "parent_assetid=\"" + parentAssetid
			+ "\" pos=\"" + index + "\" type_code=\""+ typeCode + "\" />";
		
		Document response = null;
		
		try {
			response = MySource.INSTANCE.doRequest(xml);
		} catch (IOException ioe) {
			System.out.println("There was an error: " + ioe.getMessage());
		}
		if (response == null)
			return;
			
		NodeList children = response.getDocumentElement().getChildNodes();
		String url = getUrlFromResponse(children);
		
		try {
			AssetMap.getUrl(new URL(url));
		} catch (MalformedURLException mue) {
		}
	}
	
	/**
	 * Returns the url from an XML response 
	 * 
	 * @param children the node list
	 * @return the URL
	 */
	private String getUrlFromResponse(NodeList children) {
		for (int i = 0; i < children.getLength(); i++) {
			if (!(children.item(i) instanceof Element))
				continue;
			Element element = (Element) children.item(i);
			if (element.getTagName().equals("url")) {
				return element.getFirstChild().getNodeValue();
			}
		}
		return null;
	}
	
	/**
	 * Generates the xml to create a link between two assets. The link type can
	 * be a move/new link or a clone.
	 * 
	 * @param linkType type type of link to create. Which can be:
	 * <pre>
	 * 	AssetManager.LINK_TYPE_CREATE_LINK
	 *  AssetManager.LINK_TYPE_MOVE
	 *  AssetManager.LINK_TYPE_CLONE
	 * </pre>
	 * @param fromParentId the asset id where the asset was linked at
	 * @param toParentId the new parent asset id 
	 * @param linkid the linkid of the previous link
	 * @param index the index of the new position
	 * @return the XML to create the link
	 */
	public String generateCreateLinkXML(
		String linkType,
		String fromParentId, 
		String toParentId, 
		String linkid, 
		int index) {
			StringBuffer xml = new StringBuffer("<command action=\"" + linkType + "\"");
			xml.append(" from_parent_assetid=\"").append(fromParentId).append("\"");
			xml.append(" to_parent_assetid=\"").append(toParentId).append("\"");
			xml.append(" linkid=\"").append(linkid).append("\"");
			xml.append(" to_parent_pos=\"").append(index).append("\" />");
		
		return xml.toString();
	}
	
	/**
	 * Reloads all assets that are curerntly expanded. If the assets are not
	 * expanded, they are marked as not loaded and will be refreshed next time
	 * they are expanded.
	 */
	public void reloadAllAssets() {
		Iterator iterator = assets.values().iterator();
		
		StringBuffer xml = new StringBuffer("<command action=\"get assets\">");
		while (iterator.hasNext()) {
			Asset asset = (Asset) iterator.next();
			Iterator nodes = asset.getTreeNodes();
			
			while(nodes.hasNext()) {
				AssetTreeNode node = (AssetTreeNode) nodes.next();
				if (tree.isExpanded(new TreePath(node.getPath()))) {
				 		addAssetToXML(xml, asset);
				 		break;
				} else {
					if (asset.childrenLoaded())
						asset.setChildrenLoaded(false);
				}
			}
		}
		xml.append("</command>");
		
		Document response = null;
		try {
			response = MySource.INSTANCE.doRequest(xml.toString());
		} catch(IOException ioe) {
			ioe.printStackTrace();
		}
		processMultipleAssetsXML(response.getDocumentElement());
	}
	
	/**
	 * Reloads the specified asset and its immediate children.
	 * 
	 * @param assetid the assetid of the asset to reload
	 */
	public void reloadAsset(String assetid) {
		
		if (!assets.containsKey(assetid))
			return;
		
		StringBuffer xml = new StringBuffer("<command action=\"get assets\">");
		addAssetToXML(xml, assetid);
		xml.append("</command>");
		
		Document response = doAssetRequest(xml.toString());
		
		String parentAssetid = MatrixToolkit.rawUrlDecode(
				response.getDocumentElement().getAttribute("assetid"));
		Asset parent = getAsset(parentAssetid);
		
		updateAsset(response.getDocumentElement(), parent);
	}
	
	/**
	 * Processes XML from the matrix system that might contain either one asset
	 * and its children, or multiple assets and their children
	 * 
	 * @param childElement the xml element to process
	 */
	private void processMultipleAssetsXML(Element childElement) {
		NodeList children = (NodeList) childElement.getChildNodes();
		
		// if there is an assetid attribute, then there is only this asset and
		// its children to reload
		
		if (childElement.hasAttribute("assetid")) {
			String assetid 
				= MatrixToolkit.rawUrlDecode(childElement.getAttribute("assetid"));
			Asset parent = getAsset(assetid);
			updateAsset(childElement, parent);
		} else {
			NodeList subChildren = childElement.getChildNodes();
			for (int i = 0; i < subChildren.getLength(); i++) {
				if (!(subChildren.item(i) instanceof Element))
					continue;
				Element nextElement = (Element) subChildren.item(i);
				String assetid 
					= MatrixToolkit.rawUrlDecode(nextElement.getAttribute("assetid"));
				Asset parent = getAsset(assetid);
				updateAsset(nextElement, parent);
			}
		}
	}
	
	/**
	 * Adds an XML node to the specified string buffer. The xml can be
	 * used for requests for assets.
	 * 
	 * @param xml the string buffer that contains the xml
	 * @param asset the asset to add.
	 */
	private void addAssetToXML(StringBuffer xml, Asset asset) {
		addAssetToXML(xml, asset.getId());
	}
	
	/**
	 * Adds an XML node to the specified string buffer. The xml can be
	 * used for requests for assets.
	 * 
	 * @param xml the string buffer that contains the xml
	 * @param assetid the assetid to add.
	 */
	private void addAssetToXML(StringBuffer xml, String assetid) {
		xml.append("<asset assetid=\"").append(
				MatrixToolkit.rawUrlEncode(assetid, false)).append("\" />");
	}
	
	/**
	 * Does a request to the Mysource Matrix System and returns the XML
	 * response 
	 * 
	 * @param xml the xml to send to the Matrix System
	 * @return the XML response
	 */
	public Document doAssetRequest(String xml) {
		Document response = null;
		try {
			response = MySource.INSTANCE.doRequest(xml);
		} catch (IOException ioe) {
			((AssetTree) tree).throwVisibleError(
					"Error", "Could not do request: " + ioe.getMessage());
		}
		
		return response;
	}
	
	/**
	 * Returns the asset type given a type code
	 * 
	 * @param typeCode the type code of the wanted asset type
	 * @return the asset type
	 */
	public AssetType getAssetType(String typeCode) {
		return (AssetType) assetTypes.get(typeCode);
	}
	
	/**
	 * Returns the current user asset
	 * 
	 * @return the current user
	 */
	public Asset getCurrentUser() {
		return getAsset(currentUserid);
	}
	
	/**
	 * Returns the asset types
	 * 
	 * @return the asset types
	 */
	public Iterator getAssetTypes() {
		return assetTypes.values().iterator();
	}
}
