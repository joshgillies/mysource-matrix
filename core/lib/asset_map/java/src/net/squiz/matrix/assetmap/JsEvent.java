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
* $Id: JsEvent.java,v 1.2 2004/06/29 01:24:47 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.util.EventObject;
import java.util.Map;

/**
* An Object that models jsEvents
*
* @author: Marc McIntyre <mmcintyre@squiz.net>
*/
public class JsEvent extends EventObject {

	/**
	* Params passed in from the javascript
	*/
	private Map params;

	/**
	* The inital command passed in from the javascript
	*/
	private String command;

	/**
	* Constructor
	*
	* @param source the source object that fired this event.
	*/
	public JsEvent(Object source, String command, Map params) {
		super(source);
		this.command = command;
		this.params = params;
	
	}//end constructor()


	/**
	* Returns the params that were passed in from the javascipt.
	*
	* @return the params that were passed in from the javascript array
	*/
	public Map getParams() {
		return params;

	}//end getParams()


	/**
	* Returns the command that was passed in from the javascipt.
	*
	* @return the command passed from javascript
	*/
	public String getCommand() {
		return command;

	}//end getCommand()

}//end class
