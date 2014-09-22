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
* $Id: cron_job_manage_pending_accounts.inc,v 1.16 2013/05/31 04:16:20 akarelia Exp $
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
* @version $Revision: 1.16 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

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
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

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
		trigger_localised_error('CRON0024', sprintf(translate('Unable to morph %s'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
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
		$assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_account_manager', FALSE);

		foreach($assetids as $assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			// Get the pending accounts group, if it exists
			$pending_group_link = $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, NULL, 'user_group', TRUE, 'pending_accounts');
			if ($asset->attr('use_email_validation') && !empty($pending_group_link)) {
				$pending_group = $GLOBALS['SQ_SYSTEM']->am->getAsset($pending_group_link['minorid']);

				// RESEND VALIDATION TIMEOUT //
				$resend_period = $asset->attr('resend_validation_period');
				if ($resend_period > 0) {
					$date_sql = db_extras_todate(MatrixDAL::getDbType(), ':created', FALSE);

					$sql = 'SELECT
								a.assetid, l.linkid
							FROM sq_ast a
								JOIN sq_ast_lnk l
								ON a.assetid = l.minorid
							WHERE
								l.majorid = :majorid AND
								l.link_type = :link_type AND
								a.created < '.$date_sql;

					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'majorid',   $pending_group->id);
					MatrixDAL::bindValueToPdo($query, 'link_type', SQ_LINK_TYPE_1);
					MatrixDAL::bindValueToPdo($query, 'created',   ts_iso8601(time() - $resend_period));
					$user_assetids = MatrixDAL::executePdoAll($query);

					foreach($user_assetids as $user_row) {

						$user_assetid = $user_row['assetid'];

						$resent_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($pending_group->id, $user_assetid, SQ_LINK_NOTICE, 'validation_email_resent');
						if (!empty($resent_link)) continue;

						$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($user_assetid);
						if(empty($user)) {
							trigger_error("User #".$user_assetid." doesn't exist", E_USER_WARNING);
							continue;
						}
						// Resend the validation and create a notice link so email does not get resent in future cron runs.
						if ($asset->resendValidation($user->attr('username'))) $GLOBALS['SQ_SYSTEM']->am->createAssetLink($pending_group, $user, SQ_LINK_NOTICE, 'validation_email_resent');

						unset($user);

					}
				}

				// REMOVE ACCOUNT TIMEOUT //
				$remove_period = $asset->attr('remove_account_period');
				if ($remove_period > 0) {
					$date_sql = db_extras_todate(MatrixDAL::getDbType(), ':created', FALSE);

					$sql = 'SELECT
								a.assetid, l.linkid
							FROM sq_ast a
								JOIN sq_ast_lnk l
								ON a.assetid = l.minorid
							WHERE
								l.majorid = :majorid AND
								a.created < '.$date_sql.' AND
								l.link_type < '.SQ_SC_LINK_SIGNIFICANT;


					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'majorid', $pending_group->id);
					MatrixDAL::bindValueToPdo($query, 'created', ts_iso8601(time() - $remove_period));
					$user_assetids = MatrixDAL::executePdoAll($query);

					foreach($user_assetids as $user_row) {
						$user_assetid = $user_row['assetid'];
						$user_linkid  = $user_row['linkid'];

						$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($user_assetid);
						if(empty($user)) {
							trigger_error("User #".$user_assetid." doesn't exist", E_USER_WARNING);
							continue;
						}
						$GLOBALS['SQ_SYSTEM']->am->trashAsset($user_assetid);

						// trick the user asset into thinking we're purging the
						// assets (which we are, just not from the trash), so it
						// will release its hold on its inbox and workspace
						$GLOBALS['SQ_PURGING_TRASH'] = TRUE;

						// Delete the inbox and remove its URLs
						$inbox_link = $user->getInboxLink();
						if (!empty($inbox_link)) {
							$inbox = $GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid']);
							$GLOBALS['SQ_SYSTEM']->am->trashAsset($inbox->id);
							$inbox->updateLookups();
							$inbox->delete(false);
						}

						// Delete the workspace and remove its URLs
						$workspace_link = $user->getWorkspaceLink();
						if (!empty($workspace_link)) {
							$workspace = $GLOBALS['SQ_SYSTEM']->am->getAsset($workspace_link['minorid']);
							$GLOBALS['SQ_SYSTEM']->am->trashAsset($workspace->id);
							$workspace->updateLookups();
							$workspace->delete(false);
						}

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
