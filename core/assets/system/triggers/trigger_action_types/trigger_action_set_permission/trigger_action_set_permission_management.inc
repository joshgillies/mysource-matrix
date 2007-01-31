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
* $Id: trigger_action_set_permission_management.inc,v 1.3 2007/01/31 22:06:57 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Action_Set_Permission_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
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

		if (version_compare($current_version, '0.2', '<')) {
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
						if (isset($action_ref['data']['userid'])) {
							//create a parameter map
							$param_map = Array();
							$param_map['user'] = Array(
													'0' => Array(
															'source' => 'STATIC_ASSET',
															'static_asset_finder' => Array(
																					'assetid' => $action_ref['data']['userid'],
																					'url' => '',
																					'linkid' => '',
																					 ),
															'index' => $action_ref['data']['userid'],
															),
														);
							// we're done with the user id now
							unset($action_ref['data']['userid']);
							$param_map = serialize($param_map);
							$action_ref['data']['user_map'] = $param_map;

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
				}
			}
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.2');

		}//end version 0.2 upgrade

		return TRUE;

	}//end _upgrade()



}//end class

?>
