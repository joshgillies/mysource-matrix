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
* $Id: cron_job_escalate_safe_trash.inc,v 1.1 2007/01/08 03:07:03 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';
require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Cron_Job_Escalate_Safe_Trash
*
* Purpose
*	Note that this is a singleton asset - when the asset type is installed it creates a single
*	instance of itself under the cron manager, which should remain there always.
*
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Escalate_Safe_Trash extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cron_Job_Escalate_Safe_Trash($assetid=0)
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
		return translate('cron_job_escalate_safe_trash');

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
	* For the expired workflow cron job, NO!  There must always be exactly one of these here.
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
		$cron_mgr_assetid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('cron_manager');
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($cron_mgr_assetid, SQ_LINK_TYPE_3, 'cron_job_attempt_safe_trash');

		foreach ($links as $link) {
			$job_assetid = $link['minorid'];
			$job_link = $GLOBALS['SQ_SYSTEM']->am->getLink($job_assetid, SQ_LINK_NOTICE, '', TRUE, 'updating');

			$job_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($job_assetid));
			$job_info = $job_info[$job_assetid];
			bam($job_info);

			$trash_user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($job_info['created_userid']);
			$trash_user_info = $trash_user_info[$job_info['created_userid']];
			bam($trash_user_info);


			if (!empty($job_link)) {
				$trashing_assetid = $job_link['minorid'];
				$trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($trashing_assetid);

				// If there are safe trash errors
				if (!empty($trash_errors)) {
					$trashing_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($trashing_assetid);
					$from = '0';

					$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
					$messages = Array();

					if (!empty($trash_errors['status'])) {
						// unsafe status
						$type = 'cron.safetrash.notify.status';
						$asset_admins = $GLOBALS['SQ_SYSTEM']->am->getPermission($trashing_assetid, SQ_PERMISSION_ADMIN, TRUE);

						$replacements = Array(
											'assetid'		=> $trashing_asset->id,
											'asset_name'	=> $trashing_asset->name,
											'asset_status'	=> get_status_description($trash_errors['status']),
											'trash_date'	=> easy_datetime(iso8601_ts($job_info['created'])),
											'trashing_user'	=> $trash_user_info['name'],
										);

						if (empty($asset_admins)) $asset_admins = Array(0);
						$msg =& $ms->newMessage($asset_admins, $type, $replacements);
						$messages[] =& $msg;
					}

					if (!empty($trash_errors['links'])) {
						// unsafe links
						$type = 'cron.safetrash.notify.links';

						foreach ($trash_errors['links'] as $link) {
							$asset_admins = $GLOBALS['SQ_SYSTEM']->am->getPermission($link['majorid'], SQ_PERMISSION_ADMIN, TRUE);
							$linked_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid']);

							$replacements = Array(
												'assetid'			=> $trashing_asset->id,
												'asset_name'		=> $trashing_asset->name,
												'link_assetid'		=> $linked_asset->id,
												'link_asset_name'	=> $linked_asset->name,
												'trash_date'		=> easy_datetime(iso8601_ts($job_info['created'])),
												'trashing_user'		=> $trash_user_info['name'],
											);

							if (empty($asset_admins)) $asset_admins = Array(0);
							$msg =& $ms->newMessage($asset_admins, $type, $replacements);
							$messages[] =& $msg;
						}


					}

					if (!empty($trash_errors['children'])) {
						// unsafe children
						$type = 'cron.safetrash.notify.children';

						foreach ($trash_errors['children'] as $link) {
							$asset_admins = $GLOBALS['SQ_SYSTEM']->am->getPermission($link['minorid'], SQ_PERMISSION_ADMIN, TRUE);
							$linked_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);

							$replacements = Array(
												'assetid'			=> $trashing_asset->id,
												'asset_name'		=> $trashing_asset->name,
												'link_assetid'		=> $linked_asset->id,
												'link_asset_name'	=> $linked_asset->name,
												'trash_date'		=> easy_datetime(iso8601_ts($job_info['created'])),
												'trashing_user'		=> $trash_user_info['name'],
											);

							if (empty($asset_admins)) $asset_admins = Array(0);
							$msg =& $ms->newMessage($asset_admins, $type, $replacements);
							$messages[] =& $msg;
						}
					}

					$ms->send($messages);

				}//end if trash errors
			}//end if job link not empty
		}//end foreach cron job link

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
