/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: AssetTypeFactory.java,v 1.2 2003/11/18 15:37:36 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

package squiz.matrix;

import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;

import java.net.URL;
import java.net.MalformedURLException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.IOException;

/** 
 * A factory for tracking and instantiating <code>AssetType</code>s. 
 * 
 * <p>
 * You cannot construct an instance of <code>AssetTypeFactory</code>; you must 
 * call the static method <code>getInstance</code>; this is to guarantee a 
 * singleton instance of the factory.
 * </p>
 *
 * <p>
 * Only one <code>AssetType</code> is ever instantiated for a particular asset type code; 
 * all classes that reference <code>AssetType</code>s must first retrieve a 
 * reference to it from here.
 * </p>
 * 
 * <p>
 * This class is responsible for interpreting the XML DOM element for asset 
 * types.
 * </p>
 * 
 * <p><code>$Id: AssetTypeFactory.java,v 1.2 2003/11/18 15:37:36 brobertson Exp $</code></p>
 *
 * @author		Dominic Wong <dwong@squiz.net>
 * @see			AssetType
 * 
 */

public class AssetTypeFactory {
	/** The single AssetTypeFactory instance */
	private static AssetTypeFactory factoryInstance = null;
	/** Map of type codes to asset type objects */
	private Map types;

	/** Constructor */
	private AssetTypeFactory() {
		types = new HashMap();
	}// end constructor

	/** 
	 * Retrieves the one and only instance of <code>AssetTypeFactory</code> allowed.
	 * 
	 * @return			the <code>AssetTypeFactory</code>
	 */
	public static AssetTypeFactory getInstance() {
		if (AssetTypeFactory.factoryInstance == null) {
			AssetTypeFactory.factoryInstance = new AssetTypeFactory();
		}
		return AssetTypeFactory.factoryInstance;
	}//end getInstance()

	/** 
	 * Processes an XML DOM <code>Element</code> containing all asset type data.
	 * 
	 * @param assetTypesElement			the <code>&lt;asset_types&gt;
	 *									...&lt;/asset_types&gt;</code> element
	 *
	 * @throws AssetTypeNotFoundException if the asset type represented by 
	 *									<code>assetTypesElement</code> 
	 *									references an unknown asset type for 
	 *									its parent
	 * @see								AssetTypeNotFoundException
	 */
	public void processAssetTypesElement(Element assetTypesElement) throws AssetTypeNotFoundException {
		// load up types
		// because we need to wait until all types are in the types map before we can resolve parent asset types,
		// we store it here and fix it after we pass through the asset types element
		Map parentMap = new HashMap();
		NodeList children = assetTypesElement.getChildNodes();
		for (int i = 0; i < children.getLength(); ++i) {
			if (children.item(i)  instanceof Element) {
				Element assetTypeElement = (Element)children.item(i);
				processAssetTypeElement(assetTypeElement, parentMap);
			}
		}
		// now resolve parent types
		Set parentMappings = parentMap.entrySet();
		Iterator nextMapping = parentMappings.iterator();
		while (nextMapping.hasNext()) {
			Map.Entry entry = (Map.Entry)nextMapping.next();
			String childTypeCode = (String)entry.getKey();
			String parentTypeCode = (String)entry.getValue();
			AssetType childType = getAssetType(childTypeCode);

			try {
				AssetType parentType = getAssetType(parentTypeCode);
				childType.setParentType(parentType);
			} catch (AssetTypeNotFoundException atnfe) {
				throw new AssetTypeNotFoundException("Parent asset type not found for asset type " + childTypeCode + " : " + parentTypeCode);
			}
		}
	}//end processAssetTypesElement()

