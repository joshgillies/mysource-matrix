<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: tool.inc,v 1.4 2006/12/06 00:13:44 bcaldwell Exp $
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
* @version $Revision: 1.4 $
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
