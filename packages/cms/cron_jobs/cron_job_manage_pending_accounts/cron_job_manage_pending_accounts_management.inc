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
* $Id: cron_job_manage_pending_accounts_management.inc,v 1.2 2006/06/26 06:50:18 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Manage_Pending_Accounts_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Manage_Pending_Accounts_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Manage_Pending_Accounts_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'type'	=> Array(
									'default'	=> 'repeating',
								   ),
						'when'	=> Array(
									'default'	=> 'ET=',
								   ),
					  );

	}//end constructor


	/**
	* Upgrade this asset to the most current version
	*
	* @param string	$current_version	The version this asset should be upgraded to
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// fill in name, (due to compile locale error)
		if (version_compare($current_version, '0.3', '<')) {
			pre_echo('UPGRADING Manage Pending Accounts Cron Job - FROM VERSION '.$current_version);
			$name = translate('cron_job_manage_pending_accounts');
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$sql = 'UPDATE
						sq_ast
					SET
						name = '.$db->quoteSmart($name).',
						short_name= '.$db->quoteSmart($name).'
					WHERE
						type_code = '.$db->quoteSmart('cron_job_manage_pending_accounts');
			$result = $db->query($sql);
			assert_valid_db_result($result);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			pre_echo('Manage Pending Accounts Cron Job UPGRADE COMPLETE - FROM VERSION '.$current_version);
		}

		return TRUE;

	}//end _upgrade()


}//end class
?>
