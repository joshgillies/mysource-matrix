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
* $Id: AssetFinderListener.java,v 1.3 2004/06/30 05:20:54 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;



/**
* Interface for objects that want to listen for Asset Finder Events which 
* get fired from Javascript when the user invokes an asset finder state (on/off)
*
* @author Marc McIntyre <mmcintyre@squiz.net>
*
* @see JsEventManager
*/
public interface AssetFinderListener extends JsEventListener {
	
	/**
	* An event that gets invoked when the user wants to choose an asset to use as
	* a reference to a component in the matrix system
	*/
	void assetFinderStarted(JsEvent e);

	/**
	* An event that gets invoked when the user cancels the asset finder mode
	*/
	void assetFinderStopped(JsEvent e);

}