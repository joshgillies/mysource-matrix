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
* $Id: AssetFactory.java,v 1.1 2004/01/13 00:41:17 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Set;
import java.util.HashSet;
import java.util.Vector;
import java.util.Iterator;
import java.util.Enumeration;

import java.util.Observer;
import java.util.Observable;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.IOException;

	
/** 
* A factory for tracking, instantiating and updating <code>Asset</code>s. 
* 
* <p>
* You cannot construct an instance of AssetFactory; you must call the static 
* method <code>getInstance</code>; this is to guarantee a singleton instance 
* of the factory.
* </p>
* <p>
* Only one Asset is ever instantiated for a particular asset ID; all classes 
* that reference <code>Asset</code>s must first retrieve a reference to it 
* from here.
* </p>
* 
* <p>
* Temporary <code>Asset</code>s can be created that don't involve an XML call
* to the MySource system - this is useful for loading up <code>Asset</code>s  
* on-demand. They can later be updated via a call to <code>updateAsset</code> 
* which reloads asset data from MySource, as well as its child links.
* </p>
*
* @author Marc McIntyre <mmcintyre@squiz.net>
* @see Asset
*/
public class AssetFactory {
	
	/** 
	* The singleton instance of this object 
	*/
	private static AssetFactory factoryInstance = null;

	/** 
	* map of assetids to <code>Asset</code> objects 
	*/
	private SortedMap assets = new TreeMap();
	
	/** 
	* map of linkids to <code>AssetLink</code> objects 
	*/
	private SortedMap links = new TreeMap();

	/**
	* The id of the trash
	*/
	private int trashId = 0;


	/**
	* Retrieves the singleton instance of the factory.
	*/
	public static AssetFactory getInstance() {
		if (AssetFactory.factoryInstance == null) {
			AssetFactory.factoryInstance = new AssetFactory();
		}
		return AssetFactory.factoryInstance;
	
	}//end getInstance()


	/**
	* Updates an group of <code>Asset</code> siblings from Matrix via an XML 
	* request.
	* 
	* <p>
	* This will retrieve new information for the direct children of 
	* <code>parent</code>.
	* </p>
	*
	* @param parent The parent of the assets to update
	*
	* @throws IOException If a communication error occurred, e.g. XML parse error, error returned by the server, no connection
	* @see AssetTree#treeWillExpand AssetTree.treeWillExpand
	*
	*/
	public void updateAsset(Asset parent) throws IOException {
		MySource ms = MySource.getInstance();
		Iterator childAssets = parent.getChildAssets();
		
		String request = "<command action=\"get assets\" load_new_links=\"0\">";
		int count = 0;

		while (childAssets.hasNext()) {
			Asset childAsset = (Asset)childAssets.next();
			if (!childAsset.isLoaded()) {
				count++;
				request += "<asset assetid=\"" + childAsset.getId() + "\" />";
			}
		}
		request += "</command>";
		
		if (count > 0) {
			Document response = ms.doRequest(request);
			processAssetsElement(response.getDocumentElement());
		}

	}//end updateAsset()


	/** 
	* Processes an DOM <code>Element</code> containing a list of 
	* <code>Element</code>s describing <code>Asset</code>s.
	* 
	* @param assetsElement the element with tag name "assets"
	* @throws IOException A parse error occurred, or an asset references an unknown <code>AssetType</code>
	*/
	public void processAssetsElement(Element assetsElement) throws IOException {
		NodeList assetElements = assetsElement.getChildNodes();
	
		Map assetLinks = new TreeMap(); 
		// because links will refer to assets that have not been loaded yet
		// we process the links after we load the assets

		for (int i = 0; i < assetElements.getLength(); ++i) {
			if (!(assetElements.item(i) instanceof Element)) {
				continue;
			}

			Element assetElement = (Element)assetElements.item(i);
			if (!(assetElement.getTagName().equals("asset"))) {
				continue;
			}
			
			try {
				processAssetElement(assetElement, assetLinks);
			} catch (AssetTypeNotFoundException atnfe) {
				throw new IOException("Could not parse asset : asset type not found: " + atnfe.getMessage());
			}
		}//end for

		// now process the links
		Iterator nextMapping = assetLinks.entrySet().iterator();
		while (nextMapping.hasNext()) {
			Map.Entry mapping = (Map.Entry)nextMapping.next();
			Asset parent = (Asset)(assets.get(mapping.getKey()));
			NodeList linkElements = (NodeList)mapping.getValue();

			processLinkElements(parent, linkElements);
		}

	}//end processAssetsElement()


