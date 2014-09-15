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
* $Id: cron_job_future_lineage_edit_fns.inc,v 1.28 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Future_Lineage_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.28 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Lineage_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the tasks interface in a one line format
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackend(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		$major = $asset->getAssetInLink('major');
		$minor = $asset->getAssetInLink('minor');

		$o->openSection(($asset->id) ? $asset->name : translate('cron_new_future_linking'));

		$o->openField(translate('delete_link'));
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');
			$link_options = Array(0 => '', 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS' => translate('cron_fl_delete_all_existing_links'));
			foreach ($links as $link) {
				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);

				// if the link id is null we got to continue
				// this line has been added to fix a problem when viewing the linking screen of
				// a LDAP backend User
				if (empty($link['linkid'])) continue;

				if (is_null($parent)) continue;
				$link_options[$link['linkid']] = translate('link').' #'.$link['linkid'].' - '.translate('parent').' : '.$parent->name.' (#'.$parent->id.')';
			}

			$default_delete_linkid = $asset->attr('delete_linkid');
			if ($asset->attr('delete_link_all')) {
				$default_delete_linkid = 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS';
			}
			combo_box($prefix.'_delete_linkid', $link_options, FALSE, $default_delete_linkid);
		$o->closeField();

		$o->openField(translate('new_link_type'));
			$link_info = $asset->attr('link_info');
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$editable_link_types = Array(
									SQ_LINK_TYPE_1	=> link_type_name(SQ_LINK_TYPE_1),
									SQ_LINK_TYPE_2	=> link_type_name(SQ_LINK_TYPE_2),
									SQ_LINK_NOTICE	=> link_type_name(SQ_LINK_NOTICE),
								   );
			$link_type = (isset($link_info['link_type'])) ? $link_info['link_type'] : '';
			combo_box($prefix.'_link_type', $editable_link_types, FALSE, $link_type);
		$o->closeField();

		$o->openField(translate('new_link_parent'));
			if ($asset->writeAccess('links')) {
				asset_finder($prefix.'_major_assetid', ((is_null($major)) ? 0 : $major->id), Array(), 'sq_sidenav', FALSE, 'null', Array('clear'));
			} else {
				if (is_null($major)) {
					echo '<a href="'.$major->getBackendHref().'">'.sprintf(translate('asset_format'), $major->name, $major->id).'</a>';
				} else {
					echo translate('not_set');
				}
			}
		$o->closeField();

		$o->openField(translate('at'));
			$this->_paintWhenBox($asset, $o, $prefix);
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$o->note(sprintf(translate('cron_manager_next_run'), $cron_mgr->readableRefreshTime(), $GLOBALS['SQ_SYSTEM']->datetime($cron_mgr->timeOfNextRun())));
		$o->closeField();

		$o->closeSection();

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackend(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_delete_linkid']) && (empty($_POST[$prefix.'_link_type']) || empty($_POST[$prefix.'_major_assetid']['assetid']))) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Are we going to delete all existing links...? Or maybe just the one that
		// the user has specified.
		$delete_linkid = $_POST[$prefix.'_delete_linkid'];
		if ($delete_linkid == 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS') {
			$attribute = Array('name' => 'delete_link_all', 'value' => TRUE);
		} else {
			$attribute = Array('name' => 'delete_linkid', 'value' => $delete_linkid);
		}
		if (!$asset->setAttrValue($attribute['name'], $attribute['value'])) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (!empty($_POST[$prefix.'_major_assetid']['assetid'])) {
			$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$prefix.'_major_assetid']['assetid']);
			if (is_null($major)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			$link_info = $asset->attr('link_info');
			$link_info['link_type'] = $_POST[$prefix.'_link_type'];
			if (!$asset->setAttrValue('link_info', $link_info) || !$asset->setAssetInLink($major, 'major')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		if (!$asset->saveAttributes() || !$asset->setAttrValue('when', $this->_processWhenBox($asset, $o, $prefix))) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end processInLineBackend()


	/**
	* Prints the create new link section
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackendCreate(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		// section to create a new link
		$o->openSection(translate('create_linking'));

		// add the asset finder
		$o->openField(translate('new_link_parent'));
		asset_finder($prefix.'_major_assetid');
		$o->closeField();

		// type of the link
		$o->openField(translate('new_link_type'));
		$editable_link_types = Array(
								SQ_LINK_TYPE_1	=> link_type_name(SQ_LINK_TYPE_1),
								SQ_LINK_TYPE_2	=> link_type_name(SQ_LINK_TYPE_2),
								SQ_LINK_NOTICE	=> link_type_name(SQ_LINK_NOTICE),
							   );
		combo_box($prefix.'_link_type', $editable_link_types, FALSE, SQ_LINK_TYPE_1);
		$o->closeField();

		// value of the link
		$o->openField(translate('use_link_value'));
		text_box($prefix.'_link_value', '', 20, 20);
		$o->closeField();

		// if the minor asset is a shadow asset we will not allow the backend user to create a new link in the future
		$minor_asset = $asset->getAssetInLink('minor');

		// check if the minor is a shadow asset or not
		$id_parts = explode(':', $minor_asset->id);

		// if it is a shadow asset the user cannot create a new link at a future time using the cron manager
		if (isset($id_parts[1])) {
			$o->openField(translate('at'));
			$o->note(translate('cron_manager_future_not_available_create'));
			$this->_paintWhenBox($asset, $o, $prefix,  '', 'disabled');
			$o->closeField();
		} else {
			// if the minor asset is not a shadow asset
			// allow the user to set the create link as a future link
			$o->openField(translate('at'));
			$o->note(translate('cron_manager_create_now_or_later'));
			$this->_paintWhenBox($asset, $o, $prefix);
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$o->note(sprintf(translate('cron_manager_next_run'), $cron_mgr->readableRefreshTime(), $GLOBALS['SQ_SYSTEM']->datetime($cron_mgr->timeOfNextRun())));
			$o->closeField();
		}


		$o->closeSection();

	}//end paintInLineBackendCreate()


	/**
	* Process the create new link section
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackendCreate(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		// if the major_assetid_create is not empty we will link this asset to the major asset
		if ($major_id_array = array_get_index($_POST, $prefix.'_major_assetid', FALSE)) {

			$major_id = $major_id_array['assetid'];

			// if the major is empty throw an error
			if (empty($major_id)) return;

			// get the link type
			$link_type = array_get_index($_POST, $prefix.'_link_type', SQ_LINK_TYPE_1);

			// get the link value
			$link_value = array_get_index($_POST, $prefix.'_link_value', '');

			// get the future time
			$future_time = Array();
			$future_time['day'] = array_get_index($_REQUEST, $prefix.'_when_day', '');
			$future_time['month'] = array_get_index($_REQUEST, $prefix.'_when_month', '');
			$future_time['year'] = array_get_index($_REQUEST, $prefix.'_when_year', '');
			$future_time['duration'] = array_get_index($_REQUEST, $prefix.'_when_duration', '');
			$future_time['period'] = array_get_index($_REQUEST, $prefix.'_when_period', '');

			// if the future_time[XX] is empty it means that the field were disabled or we are getting a wrong future time
			$link_now = ($future_time['day'] == '') || ($future_time['month'] == '') || ($future_time['year'] == '') || ($future_time['duration'] == '') |
			(($future_time['day'] == '--') && ($future_time['month'] == '--') && ($future_time['year'] == '--') && ($future_time['duration'] == '-'));

			// if no time is set of if the future time is incorrect
			// we do the linking now
			if ($link_now) {
				// we create the link immediately
				$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($major_id);
				$value = $link_value;
				$dependant = FALSE;
				$exclusive = FALSE;
				$sort_order = '';
				$am = $GLOBALS['SQ_SYSTEM']->am;

				$minor_asset = $asset->getAssetInLink('minor');

				// this array contains the information of the minor asset id
				// old parent id and link id and link type wanted type
				$_assets = Array();

				// this array will be passed to the hipo job
				// the $_assets array will be part of this array
				$hipo_var = Array();

				// check if the minor is a shadow asset or not
				$id_parts = explode(':', $minor_asset->id);

				// if it is a shadow asset get the bridge to get linkid, parents, etc.
				if (isset($id_parts[1])) {
					$bridge = $am->getAsset($id_parts[0]);
					$_links = $bridge->getLinks($minor_asset->id, SQ_SC_LINK_BACKEND_NAV, '', TRUE, 'minor');

					// of there is no link do nothing
					if (count($_links) <= 0) return FALSE;

					// build the $_assets for an shadow asset
					// The link id is composed with the bridge id and the shadowasset id
					// parentid contains the old parent id
					$_assets[$minor_asset->id] = Array(
													'linkid'	=> $bridge->id.':'.$minor_asset->id,
													'link_type'	=> $link_type,
													'parentid'	=> $_links[0]['majorid'],
												 );

				} else {
					// the minor is a normal asset so ask the asset manager to get linkid, parents, etc.
					$_links = $am->getLink($minor_asset->id, NULL, '', TRUE, NULL, 'minor');

					// parentid contains the old parent id
					$_assets[$minor_asset->id] = Array(
													'linkid'	=> $_links['linkid'],
													'link_type'	=> $link_type,
													'parentid'	=> $_links['majorid'],
												 );
				}

				// check if we can create a link under the new parent
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($minor_asset->id);
				$value = $link_value;
				$dependant = FALSE;
				$exclusive = FALSE;
				$sort_order = 1;
				$asset->prepareLink($new_parent, 'minor', $link_type, $value, $sort_order, $dependant, $exclusive);
				$new_parent->prepareLink($asset, 'major', $link_type, $value, $sort_order, $dependant, $exclusive);
				$can_create = $new_parent->canCreateLink($asset, $link_type, $exclusive);

				// if we cannot create the link do nothing
				if ($can_create !== TRUE) {
					trigger_error($can_create);
					return FALSE;
				}

				$hipo_var = Array(
								'link_action'		=> 'create',
								'to_parent_assetid'	=> $major_id,
								'to_parent_pos'		=> 1,
								'assets'			=> $_assets,
								'link_value'		=> $value,
							);
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->queueHipo('hipo_job_create_links', $hipo_var);

				// check for any HIPOs that have been queued
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

				$url = $hh->runQueuedJobs();

				if (!empty($url)) $o->setRedirect($url);

				return FALSE;
			} else {
				// if a time is set so we need to add a job in the cron manager
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($major_id);

				if (is_null($major)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$link_info = $asset->attr('link_info');
				$link_info['link_type'] = $link_type;
				$link_info['value'] = $link_value;
				if (!$asset->setAttrValue('link_info', $link_info) || !$asset->setAssetInLink($major, 'major')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				if (!$asset->saveAttributes() || !$asset->setAttrValue('when', $this->_processWhenBox($asset, $o, $prefix))) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return TRUE;
			}

		}//end if

	}//end processInLineBackendCreate()


	/**
	* Prints the delete section
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackendDelete(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		$major = $asset->getAssetInLink('major');
		$minor = $asset->getAssetInLink('minor');

		$o->openSection(translate('delete_link'));

		$o->openField(translate('delete_link'));
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT|SQ_LINK_NOTICE, '', TRUE, 'minor');
		$link_options = Array(0 => '', 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS' => translate('cron_fl_delete_all_existing_links'));
		foreach ($links as $link) {
			$print_link_type = $this->_printLinkType($link);
			$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
			if (is_null($parent)) continue;

			// if the link id is null we got to continue
			// this line has been added to fix a problem when viewing the linking screen of
			// a LDAP backend User/Shadow assets
			if (empty($link['linkid'])) continue;
			if (empty($link['linkid'])) {

				// if the link id is equal to zero and the minor id exists it is more likely to be a shadow asset
				if (!empty($minor->id)) {
					$id_parts = explode(':', $minor->id);
					if (isset($id_parts[1])) {
						// it is a shadow asset the link id is the bridge id following with the minor id
						$link_options[$id_parts[0].':'.$minor->id] = translate('link').' #'.$id_parts[0].':'.$minor->id.' ('.$print_link_type.') - '.translate('parent').' : '.$parent->name.' (#'.$id_parts[0].')';
						continue;
					} else {
						// the minor asset is not a shadow asset but the link is equal to zero
						// we just ignore that asset
						continue;
					}
				} else {
					// the minor id is empty, ignore this asset
					continue;
				}
			}


			$link_options[$link['linkid']] = translate('link').' #'.$link['linkid'].' ('.$print_link_type.') - '.translate('parent').' : '.$parent->name.' (#'.$parent->id.')';
		}//end foreach

		$default_delete_linkid = $asset->attr('delete_linkid');
		if ($asset->attr('delete_link_all')) {
			$default_delete_linkid = 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS';
		}
		combo_box($prefix.'_delete_linkid', $link_options, FALSE, $default_delete_linkid);
		$o->closeField();

		// if the minor asset is a shadow asset we will not allow the backend user to create a new link in the future
		$minor_asset = $asset->getAssetInLink('minor');

		// check if the minor is a shadow asset or not
		$id_parts = explode(':', $minor_asset->id);

		// if the minor asset is a shadow asset the user cannot create a new link at a future time using the cron manager
		if (isset($id_parts[1])) {
			$o->openField(translate('at'));
			$o->note(translate('cron_manager_future_not_available_delete'));
			$this->_paintWhenBox($asset, $o, $prefix, '', 'disabled');
			$o->closeField();
		} else {
			// if the minor asset is not a shadow asset
			// allow the user to set the delete link at a future time
			$o->openField(translate('at'));
			$o->note(translate('cron_manager_delete_now_or_later'));
			$this->_paintWhenBox($asset, $o, $prefix);
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$o->note(sprintf(translate('cron_manager_next_run'), $cron_mgr->readableRefreshTime(), $GLOBALS['SQ_SYSTEM']->datetime($cron_mgr->timeOfNextRun())));
			$o->closeField();
		}
		$o->closeSection();

		return TRUE;

	}//end paintInLineBackendDelete()


	/**
	* Process the tasks interface
	* we assume that the links id passed are not a link between an shadow asset and another shadow asset
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackendDelete(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_delete_linkid'])) {
			return FALSE;
		}

		$minor_asset = $asset->getAssetInLink('minor');
		// link to delete
		$delete_linkid = $_POST[$prefix.'_delete_linkid'];


		// get the future time
		$future_time[] = Array();
		$future_time['day'] = array_get_index($_REQUEST, $prefix.'_when_day', '');
		$future_time['month'] = array_get_index($_REQUEST, $prefix.'_when_month', '');
		$future_time['year'] = array_get_index($_REQUEST, $prefix.'_when_year', '');
		$future_time['duration'] = array_get_index($_REQUEST, $prefix.'_when_duration', '');
		$future_time['period'] = array_get_index($_REQUEST, $prefix.'_when_period', '');

		// if the future_time[XX] is empty it means that the field were disabled or we are getting a wrong future time
		$link_now = ($future_time['day'] == '') || ($future_time['month'] == '') || ($future_time['year'] == '') || ($future_time['duration'] == '') |
		(($future_time['day'] == '--') && ($future_time['month'] == '--') && ($future_time['year'] == '--') && ($future_time['duration'] == '-'));

		// if no time is set of if the future time is incorrect
		// we delete the link now
		if ($link_now) {
			// we delete the link immediately

			// trash_folder
			$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

			// get the asset manager
			$am = $GLOBALS['SQ_SYSTEM']->am;

			// get the hipo herder
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			// we delete all the links to the minor asset
			if ($delete_linkid == 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS') {
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor_asset->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');
				$link_options = Array();

				$_assets[$minor_asset->id] = Array();

				// loop through all the links and add them to the hipo herder
				foreach ($links as $link) {
					// if the linkid is empty it means that it is a wrong link or a link to and bridge and shadow asset
					if (empty($link['linkid'])) continue;

					// get the information about the link we want to move to the trash
					$link_info = $am->getLinkById($link['linkid'], $minor_asset->id, 'minor');

					// if the link is already in the trash then skip it
					if ($link_info['majorid'] == $trash_folder->id) {
						continue;
					}

					// this array contains the link information to move to the trash
					// parentid contains the old parent id
					$_assets[$minor_asset->id][] = Array(
													'linkid'	=> $link_info['linkid'],
													'link_type'	=> $link_info['link_type'],
													'parentid'	=> $link_info['majorid'],
												   );

				}//end foreach

				if (!empty($_assets[$minor_asset->id])) {
					// gathered links, now give them to the HIPO
					$hipo_vars = Array(
									'link_action'		=> 'move',
									'to_parent_assetid'	=> $trash_folder->id,
									'to_parent_pos'		=> 1,
									'assets'			=> $_assets,
								 );


					// add the job to the hipo queue
					$hh->queueHipo('hipo_job_create_links', $hipo_vars);

					// check for any HIPOs that have been queued
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

					$url = $hh->runQueuedJobs();

					// if the hipo URL is not empty redirect to the hipo
					if (!empty($url)) $o->setRedirect($url);
				}

				// once all the jobs have been added to the queue




			} else {
				// we delete a specific link id

				// get the information about the link we want to move to the trash
				$link_info = $am->getLinkById($delete_linkid, $minor_asset->id, 'minor');

				// Delete notice links first
				if ($link_info['link_type'] == SQ_LINK_NOTICE) {
					// if type code is not there, assume we are a shadow link
					if (is_null($link_info['major_type_code'])) {
						$GLOBALS['SQ_SYSTEM']->am->deleteShadowAssetLink($link_info['linkid']);
					} else {
						$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link_info['linkid']);
					}

					// I'm OK! FALSE is needed not to throw an error
					return FALSE;
				}

				// if the link is already in the trash then skip it
				if ($link_info['majorid'] == $trash_folder->id) {
					return FALSE;
				}

				// this array contains the link information to move to the trash
				// parentid contains the old parent id
				$_assets[$minor_asset->id] = Array(
												'linkid'	=> $link_info['linkid'],
												'link_type'	=> $link_info['link_type'],
												'parentid'	=> $link_info['majorid'],
											 );

				// this array will be given to the hipo
				$hipo_var = Array(
								'link_action'		=> 'move',
								'to_parent_assetid'	=> $trash_folder->id,
								'to_parent_pos'		=> 1,
								'assets'			=> $_assets,
							);

				$hh->queueHipo('hipo_job_create_links', $hipo_var);

				// check for any HIPOs that have been queued
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

				$url = $hh->runQueuedJobs();

				// if the hipo URL is not empty redirect to the hipo
				if (!empty($url)) $o->setRedirect($url);

			}//end else

			return FALSE;
		} else {

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// Are we going to delete all existing links...? Or maybe just the one that
			// the user has specified.
			if ($delete_linkid == 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS') {
				$attribute = Array('name' => 'delete_link_all', 'value' => TRUE);
			} else {
				$attribute = Array('name' => 'delete_linkid', 'value' => $delete_linkid);
			}
			if (!$asset->setAttrValue($attribute['name'], $attribute['value'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if (!$asset->saveAttributes() || !$asset->setAttrValue('when', $this->_processWhenBox($asset, $o, $prefix))) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		}

	}//end processInLineBackendDelete()


	/**
	* Prints the create new link section for linked children
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackendCreateReverse(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		// section to create a new link
		$o->openSection(translate('create_linking'));

		// add the asset finder
		$o->openField(translate('new_link_child'));
		asset_finder($prefix.'_minor_assetid');
		$o->closeField();

		// type of the link
		$o->openField(translate('new_link_type'));
		$editable_link_types = Array(
								SQ_LINK_TYPE_1	=> link_type_name(SQ_LINK_TYPE_1),
								SQ_LINK_TYPE_2	=> link_type_name(SQ_LINK_TYPE_2),
								SQ_LINK_NOTICE	=> link_type_name(SQ_LINK_NOTICE),
							   );
		combo_box($prefix.'_link_type', $editable_link_types, FALSE, SQ_LINK_TYPE_1);
		$o->closeField();

		// value of the link
		$o->openField(translate('use_link_value'));
		text_box($prefix.'_link_value', '', 20, 20);
		$o->closeField();

		// if the major asset is a shadow asset we will not allow the backend user to create a new link in the future
		$major_asset = $asset->getAssetInLink('major');

		// check if the minor is a shadow asset or not
		$id_parts = explode(':', $major_asset->id);

		// if it is a shadow asset the user cannot create a new link at a future time using the cron manager
		if (isset($id_parts[1])) {
			$o->openField(translate('at'));
			$o->note(translate('cron_manager_future_not_available_create'));
			$this->_paintWhenBox($asset, $o, $prefix,  '', 'disabled');
			$o->closeField();
		} else {
			// if the minor asset is not a shadow asset
			// allow the user to set the create link as a future link
			$o->openField(translate('at'));
			$o->note(translate('cron_manager_create_now_or_later'));
			$this->_paintWhenBox($asset, $o, $prefix);
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$o->note(sprintf(translate('cron_manager_next_run'), $cron_mgr->readableRefreshTime(), $GLOBALS['SQ_SYSTEM']->datetime($cron_mgr->timeOfNextRun())));
			$o->closeField();
		}


		$o->closeSection();

	}//end paintInLineBackendCreateReverse()


	/**
	* Process the create new link section for linked children
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackendCreateReverse(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		// if the major_assetid_create is not empty we will link this asset to the major asset
		if ($minor_id_array = array_get_index($_POST, $prefix.'_minor_assetid', FALSE)) {

			$minor_id = $minor_id_array['assetid'];

			// if the major is empty throw an error
			if (empty($minor_id)) return;

			// get the link type
			$link_type = array_get_index($_POST, $prefix.'_link_type', SQ_LINK_TYPE_1);

			// get the link value
			$link_value = array_get_index($_POST, $prefix.'_link_value', '');

			// get the future time
			$future_time = Array();
			$future_time['day'] = array_get_index($_REQUEST, $prefix.'_when_day', '');
			$future_time['month'] = array_get_index($_REQUEST, $prefix.'_when_month', '');
			$future_time['year'] = array_get_index($_REQUEST, $prefix.'_when_year', '');
			$future_time['duration'] = array_get_index($_REQUEST, $prefix.'_when_duration', '');
			$future_time['period'] = array_get_index($_REQUEST, $prefix.'_when_period', '');

			// if the future_time[XX] is empty it means that the field were disabled or we are getting a wrong future time
			$link_now = ($future_time['day'] == '') || ($future_time['month'] == '') || ($future_time['year'] == '') || ($future_time['duration'] == '') |
			(($future_time['day'] == '--') && ($future_time['month'] == '--') && ($future_time['year'] == '--') && ($future_time['duration'] == '-'));

			// if no time is set of if the future time is incorrect
			// we do the linking now
			if ($link_now) {
				// we create the link immediately
				$value = $link_value;
				$dependant = FALSE;
				$exclusive = FALSE;
				$sort_order = '';
				$am = $GLOBALS['SQ_SYSTEM']->am;

				$major_asset = $asset->getAssetInLink('major');

				// this array contains the information of the minor asset id
				// old parent id and link id and link type wanted type
				$_assets = Array();

				// this array will be passed to the hipo job
				// the $_assets array will be part of this array
				$hipo_var = Array();

				// check if the minor is a shadow asset or not
				$id_parts = explode(':', $major_asset->id);

				// if it is a shadow asset get the bridge to get linkid, parents, etc.
				if (isset($id_parts[1])) {
					$bridge = $am->getAsset($id_parts[0]);
					$_links = $bridge->getLinks($major_asset->id, SQ_SC_LINK_BACKEND_NAV, '', TRUE, 'major');

					// of there is no link do nothing
					if (count($_links) <= 0) return FALSE;

					// build the $_assets for an shadow asset
					// The link id is composed with the bridge id and the shadowasset id
					// parentid contains the old parent id
					$_assets[$minor_asset->id] = Array(
													'linkid'	=> $bridge->id.':'.$minor_asset->id,
													'link_type'	=> $link_type,
													'parentid'	=> $_links[0]['majorid'],
												 );

				} else {
					// the minor is a normal asset so ask the asset manager to get linkid, parents, etc.
					$_links = $am->getLink($major_asset->id, NULL, '', TRUE, NULL, 'major');

						// parentid contains the old parent id
						$_assets[$minor_id] = Array(
														'linkid'	=> isset($_links['linkid']) ? $_links['linkid'] : '',
														'link_type'	=> $link_type,
														'parentid'	=> $major_asset->id,
													 );
				}

				// check if we can create a link under the new parent
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($minor_id);
				$value = $link_value;
				$dependant = FALSE;
				$exclusive = FALSE;
				$sort_order = NULL;
				$asset->prepareLink($major_asset, 'minor', $link_type, $value, $sort_order, $dependant, $exclusive);
				$major_asset->prepareLink($asset, 'major', $link_type, $value, $sort_order, $dependant, $exclusive);
				$can_create = $major_asset->canCreateLink($asset, $link_type, $exclusive);

				// if we cannot create the link do nothing
				if ($can_create !== TRUE) return FALSE;

				$hipo_var = Array(
								'link_action'		=> 'create',
								'to_parent_assetid'	=> $major_asset->id,
								'to_parent_pos'		=> 1,
								'assets'			=> $_assets,
								'link_value'		=> $value,
							);
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->queueHipo('hipo_job_create_links', $hipo_var);

				// check for any HIPOs that have been queued
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

				$url = $hh->runQueuedJobs();

				if (!empty($url)) $o->setRedirect($url);

				return FALSE;
			} else {
				// if a time is set so we need to add a job in the cron manager
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$minor_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($minor_id);

				if (is_null($minor_asset)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$link_info = $asset->attr('link_info');
				$link_info['link_type'] = $link_type;
				$link_info['value'] = $link_value;
				if (!$asset->setAttrValue('link_info', $link_info) || !$asset->setAssetInLink($minor_asset, 'minor')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				if (!$asset->setAttrValue('reverse_mode', 1)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				if (!$asset->saveAttributes() || !$asset->setAttrValue('when', $this->_processWhenBox($asset, $o, $prefix))) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return TRUE;
			}

		}//end if

	}//end processInLineBackendCreateReverse()


	/**
	* Prints the delete section for linked children
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackendDeleteReverse(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		$major = $asset->getAssetInLink('major');

		$o->openSection(translate('delete_link'));

		$o->openField(translate('delete_link'));
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($major->id, SQ_SC_LINK_SIGNIFICANT|SQ_LINK_NOTICE, '', TRUE, 'major');
		$link_options = Array(0 => '', 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS' => translate('cron_fl_delete_all_existing_links'));
		foreach ($links as $link) {
			$print_link_type = $this->_printLinkType($link);
			$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($child)) continue;

			// if the link id is null we got to continue
			// this line has been added to fix a problem when viewing the linking screen of
			// a LDAP backend User/Shadow assets
			if (empty($link['linkid'])) continue;


			$link_options[$link['linkid']] = translate('link').' #'.$link['linkid'].' ('.$print_link_type.') - '.translate('child').' : '.$child->name.' (#'.$child->id.')';
		}//end foreach

		$default_delete_linkid = $asset->attr('delete_linkid');
		if ($asset->attr('delete_link_all')) {
			$default_delete_linkid = 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS';
		}
		combo_box($prefix.'_delete_linkid', $link_options, FALSE, $default_delete_linkid);
		$o->closeField();

		// if the minor asset is a shadow asset we will not allow the backend user to create a new link in the future
		$major_asset = $asset->getAssetInLink('major');

		// check if the minor is a shadow asset or not
		$id_parts = explode(':', $major_asset->id);

		// if the minor asset is a shadow asset the user cannot create a new link at a future time using the cron manager
		if (isset($id_parts[1])) {
			$o->openField(translate('at'));
			$o->note(translate('cron_manager_future_not_available_delete'));
			$this->_paintWhenBox($asset, $o, $prefix, '', 'disabled');
			$o->closeField();
		} else {
			// if the minor asset is not a shadow asset
			// allow the user to set the delete link at a future time
			$o->openField(translate('at'));
			$o->note(translate('cron_manager_delete_now_or_later'));
			$this->_paintWhenBox($asset, $o, $prefix);
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$o->note(sprintf(translate('cron_manager_next_run'), $cron_mgr->readableRefreshTime(), $GLOBALS['SQ_SYSTEM']->datetime($cron_mgr->timeOfNextRun())));
			$o->closeField();
		}
		$o->closeSection();

		return TRUE;

	}//end paintInLineBackendDeleteReverse()


	/**
	* Process the deletion of links of linked children
	* we assume that the links id passed are not a link between an shadow asset and another shadow asset
	*
	* @param Cron_Job_Future_Lineage	$asset	the owning asset
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackendDeleteReverse(Cron_Job_Future_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_delete_linkid'])) {
			return FALSE;
		}

		$major_asset = $asset->getAssetInLink('major');
		// link to delete
		$delete_linkid = $_POST[$prefix.'_delete_linkid'];


		// get the future time
		$future_time[] = Array();
		$future_time['day'] = array_get_index($_REQUEST, $prefix.'_when_day', '');
		$future_time['month'] = array_get_index($_REQUEST, $prefix.'_when_month', '');
		$future_time['year'] = array_get_index($_REQUEST, $prefix.'_when_year', '');
		$future_time['duration'] = array_get_index($_REQUEST, $prefix.'_when_duration', '');
		$future_time['period'] = array_get_index($_REQUEST, $prefix.'_when_period', '');

		// if the future_time[XX] is empty it means that the field were disabled or we are getting a wrong future time
		$link_now = ($future_time['day'] == '') || ($future_time['month'] == '') || ($future_time['year'] == '') || ($future_time['duration'] == '') |
		(($future_time['day'] == '--') && ($future_time['month'] == '--') && ($future_time['year'] == '--') && ($future_time['duration'] == '-'));

		// if no time is set of if the future time is incorrect
		// we delete the link now
		if ($link_now) {
			// we delete the link immediately

			// trash_folder
			$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

			// get the asset manager
			$am = $GLOBALS['SQ_SYSTEM']->am;

			// get the hipo herder
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			// we delete all the links to the minor asset
			if ($delete_linkid == 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS') {
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($major_asset->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'major');
				$link_options = Array();



				// loop through all the links and add them to the hipo herder
				foreach ($links as $link) {

					// if the linkid is empty it means that it is a wrong link or a link to and bridge and shadow asset
					if (empty($link['linkid'])) continue;

					// get the information about the link we want to move to the trash
					$link_info = $am->getLinkById($link['linkid'], $major_asset->id, 'major');

					$_assets[$link_info['minorid']] = Array();
					// this array contains the link information to move to the trash
					// parentid contains the old parent id
					$_assets[$link_info['minorid']][] = Array(
													'linkid'	=> $link_info['linkid'],
													'link_type'	=> $link_info['link_type'],
													'parentid'	=> $major_asset->id,
												   );

				}//end foreach

				if (!empty($_assets)) {
					// gathered links, now give them to the HIPO
					$hipo_vars = Array(
									'link_action'		=> 'move',
									'to_parent_assetid'	=> $trash_folder->id,
									'to_parent_pos'		=> 1,
									'assets'			=> $_assets,
								 );

					// add the job to the hipo queue
					$hh->queueHipo('hipo_job_create_links', $hipo_vars);

					// check for any HIPOs that have been queued
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

					$url = $hh->runQueuedJobs();

					// if the hipo URL is not empty redirect to the hipo
					if (!empty($url)) $o->setRedirect($url);
				}

				// once all the jobs have been added to the queue




			} else {
				// we delete a specific link id

				// get the information about the link we want to move to the trash
				$link_info = $am->getLinkById($delete_linkid, $major_asset->id, 'major');

				// Delete notice links first
				if ($link_info['link_type'] == SQ_LINK_NOTICE) {
					// if type code is not there, assume we are a shadow link
					if (is_null($link_info['minor_type_code'])) {
						$GLOBALS['SQ_SYSTEM']->am->deleteShadowAssetLink($link_info['linkid']);
					} else {
						$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link_info['linkid']);
					}

					// I'm OK! FALSE is needed not to throw an error
					return FALSE;
				}


				// this array contains the link information to move to the trash
				// parentid contains the old parent id
				$_assets[$link_info['minorid']] = Array(
												'linkid'	=> $link_info['linkid'],
												'link_type'	=> $link_info['link_type'],
												'parentid'	=> $major_asset->id,
											 );

				// this array will be given to the hipo
				$hipo_var = Array(
								'link_action'		=> 'move',
								'to_parent_assetid'	=> $trash_folder->id,
								'to_parent_pos'		=> 1,
								'assets'			=> $_assets,
							);

				$hh->queueHipo('hipo_job_create_links', $hipo_var);

				// check for any HIPOs that have been queued
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

				$url = $hh->runQueuedJobs();

				// if the hipo URL is not empty redirect to the hipo
				if (!empty($url)) $o->setRedirect($url);

			}//end else

			return FALSE;
		} else {

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// Are we going to delete all existing links...? Or maybe just the one that
			// the user has specified.
			if ($delete_linkid == 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS') {
				$attribute = Array('name' => 'delete_link_all', 'value' => TRUE);
			} else {
				$attribute = Array('name' => 'delete_linkid', 'value' => $delete_linkid);
			}
			if (!$asset->setAttrValue($attribute['name'], $attribute['value'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if (!$asset->setAttrValue('reverse_mode', 1)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			if (!$asset->saveAttributes() || !$asset->setAttrValue('when', $this->_processWhenBox($asset, $o, $prefix))) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		}
	}//end processInLineBackendDeleteReverse()


	/**
	* Prints link type
	*
	* @param array	$link_info	The link information array
	*
	* @return boolean
	* @access public
	*/
	private function _printLinkType($link_info)
	{
		$type_return = '';
		if (!isset($link_info['link_type'])) {
			return $type_return;
		}

		switch ($link_info['link_type']) {
			case SQ_LINK_TYPE_1:
				$type_return = 'TYPE 1';
			break;
			case SQ_LINK_TYPE_2:
				$type_return = 'TYPE 2';
			break;
			case SQ_LINK_NOTICE:
				$type_return = 'NOTICE';
			break;
			default:
				$type_return = '';
		}//end switch

		return $type_return;

	}//end _printLinkType()


}//end class
?>
