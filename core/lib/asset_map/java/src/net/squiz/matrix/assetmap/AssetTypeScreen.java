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
* $Id: AssetTypeScreen.java,v 1.2 2004/06/29 01:24:21 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

/**
 * An object that represents a screen for an <code>AssetType</code>.
 * 
 * @author Marc McIntyre <mmcintyre@squiz.net>
 * @see AssetType
 */ 
public class AssetTypeScreen
{
	/** the code name for this screen */
	public String codeName;
	
	/** the screen name for this screen */
	public String screenName;

	/**
	* Constructor
	* 
	* @param codeName the screen code name
	* @param screenName the screen's pretty name
	*/
	public AssetTypeScreen(String codeName, String screenName) {
		this.codeName = codeName;
		this.screenName = screenName;
	}

}
