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
* $Id: cron_job_future_lineage_edit_fns.inc,v 1.17 2006/01/26 22:34:06 lwright Exp $
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
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Lineage_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Future_Lineage_Edit_Fns()
	{
		$this->Cron_Job_Edit_Fns();

	}//end constructor


	/**
	* Prints the tasks interface in a one line format
	*
	* @param object	&$asset	the owning asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInLineBackend(&$asset, &$o, $prefix)
	{
		$major =& $asset->getAssetInLink('major');
		$minor =& $asset->getAssetInLink('minor');

		$o->openSection(($asset->id) ? $asset->name : translate('cron_new_future_linking'));

		$o->openField(translate('delete_link'));
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');
			$link_options = Array(0 => '', 'SQ_CRON_JOB_FUTURE_LINEAGE_DELETE_ALL_LINKS' => translate('cron_fl_delete_all_existing_links'));
			foreach ($links as $link) {
				$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
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
								   );
			$link_type = (isset($link_info['link_type'])) ? $link_info['link_type'] : '';
			combo_box($prefix.'_link_type', $editable_link_types, FALSE, $link_type);
		$o->closeField();

		$o->openField(translate('new_link_parent'));
			if ($asset->writeAccess('links')) {
				asset_finder($prefix.'_major_assetid', ((is_null($major)) ? 0 : $major->id), Array(), 'sq_sidenav', FALSE, 'null', Array('clear'));
			} else {
				if (is_null($major)) {
					echo '<a href="'.$major->getBackendHref().'">'.translate('asset_format', $major->name, $major->id).'</a>';
				} else {
					echo translate('not_set');
				}
			}
		$o->closeField();

		$o->openField(translate('at'));
			$this->_paintWhenBox($asset, $o, $prefix);
			$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$o->note(translate('cron_manager_next_run', $cron_mgr->readableRefreshTime(), $GLOBALS['SQ_SYSTEM']->datetime($cron_mgr->timeOfNextRun())));
		$o->closeField();

		$o->closeSection();

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param object	&$asset	the owning asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInLineBackend(&$asset, &$o, $prefix)
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
			$major =& $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$prefix.'_major_assetid']['assetid']);
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


}//end class
?>
