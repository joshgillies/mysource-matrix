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
* $Id: JsEventNotFoundException.java,v 1.5 2004/06/30 05:33:28 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;


/** 
 * An exception indicating that Event was not found.
 * 
 * @author Marc McIntyre <mmcintyre@squiz.net>
 * @see JsEventManager
 */
public class JsEventNotFoundException extends Exception { 
	
	/** 
	 * Constructor 
	 */
	public JsEventNotFoundException() {
		super();
	
	}


	/**
	 * Constructor with message.
	 *
	 * @param msg the error message
	 */
	public JsEventNotFoundException(String msg) {
		super(msg);
	
	}
}
