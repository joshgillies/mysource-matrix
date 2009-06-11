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
* $Id: trigger_management.inc,v 1.7.4.1 2009/06/11 06:53:09 wszymanski Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Management
*
* @author Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.7.4.1 $
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

		return TRUE;
 
 	}//end _upgrade()
	

}//end class

?>
