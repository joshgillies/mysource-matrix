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
* $Id: trigger_action_remove_workflow_schema.inc,v 1.3 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Remove_Workflow_Schema
*
* A trigger action that deletes a non running workflow schema
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Remove_Workflow_Schema extends Trigger_Action
{


	/**
	* Execute this action
	* Returns true on success, or false if any error occurred
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'schemaid' 				=> assetid // the workflow schema assetid
	* 		'cascade_to_children'	=> boolean // defaults to FALSE; TRUE will cascade the removal of the workflow schema from all children.
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

		// check required settings
		if (empty($settings['schemaid'])) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$cascade_to_children = array_get_index($settings, 'cascade_to_children', FALSE);

		$schema = $am->getAsset($settings['schemaid']);
		if (is_null($schema)) return FALSE;
		if ($schema->type() != 'workflow_schema') return FALSE;

		$result = TRUE;
		$assets = Array ($state['assetid']);

		if (!$cascade_to_children) {
			//This part ensures schemas are removed from depedant children of a single targeted asset (cascade off).
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($state['assetid']);
			foreach (array_keys($dependants) as $child_id) {
				$assets[] = $child_id;
			}
		}

		foreach ($assets as $assetid) {
			$schema_info = $wfm->getAssetSchemaInfo($assetid, $schema->id);
			if (!empty($schema_info)) {
				$running_schemas = $wfm->getSchemas($assetid, NULL, TRUE);
				if (empty($running_schemas) || !array_key_exists($schema->id, $running_schemas)) {
					$success = $wfm->deleteSchema($assetid, $schema->id, FALSE);
					if (!$success) $result = FALSE;
				}
			}
		}

		if ($cascade_to_children) {
			$children = $am->getChildren($state['assetid']);
			foreach (array_keys($children) as $child_id) {
				$schema_info = $wfm->getAssetSchemaInfo($child_id, $schema->id);
				if (!empty($schema_info)) {
					$running_schemas = $wfm->getSchemas($child_id, NULL, TRUE);
					if (empty($running_schemas) || !array_key_exists($schema->id, $running_schemas)) {
						$success = $wfm->deleteSchema($child_id, $schema->id, FALSE);
						if (!$success) $result = FALSE;
					}
				}
			}
		}

		if (!$result) {
			return FALSE;
		} else {
			return TRUE;
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
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$current_schemaid      = array_get_index($settings, 'schemaid', 0);
		$cascade_to_children = array_get_index($settings, 'cascade_to_children', FALSE);

		ob_start();
			if ($write_access) {
				echo '<div class="sq-break-line-inputs">';
				echo translate('Remove workflow schema').': ';

				asset_finder($prefix.'[schemaid]', $current_schemaid, Array('workflow_schema' => 'I'));

				echo '<br />';
				check_box($prefix.'[cascade_to_children]', '1', $cascade_to_children);
				label(translate('Cascade the removal of workflow schema to all children'), $prefix.'[cascade_to_children]');

				echo '</div>';

			} else {
				if (!empty($current_schemaid)) {
					$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_schemaid));
					echo translate('Remove workflow schema').': ';

					echo get_asset_tag_line($current_schemaid);
				} else {
					echo translate('Remove workflow schema').': ';

					echo '<em>'.translate('No schema specified').'</em>';

				}

				echo '<br />';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($cascade_to_children ? 'tick' : 'cross').'.png" alt="'.($cascade_to_children ? translate('Yes') : translate('No')).'" /> ';


				echo translate('Cascade the removal of workflow schema to all children');

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
	public static function processInterface(&$settings, $request_data)
	{
		$schema_info = array_get_index($request_data, 'schemaid', Array('assetid' => 0));
		$settings['schemaid'] = array_get_index($schema_info, 'assetid', 0);
		$settings['cascade_to_children']  = array_get_index($request_data, 'cascade_to_children', FALSE);

		return FALSE;

	}//end processInterface()


}//end class

?>
