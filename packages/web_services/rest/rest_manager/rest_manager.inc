<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: rest_manager.inc,v 1.6 2012/08/30 00:59:46 ewang Exp $
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
* @version $Revision: 1.6 $
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
