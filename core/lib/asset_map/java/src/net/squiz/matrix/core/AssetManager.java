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
* $Id: AssetManager.java,v 1.2 2005/03/01 00:48:52 mmcintyre Exp $
*
*/

package net.squiz.matrix.core;

import net.squiz.matrix.matrixtree.*;
import java.util.*;
import org.w3c.dom.*;
import java.io.IOException;
import javax.swing.*;
import javax.swing.event.*;
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
	private static String workspaceid;
	// MM: thinking of a better way to do this stuff with initialisiation
	private static EventListenerList listenerList = new EventListenerList();

	// cannot instantiate
	private AssetManager() {}

	/**
	 * Initialises the information needed for the Asset Map.
	 * An xml request is made to the matrix system, which returns xml.
	 * @return the root folder node of the matrix system.
	 * @throws IOException if the request fails
	 */
	public static MatrixTreeNode init() throws IOException {
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

	public static void refreshAssets(String assetidsStr) {
		String[] assetids = assetidsStr.split(",");
		refreshAssets(assetids);
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
	 * @param xmlNodes the xmlNodes that represent the current user
	 */
	private static void processCurrentUserXML(Element xmlNodes) {
		currentUserid = getIdFromElement(xmlNodes);
		currentUserType = xmlNodes.getAttribute("type_code");
		String name = xmlNodes.getAttribute("name");
	}


	public static void refreshAsset(Asset parent) {
		refreshAssets(new Asset[] { parent });
	}

	public static void refreshAssets(Asset[] parents) {
		String[] assetids = new String[parents.length];
		for (int i = 0; i < parents.length; i++) {
			assetids[i] = parents[i].getId();
		}
		refreshAssets(assetids);
	}

	public static void refreshAssets(String[] assetids) {
		StringBuffer xml = new StringBuffer("<command action=\"get assets\">");
		for (int i = 0; i < assetids.length; i++) {
			MatrixToolkit.addAssetToXML(xml, assetids[i]);
		}
		xml.append("</command>");
		Document response = null;

		try {
			response = Matrix.doRequest(xml.toString());
		} catch(IOException ioe) {
			ioe.printStackTrace();
		}
		NodeList childNodes = (NodeList) response.getDocumentElement().getChildNodes();
		for (int i = 0; i < childNodes.getLength(); i++) {
			if (!(childNodes.item(i) instanceof Element))
				continue;
			Element assetElement = (Element) childNodes.item(i);
			updateAsset(assetElement);
		}
	}

	public static void refreshAsset(MatrixTreeNode parent) {
		StringBuffer xml = new StringBuffer("<command action=\"get assets\">");
		MatrixToolkit.addAssetToXML(xml, parent.getAsset());
		xml.append("</command>");
		Document response = null;
		try {
			response = Matrix.doRequest(xml.toString());
		} catch (IOException ioe) {
			GUIUtilities.error(
				ioe.getMessage(),
				"Could not load assets"
			);
			ioe.printStackTrace();
			MatrixStatusBar.setStatusAndClear("Request Failed!", 1000);
		}
		processAssetsXML(response.getDocumentElement(), parent);
	}

	public static boolean isShadowAsset(Asset asset) {
		return (asset.getId().indexOf(":") != -1) ? true : false;
	}

	public static void refreshAllKnownAssets() {
		Iterator iterator = assets.values().iterator();
		List parents = new ArrayList();

		while (iterator.hasNext()) {
			Asset asset = (Asset) iterator.next();
			Iterator nodes = asset.getTreeNodes();

			while(nodes.hasNext()) {
				MatrixTreeNode node = (MatrixTreeNode) nodes.next();
				if (node.getAsset().childrenLoaded()) {
					parents.add(node.getAsset());
					break;
				}
			}
		}
		refreshAssets((Asset[]) parents.toArray(new Asset[parents.size()]));
	}

	private static void updateAsset(Element childElement) {
		String assetid = getIdFromElement(childElement);
		// if we dont have this asset then we can't update it or its children
		if (!assets.containsKey(assetid))
			return;
		Asset asset = getAsset(assetid);
		updateAsset(childElement, asset);
	}



	public static void updateAsset(Element childElement, Asset parent) {

		NodeList childNodes = (NodeList) childElement.getChildNodes();

		// get the parent to update its information also
		parent.processAssetXML(childElement);

		// create a set of linkids so that we can remove any nodes
		// that are no longer children of this asset
		List linkids = null;
		for (int i = 0; i < childNodes.getLength(); i++) {
			if (!(childNodes.item(i) instanceof Element))
				continue;
			Element assetElement = (Element) childNodes.item(i);

			String assetid = getIdFromElement(assetElement);
			int index      = Integer.parseInt(assetElement.getAttribute("sort_order"));
			String linkid  = assetElement.getAttribute("linkid");
			Asset asset    = loadAsset(assetid, assetElement, null, index);

			// lazily create
			if (linkids == null)
				linkids = new ArrayList();
			linkids.add(linkid);
			// this node might be new so we need to give
			// a parent a chance to add it
			parent.propagateNode(asset, linkid, index);
			parent.setChildrenLoaded(true);
		}

		if (linkids != null) {
			parent.removeDiffChildNodes(
				(String[]) linkids.toArray(new String[linkids.size()]));
		}
	}

	private static String getIdFromElement(Element element) {
		return MatrixToolkit.rawUrlDecode(element.getAttribute("assetid"));
	}

	/**
	 * Returns the <code>Asset</code> with the specifed assetid.
	 *
	 * @param assetid the asset of the wanted asset
	 * @return the <code>Asset</code>
	 */
	public static Asset getAsset(String assetid) {
		return (Asset) assets.get(assetid);
	}

	/**
	 * Returns the asset type given a type code
	 *
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
