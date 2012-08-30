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
* $Id: trigger_condition_tree_location_management.inc,v 1.5 2012/08/30 01:09:15 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Condition_Tree_Location_Management
*
* @see Asset_Management
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Tree_Location_Management extends Asset_Management
{

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

		if (version_compare($current_version, '0.3', '<')) {
			pre_echo('UPGRADING TRIGGER CONDITION TREE LOCATION FROM v < 0.3');

			$tm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');
			$trigger_list = $tm->getTriggerList();
			foreach ($trigger_list as $info){
				$trigger = $tm->getAsset($info['id']);
				$conditions = $trigger->attr('conditions');
			
				foreach ($conditions as &$condition_info){
					if ($condition_info['type'] == 'trigger_condition_tree_location'){
						if (!empty($condition_info['data']['assetid'])){
							$condition_info['data']['assetids'] = Array($condition_info['data']['assetid']);
							$condition_info['data']['immed_child'] = Array(
																		$condition_info['data']['assetid'] => $condition_info['data']['immed_child']
																	);
							$condition_info['data']['propagate'] = Array(
																		$condition_info['data']['assetid'] => $condition_info['data']['propagate']
																	);
							//unset old settings indices
							unset($condition_info['data']['assetid']);
							unset($condition_info['data']['treeid']);
						}
					}
				}
			
				$trigger->setAttrValue('conditions', $conditions);
				$trigger->saveAttributes();
				unset($condition_info);
			}
		}//end if
		
		return TRUE;

	}//end _upgrade()
	
}//end class

?>
