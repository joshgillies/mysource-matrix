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
* $Id: cron_job_manage_pending_accounts.inc,v 1.2 2006/05/02 04:45:32 rong Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

/**
* Cron_Job_Manage_Pending_Accounts
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Manage_Pending_Accounts extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cron_Job_Manage_Pending_Accounts($assetid=0)
	{
		$this->Cron_Job($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Return either the linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Return name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return translate('cron_job_manage_pending_accounts');

	}//end _getName()


	/**
	* returns FALSE always because we don't want to be able to clone this cron job
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Morphs this asset to either one of its parent types or one of its children types.
	*
	* Only succeeds if $new_type_code assets are allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	* @see Asset::morph()
	*/
	function morph($new_type_code)
	{
		trigger_localised_error('CRON0024', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		return FALSE;

	}//end morph()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For the Manage Pending Accounts cron job, NO!  There must always be exactly one of these here.
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Run this cron job
	*
	* Looks at all the currently-running workflows and processes any whose current step has
	* expired since the last run of the cron manager
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	function _exec(&$msg)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_account_manager', FALSE);

		foreach($assetids as $assetid) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			// Get the pending accounts group, if it exists
			$pending_group_link = $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, NULL, 'user_group', TRUE, 'pending_accounts');
			if ($asset->attr('use_email_validation') && !empty($pending_group_link)) {
				$pending_group =& $GLOBALS['SQ_SYSTEM']->am->getAsset($pending_group_link['minorid']);

				// RESEND VALIDATION TIMEOUT //
				$resend_period = $asset->attr('resend_validation_period');
				if ($resend_period > 0) {
					$sql = 'SELECT
								a.assetid, l.linkid
							FROM sq_ast a
								JOIN sq_ast_lnk l
								ON a.assetid = l.minorid
							WHERE
								l.majorid = '.$db->quote($pending_group->id).' AND
								l.link_type = '.$db->quoteSmart(SQ_LINK_TYPE_1).' AND
								a.created < '.db_extras_todate($db, ts_iso8601(time() - $resend_period));

					$user_assetids = $db->getAssoc($sql);
					assert_valid_db_result($user_assetids);

					foreach($user_assetids as $user_assetid => $user_linkid) {
						$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($user_assetid);

						// Resend the validation and update the link to TYPE_2
						$asset->resendValidation($user->attr('username'));
						$GLOBALS['SQ_SYSTEM']->am->updateLink($user_linkid, SQ_LINK_TYPE_2);

						unset($user);
					}
				}

				// REMOVE ACCOUNT TIMEOUT //
				$remove_period = $asset->attr('remove_account_period');
				if ($remove_period > 0) {
					$sql = 'SELECT
								a.assetid, l.linkid
							FROM sq_ast a
								JOIN sq_ast_lnk l
								ON a.assetid = l.minorid
							WHERE
								l.majorid = '.$db->quote($pending_group->id).' AND
								a.created < '.db_extras_todate($db, ts_iso8601(time() - $remove_period));

					$user_assetids = $db->getAssoc($sql);
					assert_valid_db_result($user_assetids);

					foreach($user_assetids as $user_assetid => $user_linkid) {
						$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($user_assetid);

						$GLOBALS['SQ_SYSTEM']->am->trashAsset($user_assetid);

						// trick the user asset into thinking we're purging the
						// assets (which we are, just not from the trash), so it
						// will release its hold on its inbox and workspace
						$GLOBALS['SQ_PURGING_TRASH'] = TRUE;

						// Delete the inbox and remove its URLs
						$inbox_link = $user->getInboxLink();
						$inbox =& $GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid']);
						$GLOBALS['SQ_SYSTEM']->am->trashAsset($inbox->id);
						$inbox->updateLookups();

						// Delete the workspace and remove its URLs
						$workspace =& $user->getWorkspace();
						$GLOBALS['SQ_SYSTEM']->am->trashAsset($workspace->id);
						$workspace->updateLookups();

						$inbox->delete(false);
						$workspace->delete(false);
						unset($GLOBALS['SQ_PURGING_TRASH']);

						// Delete the account
						$user->delete(false);

						unset($user);
					}
				}

				unset($asset);

			}//end if
		}//end foreach

		return SQ_CRON_JOB_COMPLETED;

	}//end _exec()


	/**
	* Deal with a workflow step that has been running for longer than its expiry time
	*
	* To do this we find the administrators of the asset and send them a message
	* explaining the situation.
	*
	* @param string	$assetid		ID of the asset whose workflow has expired
	* @param string	$asset_name		Name of that asset
	* @param string	$schema_name	Name of the workflow schema
	* @param array	$step_data		Workflow step data
	*
	* @return void
	* @access private
	*/
	function _processExpiredWorkflowStep($assetid, $asset_name, $schema_name, $step_data)
	{


	}//end _processExpiredWorkflowStep()


	/**
	* Runs this job
	*
	* This method is overridden because we want to make sure the cron manager never deletes the
	* expired_workflow cron job
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @return int
	* @access public
	*/
	function run()
	{
		$res = parent::run();
		if ($res & SQ_CRON_JOB_REMOVE) {
			$res = $res - SQ_CRON_JOB_REMOVE;
		}
		return $res;

	}//end run()


}//end class

?>