	/**
	* Processes an individual DOM <code>Element</code>. Creates an asset if 
	* it does not already exist, and updates that asset if it does exist.
	*
	* @param assetElement	the assetElement (XML representation of the asset)
	* @param assetLinks		links found for this asset are put here for later processing by processAssetsElement
	*
	* @throws AssetTypeNotFoundException if this element references an unknown asset type
	* @see #processAssetsElement processAssetsElement
	*/
	private void processAssetElement(Element assetElement, Map assetLinks) throws AssetTypeNotFoundException {
		int assetid				=Integer.parseInt(assetElement.getAttribute("assetid"));
		String name				= assetElement.getAttribute("name");
		String typeCode			= assetElement.getAttribute("type_code");
		boolean accessible		= assetElement.getAttribute("accessible").equals("1");
		int status				= Integer.parseInt(assetElement.getAttribute("status"));
		String url				= assetElement.getAttribute("url");
		String webPathsString	= assetElement.getAttribute("web_paths");

		// get the trash folder id so we can do lazy deletes
		if (typeCode.equals("trash_folder")) {
			trashId = assetid;
		}

		// resolve asset type
		AssetTypeFactory atf = AssetTypeFactory.getInstance();
		AssetType type = atf.getAssetType(typeCode);

		// process web paths 
		String[] webPaths = webPathsString.split(";");

		// set asset data
		// create the asset if it does not already exist
		Asset asset = getAsset(assetid, true);
		asset.setInfo(name, type, status, accessible, url, webPaths);

		// set the list of link elements for this asset for later processing
		NodeList linkElements = assetElement.getChildNodes();
		assetLinks.put(new Integer(asset.getId()), linkElements);

	}//end processAssetElement()


	/**
	* Delegates processing of DOM elements for links to the parent asset of 
	* those links and adds those links to <code>links</code>.
	* 
	* @param parent			The parent <code>Asset</code>
	* @param linkElements	The NodeList from the asset DOM element.
	*
	* @see	#links AssetFactorylinks
	*/
	private void processLinkElements(Asset parent, NodeList linkElements) {
		// pass it off to the asset for processing
		AssetLink[] newLinks = parent.processLinkElements(linkElements);
		// add the returned links to our links map 
		for (int i = 0; i < newLinks.length; ++i) {
			AssetLink link = newLinks[i];
			links.put(new Integer(link.getId()), link);
		}

	}//end processLinkElements()


	/** 
	* Returns an <code>Asset</code> for the given ID, without creating 
	* a temporary <code>Asset</code>.
	*
	* @param assetid the asset ID
	*
	* @return the <code>Asset</code> for the given ID
	* @see #getAsset(int,boolean) getAsset(int assetid, boolean createIfMissing)
	*/
	public Asset getAsset(int assetid) {
		return getAsset(assetid, false);

	}//end getAsset()


	/** 
	* Returns an <code>Asset</code> for the given ID, and optionally creates
	* a temporary <code>Asset</code> for this asset ID.
	* 
	* @param assetid			the asset ID
	* @param createIfMissing	whether to create a temporary <code>Asset</code> that contains the ID only
	*
	* @return the <code>Asset</code> for the given ID
	*/
	public Asset getAsset(int assetid, boolean createIfMissing) {
		Integer assetidInteger = new Integer(assetid);
		Asset asset = null;
		if (!this.assets.containsKey(assetidInteger)) {
			if (createIfMissing) {
				// make a temporary asset without any info
				asset = new Asset(assetid);
				this.assets.put(assetidInteger, asset);
			}
		} else {
			asset = (Asset)this.assets.get(assetidInteger);
		}

		return asset;

	}//end getAsset()


	/**
	* Returns the <code>AssetLink</code> for this link ID.
	*
	* @param linkid		the link ID
	* @return				the <code>AssetLink</code> for this link ID,
	*						or null if we don't know about this link ID
	*/
	public AssetLink getLink(int linkid) {
		Integer linkidInteger = new Integer(linkid);
		if (!links.containsKey(linkidInteger)) {
			return null;
		}
		return (AssetLink)assets.get(linkidInteger);

	}//end getLink()


	/**
	* Returns a string representation of this <code>AssetFactory</code>. Used for testing.
	* 
	* @return a string representation of this <code>AssetFactory</code>
	*/
	public String toString() {
		String out = "Asset Factory (assets: " + assets.size() + ", links: " + links.size() + ")\n";
		Iterator assetsIterator = assets.values().iterator();
		while (assetsIterator.hasNext()) {
			Asset nextAsset = (Asset)assetsIterator.next();
			out += nextAsset + " [" + nextAsset.getId() + "]\nChild Links:\n";
			Iterator linksIterator = nextAsset.getChildLinks();
			while(linksIterator.hasNext()) {
				AssetLink link = (AssetLink)linksIterator.next();
				out += link + "\n";
			}
		}
		return out;

	}//end toString()


	/** 
	* Prints out each link known about (used for testing).
	* 
	*/
	public void printLinks() {
		Iterator nextLink = links.values().iterator();

		while (nextLink.hasNext()) {
			AssetLink link = (AssetLink)nextLink.next();
		}

	}//end printLinks()


}//end class