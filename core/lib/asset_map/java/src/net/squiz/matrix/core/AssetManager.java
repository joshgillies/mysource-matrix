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
* $Id: AssetManager.java,v 1.9 2008/12/14 22:50:32 bpearson Exp $
*
*/

package net.squiz.matrix.core;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.debug.*;
import java.util.*;
import org.w3c.dom.*;
import java.io.IOException;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import net.squiz.matrix.ui.*;
import net.squiz.matrix.assetmap.*;


/**
 * The Asset Manager handles processing the xml return upon request to the matrix
 * system. It also serves as a central repository for the assets and asset types.
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetManager {

	/* The map of assets (assetid => asset) */
	private static Map assets = new HashMap();
	/* The map of asset types (type_code => assetType) */
	private static Map assetTypes = new HashMap();
	private static MatrixTreeNode root;
	private static String currentUserid;
	private static String currentUserType = "root_user";
	private static String lastRequest = "";
	private static Date now = new Date();
	private static Date lastRequestTime = new Date(now.getTime() - (2 * 60 * 60 * 1000));
	private static Element lastResponse = null;
	private static String workspaceid;
	private static boolean isInited = false;
	private static int limit = 0;
	//TODO: MM: thinking of a better way to do this stuff with initialisiation
	private static EventListenerList listenerList = new EventListenerList();

	// cannot instantiate
	private AssetManager() {}

	/**
	 * Initialises the information needed for the Asset Map.
	 * An xml request is made to the matrix system, which returns xml.
	 * @return the root folder node of the matrix system.
	 * @throws IOException if the request to matrix fails
	 */
	public static MatrixTreeNode init() throws IOException {

		if (isInited)
			throw new IllegalStateException("Already Inited");

		Document response = null;
		response = Matrix.doRequest("<command action=\"initialise\" />");
		NodeList children = response.getDocumentElement().getChildNodes();

		for (int i = 0; i < children.getLength(); i++) {
			if (!(children.item(i) instanceof Element))
				continue;
			Element childElement = (Element) children.item(i);

			if (childElement.getTagName().equals("asset_types")) {
				NodeList xmlNodes = childElement.getChildNodes();
				processAssetTypesXML(xmlNodes);
			} else if (childElement.getTagName().equals("assets")) {
				root = processAssetsXML(childElement);
			}
		}
		fireInitialisationComplete(root);
		isInited = true;

		return root;
	}

	public static void addInitialisationListener(InitialisationListener l) {
		listenerList.add(InitialisationListener.class, l);
	}

	private static void fireInitialisationComplete(MatrixTreeNode root) {
		// Guaranteed to return a non-null array
		Object[] listeners = listenerList.getListenerList();
		InitialisationEvent evt = null;

		// Process the listeners last to first, notifying
		// those that are interested in this event
		for (int i = listeners.length - 2; i >= 0; i -= 2) {
			if (listeners[i] == InitialisationListener.class) {
				// Lazily create the event:
				if (evt == null)
					evt = new InitialisationEvent(root, root);
				((InitialisationListener) listeners[i + 1]).
					initialisationComplete(evt);
			}
		}
	}

	/**
	 * Processes the AssetTypes XML element at init time and constructs
	 * a store of asset types
	 * @param xmlNodes the xml nodes that represent the asset types
	 */
	private static void processAssetTypesXML(NodeList xmlNodes) {

		for (int i = 0; i < xmlNodes.getLength(); i++) {
			if (!(xmlNodes.item(i) instanceof Element))
				continue;
			Element assetTypeElement = (Element) xmlNodes.item(i);

			AssetType type = null;
			AssetType parentType = null;
			String typeCode = assetTypeElement.getAttribute("type_code");
			String parentTypeCode = assetTypeElement.getAttribute("parent_type_code");

			if (!(assetTypes.containsKey(parentTypeCode))) {
				// create a placement for the parent asset type if it doesn't
				// exist. We will set its info later when we come to it
				parentType = new AssetType(parentTypeCode);
				assetTypes.put(parentTypeCode, parentType);
			} else {
				parentType = getAssetType(parentTypeCode);
			}

			if (!assetTypes.containsKey(typeCode)) {
				type = new AssetType(typeCode);
				assetTypes.put(typeCode, type);
			} else {
				type = getAssetType(typeCode);
			}
			type.setInfo(assetTypeElement);
			type.setParentType(parentType);
		}
	}

	/**
	 * Processes the asset xml elements and returns the root folder node.
	 * @param rootElement the root xml element of assets
	 * @return the root folder node.
	 */
	private static MatrixTreeNode processAssetsXML(Element rootElement) {
		Node nextElement = null;
		NodeList nodes = (NodeList) rootElement.getChildNodes();
		int i = 0;

		// get the first Element which is the root folder element
		do {
			nextElement = nodes.item(i++);
		} while (!(nextElement instanceof Element));

		Element rootFolderElement = (Element) nextElement;
		RootFolder parentAsset = new RootFolder(rootFolderElement);
		assets.put(parentAsset.getId(), parentAsset);
		processAssetsXML(rootElement, parentAsset.getRootNode());

		return parentAsset.getRootNode();
	}

	/**
	 * Processes the Asset XML element and creates/updates a store of assets
	 * @param childElement the XML element representing the Assets
	 * @param parent the parent where these asset belong under
	 * @param refresh if TRUE frefresh current assets in the system
	 */
	private static void processAssetsXML(Element rootElement, MatrixTreeNode parent) {
		/*
		 The XML structure that is processed by this method is as follows:
		   <assets>                1
			 <asset ...>           2
				<asset ...>        3
				<asset ...>        3
			 </asset>              2
		   </assets>               1
		*/


		NodeList parentNodes = (NodeList) rootElement.getChildNodes();
		// level 2
		for (int i = 0; i < parentNodes.getLength(); i++) {
			if (!(parentNodes.item(i) instanceof Element))
				continue;
			Element parentElement = (Element) parentNodes.item(i);
			NodeList childNodes = (NodeList) parentElement.getChildNodes();
			int index = 0;

			// level 3
			for (int j = 0; j < childNodes.getLength(); j++) {
				if (!(childNodes.item(j) instanceof Element))
					continue;
				Element childElement = (Element) childNodes.item(j);
				String assetid = getIdFromElement(childElement);
				Asset asset = loadAsset(assetid, childElement, parent, index);
				index++;
			}
			parent.getAsset().setChildrenLoaded(true);
		}//end for
	}


	private static Asset loadAsset(
		String assetid,
		Element assetElement,
		MatrixTreeNode parent,
		int index) {
			Asset asset = null;
			if (!assets.containsKey(assetid)) {
				asset = new Asset(assetElement, parent, index);
				assets.put(assetid, asset);
			} else {
				asset = getAsset(assetid);
				if (parent == null)
					asset.processAssetXML(assetElement);
				else
					asset.processAssetXML(assetElement, parent, index);
			}

			return asset;
	}

	/*
	 * Processes the current user XML element at init time
	 * @param xmlNodes the xmlNodes that represent the current user Types
	 */
	private static void processCurrentUserXML(Element xmlNodes) {
		currentUserid = getIdFromElement(xmlNodes);
		currentUserType = xmlNodes.getAttribute("type_code");
		String name = xmlNodes.getAttribute("name");
	}

	public static void setLimit(int newLimit) {
		limit = newLimit;
	}

	public static int getLimit() {
		// default limit
		if (limit != 0) {
			return limit;
		} else	if (Matrix.getProperty("parameter.asset.limit") != null) {
			limit = Integer.parseInt(Matrix.getProperty("parameter.asset.limit"));
		} else {
			limit = 50;
		}
		return limit;
	}

	public static void refreshAsset(MatrixTreeNode parent, String direction, int start, int limit) throws IOException {
		String[] assetids = new String[] { parent.getAsset().getId() };
		Element element = makeRefreshRequest(assetids, direction, start, limit);
		processAssetsXML(element, parent);
	}

	public static void refreshAsset(MatrixTreeNode parent, String direction) throws IOException {
		refreshAsset(parent, direction, -1, -1);
	}

	public static int calcNextOffset(String assetid) {
		Asset asset = getAsset(assetid);
		asset.setTotalKidsLoaded(asset.getTotalKidsLoaded()+getLimit());
		return asset.getTotalKidsLoaded();
	}

	public static int calcPrevOffset(String assetid) {
		Asset asset = getAsset(assetid);
		asset.setTotalKidsLoaded(asset.getTotalKidsLoaded() - getLimit());
		return asset.getTotalKidsLoaded();
	}


	/**
	 * Performs a request to the matrix system for the specified assets. The
	 * xml is returned in the following format.
	 *
	 * <pre>
	 *    <assets>
	 *       <asset ...>
	 *          <asset ...>
	 *          <asset ...>
	 *       </asset>
	 *     </assets>
	 * </pre>
	 *
	 * @param assetids the list of assetids to refresh
	 * @return the xml element for the specified assetids
	 * @throws IOException if the request fails
	 */
	public static Element makeRefreshRequest(String[] assetids, String direction, int start, int limit) throws IOException {

		int startLoc = start;
		if (limit < 0) {
			limit = getLimit();
		}

		StringBuffer xml = new StringBuffer("<command action=\"get assets\" >");
		for (int i = 0; i < assetids.length; i++) {

			if (start < 0) {
				// start was not specified, calculate it
				if (direction.equals("prev")) {
					startLoc = calcPrevOffset(assetids[i]);
				} else if (direction.equals("next")) {
					startLoc = calcNextOffset(assetids[i]);
				} else if (direction.equals("base")) {
					Asset asset = getAsset(assetids[i]);
					asset.setTotalKidsLoaded(0);
					startLoc = 0;
				} else {
					Asset asset = getAsset(assetids[i]);
					if (asset != null) {
						startLoc = asset.getTotalKidsLoaded();
					}
				}
			} else {
				Asset asset = getAsset(assetids[i]);
				if (asset != null) {
					asset.setTotalKidsLoaded(startLoc);
				}
			}

			Asset asset = getAsset(assetids[i]);
			if (asset != null) {
				String[] linkids = asset.getLinkIds();
				if (assetids[i].equals("1")) {
					MatrixToolkit.addAssetToXML(xml, assetids[i], "0", startLoc, limit);
				} else {
					for (int j=0; j< linkids.length; j++) {
						MatrixToolkit.addAssetToXML(xml, assetids[i], linkids[j], startLoc, limit);
					}
				}
			}
		}
		xml.append("</command>");

		// If the request is the same as the last just sent out, why bother doing it again?
		Date currentTime = new Date(System.currentTimeMillis());
		long diffResponse = (currentTime.getTime() - lastRequestTime.getTime());
		Element currentElement = null;
		if (lastRequest.equals(xml.toString()) && (diffResponse >= 0 && diffResponse < 2000) && lastResponse != null) {
			currentElement = lastResponse;
		} else {
			Document doc = Matrix.doRequest(xml.toString());
			// Remember the last request/response
			lastRequest = xml.toString();
			lastRequestTime = new Date(System.currentTimeMillis());
			lastResponse = doc.getDocumentElement();
			currentElement = doc.getDocumentElement();
		}

		return currentElement;

//		Document doc = Matrix.doRequest(xml.toString());
//		return doc.getDocumentElement();
	}

	public static Element makeRefreshRequest(String[] assetids, String direction) throws IOException {
		return makeRefreshRequest(assetids, direction, -1, -1);
	}

	public static void refreshAssets(Element element) {
		NodeList childNodes = (NodeList) element.getChildNodes();
		for (int i = 0; i < childNodes.getLength(); i++) {
			if (!(childNodes.item(i) instanceof Element))
				continue;
			Element assetElement = (Element) childNodes.item(i);
			updateAsset(assetElement);
		}
	}

	public static String[] getAllRefreshableAssetids() {
		Iterator iterator = assets.values().iterator();
		List assetids = new ArrayList();

		while (iterator.hasNext()) {
			Asset asset = (Asset) iterator.next();
			Iterator nodes = asset.getTreeNodes();

			while(nodes.hasNext()) {
				MatrixTreeNode node = (MatrixTreeNode) nodes.next();
				if (node.getAsset().childrenLoaded()) {
					assetids.add(node.getAsset().getId());
					break;
				}
			}
		}
		return (String[]) assetids.toArray(new String[assetids.size()]);
	}

	public static void updateAsset(Element childElement) {
		String assetid = getIdFromElement(childElement);
		// if we dont have this asset then we can't update it or its children
		if (!assets.containsKey(assetid))
			return;
		Asset asset = getAsset(assetid);
		updateAsset(childElement, asset);
	}

	/**
	 * Updates an asset and its children using the information from the specified
	 * xml element. Calling this method may make changes to the tree structure
	 * and therefore  must be executed within the Event Dispach Thread.
	 * @param childElement the xml element to process
	 * @param parent the parent asset of the xml element.
	 */
	public static void updateAsset(Element childElement, Asset parent) {

		NodeList childNodes = (NodeList) childElement.getChildNodes();
		parent.processAssetXML(childElement);

		// create a set of linkids so that we can remove any nodes
		// that are no longer children of this asset
		List linkids = new ArrayList();
		boolean hasShadowChild = false;

		// assets in the xml are in the correct order. We dont use the sort
		// order because notice links and type 3 links have a sort order, but
		// are not shown in the tree.
		int index = 0;

		for (int i = 0; i < childNodes.getLength(); i++) {

			if (!(childNodes.item(i) instanceof Element))
				continue;
			Element assetElement = (Element) childNodes.item(i);


			String assetid = getIdFromElement(assetElement);
			if (assetid.indexOf(":") != -1) {
				hasShadowChild = true;
			}
			String linkid  = assetElement.getAttribute("linkid");
			int linkType   = Integer.parseInt(assetElement.getAttribute("link_type"));
			Asset asset    = loadAsset(assetid, assetElement, null, index);
			linkids.add(linkid);
			// this node might be new so we need to give
			// a parent a chance to add it
			parent.propagateNode(asset, linkid, linkType, index);
			parent.setChildrenLoaded(true);

			index++;
		}

		// remove the children that are not currently in the matrix system
		// if there were no nodes in the xml, this will remove all children
		parent.removeDiffChildNodes((String[]) linkids.toArray(new String[linkids.size()]));

		try {
			if (!parent.getId().equals("1")) {
				// refresh parent node to update the tree
				Iterator nodeIterator = parent.getTreeNodes();
				while (nodeIterator.hasNext()) {
					MatrixTreeNode node = (MatrixTreeNode) nodeIterator.next();
					if (node != null) {
						refreshAsset(node, "");
					}
					if (hasShadowChild) {
						// bug2346: collapse and expand, as a refresh action
						// for assets that implement bridge (e.g. form section)
						TreePath tp = new TreePath(node.getPath());
						if (MatrixTreeBus.getActiveTree().isExpanded(tp)) {
							MatrixTreeBus.getActiveTree().collapsePath(tp);
							MatrixTreeBus.getActiveTree().expandPath(tp);
						}
					}
				}
			} else {
				// refresh root node
				refreshAsset((MatrixTreeNode)MatrixTreeBus.getActiveTree().getModel().getRoot(), "");
			}
		} catch (IOException ex) {}
	}

	public static boolean isShadowAsset(Asset asset) {
		return (asset.getId().indexOf(":") != -1) ? true : false;
	}

	private static String getIdFromElement(Element element) {
		return MatrixToolkit.rawUrlDecode(element.getAttribute("assetid"));
	}

	/**
	 * Returns the <code>Asset</code> with the specifed assetid.
	 * @param assetid the asset of the wanted asset
	 * @return the <code>Asset</code>
	 */
	public static Asset getAsset(String assetid) {
		return (Asset) assets.get(assetid);
	}

	/**
	 * Returns the asset type given a type code
	 * @param typeCode the type code of the wanted asset type
	 * @return the asset type
	 */
	public static AssetType getAssetType(String typeCode) {
		return (AssetType) assetTypes.get(typeCode);
	}

	public static Iterator getAssetTypes() {
		return assetTypes.values().iterator();
	}

	public static String getWorkspaceid() {
		return workspaceid;
	}

	public static Asset getCurrentUser() {
		return getAsset(currentUserid);
	}

	public static AssetType getCurrentUserType() {
		return getAssetType(currentUserType);
	}

	public static MatrixTreeNode getRootFolderNode() {
		return root;
	}

	/**
	 * Returns all the assetids of assets of the specified type.
	 * @param typeCode the typeCode of the wanted assets
	 * @return the assetids of the assets of the specified type
	 * @see getAsset(String)
	 * @see getAssetTypes()
	 * @see getAssetType(String)
	 */
	public static String[] getAssetsOfType(String typeCode) {
		Iterator iterator = assets.values().iterator();
		List assets = new ArrayList();
		while (iterator.hasNext()) {
			Asset asset = (Asset) iterator.next();
			if (asset.getType().getTypeCode().equals(typeCode)) {
				assets.add(asset.getId());
			}
		}
		return (String[]) assets.toArray(new String[assets.size()]);
	}


	public static String[] getTypeCodeNames() {
		Iterator assetTypesIterator = assetTypes.values().iterator();
		String[] names = new String[assetTypes.size()];
		int i = 0;
		while (assetTypesIterator.hasNext()) {
			AssetType type = (AssetType) assetTypesIterator.next();
			names[i++] = type.getName();
		}
		return names;
	}
}