	/**
	 * Processes an XML DOM Element containing data for a particular asset type. 
	 * Will instantiate the <code>AssetType</code> if it does not exist.
	 * 
	 * @param assetTypeElement	the <code>&lt;asset_type&gt;
	 *							...&lt;/asset_type&gt;</code> element
	 * @param parentMap			map for storing the typecode that this type 
	 *							inherits from for later resolution
	 * @see						#processAssetTypesElement
	 */
	private void processAssetTypeElement(Element assetTypeElement, Map parentMap) {
		String typeCode			= assetTypeElement.getAttribute("type_code");
		String name				= assetTypeElement.getAttribute("name");
		boolean instantiable	= assetTypeElement.getAttribute("instantiable").equals("1");
		String version			= assetTypeElement.getAttribute("version");
		String allowedAccess	= assetTypeElement.getAttribute("allowed_access");
		String parentTypeCode	= assetTypeElement.getAttribute("parent_type");
		URL iconURL;

		try {
			iconURL = new URL(MySource.getInstance().getBaseURL().toString() +  "/__data/asset_types/" + typeCode + "/icon.png");
		} catch (MalformedURLException mue) {
			iconURL = null;
		}

		if (!types.containsKey(typeCode)) {
			AssetType type = new AssetType(typeCode, name, instantiable, version, allowedAccess, iconURL);
			types.put(typeCode, type);

			type.processScreenElements(assetTypeElement.getChildNodes());

			if (!parentTypeCode.equals("asset")) {
				parentMap.put(typeCode, parentTypeCode);
			}
		}
	}//end processAssetTypeElement()

	/**
	 * Retrieves the instance of <code>AssetType</code> corresponding to 
	 * <code>typeCode</code>.
	 *
     * @param typeCode	The type code of the asset type
	 * @return			The <code>AssetType</code> for <code>typeCode</code>
	 * @throws AssetTypeNotFoundException	if the asset type for 
	 *										<code>typeCode</code> is not found
	 */
	public AssetType getAssetType(String typeCode) throws AssetTypeNotFoundException {
		if (!types.containsKey(typeCode)) {
			throw new AssetTypeNotFoundException(typeCode);
		}
		return (AssetType)types.get(typeCode);
	}//end getAssetType()

	/**
	 * Resolves a set of <Code>String</code>s of asset type codes to the 
	 * actual <code>AssetType</code> objects.
	 * 
	 * @param typeCodes		the type codes
	 * @return				the <code>AssetType</code> objects
	 * @throws AssetTypeNotFoundException
	 */
	public Set getAssetTypesFromTypeCodes(Set typeCodes) throws AssetTypeNotFoundException {
		Iterator i = typeCodes.iterator();
		Set outTypes = new HashSet();

		while(i.hasNext()) {
			String typeCode = (String)i.next();
			outTypes.add(getAssetType(typeCode));
		}
		return outTypes;
	}//end getAssetType()
	
	/**
	 * Returns a string representation of this <code>AssetTypeFactory</code>.
	 */
	public String toString() {
		return "Asset Type Factory (" + types.size() + " asset types)";
	}//end toString()

	/**
	 * <code>main</code> (testing) function. Used for testing by retrieving the asset types from 
	 * MySource Matrix system and printing out some stats.
	 *
	 * @param args		Command line arguments (ignored)
	 *
	 */
	public static void main(String[] args) throws Exception {
		MySource mysource = MySource.getInstance();
		AssetTypeFactory af = AssetTypeFactory.getInstance();

		Document response = mysource.doRequest("<command action=\"initialise\" />");
		NodeList children = response.getDocumentElement().getChildNodes();

		for (int i = 0; i < children.getLength(); ++i) {
			if (!(children.item(i) instanceof Element))
				continue;
			
			Element childElement = (Element)children.item(i);
			if (childElement.getTagName().equals("asset_types")) {
				af.processAssetTypesElement(childElement);
				continue;
			}
		}

		System.out.println ("Asset types registered: " + af.types.size());
		for (Iterator values = af.types.values().iterator(); values.hasNext(); ) {
			AssetType type = (AssetType)values.next();
			System.out.println(type);
		}
	}//end main

}//end class