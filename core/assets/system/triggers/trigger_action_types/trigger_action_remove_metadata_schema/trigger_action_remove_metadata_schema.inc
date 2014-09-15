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
* $Id: trigger_action_remove_metadata_schema.inc,v 1.6 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Remove_Metadata_Schema
*
* A trigger action that removes a metadata schema
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Remove_Metadata_Schema extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	* 	'schemaid'	=> string  // the schema to set
	* 	'removeall'	=> boolean // whether to remove all schemas (this will override the schemaid)
	* 	'cascade'	=> boolean // if true, metadata schemas will be cascaded to children
	* );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$master_list = Array();

		// check required settings
		if (empty($settings['schemaid']) && (!isset($settings['removeall']) && !$settings['removeall'])) {
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$remove_all = array_get_index($settings, 'removeall', FALSE);
		$cascade = array_get_index($settings, 'cascade', FALSE);

		// Get the master list of metadata schema from the parent asset
		if ($remove_all) {
			$master_list = $mm->getSchemas($state['asset']->id);
			$schema = NULL;
		} else {
			$schema = $am->getAsset($settings['schemaid']);
		}//end if

		// Error checking
		// Test 1 - No schema and no remove all schema ie. nothing to do!
		// Test 2 - No schema and remove all with no metadata schemas ie. again nothing to do!
		if ((is_null($schema) || $schema->type() != 'metadata_schema') && !$remove_all) {
			return TRUE;
		} else if (empty($master_list) && $remove_all && (is_null($schema) || $schema->type() != 'metadata_schema')) {
			return TRUE;
		}

		if (!$cascade) {
			if ($remove_all) {
				foreach ($master_list as $schema_to_delete => $grant_return) {
					$result[] = $mm->deleteSchema($state['asset']->id, $schema_to_delete);
				}
			} else {
				$result = $mm->deleteSchema($state['asset']->id, $schema->id);

			}//end if
		} else {
			if ($remove_all) {
				foreach ($master_list as $schema_to_delete => $grant_return) {
					$schema_vars[]	= Array(
										'assetids'			=> Array($state['asset']->id),
										'granted'			=> -1,
										'schemaid'			=> $schema_to_delete,
										'previous_access'	=> NULL,
										'cascades'			=> TRUE,
									  );
				}//end foreach
				$vars	= Array(
							'assets'			=> Array(
													$state['asset']->id	=> Array(
																			'type_code'	=> $state['asset']->type(),
																		   ),
												   ),
							'schema_changes'	=> $schema_vars,
						  );

				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$errors = $hh->freestyleHIPO('hipo_job_edit_metadata_schemas', $vars);
				$result = empty($errors);
			} else {
				$schema_vars[]	= Array(
									'assetids'			=> Array($state['asset']->id),
									'granted'			=> -1,
									'schemaid'			=> $settings['schemaid'],
									'previous_access'	=> NULL,
									'cascades'			=> TRUE,
								  );
				$vars	= Array(
							'assets'			=> Array(
													$state['asset']->id	=> Array(
																			'type_code'	=> $state['asset']->type(),
																		   ),
												   ),
							'schema_changes'	=> $schema_vars,
						  );

				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$errors = $hh->freestyleHIPO('hipo_job_edit_metadata_schemas', $vars);
				$result = empty($errors);
			}//end if
		}//end else

		// Hmm result is an array, let me double check
		if (is_array($result)) {
			$trigger_result = TRUE;
			foreach ($result as $result_value) {
				if (!$result_value) $trigger_result = FALSE;
			}//end foreach

			// Return the result, so the trigger can finish
			$result = $trigger_result;
		}//end if

		if (!$result) {
			return FALSE;
		} else {
			return Array(
					'assetid'		=> $state['asset']->id,
					'schemaid'		=> $settings['schemaid'],
					'remove_all'	=> $remove_all,
					'cascade'		=> $cascade,
				   );
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
		$current_schemaid = array_get_index($settings, 'schemaid', 0);
		$remove_all = array_get_index($settings, 'removeall', 0);
		$cascade = array_get_index($settings, 'cascade', 0);

		ob_start();
			if ($write_access) {
				echo '<div class="sq-break-line-inputs">';
				echo translate('trigger_schema_remove_schema_message').': ';
				asset_finder($prefix.'[schemaid]', $current_schemaid, Array('metadata_schema' => 'I'));
				echo '<br />';
				check_box($prefix.'[removeall]', '1', $remove_all, 'if (this.checked == true) { if (!confirm("'.translate('trigger_schema_remove_all_schema_confirm_message').'")) { this.checked = false; } }');
				label(translate('trigger_schema_remove_all_schema_message'), $prefix.'[removeall]');
				echo '<br />';
				check_box($prefix.'[cascade]', '1', $cascade);
				label(translate('cascade_schema_changes'), $prefix.'[cascade]');
				echo '</div>';

			} else {
				if (!empty($current_schemaid)) {
					$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_schemaid));
					echo translate('trigger_schema_remove_schema_message').': ';
					echo get_asset_tag_line($current_schemaid);
				} else {
					echo translate('trigger_schema_remove_schema_message').': ';
					echo '<em>'.translate('trigger_schema_no_schema_message').'</em>';
				}
				echo '<br />';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($remove_all ? 'tick' : 'cross').'.gif" alt="'.($remove_all ? translate('yes') : translate('no')).'" /> ';
				echo translate('trigger_schema_remove_all_schema_message');
				echo '<br />';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($cascade ? 'tick' : 'cross').'.gif" alt="'.($cascade ? translate('yes') : translate('no')).'" /> ';
				echo translate('trigger_schema_removal_to_children');
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
		$settings['removeall'] = array_get_index($request_data, 'removeall', FALSE);
		$settings['cascade'] = array_get_index($request_data, 'cascade', FALSE);
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
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('metadata'));

	}//end getLocks()


}//end class

?>
