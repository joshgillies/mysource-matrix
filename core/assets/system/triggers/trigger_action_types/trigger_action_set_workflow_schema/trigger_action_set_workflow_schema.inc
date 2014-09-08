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
* $Id: trigger_action_set_workflow_schema.inc,v 1.13 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Set_Metadata_Schema
*
* A trigger action that applies or denies a workflow schema
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.13 $
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
	*       'schemaid' 				=> assetid // the workflow schema assetid
	*       'granted'  				=> boolean // true or false
	* 		'cascade_to_dependents'	=> boolean // defaults to FALSE; TRUE will cascade the workflow down to any dependant assets.
	* 		'cascade'				=> boolean // defaults to FALSE; TRUE will cascade the workflow down to any children underneath.
	* 		'cascade_to_new'		=> boolean // defaults to TRUE; TRUE will cascade the workflow down to newly-linked children.
	*       );
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

		$granted               = array_get_index($settings, 'granted', TRUE);
		$cascade_to_dependents = array_get_index($settings, 'cascade_to_dependents', FALSE);
		$cascade               = array_get_index($settings, 'cascade', FALSE);
		$cascade_to_new        = array_get_index($settings, 'cascade_to_new', TRUE);

		$schema = $am->getAsset($settings['schemaid']);
		if (is_null($schema)) return FALSE;
		if ($schema->type() != 'workflow_schema') return FALSE;

		if (!$cascade_to_dependents && !$cascade) {
			$result = $wfm->setSchema($state['asset']->id, $schema->id, $granted, $cascade_to_new);
		} else {
			$schema_vars[]	= Array(
								'assetids'			=> Array($state['asset']->id),
								'granted'			=> $granted,
								'schemaid'			=> $settings['schemaid'],
								'cascades'			=> $cascade_to_new,
								'previous_access'	=> NULL,
								'dependants_only'	=> $cascade_to_dependents,
							  );
			$vars = Array(
						'schema_changes'	=> $schema_vars,
					);

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$errors = $hh->freestyleHIPO('hipo_job_edit_workflow_schemas', $vars);
			$result = empty($errors);
		}


		if (!$result) {
			return FALSE;
		} else {
			return Array(
					'assetid'				=> $state['asset']->id,
					'schemaid'				=> $settings['schemaid'],
					'granted'				=> $granted,
					'cascade'				=> $cascade,
					'cascade_to_dependents'	=> $cascade_to_dependents,
					'cascade_to_new'		=> $cascade_to_new,
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
		$current_schemaid      = array_get_index($settings, 'schemaid', 0);
		$granted               = array_get_index($settings, 'granted', 1);
		$cascade               = array_get_index($settings, 'cascade', FALSE);
		$cascade_to_dependents = array_get_index($settings, 'cascade_to_dependents', FALSE);
		$cascade_to_new        = array_get_index($settings, 'cascade_to_new', TRUE);

		$options = Array(
					TRUE	=> translate('apply'),
					FALSE	=> translate('deny'),
				   );

		ob_start();
			echo '<div class="sq-break-line-inputs">';
			if ($write_access) {
				ob_start();
					combo_box($prefix.'[granted]', $options, FALSE, $granted);
				$combo_box = ob_get_clean();
				echo translate('trigger_schema_apply-deny_the_following_schema', $combo_box).' ';
				asset_finder($prefix.'[schemaid]', $current_schemaid, Array('workflow_schema' => 'I'));
				echo '<br />';
				check_box($prefix.'[cascade]', '1', $cascade);
				label(translate('cascade_schema_changes'), $prefix.'[cascade]');

				echo '<br />';
				check_box($prefix.'[cascade_to_dependents]', '1', $cascade_to_dependents);
				label(translate('cascade_schema_changes_to_dependents'), $prefix.'[cascade_to_dependents]');

				echo '<br />';
				check_box($prefix.'[cascade_to_new]', '1', $cascade_to_new);
				label(translate('cascade_schema_changes_to_newly_linked_children'), $prefix.'[cascade_to_new]');
			} else {
				if (!empty($current_schemaid)) {
					$schema_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_schemaid));
					echo translate('trigger_schema_apply-deny_the_following_schema', $options[$granted]).' ';
					echo get_asset_tag_line($current_schemaid);
				} else {
					echo translate('trigger_schema_apply-deny_the_following_schema', $options[$granted]).' ';
					echo '<em>'.translate('trigger_schema_no_schema_specified').'</em>';
				}

				echo '<br />';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($cascade ? 'tick' : 'cross').'.gif" alt="'.($cascade ? translate('yes') : translate('no')).'" /> ';
				echo translate('cascade_schema_changes');

				echo '<br />';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($cascade_to_dependents ? 'tick' : 'cross').'.gif" alt="'.($cascade_to_dependents ? translate('yes') : translate('no')).'" /> ';
				echo translate('cascade_schema_changes_to_dependents');

				echo '<br />';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($cascade_to_new ? 'tick' : 'cross').'.gif" alt="'.($cascade_to_new ? translate('yes') : translate('no')).'" /> ';
				echo translate('cascade_schema_changes_to_newly_linked_children');
			}
			echo '</div>';

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
		$settings['granted']  = array_get_index($request_data, 'granted', TRUE);
		$settings['cascade']  = array_get_index($request_data, 'cascade', FALSE);
		if ($settings['cascade']) {
			$settings['cascade_to_dependents'] = TRUE;
		} else {
			$settings['cascade_to_dependents'] = array_get_index($request_data, 'cascade_to_dependents', FALSE);
		}
		$settings['cascade_to_new'] = array_get_index($request_data, 'cascade_to_new', FALSE);
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
		return Array($state['assetid'] => Array('workflow'));

	}//end getLocks()


}//end class

?>
