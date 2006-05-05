<?php
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
* $Id: tool.inc,v 1.3 2006/05/05 06:48:09 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Tool
*
* Purpose
*
*    A base tool asset which can be used to create general purpose tools and
*    functions.
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Tool extends Asset
{


	/**
	* Constructor
	*
	* @param int  $assetid  the asset id to be loaded
	*
	*/
	function Tool($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	*/
	function paintTool(&$o, $type_code)
	{

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.  Returns false if any immediate errors.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	*/
	function processTool(&$o, $type_code)
	{
		return TRUE;

	}//end processTool()


}//end class


?>
