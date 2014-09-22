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
* $Id: cron_job_manage_incomplete_submissions.inc,v 1.9 2013/05/07 09:14:39 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Cron_Job_Manage_Incomplete_Submissions
*
* Purpose
*    Handles the incomplete submissions of multiple-page Custom Form assets.
*    Each form will be allowed to specify a "warning period" and a "prune
*    period" (or similar names to):
*
*    - Users will get an email warning after a submission has been incomplete
*      for longer than the "warning period". A user will not receive an email if
*      the warning period is unset (or set to zero), or if a user does not have
*      their User asset's email field set.
*
*    - Incomplete submissions will be deleted from the system after a submission
*      has been incomplete for longer than the "prune period".
*
*    This asset borrows code somewhat from the "Manage Pending Accounts" cron
*    job, which manages not-yet-validated users created by Account Manager in a
*    similar way.
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Manage_Incomplete_Submissions extends Cron_Job
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
	* Well, not for this asset... if the CMS package is there, there'll always
	* be one of these in the system.
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
	* Does these things:
	* - First, gets its cache of submissions that have had warning emails sent
	*   out, and clears those that are completed (whether they exist or not)
	* - Then, works out which submissions need to have their warning emails
	*   sent this run, and stores them in the cache
	*
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
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		$assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form', FALSE);

		$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

		// Store our list of assets that we have previously warned about
		$filepath = $this->data_path.'/warning_assets.php';
		if (is_file($filepath)) {
			$warning_assetids = require_once($filepath);
			if (!is_array($warning_assetids)) {
				$warning_assetids = Array();
			}
		} else {
			$warning_assetids = Array();
		}

		foreach ($assetids as $assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			// Need to get the form URL from its parent wrapper page, as the
			// Form Contents asset (which is the actual form) has no URL
			$parent_assetids = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($assetid);
			// Value can't be passed directly to reset function
			$parent_assetid = reset($parent_assetids);

			$form_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($parent_assetid);

			// Get the pending accounts group, if it exists
			$incomplete_subs_folder = $asset->getSubmissionsFolder('incomplete_submissions');

			if ($incomplete_subs_folder) {
				// Remove incomplete submissions that have expired
				$remove_period = $asset->attr('submission_expiry_time');

				if ($remove_period > 0) {
					$date_sql = db_extras_todate(MatrixDAL::getDbType(), ':created', FALSE);

					$sql = 'SELECT DISTINCT
								a.assetid
							FROM sq_ast a
								JOIN sq_ast_lnk l
								ON a.assetid = l.minorid
							WHERE
								l.majorid = :majorid AND
								a.created < '.$date_sql;

					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'majorid', $incomplete_subs_folder->id);
					MatrixDAL::bindValueToPdo($query, 'created', ts_iso8601(time() - $remove_period));
					$submission_assetids = MatrixDAL::executePdoAssoc($query);

					foreach($submission_assetids as $submission_record) {
						$submission_assetid = $submission_record['assetid'];
						$submission_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($submission_assetid);

						$GLOBALS['SQ_SYSTEM']->am->trashAsset($submission_assetid);
						$submission_asset->delete(FALSE);

						// Don't need the incomplete attachments anymore, cleanup....
						$path = $asset->data_path.'/incomplete_attachments/s'.$submission_assetid;
						if (is_dir($path)) {
							delete_directory($path);
						}//end if

						unset($submission_asset);
					}
				}

				// From those that are left, see which ones have exceeded the
				// warning period.
				$warning_period = $asset->attr('submission_warning_time');

				if ($warning_period > 0) {
					// Email question assetid for the Public User
					$public_email_qid = $asset->attr('public_auth_email_q');

					$email_format_logged = $asset->getAttribute('warning_email_format');
					$email_format_public = $asset->getAttribute('warning_email_format_public');
					$date_sql = db_extras_todate(MatrixDAL::getDbType(), ':created', FALSE);

					$sql = 'SELECT DISTINCT
								a.assetid
							FROM sq_ast a
								JOIN sq_ast_lnk l
								ON a.assetid = l.minorid
							WHERE
								l.majorid = :majorid AND
								a.created < '.$date_sql;

					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'majorid', $incomplete_subs_folder->id);
					MatrixDAL::bindValueToPdo($query, 'created', ts_iso8601(time() - $warning_period));
					$submission_assetids = MatrixDAL::executePdoAll($query);

					foreach($submission_assetids as $submission_record) {
						$submission_assetid = $submission_record['assetid'];
						$submission_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($submission_assetid);

						if(empty($submission_asset)) continue;

						$created_userid = $submission_asset->created_userid;

						if (!isset($warning_assetids[$submission_assetid])) {
							// Use the respective email format based on the user type
							if ($public_userid == $created_userid) {
								$email_format = $email_format_public;
								$previous_value = $current_value = unserialize($email_format->value);
								if ($public_email_qid) {
									// If we have the email address registered for Public User for this submission
									$public_reg_email = $submission_asset->getAnswer($public_email_qid);
									if (!empty($public_reg_email)) {
										$current_value['to'][] = $public_reg_email;
									}
								}
							} else {
								$email_format = $email_format_logged;
								$previous_value = $current_value = unserialize($email_format->value);
								$current_value['to_assetids'][] = $created_userid;
							}

							// Add keyword replacements
							$keywords = $email_format->getKeywords();
							$keywords_str = !empty($keywords) ? ('%'.implode('%%', $keywords).'%') : '';
							$replacements = $asset->getIncompleteSubmissionMessageKeywordsReplacement($keywords_str, $submission_assetid);

							$email_format->setValue(serialize($current_value));
							$email_format->sendMail($replacements);
							$email_format->setValue(serialize($previous_value));

							$warning_assetids[$submission_assetid] = TRUE;
						}

						unset($submission_asset);
					}
				}

				unset($asset);

			}//end if
		}//end foreach

		// Clear out those assets from warning list that no longer exist, or
		// are no longer incomplete
		if (!empty($warning_assetids)) {

			$asset_attrs = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('complete', 'form_submission', array_keys($warning_assetids));

			foreach ($warning_assetids as $warning_assetid => $dummy) {
				if (!isset($asset_attrs[$warning_assetid])) {
					// Submission does not exist anymore - perhaps completed
					// but we are not logging submissions from its parent form
					unset($warning_assetids[$warning_assetid]);

				} else if ($asset_attrs[$warning_assetid] === 1) {
					// Submission exists but is complete
					unset($warning_assetids[$warning_assetid]);

				}

			}//end foreach

		}

		// recache our warning list
		create_directory($this->data_path);
		string_to_file('<'.'?php return '.var_export($warning_assetids, TRUE).'?'.'>', $filepath);
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
