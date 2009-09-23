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
* $Id: rest_manager.inc,v 1.1 2009/09/23 07:20:30 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* REST_Manager
*
* Purpose
*
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.1 $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class REST_Manager extends Asset
{

	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	 */
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	 * Returns name of the asset
	 *
	 * @param bool   $short_name whether or not we are after the shortname or the full name
	 *
	 * @return string
	 * @access private
	 */
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	 * Can this asset have its last significant link removed (putting it in the trash)?
	 *
	 * @return boolean
	 * @access public
	 */
	function canDelete()
	{
		return false;

	}//end canDelete()


	/**
	 * Return FALSE always because we don't want to be able to clone a feed
	 * manager
	 *
	 * @return boolean
	 * @access public
	 */
	function canClone()
	{
		return false;

	}//end canClone()


}//end class

?>
