<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: bulkmail_user.inc,v 1.5 2006/01/05 22:39:05 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Bulkmail_User
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_User extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bulkmail_User($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Pages ensure a name and short_name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$email = trim($this->attr('email'));
		if (empty($email)) return FALSE;

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		$name = $this->attr('name');
		if ($short_name || empty($name)) {
			return $this->attr('email');
		} else {
			return $name;
		}

	}//end _getName()


}//end class
?>