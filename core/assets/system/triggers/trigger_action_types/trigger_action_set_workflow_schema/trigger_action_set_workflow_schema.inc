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
* $Id: trigger_action_set_workflow_schema.inc,v 1.1.2.1 2006/12/20 04:07:55 skim Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Set_Metadata_Schema
*
* A trigger action that applies or denies a workflow schema
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Workflow_Schema extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'attribute' => string // the attribute to modify
	*       'value'     => string // the value to set
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$wfm =& $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

		// check required settings
		if (empty($settings['schemaid'])) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] =& $am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$schema =& $am->getAsset($settings['schemaid']);
		if (is_null($schema)) return FALSE;
		if ($schema->type() != 'workflow_schema') return FALSE;

		$success = $wfm->setSchema($state['asset']->id, $schema->id, $settings['granted']);

		if ($success) {
			return Array(
					'assetid'	=> $state['asset']->id,
					'schemaid'	=> $schema->id,
				   );
		} else {
			return FALSE;
		}

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$current_schemaid = array_get_index($settings, 'schemaid', 0);
		$granted = array_get_index($settings, 'granted', 1);
		$options = Array(
					TRUE	=> 'Apply',
					FALSE	=> 'Deny',
				   );

		ob_start();
			if ($write_access) {
				combo_box($prefix.'[granted]', $options, FALSE, $granted);
				echo ' the following schema ';
				asset_finder($prefix.'[schemaid]', $current_schemaid, Array('workflow_schema' => 'I'));
			} else {
				$granted_str = ($granted) ? 'Apply' : 'Deny';
				if (!empty($current_schemaid)) {
					$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_schemaid));
					echo $options[$granted].' the following schema <b>'.$schema_info[$current_schemaid]['name'].' (#'.$current_schemaid.')</b>';
				} else {
					echo $options[$granted].' the following schema <b>No schema specified</b>';
				}
			}

			$output = ob_get_contents();
		ob_end_clean();

		return $output;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	function processInterface(&$settings, $request_data)
	{
		$schema_info = array_get_index($request_data, 'schemaid', Array('assetid' => 0));
		$settings['schemaid'] = array_get_index($schema_info, 'assetid', 0);
		$settings['granted'] = array_get_index($request_data, 'granted', TRUE);
		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('workflow'));

	}//end getLocks()


}//end class

?>
