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
* $Id: AssetTypeNotFoundException.java,v 1.1 2004/01/13 00:48:41 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

/** 
* An exception indicating that an asset type was not found.
*
* @author Marc McIntyre <mmcintyre@squiz.net>
* 
* @see AssetTypeFactory
*/
public class AssetTypeNotFoundException extends Exception { 
	
	/**
	* Constructor
	*/
	public AssetTypeNotFoundException() {
		super();
	
	}//end constructor

	/**
	* Constructor with message.
	*/
	public AssetTypeNotFoundException(String msg) {
		super(msg);
	
	}//end constructor


}//end class

