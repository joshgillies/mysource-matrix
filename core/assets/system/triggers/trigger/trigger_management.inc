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
* $Id: trigger_management.inc,v 1.10 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Management
*
* @author Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Trigger_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The name of the trigger',
										   ),
						'description'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'A description of the trigger',
											'parameters'	=> Array(
																'max_length'	=> 255,
															   ),
										   ),
						'category'	=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Category of the trigger',
											'parameters'	=> Array(
																'max_length'	=> 255,
															   ),
										   ),
						'events'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Events when trigger will be executed',
										   ),
						'conditions'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Conditions that must be met for trigger to execute',
										   ),
						'actions'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Actions that will be performed when the trigger executes',
										   ),
						'active'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
											'description'	=> 'If this trigger is active or not',
										   ),
						'blocking'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
											'description'	=> 'If this trigger will prevent further trigger execution upon error',
										   ),
					  );

	}//end constructor


	/**
	* Upgrade script
	*
	* @param string	$current_version	the version we are upgrading FROM
	*
	* @return boolean
	* @access private
	* @see Asset_Management::_upgrade()
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '0.3', '<')) {
			pre_echo('UPGRADING TRIGGER ASSETS - FROM VERSION '.$current_version);
			$trigger_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');
			$children = $trigger_manager->getTriggerList();
			foreach ($children as $trigger_data) {
				$trigger = $trigger_manager->getAsset($trigger_data['id']);
				$conditions = $trigger->attr('conditions');
				foreach ($conditions as &$condition) {
					if (!isset($condition['inverse_condition'])) {
						$condition['inverse_condition'] = FALSE;	
					}
				}
				$trigger->setAttrValue('conditions', $conditions);
				$trigger->saveAttributes();
			}
		}//end if version 0.3

		if (version_compare($current_version, '0.4', '<')) {
			pre_echo('UPGRADING TRIGGER ASSETS - FROM VERSION '.$current_version);
			$trigger_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');
			$children = $trigger_manager->getTriggerList();
			foreach ($children as $trigger_data) {
				$trigger = $trigger_manager->getAsset($trigger_data['id']);
				$actions = $trigger->attr('actions');
				foreach ($actions as $index => $action) {
					if (isset($action['type']) && $action['type'] == 'trigger_action_set_thumbnail') {
						$data = array_get_index($action, 'data', Array());
						if (isset($data['index'])) {
							$parameter_map = Array( 'thumbnail' => Array( Array( 'source' => 'GLOBALS', 'index' => 'thumbnail') ) );
							$data['thumbnail'] = serialize($parameter_map);
							unset($data['index']);
							$actions[$index]['data'] = $data;
						}//end if
					}//end if
				}//end foreach
				$trigger->setAttrValue('actions', $actions);
				$trigger->saveAttributes();
			}
		}//end if version 0.4

		if (version_compare($current_version, '0.5', '<')) {
			pre_echo('UPGRADING TRIGGER ASSETS - FROM VERSION '.$current_version);
			$trigger_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');
			$children = $trigger_manager->getTriggerList();
			foreach ($children as $trigger_data) {
				$trigger = $trigger_manager->getAsset($trigger_data['id']);
				$actions = $trigger->attr('actions');
				foreach ($actions as $index => $action) {
					// Move asset selection options to the generic trigger action's "Action on Other Assets" setting
					if (isset($action['type']) && ($action['type'] == 'trigger_action_purge_squid_cache' || $action['type'] == 'trigger_action_clear_cache' || $action['type'] == 'trigger_action_set_cache_expiry')) {
						$asset_level_index = $action['type'] == 'trigger_action_purge_squid_cache' ? 'cascade' : 'level';
						$data = array_get_index($action, 'data', Array());
						$other_assets = array_get_index($action, 'other_assets', Array());
	
						if (isset($data['use_current_asset'])) {
							$other_assets['current_asset'] = 1;
							unset($data['use_current_asset']);
						}
						if (isset($data['selected_assets']) && is_array($data['selected_assets'])) {
							$other_assets['assets'] = $data['selected_assets'];
							unset($data['selected_assets']);
						}
						if (isset($data[$asset_level_index])) {
							$other_assets['asset_level'] = $data[$asset_level_index];
							unset($data[$asset_level_index]);
						}
						if (isset($data['asset_types']) && is_array($data['asset_types'])) {
							$other_assets['asset_types'] = $data['asset_types'];
							unset($data['asset_types']);
						}
						if (!empty($other_assets)) {
							$other_assets['enabled'] = 1;
						}

						$action['data'] = $data;
						$action['other_assets'] = $other_assets;
						$actions[$index] = $action;
					}//end if
				}//end foreach
				$trigger->setAttrValue('actions', $actions);
				$trigger->saveAttributes();
			}
		}//end if version 0.5

		return TRUE;

	}//end _upgrade()
	

}//end class

?>
