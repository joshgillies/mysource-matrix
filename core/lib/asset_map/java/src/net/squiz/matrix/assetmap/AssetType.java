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
* $Id: AssetType.java,v 1.2 2004/06/29 01:24:04 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.util.*;
import javax.swing.Icon;

/**
 * The <code>AssetType</code> class represents an Asset Type in the mysource 
 * Matrix system.
 * 
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetType {
	
	/** the type code of this asset type*/
	private String typeCode;
	
	/** the name of this asset type*/
	private String name;
	
	/** TRUE if this asset type is instantiable */
	private boolean instantiable;
	
	/** The version of this asset type */
	private String version;
	
	/** the allowed access of this asset type */
	private String allowedAccess;
	
	/** the parent type of this asset type */
	private AssetType parentType = null;
	
	/** A list of child types of this asset type */
	private Set childTypes;
	
	/** 
	 * The menuPath for this asset type
	 * This used to create the menus and sub menus in the add menu for
	 * this asset type 
	 */
	private String[] menuPath;
	
	/** a list of screens that is used for the right click menu */
	private List screens = new Vector();
	
	/**
	 * Constructs a new asset type for the given type code 
	 */
	public AssetType(String typeCode) {
		this.typeCode = typeCode;
	}

	/**
	 * Sets the information about this asset type code 
	 * 
	 * @param name the name of this asset type
	 * @param instantiable TRUE if this asset type is instantiable
	 * @param version the version of this asset type
	 * @param allowedAccess the allowed access of this asset type
	 * @param menuPath
	 */
	public void setInfo(
			String name, 
			boolean instantiable, 
			String version, 
			String allowedAccess, 
			String[] menuPath
		) {

		this.name          = name;
		this.instantiable  = instantiable;
		this.version       = version;
		this.allowedAccess = allowedAccess;
		this.menuPath      = menuPath;
	}
	
	/**
	 * Returns TRUE if this asset type is createable.
	 * 
	 * @return TRUE if this asset type is createable
	 */
	public boolean isCreatable() {
	
		if (!instantiable)
			return false;
		if (allowedAccess.equals("system"))
			return false;
	
		AssetType parentUserType = AssetManager.INSTANCE.getAssetType(
					AssetManager.INSTANCE.getCurrentUserTypeCode());
		
		while (parentUserType != null) {
			if (parentUserType.getTypeCode().equals(allowedAccess))
				return true;
			
			parentUserType = parentUserType.getParentType();
		}

		return false;
	}
	
	/**
	 * Returns the menu path for this asset type
	 * 
	 * @return the menu path for this asset type
	 */
	public String[] getMenuPath() {
		return menuPath;
	}
	
	/**
	 * Returns this asset type's name
	 * 
	 * @return the name of this asset type
	 */
	public String getName() {
		return name;
	}
	
	/**
	 * Returns the icon for this asset type
	 * 
	 * @return the icon for this asset type
	 */
	public Icon getIcon() {
		return MatrixToolkit.getIconForTypeCode(typeCode);
	}
	
	/**
	 * Returns the type code of this asset type
	 * 
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
	public void addScreen(String codeName, String name) {
		AssetTypeScreen screen = new AssetTypeScreen(codeName, name);
		screens.add(screen);
	}
	
	/**
	 * Returns a Iterator of the screens for this asset type
	 * 
	 * @return a list of the screens for this asset type
	 */
	public Iterator getScreenNames() {
		return screens.iterator();
	}
	
	/**
	 * Returns the parent type of this asset type
	 * 
	 * @return the parent type of this asset type
	 */
	public AssetType getParentType() {
		return parentType;
	}
	
	/**
	 * Sets the parent type of this asset type
	 * 
	 * @param parentType the parent type of this asset type
	 */
	public void setParentType(AssetType parentType) {
		this.parentType = parentType;
	}
	
	/**
	 * Returns the unique hash code for this asset type
	 * 
	 * @return the hash code
	 */
	public int hashCode() {
		return typeCode.hashCode();
	}
	
	/**
	 * returns TRUE if this Asset Type is equal to the given object
	 * 
	 * @return TRUE if this asset type is equal to the give object
	 */
	public boolean equals(Object obj) {
		if (!(obj instanceof AssetType))
			return false;
		return (((AssetType) obj).typeCode.equals(typeCode));
	}
	
	
}
