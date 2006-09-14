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
* $Id: trigger_action_set_future_lineage.inc,v 1.4 2006/09/14 03:56:21 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Set Future Lineage
*
* A trigger action that sets the future lineage of an asset.
*
*
* @author Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Future_Lineage extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	* 		'major_assetid'	=> int, // the new parent of the asset when the cron job runs
	* 		'when'   		=> int, // the time/date to set the cron job to
	* 		);
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check settings, status
		if (empty($settings['link_type']) || empty($settings['major_assetid'])) {
			// if no settings, fail
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
		$fs =& new Cron_Job_Future_Lineage();

		// Set up the type of the link to make.
		$link_info = $fs->attr('link_info');
		$link_info['link_type'] = $settings['link_type'];

		$fs->setAttrValue('when', $settings['when']);
		$fs->setAttrValue('link_info', $link_info);
		$fs->setAttrValue('delete_link_all', $settings['delete_link_all']);

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$user_for_status_change = $GLOBALS['SQ_SYSTEM']->user;
		} else {
			// not doing the whole "security" thing - so pretend we are root
			$user_for_status_change =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$major_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['major_assetid']);
		if ($fs->setAssetInLink($state['asset'], 'minor') && $fs->setAssetInLink($major_asset, 'major')) {
			if ($cron_mgr->addJob($fs, $user_for_status_change)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return Array(
						'jobid'			=> $fs->id,
						'userid'		=> $user_for_status_change->id,
						'major_assetid'	=> $settings['major_assetid'],
						'when'			=> $fs->attr('when'),
					   );
			}
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return FALSE;

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
		// First, munge the prefix so that we can use the Cron Job interfaces to paint/process
		$munge_prefix = str_replace('[', '_', $prefix);
		$munge_prefix = str_replace(']', '', $munge_prefix);

		// set defaults
		$settings['when']	= array_get_index($settings, 'when');
		$settings['link_type'] = array_get_index($settings, 'link_type');
		$settings['major_assetid'] = array_get_index($settings, 'major_assetid');
		$settings['delete_link_all'] = array_get_index($settings, 'delete_link_all', FALSE);
		$settings['prefix']	= $munge_prefix;

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		// Combo box for selecting the type of link.
		$editable_link_types = Array(
								SQ_LINK_TYPE_1	=> link_type_name(SQ_LINK_TYPE_1),
								SQ_LINK_TYPE_2	=> link_type_name(SQ_LINK_TYPE_2),
							   );
		ob_start();
			combo_box($prefix.'[link_type]', $editable_link_types, FALSE, $settings['link_type']);
		$output['new_link_type'] = ob_get_clean();

		// Asset finder for selecting the new parent.
		ob_start();
			asset_finder($prefix.'[major_assetid]', $settings['major_assetid'], Array(), 'sq_sidenav', FALSE, 'null', Array('clear'));
			echo ' ';
			check_box($prefix.'[delete_link_all]', 1, $settings['delete_link_all']);
			echo ' ';
			label(translate('cron_fl_delete_all_existing_links_long'), $prefix.'[delete_link_all]');
		$output['new_link_parent'] = ob_get_clean();

		// We'll use the cron job we're going to create to print the 'when' part of the interface.
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
		$temporary_cron_job =& new Cron_Job_Future_Lineage();
		$temporary_edit_fns =& $temporary_cron_job->getEditFns();

		// Use the cron job edit functions to paint the 'when' box.
		ob_start();
			$temporary_edit_fns->_paintWhenBox($temporary_cron_job, $this, $munge_prefix, $settings['when']);
		$output['at'] = ob_get_clean();

		// Remember our munge-iness.
		hidden_field($prefix.'[prefix]', $munge_prefix);

		if (!$write_access) return '';

		// Stick it all together in a table (yes, I know) so it looks pretty.
		ob_start();
			?><table border="0" cellpadding="4" cellspacing="0"><?php
				foreach ($output as $title => $text) {
					?><tr><td><strong><?php echo translate($title); ?></strong></td><td><?php echo $text; ?></td></tr><?php
				}
			?></table><?php
		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface
	*
	* @return boolean
	* @access public
	*/
	function processInterface(&$settings, $request_data)
	{
		$munge_prefix = $request_data['prefix'];
		$link_type = $request_data['link_type'];
		$major_assetid = $request_data['major_assetid']['assetid'];

		$delete_link_all = FALSE;
		if (isset($request_data['delete_link_all'])) {
			$delete_link_all = TRUE;
		}

		if (!$major_assetid) return 'New parent not specified';

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
		$fs =& new Cron_Job_Future_Lineage();
		$edit_fns = $fs->getEditFns();

		$settings['when'] = $edit_fns->_processWhenBox($fs, $this, $munge_prefix);
		$settings['prefix'] = $munge_prefix;

		$settings['link_type'] = $link_type;
		$settings['major_assetid'] = $major_assetid;
		$settings['delete_link_all'] = $delete_link_all;

		// Right, everything went well, so let's return FALSE?!!
		return FALSE;

	}//end processInterface()


}//end class

?>