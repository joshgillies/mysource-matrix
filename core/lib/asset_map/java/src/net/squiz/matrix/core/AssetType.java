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
* $Id: AssetType.java,v 1.1 2005/02/18 05:20:07 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.core;

import java.util.*;
import javax.swing.Icon;
import org.w3c.dom.*;
import java.io.*;


/**
 * The <code>AssetType</code> class represents an Asset Type in the mysource
 * Matrix system.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetType implements Serializable {

	private String typeCode;
	private String name = "";
	private boolean instantiable;
	private String version;
	private String allowedAccess;
	private AssetType parentType = null;
	private Set childTypes;
	private String[] menuPath = new String[0];
	private List screens = new Vector();

	/**
	 * Constructs a new asset type for the given type code
	 */
	public AssetType(String typeCode) {
		this.typeCode = typeCode;
	}

	void setInfo(Element typeElement) {
		name            = typeElement.getAttribute("name");
		instantiable    = typeElement.getAttribute("instantiable").equals("1");
		version         = typeElement.getAttribute("version");
		allowedAccess   = typeElement.getAttribute("allowed_access");
		String menuPath = typeElement.getAttribute("flash_menu_path");

		if (!menuPath.trim().equals(""))
			this.menuPath = menuPath.split("/\\/");

		NodeList screenNodes = typeElement.getChildNodes();
		for (int j = 0; j < screenNodes.getLength(); j++) {
			if (!(screenNodes.item(j) instanceof Element))
				continue;
			Element screenElement = (Element) screenNodes.item(j);
			String codeName = screenElement.getAttribute("code_name");
			String screenName = screenElement.getFirstChild().getNodeValue();
			addScreen(codeName, screenName);
		}
	}


	/**
	 * Returns TRUE if this asset type is createable.
	 * @return TRUE if this asset type is createable
	 */
	public boolean isCreatable() {

		if (!instantiable || allowedAccess.equals("system"))
			return false;

		return true;

/*		AssetType parentUserType = AssetManager.getCurrentUserType();
		while (parentUserType != null) {
			if (parentUserType.getTypeCode().equals(allowedAccess)) {
				return true;
			}
			parentUserType = parentUserType.getParentType();
		}
		return false;
*/	}

	/**
	 * Returns the menu path for this asset type
	 * @return the menu path for this asset type
	 */
	public String[] getMenuPath() {
		return menuPath;
	}

	/**
	 * Returns this asset type's name
	 * @return the name of this asset type
	 */
	public String getName() {
		return name;
	}

	/**
	 * Returns the icon for this asset type
	 * @return the icon for this asset type
	 */
	public Icon getIcon() {
		return GUIUtilities.getIconForTypeCode(typeCode);
	}

	/**
	 * Returns the type code of this asset type
	 * @return the type code of this asset type
	 */
	public String getTypeCode() {
		return typeCode;
	}

	/**
	 * Adds a screen to this asset type
	 *
	 * @param codeName the code name of the screen
	 * @param name the display name of this screen
	 */
	private void addScreen(String codeName, String name) {
		AssetTypeScreen screen = new AssetTypeScreen(codeName, name);
		screens.add(screen);
	}

	/**
	 * Returns a Iterator of the screens for this asset type
	 * @return a list of the screens for this asset type
	 */
	public Iterator getScreens() {
		return screens.iterator();
	}

	/**
	 * Returns the parent type of this asset type
	 * @return the parent type of this asset type
	 */
	public AssetType getParentType() {
		return parentType;
	}
	
	public boolean isAncestor(AssetType parentType) {
		AssetType child = this;
		while (child != null) {
			if (child.equals(parentType))
				return true;
			child = child.getParentType();
		}
		return false;
	}
	
	public boolean isAncestor(String assetType) {
		return isAncestor(AssetManager.getAssetType(assetType));
	}

	/**
	 * Sets the parent type of this asset type
	 * @param parentType the parent type of this asset type
	 */
	public void setParentType(AssetType parentType) {
		this.parentType = parentType;
	}

	/**
	 * Returns the unique hash code for this asset type
	 * @return the hash code
	 */
	public int hashCode() {
		return typeCode.hashCode();
	}
	
	public String toString() {
		return typeCode;
	}

	/**
	 * returns TRUE if this Asset Type is equal to the given object
	 * @return TRUE if this asset type is equal to the give object
	 */
	public boolean equals(Object obj) {
		if (!(obj instanceof AssetType))
			return false;
		return (((AssetType) obj).typeCode.equals(typeCode));
	}
}
