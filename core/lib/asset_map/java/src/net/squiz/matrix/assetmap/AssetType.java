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
* $Id: AssetType.java,v 1.1 2004/01/13 00:45:55 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.net.URL;

import javax.swing.ImageIcon;
import javax.swing.Icon;

import java.util.*;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
* Class for objects representing a Matrix asset type.
* 
* An object of this class keeps track of useful data on a particular 
* asset type, such as its type code, name, icon, version, etc.
* 
* @author Marc McIntyre <mmcintyre@squiz.net>
* @see AssetTypeFactory AssetTypeFactory
*/
public class AssetType {
	
	/**
	* asset type code
	*/
	private String typeCode;
	
	/**
	* asset type name
	*/
	private String name;

	/** 
	* asset type icon 
	*/
	private Icon icon;
	
	/**
	* whether the asset type is instantiable
	*/
	private boolean instantiable;

	/**
	* asset type version
	*/
	private String version;

	/**
	* asset type allowed access e.g. backend_user, system_user
	*/
	private String allowedAccess;

	/**
	* A list of screens for this asset type
	*/
	private List screens;

	/**
	* the parent type of this asset type
	*/
	private AssetType parentType;

	/**
	* A list of child types that inherit this asset type
	*/
	private Set childTypes;


	/**
	* Constructor
	*/
	public AssetType(
			String typeCode, 
			String name, 
			boolean instantiable, 
			String version, 
			String allowedAccess, 
			URL iconURL
		) {
		this.typeCode = typeCode;
		this.name = name;
		this.icon = icon;
		this.instantiable = instantiable;
		this.version = version;
		this.allowedAccess = allowedAccess;

		if (iconURL != null)
			this.icon = new ImageIcon(iconURL);
		else
			this.icon = null;
		
		this.screens = new Vector();

		this.parentType = null;
		this.childTypes = new HashSet();

	}//end constructor


	/**
	* Sets a parent type for an Asset Type
	* 
	* @param parentType the parent type that an asset type inherits from
	*/
	public void setParentType(AssetType parentType) {
		this.parentType = parentType;
		parentType.childTypes.add(this);

	}//end setParentType()

	
	/**
	* Returns a string representation of an asset type
	*
	* @return the string representation of an asset type
	*/
	public String toString() {
		String out = new String();
		AssetType[] ancestorTypes = getAncestorTypes();
		for (int i = ancestorTypes.length - 1; i >= 0; --i) {
			out += ancestorTypes[i].getTypeCode() + ".";
		}
		out += typeCode + " [name: " + name + "; screens: " + screens.size() + "; child types: " + childTypes.size() + "]";
		return out;
	
	}//end toString()


	/**
	* Returns the typecode of this asset type
	* 
	* @return the type code of the asset type
	*/
	public String getTypeCode() {
		return typeCode;

	}//enmd getTypeCode()


	/**
	* Returns the name of the asset type
	*
	* @return the name of the asset type
	*/
	public String getName() {
		return name;
	
	}//end getName()


	/**
	* returns the icon for the asset type
	*/
	public Icon getIcon() {
		return icon;

	}//end getIcon()

	/**
	* returns a hashcode for this asset type
	*
	* @return the hashcode for this asset type
	*/
	public int hashCode() {
		return typeCode.hashCode();
	
	}//end hashCode()

	
	/**
	* Returns a list of Ancestor types that the asset type inherits from
	*
	* @return an array of thie asset type objects
	*/
	public AssetType[] getAncestorTypes() {
		Vector lineage = new Vector();
		AssetType nextType = this;
		while (nextType.parentType != null) {
			lineage.add(nextType.parentType);
			nextType = nextType.parentType;
		}

		AssetType[] out = new AssetType[lineage.size()];
		int index = 0;
		Enumeration types = lineage.elements();
		while (types.hasMoreElements()) {
			out[index++] = (AssetType)types.nextElement();
		}
		return out;

	}//end getAcestorTypes()


	/**
	* Returns the parent type that this asset type inherits from
	* 
	* @return the parent type
	*/
	public AssetType getParentType() {
		return this.parentType;
	
	}//end getParentType()


	/**
	* Processes the XML that represents the screens of this asset type
	*
	* @param screenElements the XML nodeList of the screens for this typecode
	*/
	public void processScreenElements(NodeList screenElements) {
		for (int i = 0; i < screenElements.getLength(); ++i) {
			if (!(screenElements.item(i) instanceof Element))
				continue;
			Element screenElement = (Element)screenElements.item(i);
			String codeName = screenElement.getAttribute("code_name");
			String screenName = screenElement.getFirstChild().getNodeValue();
			screens.add(new AssetTypeScreen(codeName, screenName));
		}

	}//end processScreenElements()


	/**
	* returns the screens of this asset type as a string representation
	*
	* @return the string representation of the type codes of this asset type
	*/
	private String screensToString() {
		String out = "[";
		boolean first = true;
		Iterator i = screens.iterator();
		while (i.hasNext()) {
			if (!first)
				out += ", ";
			AssetTypeScreen screen = (AssetTypeScreen)i.next();
			out += "(" + screen.codeName + "/" + screen.screenName + ")";
			first = false;
		}
		out += "]";

		return out;

	}//end screensToString()


	/**
	* returns an iterator for iterating through the screens of this asset type
	*
	* @return the iterator for iterating through the asset screens
	*/
	public Iterator getScreens() {
		return screens.iterator();
	
	}//end getScreens()

}
