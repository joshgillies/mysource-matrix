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
* $Id: trigger_action_set_permission_management.inc,v 1.5 2007/02/08 23:08:29 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Set_Permission_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Permission_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Set_Permission_Management(&$pm)
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
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));


		// 0.2 upgrade made changes which need to be reverted in 0.3, so 0.1 uses the same data format as 0.3
		if (version_compare($current_version, '0.2', '=')) {
			// get all triggers from the db and check their actions

			$db =& $GLOBALS['SQ_SYSTEM']->db;
			// compose query
			$sql = 'SELECT
						id,
						data
					FROM
						sq_trig
					';

			// query the database
			$result = $db->getAll($sql);

			// check result
			assert_valid_db_result($result);
			pre_echo('Upgrading '.count($result).' triggers...');
			foreach ($result as $trigger) {

				$trigger_data = unserialize($trigger['data']);
				$trigger_id = $trigger['id'];
				$actions =& $trigger_data['actions'];
				foreach ($actions as $actionid => $action) {
					$action_ref =&  $actions[$actionid];

					if ($action_ref['type'] == 'trigger_action_set_permission') {
						if (isset($action_ref['data']['user_map'])) {
							$param_map = unserialize($action_ref['data']['user_map']);
							// get the first static asset entry from the map's user parameter
							$userid = '';
							foreach ($param_map['user'] as $source) {

								if ($source['source'] == 'STATIC_ASSET') {
									$userid = array_get_index($source, 'index');
								}
							}
							if (!empty($userid)) {
								$action_ref['data']['userid'] = $userid;

								$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
								$db =& $GLOBALS['SQ_SYSTEM']->db;
								$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

								$sql = 'UPDATE
											SQ_TRIG
										SET
											data = '.$db->quoteSmart(serialize($trigger_data)).'
										WHERE
											id = '.$db->quoteSmart($trigger_id).'
										';
								$result = $db->query($sql);
								assert_valid_db_result($result);

								$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
								$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							}
						}
					}//end if
				}//end foreach
			}//end foreach
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.3');

		}//end if version 0.2 upgrade

		return TRUE;

	}//end _upgrade()


}//end class

?>
