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
* $Id: public_user_management.inc,v 1.6 2006/12/06 04:40:09 emcdonald Exp $
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