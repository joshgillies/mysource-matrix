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
* $Id: public_user_management.inc,v 1.4.2.1 2006/04/30 23:05:57 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Public_User_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Public_User_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Public_User_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (version_compare($current_version, '0.1', '=')) {
			// all references to 0 as the "public" userid were removed from core code
			// recently, so we will run a query here to remove them from the perms table
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'UPDATE sq_ast_perm
						SET userid = '.$db->quoteSmart((string) $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user')).'
						WHERE userid = '.$db->quoteSmart((string) 0);
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$res = $db->query($sql);
			assert_valid_db_result($res);
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}
		return parent::_upgrade($current_version);

	}//end _upgrade()


}//end class

?>